/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "queryparameters.h"

#include <util/logstream.h>
#include <util/stringutil.h>

#include <string_view>

#include "asap3helper.h"

using namespace util::log;
using namespace util::string;

namespace {
constexpr std::string_view kGetNofParameters = "Get Number of Parameters";
constexpr std::string_view kGetParameterConfig = "Get Parameter Configuration";
const asap3::DataValueList kEmptyList;
}  // namespace

namespace asap3 {

QueryParameters::~QueryParameters() { QueryParameters::Stop(); }

bool QueryParameters::Start() {
  const bool start = Asap3Client::Start();
  if (!start) {
    LOG_ERROR() << "Failed to connect to the ASAP 3 server. Host: " << Host()
                << ", Port: " << Port();
    return false;
  }
  // Block call until the connection is ready with initialization
  const bool wait1 = Asap3Client::WaitOnIdle();
  if (!wait1) {
    LOG_ERROR() << "Failed to wait on ASAP 3 to initialize. Host: " << Host()
                << ", Port: " << Port();
    return false;
  }
  if (!HasService(kGetNofParameters.data()) ||
      !HasService(kGetParameterConfig.data())) {
    LOG_ERROR() << "The server doesnt support the required services. Host: "
                << Host() << ", Port: " << Port();
    return false;
  }

  ClearParameterList();
  DataValueList nof_par_list = {
      {"Service", Mc3DataType::MC3_STRING, std::string(kGetNofParameters)},
      {"Input", Mc3DataType::MC3_STRING, std::string()},
  };
  SendTelegram(CommandCode::EXECUTE_SERVICE, nof_par_list);

  // Block call until the request is ready
  const bool wait2 = Asap3Client::WaitOnIdle();
  if (!wait2) {
    LOG_ERROR() << "Failed to wait on ASAP 3 service request. Host: " << Host()
                << ", Port: " << Port();
    return false;
  }

  return true;
}

bool QueryParameters::HandleTelegram(ITelegram& telegram) {
  const auto* request = telegram.Request();
  const auto* response = telegram.Response();
  if (response == nullptr) {
    return false;
  }

  if (response->Status() != StatusCode::STATUS_OK &&
      response->Status() != StatusCode::STATUS_SUCCESS) {
    return Asap3Client::HandleTelegram(telegram);
  }

  if (response->Cmd() == CommandCode::EXECUTE_SERVICE && request != nullptr) {
    SetExecuteService(request->GetData<std::string>(0),
                      request->GetData<std::string>(1),
                      response->GetData<std::string>(0));
  }
  return Asap3Client::HandleTelegram(telegram);
}

void QueryParameters::SetExecuteService(const std::string& service,
                                        const std::string& input,
                                        const std::string& output) {
  if (IEquals(service, "Get Number of Parameters")) {
    ClearParameterList();
    const int parameters = std::stoi(output);
    for (int parameter = 0; parameter < parameters; parameter += 50) {
      const int min_index = parameter;
      int max_index = parameter + 49;
      if (max_index >= parameters) {
        max_index = parameters - 1;
      }
      std::ostringstream min_max;
      min_max << min_index << "," << max_index;
      const auto min_max_text = min_max.str();
      DataValueList get_par_list = {
          {"Service", Mc3DataType::MC3_STRING,
           std::string(kGetParameterConfig)},
          {"Input", Mc3DataType::MC3_STRING, std::string("1,44")},
      };
      SendTelegram(CommandCode::EXECUTE_SERVICE, get_par_list);
    }
  } else if (IEquals(service, "Get Parameter Configuration")) {
    Asap3Helper::ParseCtParameterConfigString(output, parameter_list_);
  }
}

void QueryParameters::OnStartMessage() {
  Asap3Client::OnStartMessage();
  SendTelegram(CommandCode::QUERY_AVAILABLE_SERVICE, kEmptyList);
}

}  // namespace asap3
