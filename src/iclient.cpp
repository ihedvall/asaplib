/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap/iclient.h"

#include "asap/itelegram.h"

namespace asap3 {
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

bool IClient::HandleTelegram(ITelegram& telegram) {
  const auto* request = telegram.Request();
  const auto* response = telegram.Response();
  if (response == nullptr) {
    return false;
  }

  const auto& data_list = response->DataList();
  bool success = true;

  if (response->Status() != StatusCode::STATUS_OK && response->Status() != StatusCode::STATUS_SUCCESS) {
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

    default:
      break;
  }
  telegram.OnComplete(success);
  return success;
}

}  // namespace asap3