/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap/itelegram.h"

namespace asap3 {
ITelegram::ITelegram(CommandCode cmd, const DataValueList &data_list)
    : request_(std::make_unique<IRequest>(cmd, data_list)) {}

ITelegram::ITelegram(CommandCode cmd, const DataValueList &data_list,
                     OnCompleteFunction on_complete)
    : request_(std::make_unique<IRequest>(cmd, data_list)),
      on_complete_(std::move(on_complete)) {}

void ITelegram::OnComplete(bool success) {
  if (on_complete_) {
    on_complete_(success, *this);
  }
}

}  // namespace asap3