/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <string>
#include <vector>

#include "asap/asap3def.h"
#include "asap/irequest.h"
#include "asap/iresponse.h"
#include "util/stringutil.h"

namespace asap3 {

class Asap3Helper {
 public:
  template <typename T>
  static size_t ToMc3Value(const std::vector<uint8_t>& data, size_t offset,
                           T& dest);

  template <typename T>
  static size_t FromMc3Value(std::vector<uint8_t>& data, size_t offset,
                             const T& source);

  static uint16_t Checksum(const std::vector<uint8_t>& message);
  static size_t DataListSize(const std::vector<DataValue>& data_list);
  static void DataListToBody(const std::vector<DataValue>& data_list,
                             std::vector<uint8_t>& body, size_t& offset);
  static void BodyToDataList(const std::vector<uint8_t>& body, size_t& offset,
                             std::vector<DataValue>& data_list);
  static std::string DataListToText(const std::vector<DataValue>& data_list);
  template <typename T>
  static void SetDataListProperty(DataValueList& data_list,
                                  const std::string& property, const T& value);

  static std::string CommandCodeToText(CommandCode command);
  static std::string StatusCodeToText(StatusCode status);
  static std::string RequestToPlainText(const IRequest& request);
  static std::string ResponseToPlainText(const IResponse& response);
};

template <typename T>
size_t Asap3Helper::ToMc3Value(const std::vector<uint8_t>& data, size_t offset,
                               T& dest) {
  const size_t data_size = sizeof(T);
  if (data.size() < data_size + offset) {
    dest = {};
  } else {
    boost::endian::endian_buffer<boost::endian::order::big, T, sizeof(T)* 8>
        buff = {};
    memcpy(buff.data(), data.data() + offset, data_size);
    dest = buff.value();
  }
  return data_size;
}

template <>
size_t Asap3Helper::ToMc3Value(const std::vector<uint8_t>& data, size_t offset,
                               std::string& dest);

template <typename T>
size_t Asap3Helper::FromMc3Value(std::vector<uint8_t>& data, size_t offset,
                                 const T& source) {
  const size_t data_size = sizeof(T);
  if (data.size() < data_size + offset) {
    data.resize(data_size + offset, 0);
  }
  boost::endian::endian_buffer<boost::endian::order::big, T, sizeof(T) * 8>
      buff(source);
  memcpy(data.data() + offset, buff.data(), data_size);
  return data_size;
}

template <>
size_t Asap3Helper::FromMc3Value(std::vector<uint8_t>& data, size_t offset,
                                 const std::string& source);

template <>
size_t Asap3Helper::FromMc3Value(std::vector<uint8_t>& data, size_t offset,
                                 const std::vector<uint8_t>& source);

template <typename T>
void Asap3Helper::SetDataListProperty(DataValueList& data_list,
                                      const std::string& property,
                                      const T& value) {
  auto itr = std::ranges::find_if(data_list, [&](const auto& data) {
    return util::string::IEquals(property, data.name);
  });
  if (itr != data_list.end()) {
    itr->value = value;
  }
}

}  // namespace asap3
