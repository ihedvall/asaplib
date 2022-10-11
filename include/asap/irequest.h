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
  [[nodiscard]] const DataValueList& DataList() const {
    return data_list_;
  }

  void CreateBody(std::vector<uint8_t>& body) const;

 protected:
  uint16_t cmd_ = 0;
  DataValueList data_list_;
};

}  // namespace asap3
