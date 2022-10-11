/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <util/threadsafequeue.h>

#include <cstdint>
#include <string>
#include <atomic>

#include "asap/asap3def.h"
#include "asap/itelegram.h"

namespace asap3 {

class IClient {
 public:
  virtual ~IClient() = default;
  void Name(const std::string& name) { name_ = name; }
  [[nodiscard]] const std::string& Name() const { return name_; }

  void Host(const std::string& host) { host_ = host; }
  [[nodiscard]] const std::string& Host() const { return host_; }

  void Port(uint16_t port) { port_ = port; }
  [[nodiscard]] uint16_t Port() const { return port_; }

  void Version(uint16_t version) { version_ = version; }
  [[nodiscard]] uint16_t Version() const { return version_; }

  [[nodiscard]] const std::string& RemoteName() const { return remote_name_; }
  [[nodiscard]] uint16_t RemoteVersion() const { return remote_version_; }

  virtual bool Start() = 0;
  virtual bool Stop() = 0;
  virtual bool HandleTelegram(ITelegram& telegram);

  bool IsConnected() const;
  virtual bool IsIdle() const = 0;

  void SendTelegram(CommandCode cmd, const std::vector<DataValue>& data_list);
  void SendTelegram(CommandCode cmd, const std::vector<DataValue>& data_list,
                    ITelegram::OnCompleteFunction on_complete_function);

 protected:
  using TelegramQueue = util::log::ThreadSafeQueue<ITelegram>;
  TelegramQueue telegram_queue_;

  std::string host_ = "127.0.0.1";
  uint16_t port_ = 22222;

  std::string name_;
  uint16_t version_ = 3 * 256 + 0;  ///< Version is 3.0

  std::string remote_name_;
  uint16_t remote_version_ = 0;

  std::atomic<bool> connected_ = false;
};

}  // namespace asap3
