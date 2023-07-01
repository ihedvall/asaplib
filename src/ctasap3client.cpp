/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "ctasap3client.h"

namespace {

constexpr std::string_view kGetNofParameters = "Get Number of Parameters";
constexpr std::string_view kGetNofSignals = "Get Number of Signals";
constexpr std::string_view kGetParameterConfig = "Get Parameter Configuration";
constexpr std::string_view kGetSignalConfig = "Get Signal Configuration";
constexpr std::string_view kGetShareName = "Get Share Memory Name";
constexpr std::string_view kUseExtendedPoll = "Use Extended Poll";
constexpr std::string_view kDisableSetValueAck = "Disable SetValue Ack";
constexpr std::string_view kSupportInvalidOutput = "Support Invalid Output";
constexpr std::string_view kGetConfigFile = "Get Config File";

const asap3::DataValueList kEmptyList;

}  // namespace

namespace asap3 {

CtAsap3Client::~CtAsap3Client() { Asap3Client::Stop(); }

void CtAsap3Client::OnStartMessage() {
  Asap3Client::OnStartMessage();
  SendTelegram(CommandCode::QUERY_AVAILABLE_SERVICE, kEmptyList,
               [this](bool, const ITelegram&) { this->SendCtInitCommand(); });
}

void CtAsap3Client::SendCtInitCommand() {
  if (HasService(kUseExtendedPoll.data())) {
    DataValueList extended_poll_list = {
        {"Service", Mc3DataType::MC3_STRING, std::string(kUseExtendedPoll)},
        {"Input", Mc3DataType::MC3_STRING, std::string("1")},
    };
    SendTelegram(CommandCode::EXECUTE_SERVICE, extended_poll_list);
  }

  if (HasService(kDisableSetValueAck.data())) {
    DataValueList disable_ack_list = {
        {"Service", Mc3DataType::MC3_STRING, std::string(kDisableSetValueAck)},
        {"Input", Mc3DataType::MC3_STRING, std::string("1")},
    };
    SendTelegram(CommandCode::EXECUTE_SERVICE, disable_ack_list);
  }

  if (HasService(kSupportInvalidOutput.data())) {
    DataValueList invalid_output_list = {
        {"Service", Mc3DataType::MC3_STRING,
         std::string(kSupportInvalidOutput)},
        {"Input", Mc3DataType::MC3_STRING, std::string("1")},
    };
    SendTelegram(CommandCode::EXECUTE_SERVICE, invalid_output_list);
  }
}

bool IClient::StartSubscription(uint16_t scan_rate) {
  if (parameter_list_.empty()) {
    listen_->ListenOut()
        << "Empty subscription detected. Cannot start the subscription";
    return false;
  }
  // First check if the subscription already is initialized. If so we
  // can go online directly.

  // This is a simple ComTest subscription so all parameter value are fetched
  // with the same sample rate and LUN numbers are omitted. Note that
  // scan rate gives a base scan rate.

  // Always use extended call to ComTest as it support the call.
  DataValueList empty_list = {
      {"Emulator LUN", Mc3DataType::A_UINT16, static_cast<uint16_t>(0)},
      {"Sample Rate", Mc3DataType::A_UINT16, scan_rate},
      {"Measurements", Mc3DataType::A_UINT16, static_cast<uint16_t>(0)},
  };
  // Reset any previously subscriptions
  SendTelegram(CommandCode::PARAMETER_FOR_VALUE_ACQUISITION_EV2, empty_list);

  // Split into smaller subscription in case
  for (size_t count = 0; count < parameter_list_.size(); count += 50) {
    uint16_t nof_meas = 50;
    bool last_sub = false;
    if (count + nof_meas >= parameter_list_.size()) {
      nof_meas = static_cast<uint16_t>(parameter_list_.size() - count);
      last_sub = true;
    }
    DataValueList sub_list;
    sub_list.push_back(
        {"Emulator LUN", Mc3DataType::A_UINT16, static_cast<uint16_t>(0)});
    sub_list.push_back({"Sample Rate", Mc3DataType::A_UINT16, scan_rate});
    sub_list.push_back({"Measurements", Mc3DataType::A_UINT16, nof_meas});
    for (uint16_t index = 0; index < nof_meas; ++index) {
      std::ostringstream label;
      label << "Name " << index + count + 1;
      const auto& parameter = parameter_list_[count + index];
      sub_list.push_back({label.str(), Mc3DataType::MC3_STRING,
                          std::string(parameter.Name())});
    }
    SendTelegram(CommandCode::PARAMETER_FOR_VALUE_ACQUISITION_EV2, sub_list);
  }
  return true;
}
}  // namespace asap3