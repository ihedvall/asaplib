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
  QueryParameters();
  ~QueryParameters() override;

};

}  // namespace asap3
