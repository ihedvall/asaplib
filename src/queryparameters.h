/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <array>
#include <atomic>
#include <boost/asio.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "asap3client.h"

namespace asap3 {

class QueryParameters : public Asap3Client {
 public:
  QueryParameters() = default;
  ~QueryParameters() override;

  bool Start() override;

 protected:
  void OnStartMessage() override;
  bool HandleTelegram(ITelegram& telegram) override;

 private:
  void SetExecuteService(const std::string& service, const std::string& input,
                         const std::string& output);
};

}  // namespace asap3
