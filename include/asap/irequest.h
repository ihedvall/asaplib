/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include <vector>

#include "asap/asap3def.h"
namespace asap3 {

class IRequest {
 public:
  IRequest() = default;
  IRequest(CommandCode cmd, const DataValueList& data_list);
  ~IRequest() = default;

  void Cmd(CommandCode cmd) { cmd_ = static_cast<uint16_t>(cmd); }
  [[nodiscard]] CommandCode Cmd() const {
    return static_cast<CommandCode>(cmd_);
  }
  [[nodiscard]] const DataValueList& DataList() const { return data_list_; }
  template <typename T>
  T GetData(size_t index) const;

  void CreateBody(std::vector<uint8_t>& body) const;

 protected:
  uint16_t cmd_ = 0;
  DataValueList data_list_;
};

template <typename T>
T IRequest::GetData(size_t index) const {
  T value = {};
  if (index >= data_list_.size()) {
    return value;
  }
  const auto& data = data_list_[index];
  try {
    switch (data.type) {
      case Mc3DataType::A_FLOAT64:
        value = static_cast<T>(std::any_cast<double>(data.value));
        break;

      case Mc3DataType::MC3_STRING:
        value = static_cast<T>(std::any_cast<std::string>(data.value));
        break;

      case Mc3DataType::A_INT16:
        value = static_cast<T>(std::any_cast<int16_t>(data.value));
        break;

      case Mc3DataType::A_UINT16:
        value = static_cast<T>(std::any_cast<uint16_t>(data.value));
        break;

      case Mc3DataType::A_INT32:
        value = static_cast<T>(std::any_cast<int32_t>(data.value));
        break;

      case Mc3DataType::A_UINT32:
        value = static_cast<T>(std::any_cast<uint32_t>(data.value));
        break;

      case Mc3DataType::A_INT64:
        value = static_cast<T>(std::any_cast<int64_t>(data.value));
        break;

      case Mc3DataType::A_UINT64:
        value = static_cast<T>(std::any_cast<uint64_t>(data.value));
        break;

      case Mc3DataType::A_FLOAT32:
      default:
        value = static_cast<T>(std::any_cast<float>(data.value));
        break;
    }
  } catch (const std::exception& err) {
  }
  return value;
}

template <>
std::string IRequest::GetData(size_t index) const;

}  // namespace asap3
