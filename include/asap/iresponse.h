/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "asap/asap3def.h"

namespace asap3 {

class IClient;

class IResponse {
 public:
  IResponse() = default;
  explicit IResponse(IClient* client,
                     const std::vector<uint8_t>& body_without_length);
  virtual ~IResponse() = default;

  void Length(uint16_t length) { length_ = length; }
  [[nodiscard]] uint16_t Length() const { return length_; }

  void Cmd(CommandCode cmd) { cmd_ = static_cast<uint16_t>(cmd); }
  [[nodiscard]] CommandCode Cmd() const {
    return static_cast<CommandCode>(cmd_);
  }

  void Status(StatusCode status) { status_ = static_cast<uint16_t>(status); }
  [[nodiscard]] StatusCode Status() const {
    return static_cast<StatusCode>(status_);
  }

  void Sum(uint16_t sum) { sum_ = sum; }
  [[nodiscard]] uint16_t Sum() const { return sum_; }

  void CreateBody(std::vector<uint8_t>& body);

  [[nodiscard]] const DataValueList& DataList() const { return data_list_; }
  template <typename T>
  T GetData(size_t index) const;

 protected:
  IClient* client_ = nullptr;
  uint16_t length_ = 0;
  uint16_t cmd_ = 0;
  uint16_t status_ = 0;
  uint16_t sum_ = 0;
  DataValueList data_list_;
  bool invalid_checksum_ =
      false;  ///< Used as invalid response message indicator

  void BodyToDataList(const std::vector<uint8_t>& body, size_t offset);

 private:
  void GetUserDefineListToDataList(const std::vector<uint8_t>& body,
                                   size_t offset);
  void QueryAvailableServicesToDataList(const std::vector<uint8_t>& body,
                                        size_t offset);
};

template <typename T>
T IResponse::GetData(size_t index) const {
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
std::string IResponse::GetData(size_t index) const;

}  // namespace asap3
