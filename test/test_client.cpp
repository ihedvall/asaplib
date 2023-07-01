/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include <util/logconfig.h>
#include <util/utilfactory.h>

#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <chrono>
#include <thread>
#include <variant>

#include "asap/asap3factory.h"

using namespace std::chrono_literals;
using namespace util::log;

namespace {}

namespace asap3::test {

TEST(Asap3Client, TestBoostSplit)  // NOLINT
{
  const std::string test_string1 = "Olle\nPelle\n";
  const std::string test_string2 = "Olle\nPelle";

  std::vector<std::string> test_list1;
  boost::algorithm::split(test_list1, test_string1, boost::is_any_of("\n"));
  EXPECT_EQ(test_list1.size(),
            3);  // Expected 2 here but gets an empty string ?

  std::vector<std::string> test_list2;
  boost::algorithm::split(test_list2, test_string2, boost::is_any_of("\n"));
  EXPECT_EQ(test_list2.size(), 2);

  for (size_t index = 0; index < 2; ++index) {
    EXPECT_EQ(test_list1[index], test_list2[index]);
    std::cout << test_list1[index] << " : " << test_list2[index] << std::endl;
  }
}

TEST(Asap3Client, TestAnyCast) {  // NOLINT
  const std::string test_string = "Olle";
  const auto& test_const = &test_string;
  const auto* test_cstring = test_string.c_str();

  try {
    std::any test1 = test_string;
    std::any test2 = test_const;
    std::any test3 = test_cstring;
    std::cout << std::any_cast<std::string>(test1) << 1 << std::endl;
    // std::cout << std::any_cast<std::string>(test2) << 2 << std::endl;
    //     std::cout << std::any_cast<std::string>(test3) << 3 << std::endl;
  } catch (const std::exception& err) {
    FAIL() << "ANY: " << err.what();
  }

  try {
    boost::any test1 = test_string;
    boost::any test2 = test_const;
    boost::any test3 = test_cstring;
    std::cout << boost::any_cast<std::string>(test1) << 1 << std::endl;
    //  std::cout << boost::any_cast<std::string>(test2) << 2 << std::endl;
    // std::cout << boost::any_cast<std::string>(test3) << 3 << std::endl;
  } catch (const std::exception& err) {
    FAIL() << "BOOST: " << err.what();
  }
}

TEST(Asap3Client, TestConnect)  // NOLINT
{
  auto& log_config = LogConfig::Instance();
  log_config.Type(LogType::LogToConsole);
  log_config.CreateDefaultLogger();

  auto listen_console =
      util::UtilFactory::CreateListen("ListenConsole", "LIS_ASAP3_CLIENT");
  listen_console->Start();
  listen_console->SetActive(true);

  auto query =
      Asap3Factory::CreateAsap3Client(Asap3ClientType::QueryCtParameters);
  query->Host("127.0.0.1");
  query->Port(6003);
  query->Name("CTPAR");
  EXPECT_TRUE(query->Start());
  EXPECT_TRUE(query->Stop());

  auto client =
      Asap3Factory::CreateAsap3Client(Asap3ClientType::BasicAsap3Client);
  ASSERT_TRUE(client);

  client->Host("127.0.0.1");
  client->Port(6003);
  client->Name("OLLE");

  const auto start = client->Start();
  EXPECT_TRUE(start);
  DataValueList emergency_list = {
      {"Event", Mc3DataType::A_UINT16, static_cast<uint16_t>(666)}};

  client->WaitOnIdle();

  const auto service_list = client->AvailableServices();
  EXPECT_FALSE(service_list.empty());

  const auto& parameter_list = query->ParameterList();
  for (const auto& parameter : parameter_list) {
    std::cout << "Parameter: " << parameter.Name() << " " << parameter.Unit()
              << std::endl;
  }
  client->StartSubscription(0);
  client->WaitOnIdle();

  client->SendTelegram(CommandCode::EMERGENCY, emergency_list,
                       [&](bool success, const ITelegram& telegram) {
                         std::cout << "Emergency" << std::endl;
                       });
  EXPECT_TRUE(client->Stop());

  client.reset();
  query.reset();

  listen_console->Stop();
  listen_console.reset();
}

}  // namespace asap3::test