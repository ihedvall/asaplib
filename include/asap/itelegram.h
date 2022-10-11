/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <functional>
#include <memory>

#include "asap/asap3def.h"
#include "asap/irequest.h"
#include "asap/iresponse.h"
namespace asap3 {

class ITelegram {
 public:
  using OnCompleteFunction =
      std::function<void(bool success, const ITelegram& telegram)>;

  ITelegram() = default;
  ITelegram(CommandCode cmd, const DataValueList& data_list);
  ITelegram(CommandCode cmd, const DataValueList& data_list,
            OnCompleteFunction on_complete);

  [[nodiscard]] const IRequest* Request() const { return request_.get(); }
  [[nodiscard]] const IResponse* Response() const { return response_.get(); }

  void Response(std::unique_ptr<IResponse>& response) {
    response_ = std::move(response);
  }

  void OnComplete(bool success);

 protected:
  std::unique_ptr<IRequest> request_;
  std::unique_ptr<IResponse> response_;
  OnCompleteFunction on_complete_;
};

}  // namespace asap3
