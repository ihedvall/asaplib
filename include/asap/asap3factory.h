/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include <memory>

#include "asap/iclient.h"

namespace asap3 {

enum class Asap3ClientType : uint8_t { QueryComTestParameters = 0 };

class Asap3Factory {
 public:
  static std::unique_ptr<IClient> CreateAsap3Client(Asap3ClientType type);
};

}  // namespace asap3
