/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap/irequest.h"

#include "asap3helper.h"

namespace asap3 {

IRequest::IRequest(CommandCode cmd, const DataValueList &data_list) {
  Cmd(cmd);
  data_list_ = data_list;
}

void IRequest::CreateBody(std::vector<uint8_t> &body) const {
  uint16_t length;
  uint16_t sum;
  const auto temp_size = sizeof(length) + sizeof(cmd_) +
                         Asap3Helper::DataListSize(data_list_) + sizeof(sum);
  length = static_cast<uint16_t>(temp_size);

  if (body.size() != length) {
    body.resize(length);
  }

  auto offset = Asap3Helper::FromMc3Value(body, 0, length);
  offset += Asap3Helper::FromMc3Value(body, offset, cmd_);
  Asap3Helper::DataListToBody(data_list_, body, offset);
  sum = Asap3Helper::Checksum(body);
  Asap3Helper::FromMc3Value(body, offset, sum);
}

template <>
std::string IRequest::GetData(size_t index) const {
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