/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <util/logconfig.h>
#include <util/utilfactory.h>

#include <chrono>
#include <thread>

#include "asap/asap3factory.h"

using namespace std::chrono_literals;
using namespace util::log;

namespace {}

namespace asap3::test {

TEST(Asap3Client, TestConnect)  // NOLINT
{
  auto& log_config = LogConfig::Instance();
  log_config.Type(LogType::LogToConsole);
  log_config.CreateDefaultLogger();

  auto listen_console =
      util::UtilFactory::CreateListen("ListenConsole", "LISASAP3CLIENT");
  listen_console->Start();
  listen_console->SetActive(true);
  auto client =
      Asap3Factory::CreateAsap3Client(Asap3ClientType::QueryComTestParameters);
  ASSERT_TRUE(client);

  client->Host("127.0.0.1");
  client->Port(6003);

  const auto start = client->Start();
  EXPECT_TRUE(start);
  DataValueList emergency_list = {
      {"Event", Mc3DataType::A_UINT16, static_cast<uint16_t>(666)}};

  for (size_t count = 0; count < 10; ++count) {
    if (client->IsIdle()) {
      break;
    }
    std::this_thread::sleep_for(1s);
  }
  client->SendTelegram(CommandCode::EMERGENCY, emergency_list,
                       [&](bool success, const ITelegram& telegram) {
                         std::cout << "Emergency" << std::endl;
                       });
  const auto stop = client->Stop();
  EXPECT_TRUE(stop);
  client.reset();
  EXPECT_TRUE(stop);
  listen_console->Stop();
  listen_console.reset();
}

}  // namespace asap3::test