/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "asap3client.h"

namespace asap3 {

class CtAsap3Client : public Asap3Client {
  CtAsap3Client() = default;
  ~CtAsap3Client() override;

 protected:
  void OnStartMessage() override;

 private:
  void SendCtInitCommand();
};

}  // namespace asap3
