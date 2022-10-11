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

class IResponse {
 public:
  IResponse() = default;
  explicit IResponse(const std::vector<uint8_t>& body_without_length);
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

 protected:
  uint16_t length_ = 0;
  uint16_t cmd_ = 0;
  uint16_t status_ = 0;
  uint16_t sum_ = 0;
  DataValueList data_list_;

  void BodyToDataList(const std::vector<uint8_t>& body, size_t offset);
  bool invalid_checksum_ =
      false;  ///< Used as invalid response message indicator
};

}  // namespace asap3
