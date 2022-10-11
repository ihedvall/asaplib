/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap/iresponse.h"

#include <sstream>

#include "asap3helper.h"

namespace {

const asap3::DataValueList kErrorResponse = {
    {"ErrorCode", asap3::Mc3DataType::A_UINT16, 0},
    {"ErrorText", asap3::Mc3DataType::MC3_STRING, ""}};

const asap3::DataValueList kIdentifyResponse = {
    {"Version", asap3::Mc3DataType::A_UINT16, 0},
    {"Name", asap3::Mc3DataType::MC3_STRING, ""}};

const asap3::DataValueList kDefineDescFileResponse = {
    {"LUN", asap3::Mc3DataType::A_UINT16, 0},
    {"DescriptionFile", asap3::Mc3DataType::MC3_STRING, ""},
    {"BinaryFile", asap3::Mc3DataType::MC3_STRING, ""},
    {"CalibrationFile", asap3::Mc3DataType::MC3_STRING, ""},
};

const asap3::DataValueList kSelectDescFileResponse = {
    {"LUN", asap3::Mc3DataType::A_UINT16, 0},
};

const asap3::DataValueList kGetCalInfoResponse = {
    {"Pages", asap3::Mc3DataType::A_UINT16, 0},
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

IResponse::IResponse(const std::vector<uint8_t> &body_without_length) {
  // Note that the body exclude the 2 length bytes.
  length_ = static_cast<uint16_t>(body_without_length.size() + 2);
  if (length_ < 8) {
    invalid_checksum_ = true;
    return;
  }

  size_t offset = Asap3Helper::ToMc3Value(body_without_length, 0, cmd_);
  offset += Asap3Helper::ToMc3Value(body_without_length, offset, status_);
  if (length_ <= 8) {
    data_list_.clear();
  } else {
    BodyToDataList(body_without_length, offset);
  }
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

    default:
      // Empty response list
      break;
  }
  Asap3Helper::BodyToDataList(body, offset, data_list_);
}

}  // namespace asap3