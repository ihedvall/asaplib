/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap/iresponse.h"

#include <sstream>

#include "asap/iclient.h"
#include "asap3helper.h"

namespace {

const asap3::DataValueList kErrorResponse = {
    {"Error Code", asap3::Mc3DataType::A_UINT16, static_cast<uint16_t>(0)},
    {"Error Text", asap3::Mc3DataType::MC3_STRING, std::string()}};

const asap3::DataValueList kIdentifyResponse = {
    {"Version", asap3::Mc3DataType::A_UINT16, static_cast<uint16_t>(0)},
    {"Name", asap3::Mc3DataType::MC3_STRING, std::string()}};

const asap3::DataValueList kDefineDescFileResponse = {
    {"LUN", asap3::Mc3DataType::A_UINT16, static_cast<uint16_t>(0)},
    {"Description File", asap3::Mc3DataType::MC3_STRING, std::string()},
    {"Binary File", asap3::Mc3DataType::MC3_STRING, std::string()},
    {"Calibration File", asap3::Mc3DataType::MC3_STRING, std::string()},
};

const asap3::DataValueList kSelectDescFileResponse = {
    {"LUN", asap3::Mc3DataType::A_UINT16, static_cast<uint16_t>(0)},
};

const asap3::DataValueList kGetCalInfoResponse = {
    {"Pages", asap3::Mc3DataType::A_UINT16, static_cast<uint16_t>(0)},
};

const asap3::DataValueList kServiceInfoResponse = {
    {"Service Info", asap3::Mc3DataType::MC3_STRING, std::string()},
};

const asap3::DataValueList kExecuteServiceResponse = {
    {"Output", asap3::Mc3DataType::MC3_STRING, std::string()},
};
}  // namespace

namespace asap3 {

void IResponse::CreateBody(std::vector<uint8_t> &body) {
  auto temp_size = sizeof(length_) + sizeof(cmd_) + sizeof(status_) +
                   Asap3Helper::DataListSize(data_list_) + sizeof(sum_);
  length_ = static_cast<uint16_t>(temp_size);
  if (body.size() != length_) {
    body.resize(length_);
  }

  auto offset = Asap3Helper::FromMc3Value(body, 0, length_);
  offset += Asap3Helper::FromMc3Value(body, offset, cmd_);
  offset += Asap3Helper::FromMc3Value(body, offset, status_);
  Asap3Helper::DataListToBody(data_list_, body, offset);
  sum_ = Asap3Helper::Checksum(body);
  Asap3Helper::FromMc3Value(body, offset, sum_);
}

IResponse::IResponse(IClient *client,
                     const std::vector<uint8_t> &body_without_length)
    : client_(client),
      length_(static_cast<uint16_t>(body_without_length.size() + 2)) {
  // Note that the body exclude the 2 length bytes.
  if (length_ < 8) {
    invalid_checksum_ = true;
    return;
  }

  size_t offset = Asap3Helper::ToMc3Value(body_without_length, 0, cmd_);
  offset += Asap3Helper::ToMc3Value(body_without_length, offset, status_);
  BodyToDataList(body_without_length, offset);
  Asap3Helper::ToMc3Value(body_without_length, body_without_length.size() - 2,
                          sum_);
  const auto sum = length_ + Asap3Helper::Checksum(body_without_length);
  invalid_checksum_ = sum != sum_;
}

void IResponse::BodyToDataList(const std::vector<uint8_t> &body,
                               size_t offset) {
  data_list_.clear();
  switch (Status()) {
    case StatusCode::STATUS_OK:
    case StatusCode::STATUS_SUCCESS:
      break;

    case StatusCode::STATUS_NOT_PROCESSED:  // Should restart the client
    case StatusCode::STATUS_MEASURING_DATA_CHANGED:  // Don't know what to do
    case StatusCode::STATUS_RESERVED:
    case StatusCode::STATUS_CMD_NOT_AVAILABLE:
    case StatusCode::STATUS_ACK:
    case StatusCode::STATUS_REPEAT_CMD:
      return;

    case StatusCode::STATUS_ERROR:
    default:  // If an error code is received
      data_list_ = kErrorResponse;
      Asap3Helper::BodyToDataList(body, offset, data_list_);
      return;
  }

  // No Error in responses
  switch (Cmd()) {
    case CommandCode::IDENTIFY:
      data_list_ = kIdentifyResponse;
      break;

    case CommandCode::DEFINE_DESCRIPTION_FILE_AND_BINARY_FILE:
      data_list_ = kDefineDescFileResponse;
      break;

    case CommandCode::SELECT_DESCRIPTION_FILE_AND_BINARY_FILE:
      data_list_ = kSelectDescFileResponse;
      break;

    case CommandCode::GET_CALPAGE_INFO: {
      uint16_t pages = 0;
      Asap3Helper::ToMc3Value(body, offset, pages);
      data_list_ = kGetCalInfoResponse;  // Contains pages item only
      for (uint16_t page = 0; page < pages; ++page) {
        std::ostringstream temp;
        temp << "Page " << page + 1 << " ";
        data_list_.push_back({temp.str() + "Index", Mc3DataType::A_UINT16,
                              static_cast<uint16_t>(0)});
        data_list_.push_back(
            {temp.str() + "Name", Mc3DataType::MC3_STRING, std::string()});
        data_list_.push_back({temp.str() + "Properties", Mc3DataType::A_UINT16,
                              static_cast<uint16_t>(0)});
      }
      break;
    }

    case CommandCode::GET_ONLINE_VALUE:
    case CommandCode::GET_ONLINE_VALUE_EV2:
      // Fills the previously defined online_value_list_
      if (client_ != nullptr) {
        client_->SetOnlineData(body, offset);
      }
      return;

    case CommandCode::GET_USER_DEFINED_VALUE:
      if (client_ != nullptr) {
        client_->SetUserDefinedData(body, offset);
      }
      return;

    case CommandCode::GET_USER_DEFINED_VALUE_LIST:
      if (client_ != nullptr) {
        client_->DefineUserDefinedData(body, offset);
      }
      GetUserDefineListToDataList(body, offset);
      return;

    case CommandCode::QUERY_AVAILABLE_SERVICE:
      QueryAvailableServicesToDataList(body, offset);
      return;

    case CommandCode::GET_SERVICE_INFORMATION:
      data_list_ = kServiceInfoResponse;
      break;

    case CommandCode::EXECUTE_SERVICE:
      data_list_ = kExecuteServiceResponse;
      break;

    default:
      // Empty response list
      break;
  }
  Asap3Helper::BodyToDataList(body, offset, data_list_);
}

void IResponse::GetUserDefineListToDataList(const std::vector<uint8_t> &body,
                                            size_t offset) {
  data_list_.clear();
  uint16_t values = 0;
  size_t index = offset;
  index += Asap3Helper::ToMc3Value(body, index, values);
  data_list_.push_back({"Values", Mc3DataType::A_UINT16, values});
  for (uint16_t value = 0; value < values && index + 6 <= body.size() - 2;
       ++value) {
    std::ostringstream label_lun;
    label_lun << "LUN " << value + 1;

    uint16_t lun = 0;
    index += Asap3Helper::ToMc3Value(body, index, lun);
    data_list_.push_back({label_lun.str(), Mc3DataType::A_UINT16, lun});

    std::ostringstream label_name;
    label_name << "Value " << value + 1;

    std::string name;
    index += Asap3Helper::ToMc3Value(body, index, name);
    data_list_.push_back({label_name.str(), Mc3DataType::MC3_STRING, name});
  }
}

void IResponse::QueryAvailableServicesToDataList(
    const std::vector<uint8_t> &body, size_t offset) {
  data_list_.clear();
  uint16_t services = 0;
  size_t index = offset;
  index += Asap3Helper::ToMc3Value(body, index, services);
  data_list_.push_back({"Services", Mc3DataType::A_UINT16, services});
  for (uint16_t service = 0; service < services && index + 2 <= body.size() - 2;
       ++service) {
    std::ostringstream label;
    label << "Service " << service + 1;

    std::string name;
    index += Asap3Helper::ToMc3Value(body, index, name);
    data_list_.push_back({label.str(), Mc3DataType::MC3_STRING, name});
  }
}

template <>
std::string IResponse::GetData(size_t index) const {
  std::string value;

  if (index >= data_list_.size()) {
    return value;
  }
  const auto &data = data_list_[index];
  try {
    switch (data.type) {
      case Mc3DataType::A_FLOAT64:
        value = std::to_string(std::any_cast<double>(data.value));
        break;

      case Mc3DataType::MC3_STRING:
        value = std::any_cast<std::string>(data.value);
        break;

      case Mc3DataType::A_INT16:
        value = std::to_string(std::any_cast<int16_t>(data.value));
        break;

      case Mc3DataType::A_UINT16:
        value = std::to_string(std::any_cast<uint16_t>(data.value));
        break;

      case Mc3DataType::A_INT32:
        value = std::to_string(std::any_cast<int32_t>(data.value));
        break;

      case Mc3DataType::A_UINT32:
        value = std::to_string(std::any_cast<uint32_t>(data.value));
        break;

      case Mc3DataType::A_INT64:
        value = std::to_string(std::any_cast<int64_t>(data.value));
        break;

      case Mc3DataType::A_UINT64:
        value = std::to_string(std::any_cast<uint64_t>(data.value));
        break;

      case Mc3DataType::A_FLOAT32:
      default:
        value = std::to_string(std::any_cast<float>(data.value));
        break;
    }
  } catch (const std::exception &err) {
  }
  return value;
}

}  // namespace asap3