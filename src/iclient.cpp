/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap/iclient.h"

#include <util/timestamp.h>
#include <util/utilfactory.h>

#include <algorithm>
#include <sstream>

#include "asap/itelegram.h"
#include "asap3helper.h"
#include "util/stringutil.h"

using namespace util::string;

namespace asap3 {
IClient::IClient()
    : listen_(
          util::UtilFactory::CreateListen("ListenProxy", "LIS_ASAP3_CLIENT")) {}
IClient::~IClient() { listen_.reset(); }

bool IClient::IsConnected() const { return connected_; }

void IClient::SendTelegram(CommandCode cmd,
                           const std::vector<DataValue>& data_list) {
  auto telegram = std::make_unique<ITelegram>(cmd, data_list);
  telegram_queue_.Put(telegram);
}

void IClient::SendTelegram(CommandCode cmd,
                           const std::vector<DataValue>& data_list,
                           ITelegram::OnCompleteFunction on_complete_function) {
  auto telegram = std::make_unique<ITelegram>(cmd, data_list,
                                              std::move(on_complete_function));
  telegram_queue_.Put(telegram);
}

void IClient::ListenRequest(const IRequest& request) {
  if (!listen_ || !listen_->IsActive()) {
    return;
  }
  // Log level 0: Show all plain text
  // Log level 1: Hide cyclic data
  // Log level 2: Hide commands
  // Log level 3: Show hex

  switch (listen_->LogLevel()) {
    case 3: {
      std::vector<uint8_t> temp_body;
      request.CreateBody(temp_body);
      const auto now = util::time::TimeStampToNs();
      std::ostringstream pre_text;
      pre_text << Name() << "T:";
      listen_->ListenTransmit(now, pre_text.str(), temp_body, nullptr);
      break;
    }

    case 1:  // Hide cyclic data
      if (request.Cmd() != CommandCode::GET_ONLINE_VALUE &&
          request.Cmd() != CommandCode::GET_ONLINE_VALUE_EV2) {
        listen_->ListenOut() << Asap3Helper::RequestToPlainText(request);
      }
      break;

    case 2:  // Hide commands data (Well opposite of case 1)
      if (request.Cmd() == CommandCode::GET_ONLINE_VALUE ||
          request.Cmd() == CommandCode::GET_ONLINE_VALUE_EV2) {
        listen_->ListenOut() << Asap3Helper::RequestToPlainText(request);
      }
      break;

    case 0:
    default:
      listen_->ListenOut() << Asap3Helper::RequestToPlainText(request);
      break;
  }
}

void IClient::ListenResponse(const IResponse& response) {
  if (!listen_ || !listen_->IsActive()) {
    return;
  }
  // Log level 0: Show all plain text
  // Log level 1: Hide cyclic data
  // Log level 2: Hide commands
  // Log level 3: Show hex

  switch (listen_->LogLevel()) {
    case 3: {
      IResponse temp(response);
      std::vector<uint8_t> temp_body;
      temp.CreateBody(temp_body);
      const auto now = util::time::TimeStampToNs();
      std::ostringstream pre_text;
      pre_text << Name() << "R:";
      listen_->ListenReceive(now, pre_text.str(), temp_body, nullptr);
      break;
    }

    case 1:  // Hide cyclic data
      if (response.Cmd() != CommandCode::GET_ONLINE_VALUE &&
          response.Cmd() != CommandCode::GET_ONLINE_VALUE_EV2) {
        listen_->ListenOut() << Asap3Helper::ResponseToPlainText(response);
      }
      break;

    case 2:  // Hide commands data (Well opposite of case 1)
      if (response.Cmd() == CommandCode::GET_ONLINE_VALUE ||
          response.Cmd() == CommandCode::GET_ONLINE_VALUE_EV2) {
        listen_->ListenOut() << Asap3Helper::ResponseToPlainText(response);
      }
      break;

    case 0:
    default:
      listen_->ListenOut() << Asap3Helper::ResponseToPlainText(response);
      break;
  }
}

bool IClient::HandleTelegram(ITelegram& telegram) {
  const auto* request = telegram.Request();
  const auto* response = telegram.Response();
  if (response == nullptr) {
    return false;
  }

  const auto& data_list = response->DataList();
  bool success = true;

  if (response->Status() != StatusCode::STATUS_OK &&
      response->Status() != StatusCode::STATUS_SUCCESS) {
    success = false;
    telegram.OnComplete(success);
    return false;
  }

  switch (response->Cmd()) {
    case CommandCode::REPEAT_REQUEST:
    case CommandCode::INIT:
    case CommandCode::EXIT:
      break;

    case CommandCode::IDENTIFY: {
      for (size_t item = 0; item < data_list.size(); ++item) {
        const auto& data = data_list[item];
        switch (item) {
          case 0:
            remote_version_ = std::any_cast<uint16_t>(data.value);
            break;
          case 1:
            remote_name_ = std::any_cast<std::string>(data.value);
            break;
          default:
            break;
        }
      }
      break;
    }

      // Update the service list with names
    case CommandCode::QUERY_AVAILABLE_SERVICE:
      SetServiceList(response->DataList());
      break;

    // Update the service list info
    case CommandCode::GET_SERVICE_INFORMATION:
      if (request != nullptr) {
        SetServiceInfo(request->GetData<std::string>(0),
                       response->GetData<std::string>(0));
      }
      break;

    default:
      break;
  }
  telegram.OnComplete(success);
  return success;
}

void IClient::SetOnlineData(const std::vector<uint8_t>& body, size_t offset) {
  std::scoped_lock lock(value_locker_);
  Asap3Helper::BodyToDataList(body, offset, online_value_list_);
}

void IClient::DefineUserDefinedData(const std::vector<uint8_t>& body,
                                    size_t offset) {
  std::scoped_lock lock(value_locker_);
  user_defined_list_.clear();
  uint16_t values = 0;
  size_t index = offset;
  index += Asap3Helper::ToMc3Value(body, index, values);
  user_defined_list_.push_back({"Values", Mc3DataType::A_UINT16, values});
  for (uint16_t value = 0; value < values && index + 6 <= body.size() - 2;
       ++value) {
    uint16_t lun = 0;
    index += Asap3Helper::ToMc3Value(body, index, lun);
    std::string name;
    index += Asap3Helper::ToMc3Value(body, index, name);
    user_defined_list_.push_back(
        {name, Mc3DataType::A_FLOAT32, Asap3Helper::InvalidFloat});
  }
}

void IClient::SetUserDefinedData(const std::vector<uint8_t>& body,
                                 size_t offset) {
  std::scoped_lock lock(value_locker_);
  Asap3Helper::BodyToDataList(body, offset, user_defined_list_);
}

ServiceList IClient::AvailableServices() const {
  std::scoped_lock lock(value_locker_);
  return service_list_;
}

void IClient::SetServiceList(const DataValueList& data_list) {
  std::scoped_lock lock(value_locker_);
  service_list_.clear();
  for (size_t index = 1; index < data_list.size(); ++index) {
    const DataValue& data = data_list[index];
    std::string name = data.type == Mc3DataType::MC3_STRING
                           ? std::any_cast<std::string>(data.value)
                           : std::string();
    service_list_.push_back({name, std::string()});
  }
}

void IClient::SetServiceInfo(const std::string& service,
                             const std::string& info) {
  std::scoped_lock lock(value_locker_);

  auto itr = std::ranges::find_if(
      service_list_, [&](const auto& serv) { return serv.name == service; });
  if (itr != service_list_.end()) {
    itr->info = info;
  }
}

bool IClient::HasService(const std::string& service) const {
  return std::ranges::any_of(service_list_, [&](const auto& serv) {
    return IEquals(service, serv.name);
  });
}

bool IClient::IsSubscriptionInitialized() const {
  for (const auto& parameter : parameter_list_) {
    if (!parameter.Exist()) {
      continue;
    }
    size_t index = parameter.ValueIndex();

    if (parameter.SetPoint()) {
      if (index >= output_value_list_.size()) {
        return false;
      }

      if (output_value_list_[index].name !=  parameter.Name()) {
        return false;
      }
    } else {
      if (index >= online_value_list_.size()) {
        return false;
      }

      if (online_value_list_[index].name !=  parameter.Name()) {
        return false;
      }
     }
  }

  return true;
}

void IClient::ParameterList(const A3ParameterList& parameter_list) {
  parameter_list_ = parameter_list;

  // Set the exist flag to true.indicating that this parameter must
  // exist in the current on-line subscription.
  for (auto& parameter : parameter_list_) {
    parameter.Exist(true);
  }
}

bool IClient::StartSubscription(uint16_t scan_rate) { return false; }
bool IClient::StopSubscription() { return false; }
bool IClient::IsScanning() const { return false; }
}  // namespace asap3