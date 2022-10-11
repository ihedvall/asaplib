/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <array>
#include <atomic>
#include <boost/asio.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <util/ilisten.h>
#include "asap/iclient.h"


namespace asap3 {

class Asap3Client : public IClient {
 public:
  Asap3Client();
  ~Asap3Client() override;
  bool Start() override;
  bool Stop() override;
  bool IsIdle() const override;
 protected:
  std::thread worker_thread_;
  std::atomic<bool> stop_thread_ = true;

  std::mutex locker_;
  std::thread message_thread_;
  std::atomic<bool> stop_message_ = true;
  std::condition_variable message_condition_;
  std::unique_ptr<ITelegram> current_message_;
  std::atomic<bool> response_handled_ = true;

  boost::asio::io_context context_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::steady_timer retry_timer_;
  boost::asio::steady_timer deadlock_timer_;

  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
  boost::asio::ip::tcp::resolver::results_type end_points_;

  std::vector<uint8_t> short_data_;  ///< Length receive buffer
  std::vector<uint8_t>
      body_data_;  ///< Receive body buffer (excluding length word)
  std::vector<uint8_t> transmit_data_;  ///< Transmit body buffer
  std::unique_ptr<util::log::IListen> listen_;

  std::atomic<bool> restart_ = false;


  void WorkerThread();
  void MessageThread();
  void DoLookup();
  void DoRetryWait();
  void DoConnect();
  void DoReadLength();
  void DoReadBody();

  void HandleResponse();
  void HandleRequest(const IRequest& request);
  void Close();
  virtual void StartMessageThread();
  void StopMessageThread();

  void ListenRequest(const IRequest& request);
  void ListenResponse(const IResponse& response);
};

}  // namespace asap3
