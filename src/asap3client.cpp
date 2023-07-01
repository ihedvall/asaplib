/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap3client.h"

#include <chrono>
#include <functional>
#include <sstream>

#include "asap3helper.h"

using namespace boost::asio;
using namespace boost::system;
using namespace std::chrono_literals;
using namespace util::log;

namespace {
asap3::DataValueList kEmptyList;
asap3::DataValueList kIdentifyList = {
    {"Version", asap3::Mc3DataType::A_UINT16, static_cast<uint16_t>(768)},
    {"Name", asap3::Mc3DataType::MC3_STRING, std::string()}};

}  // namespace

namespace asap3 {

Asap3Client::Asap3Client()
    : resolver_(context_), retry_timer_(context_), deadlock_timer_(context_) {
  short_data_.resize(2, 0);
}

Asap3Client::~Asap3Client() {
  Asap3Client::Stop();
  listen_.reset();
}

bool Asap3Client::Start() {
  // Fix some type of name if no name given.
  if (Name().empty()) {
    std::ostringstream temp;
    temp << "A3C-" << Port();
    Name(temp.str());
  }

  // Set the listen pre-text to the Name()
  listen_->PreText(Name());
  listen_->ListenOut() << "Starting ASAP3 client";
  telegram_queue_.Clear();
  stop_thread_ = false;
  worker_thread_ = std::thread(&Asap3Client::WorkerThread, this);

  return true;
}

bool Asap3Client::Stop() {
  listen_->ListenOut() << "Stopping ASAP3 client";
  if (!worker_thread_.joinable()) {
    return true;
  }

  SendTelegram(CommandCode::EXIT, kEmptyList);
  StopMessageThread();

  stop_thread_ = true;
  if (!context_.stopped()) {
    context_.stop();
  }
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
  return true;
}

void Asap3Client::WorkerThread() {
  try {
    DoLookup();
    context_.run();
  } catch (const std::exception& err) {
    listen_->ListenOut() << "Worker (receiver) thread failure. Error: "
                         << err.what();
  }
}

void Asap3Client::Close() {
  connected_ = false;
  StopMessageThread();
  if (socket_) {
    try {
      boost::system::error_code dummy;
      socket_->shutdown(ip::tcp::socket::shutdown_both, dummy);
      socket_->close(dummy);
    } catch (const std::exception& err) {
      listen_->ListenOut() << "Socket close failure. Error: " << err.what();
    }
  }
  socket_.reset();
}

void Asap3Client::DoLookup() {
  resolver_.async_resolve(
      host_, std::to_string(port_),
      [&](const error_code& error, ip::tcp::resolver::results_type result) {
        if (error) {
          listen_->ListenOut() << "Lookup failure. Error: " << error.message();
          DoRetryWait();
        } else {
          socket_ = std::make_unique<ip::tcp::socket>(context_);
          end_points_ = std::move(result);
          DoConnect();
        }
      });
}

void Asap3Client::DoRetryWait() {
  Close();
  retry_timer_.expires_after(5s);
  retry_timer_.async_wait(
      [&](const boost::system::error_code error) { DoLookup(); });
}

void Asap3Client::DoConnect() {
  socket_->async_connect(
      *end_points_, [&](const boost::system::error_code error) {
        if (error) {
          listen_->ListenOut() << "Connect failure. Error: " << error.message();
          DoRetryWait();
        } else {
          listen_->ListenOut() << "Connected";
          DoReadLength();
          StartMessageThread();
          connected_ = true;
        }
      });
}

void Asap3Client::DoReadLength() {  // NOLINT
  if (!socket_ || !socket_->is_open()) {
    listen_->ListenOut() << "Read length socket close";
    DoRetryWait();
    return;
  }

  async_read(
      *socket_, boost::asio::buffer(short_data_),
      [&](const boost::system::error_code& error, size_t bytes) {  // NOLINT
        if (error && error == error::eof) {
          listen_->ListenOut()
              << "Read length socket eof. Error: " << error.message();
          DoRetryWait();
        } else if (error) {
          listen_->ListenOut()
              << "Read length socket error. Error: " << error.message();
          DoRetryWait();
        } else if (bytes != short_data_.size()) {
          listen_->ListenOut() << "Read length (nof bytes) error. Expected: "
                               << short_data_.size() << ", Read: " << bytes;
          DoRetryWait();
        } else {
          uint16_t length = 0;
          Asap3Helper::ToMc3Value(short_data_, 0, length);
          if (length >= 8) {
            body_data_.clear();
            body_data_.resize(length - 2, 0);
            DoReadBody();
          } else {
            listen_->ListenOut()
                << "Read length invalid length. Length: " << length;
            DoRetryWait();  // TCP/IP
          }
        }
      });
}

void Asap3Client::DoReadBody() {  // NOLINT
  if (!socket_ || !socket_->is_open()) {
    DoRetryWait();
    return;
  }
  deadlock_timer_.expires_after(10s);
  deadlock_timer_.async_wait([&](const error_code error) {
    if (error != error::operation_aborted) {
      listen_->ListenOut() << "Read body error. Error: " << error.message();
      DoRetryWait();  // If timer expires, then disconnect
    }
  });

  async_read(*socket_, buffer(body_data_),
             [&](const error_code& error, size_t bytes) {  // NOLINT
               deadlock_timer_.cancel();
               if (error) {
                 listen_->ListenOut()
                     << "Read body socket error. Error: " << error.message();
                 DoRetryWait();
               } else if (bytes != body_data_.size()) {
                 listen_->ListenOut()
                     << "Read body (nof bytes) error. Expected: "
                     << body_data_.size() << ", Read: " << bytes;
                 DoRetryWait();
               } else {
                 HandleResponse();
                 if (restart_) {
                   restart_ = false;
                   DoRetryWait();
                 } else {
                   DoReadLength();
                 }
               }
             });
}

void Asap3Client::HandleResponse() {
  std::unique_ptr<IResponse> response =
      std::make_unique<IResponse>(this, body_data_);
  ListenResponse(*response);
  const auto status = response->Status();
  const auto& response_list = response->DataList();
  const auto* request =
      current_message_ ? current_message_->Request() : nullptr;
  if (request != nullptr && request->Cmd() == response->Cmd()) {
    current_message_->Response(response);
  }

  switch (status) {
    case StatusCode::STATUS_NOT_PROCESSED:
      restart_ = true;
      response_handled_ = true;
      // message_condition_.notify_all();
      break;

    case StatusCode::STATUS_ACK:
      // Do nothing but prolong the timeout as remote indicate a longer timeout
      break;

    case StatusCode::STATUS_REPEAT_CMD:
      if (request != nullptr) {
        HandleRequest(*request);
      }
      break;

    case StatusCode::STATUS_ERROR: {
      const auto error_code =
          response_list.empty()
              ? 0
              : std::any_cast<uint16_t>(response_list[0].value);
      const auto error =
          response_list.size() <= 1
              ? std::string()
              : std::any_cast<std::string>(response_list[1].value);
      listen_->ListenOut() << "Error message. Error: " << error_code << ":"
                           << error;
      HandleTelegram(*current_message_);
      response_handled_ = true;
      message_condition_.notify_all();
      break;
    }

    case StatusCode::STATUS_RESERVED:
    case StatusCode::STATUS_MEASURING_DATA_CHANGED:
    case StatusCode::STATUS_CMD_NOT_AVAILABLE:
    case StatusCode::STATUS_SUCCESS:
    case StatusCode::STATUS_OK:
    default:
      HandleTelegram(*current_message_);
      response_handled_ = true;
      message_condition_.notify_all();
      break;
  }
  response.reset();
}

void Asap3Client::MessageThread() {
  while (!stop_message_) {
    const bool message = telegram_queue_.Get(current_message_, true);
    if (message && !stop_message_ && current_message_ &&
        current_message_->Request() != nullptr) {
      response_handled_ = false;
      HandleRequest(
          *current_message_->Request());  // Send this message to server
      // Wait on response on this message before
      for (size_t timeout = 0;
           !response_handled_ && !stop_message_ && timeout < 600; ++timeout) {
        std::unique_lock lock(locker_);
        message_condition_.wait_for(lock, 1s, [&] {
          return response_handled_.load() || stop_message_.load();
        });
      }
      current_message_.reset();
    }
  }
}

void Asap3Client::StartMessageThread() {
  StopMessageThread();  // Just in case it is running.
  stop_message_ = false;
  telegram_queue_.Clear();
  telegram_queue_.Start();
  message_thread_ = std::thread(&Asap3Client::MessageThread, this);
  SendTelegram(CommandCode::INIT, kEmptyList);

  auto identify_list = kIdentifyList;
  Asap3Helper::SetDataListProperty(identify_list, "Name", Name());
  SendTelegram(CommandCode::IDENTIFY, identify_list);

  OnStartMessage();
}

void Asap3Client::StopMessageThread() {
  if (!message_thread_.joinable()) {
    return;
  }

  for (size_t timeout = 0; timeout < 500; ++timeout) {
    if (telegram_queue_.Empty() && response_handled_) {
      break;
    }
    std::this_thread::sleep_for(10ms);
  }
  stop_message_ = true;
  telegram_queue_.Stop();
  message_condition_.notify_all();

  if (message_thread_.joinable()) {
    message_thread_.join();
  }
}

void Asap3Client::HandleRequest(const IRequest& request) {
  transmit_data_.clear();
  request.CreateBody(transmit_data_);
  ListenRequest(request);
  async_write(*socket_, buffer(transmit_data_),
              [&](const error_code& error, size_t nof_bytes) {
                if (error) {
                  listen_->ListenOut()
                      << "Write error. Error: " << error.message();
                }
              });
}

bool Asap3Client::IsIdle() const {
  return IsConnected() && telegram_queue_.Empty() && response_handled_;
}

bool Asap3Client::WaitOnIdle() const {
  while (!stop_thread_) {
    if (IsIdle()) {
      return true;
    }
    std::this_thread::sleep_for(10ms);
  }
  return false;
}

void Asap3Client::OnStartMessage() {}

}  // namespace asap3
