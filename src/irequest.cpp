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

}  // namespace asap3