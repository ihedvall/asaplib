/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap/asap3factory.h"

#include "queryparameters.h"

namespace asap3 {

std::unique_ptr<IClient> Asap3Factory::CreateAsap3Client(Asap3ClientType type) {
  std::unique_ptr<IClient> client;
  switch (type) {
    case Asap3ClientType::QueryCtParameters: {
      auto temp = std::make_unique<QueryParameters>();
      client = std::move(temp);
      break;
    }

    case Asap3ClientType::BasicAsap3Client:
    default: {
      auto temp = std::make_unique<Asap3Client>();
      client = std::move(temp);
      break;
    }

    break;
  }
  return client;
}

}  // namespace asap3