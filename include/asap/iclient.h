/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <util/ilisten.h>
#include <util/threadsafequeue.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

#include "asap/a3parameter.h"
#include "asap/asap3def.h"
#include "asap/itelegram.h"

namespace asap3 {

class IClient {
 public:
  IClient();
  virtual ~IClient();

  IClient(const IClient&) = delete;
  IClient& operator=(const IClient&) = delete;

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

  [[nodiscard]] ServiceList AvailableServices() const;
  [[nodiscard]] bool HasService(const std::string& service) const;

  void ParameterList(const A3ParameterList& parameter_list);
  [[nodiscard]] const A3ParameterList& ParameterList() {
    return parameter_list_;
  }
  void ClearParameterList() { parameter_list_.clear(); }

  virtual bool Start() = 0;
  virtual bool Stop() = 0;

  bool IsConnected() const;
  virtual bool IsIdle() const = 0;
  virtual bool WaitOnIdle() const = 0;

  void SendTelegram(CommandCode cmd, const std::vector<DataValue>& data_list);
  void SendTelegram(CommandCode cmd, const std::vector<DataValue>& data_list,
                    ITelegram::OnCompleteFunction on_complete_function);

  virtual bool StartSubscription(uint16_t scan_rate);
  virtual bool StopSubscription();
  [[nodiscard]] bool IsScanning() const;

  void SetOnlineData(const std::vector<uint8_t>& body, size_t offset);
  void DefineUserDefinedData(const std::vector<uint8_t>& body, size_t offset);
  void SetUserDefinedData(const std::vector<uint8_t>& body, size_t offset);

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
  std::unique_ptr<util::log::IListen> listen_;

  mutable std::mutex value_locker_;
  DataValueList user_defined_list_;  ///< User defined list (Name, type, value)
  ServiceList service_list_;  ///< List of available services in the server
  A3ParameterList parameter_list_;   ///< Requested parameter list
  DataValueList online_value_list_;  ///< Current subscription (read) values
  DataValueList output_value_list_;  ///< Set-point value list

  virtual bool HandleTelegram(ITelegram& telegram);
  [[nodiscard]] bool IsSubscriptionInitialized() const;

  void SetServiceList(const DataValueList& data_list);
  void SetServiceInfo(const std::string& service, const std::string& info);

  void ListenRequest(const IRequest& request);
  void ListenResponse(const IResponse& response);
};

}  // namespace asap3
