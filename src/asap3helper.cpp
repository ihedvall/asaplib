/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "asap3helper.h"

#include <sstream>

namespace asap3 {

uint16_t Asap3Helper::Checksum(const std::vector<uint8_t> &message) {
  uint16_t sum = 0;
  uint16_t temp = 0;
  for (size_t count = 0; count < message.size() - 2; count += 2) {
    ToMc3Value(message, count, temp);
    sum += temp;
  }
  return sum;
}

size_t Asap3Helper::DataListSize(const std::vector<DataValue> &data_list) {
  size_t count = 0;
  for (const auto &data : data_list) {
    switch (data.type) {
      case Mc3DataType::A_FLOAT64:
        count += sizeof(double);
        break;

      case Mc3DataType::MC3_STRING: {
        auto temp = std::any_cast<std::string>(data.value).size();
        if ((temp % 2) != 0) {
          ++temp;
        }
        count += 2;
        break;
      }

      case Mc3DataType::A_INT16:
      case Mc3DataType::A_UINT16:
        count += sizeof(uint16_t);
        break;

      case Mc3DataType::A_INT32:
      case Mc3DataType::A_UINT32:
        count += sizeof(uint32_t);
        break;

      case Mc3DataType::A_INT64:
      case Mc3DataType::A_UINT64:
        count += sizeof(uint64_t);
        break;

      default:
        count += sizeof(float);
        break;
    }
  }
  return count;
}

void Asap3Helper::DataListToBody(const std::vector<DataValue> &data_list,
                                 std::vector<uint8_t> &body, size_t &offset) {
  for (const auto &data : data_list) {
    switch (data.type) {
      case Mc3DataType::A_FLOAT64: {
        const auto value = std::any_cast<double>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::MC3_STRING: {
        const auto value = std::any_cast<std::string>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::A_INT16: {
        const auto value = std::any_cast<int16_t>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::A_UINT16: {
        const auto value = std::any_cast<uint16_t>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::A_INT32: {
        const auto value = std::any_cast<int32_t>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::A_UINT32: {
        const auto value = std::any_cast<uint32_t>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::A_INT64: {
        const auto value = std::any_cast<int64_t>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::A_UINT64: {
        const auto value = std::any_cast<uint64_t>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }

      case Mc3DataType::A_FLOAT32:
      default: {
        const auto value = std::any_cast<float>(data.value);
        offset += Asap3Helper::FromMc3Value(body, offset, value);
        break;
      }
    }
  }
}

void Asap3Helper::BodyToDataList(const std::vector<uint8_t> &body,
                                 size_t &offset,
                                 std::vector<DataValue> &data_list) {
  size_t index = offset;
  for (auto &data : data_list) {
    size_t data_size = 0;
    switch (data.type) {
      case Mc3DataType::A_FLOAT64:
        data_size = sizeof(double);
        break;

      case Mc3DataType::MC3_STRING: {
        uint16_t text_length = 0;
        if (index + 2 <= body.size()) {
          ToMc3Value(body, index, text_length);
        }
        data_size = text_length + 2;
        if ((data_size % 2) == 1) {
          ++data_size;
        }
        break;
      }

      case Mc3DataType::A_INT16:
      case Mc3DataType::A_UINT16:
        data_size = sizeof(uint16_t);
        break;

      case Mc3DataType::A_INT32:
      case Mc3DataType::A_UINT32:
        data_size = sizeof(uint32_t);
        break;

      case Mc3DataType::A_INT64:
      case Mc3DataType::A_UINT64:
        data_size = sizeof(uint64_t);
        break;

      default:
        data_size = sizeof(float);
        break;
    }
    if (index + data_size > body.size()) {
      break;
    }

    switch (data.type) {
      case Mc3DataType::A_FLOAT64: {
        double temp = 0.0;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::MC3_STRING: {
        std::string temp;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::A_INT16: {
        int16_t temp = 0;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::A_UINT16: {
        uint16_t temp = 0;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::A_INT32: {
        int32_t temp = 0;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::A_UINT32: {
        uint32_t temp = 0;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::A_INT64: {
        int64_t temp = 0;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::A_UINT64: {
        uint64_t temp = 0;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }

      case Mc3DataType::A_FLOAT32:
      default: {
        float temp = 0.0F;
        index += ToMc3Value(body, index, temp);
        data.value = temp;
        break;
      }
    }
  }
}

std::string Asap3Helper::DataListToText(
    const std::vector<DataValue> &data_list) {
  std::ostringstream temp;
  for (const auto &data : data_list) {
    temp << data.name << ": ";
    switch (data.type) {
      case Mc3DataType::A_FLOAT64: {
        const auto value = std::any_cast<double>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::MC3_STRING: {
        const auto value = std::any_cast<std::string>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::A_INT16: {
        const auto value = std::any_cast<int16_t>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::A_UINT16: {
        const auto value = std::any_cast<uint16_t>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::A_INT32: {
        const auto value = std::any_cast<int32_t>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::A_UINT32: {
        const auto value = std::any_cast<uint32_t>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::A_INT64: {
        const auto value = std::any_cast<int64_t>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::A_UINT64: {
        const auto value = std::any_cast<uint64_t>(data.value);
        temp << value;
        break;
      }

      case Mc3DataType::A_FLOAT32:
      default: {
        const auto value = std::any_cast<float>(data.value);
        temp << value;
        break;
      }
    }
    temp << " ";
  }
  return temp.str();
}

std::string Asap3Helper::CommandCodeToText(CommandCode command) {
  switch (command) {
    case CommandCode::REPEAT_REQUEST:
      return "REPEAT REQUEST";
    case CommandCode::EMERGENCY:
      return "EMERGENCY";
    case CommandCode::INIT:
      return "INIT";
    case CommandCode::SELECT_DESCRIPTION_FILE_AND_BINARY_FILE:
      return "SELECT DESCRIPTION FILE AND BINARY_FILE";
    case CommandCode::COPY_BINARY_FILE:
      return "COPY BINARY FILE";
    case CommandCode::CHANGE_BINARY_FILE:
      return "CHANGE BINARY FILE";
    case CommandCode::SELECT_LOOKUP_TABLE:
      return "SELECT LOOKUP TABLE";
    case CommandCode::PUT_LOOKUP_TABLE:
      return "PUT LOOKUP TABLE";
    case CommandCode::GET_LOOKUP_TABLE:
      return "GET LOOKUP TABLE";
    case CommandCode::INCREASE_LOOKUP_TABLE:
      return "INCREASE LOOKUP TABLE";
    case CommandCode::PARAMETER_FOR_VALUE_ACQUISITION:
      return "PARAMETER FOR VALUE ACQUISITION";
    case CommandCode::SWITCHING_OFFLINE_ONLINE:
      return "SWITCHING OFFLINE ONLINE";
    case CommandCode::GET_PARAMETER:
      return "GET PARAMETER";
    case CommandCode::SET_PARAMETER:
      return "SET PARAMETER";
    case CommandCode::SET_GRAPHIC_MODE:
      return "SET GRAPHIC MODE";
    case CommandCode::RESET_DEVICE:
      return "RESET DEVICE";
    case CommandCode::SET_FORMAT:
      return "SET FORMAT";
    case CommandCode::GET_ONLINE_VALUE:
      return "GET ONLINE VALUE";
    case CommandCode::IDENTIFY:
      return "IDENTIFY";
    case CommandCode::GET_USER_DEFINED_VALUE:
      return "GET USER DEFINED VALUE";
    case CommandCode::GET_USER_DEFINED_VALUE_LIST:
      return "GET USER DEFINED VALUE_LIST";
    case CommandCode::DEFINE_DESCRIPTION_FILE_AND_BINARY_FILE:
      return "DEFINE DESCRIPTION FILE AND BINARY FILE";
    case CommandCode::EXIT:
      return "EXIT";
    case CommandCode::DEFINE_RECORDER_PARAMETERS:
      return "DEFINE RECORDER PARAMETERS";
    case CommandCode::DEFINE_TRIGGER_CONDITION:
      return "DEFINE TRIGGER CONDITION";
    case CommandCode::ACTIVATE_RECORDER:
      return "ACTIVATE RECORDER";
    case CommandCode::GET_RECODER_STATUS:
      return "GET RECODER STATUS";
    case CommandCode::GET_RECORDER_RESULT_HEADER:
      return "GET RECORDER RESULT HEADER";
    case CommandCode::GET_RECORDER_RESULTS:
      return "GET RECORDER RESULTS";
    case CommandCode::SAVE_RECORDER_FILE:
      return "SAVE RECORDER FILE";
    case CommandCode::LOAD_RECORDER_FILE:
      return "LOAD RECORDER FILE";
    case CommandCode::SET_CASE_SENSITIVE_LABELS:
      return "SET CASE SENSITIVE LABELS";
    case CommandCode::PUT_LOOKUP_TABLE_EV2:
      return "PUT LOOKUP TABLE EV2";
    case CommandCode::GET_LOOKUP_TABLE_EV2:
      return "GET LOOKUP TABLE EV2";
    case CommandCode::INCREASE_LOOKUP_TABLE_EV2:
      return "INCREASE LOOKUP TABLE EV2";
    case CommandCode::SELECT_LOOKUP_TABLE_EV2:
      return "SELECT LOOKUP TABLE EV2";
    case CommandCode::PARAMETER_FOR_VALUE_ACQUISITION_EV2:
      return "PARAMETER FOR VALUE ACQUISITION EV2";
    case CommandCode::GET_PARAMETER_EV2:
      return "GET PARAMETER EV2";
    case CommandCode::SET_PARAMETER_EV2:
      return "SET PARAMETER EV2";
    case CommandCode::GET_ONLINE_VALUE_EV2:
      return "GET ONLINE VALUE EV2";
    case CommandCode::GET_RECODER_RESULTS_EV2:
      return "GET RECODER RESULTS EV2";
    case CommandCode::GET_RECORDER_RESULT_DATA_EV2:
      return "GET RECORDER RESULT DATA EV2";
    case CommandCode::GET_CALPAGE_INFO:
      return "GET CALPAGE INFO";
    case CommandCode::GET_CURRENT_CALPAGE:
      return "GET CURRENT CALPAGE";
    case CommandCode::GET_MEASUREMENT_INFO:
      return "GET MEASUREMENT INFO";
    case CommandCode::GET_RASTER_OVERVIEW:
      return "GET RASTER OVERVIEW";
    case CommandCode::GET_CHARACTERISTIC_INFO:
      return "GET CHARACTERISTIC INFO";
    case CommandCode::READ_CHARACTERISTIC:
      return "READ CHARACTERISTIC";
    case CommandCode::READ_CELL_VALUES:
      return "READ CELL VALUES";
    case CommandCode::WRITE_CHARACTERISTIC:
      return "WRITE CHARACTERISTIC";
    case CommandCode::WRITE_CELL_VALUES:
      return "WRITE CELL VALUES";
    case CommandCode::SELECT_CHARACTERISTIC:
      return "SELECT CHARACTERISTIC";
    case CommandCode::QUERY_AVAILABLE_SERVICE:
      return "QUERY AVAILABLE SERVICE";
    case CommandCode::GET_SERVICE_INFORMATION:
      return "GET SERVICE INFORMATION";
    case CommandCode::EXECUTE_SERVICE:
      return "EXECUTE SERVICE";
    default:
      break;
  }
  return {};
}

std::string Asap3Helper::StatusCodeToText(StatusCode status) {
  switch (status) {
    case StatusCode::STATUS_OK:
      return "OK";
    case StatusCode::STATUS_SUCCESS:
      return "SUCCESS";
    case StatusCode::STATUS_NOT_PROCESSED:
      return "NOT PROCESSED";
    case StatusCode::STATUS_MEASURING_DATA_CHANGED:
      return "MEASURING DATA CHANGED";
    case StatusCode::STATUS_RESERVED:
      return "RESERVED";
    case StatusCode::STATUS_CMD_NOT_AVAILABLE:
      return "CMD NOT AVAILABLE";
    case StatusCode::STATUS_ACK:
      return "ACK";
    case StatusCode::STATUS_REPEAT_CMD:
      return "REPEAT CMD";
    case StatusCode::STATUS_ERROR:
      return "ERROR";
    default:
      break;
  }
  return {};
}

std::string Asap3Helper::RequestToPlainText(const IRequest &request) {
  std::ostringstream temp;
  temp << "T:" << CommandCodeToText(request.Cmd()) << " "
       << DataListToText(request.DataList());
  return temp.str();
}
std::string Asap3Helper::ResponseToPlainText(const IResponse &response) {
  std::ostringstream temp;
  temp << "R:" << CommandCodeToText(response.Cmd())
       << " S:" << StatusCodeToText(response.Status()) << " "
       << DataListToText(response.DataList());
  return temp.str();
}

template <>
size_t Asap3Helper::ToMc3Value(const std::vector<uint8_t> &data, size_t offset,
                               std::string &dest) {
  size_t index = 0;
  uint16_t text_size = 0;
  if (data.size() < offset + 2) {
    dest = {};
    return 2;
  }
  index += ToMc3Value(data, offset, text_size);
  std::ostringstream temp;
  for (size_t input = 0;
       (input < text_size) && (offset + input + 2 < data.size()); ++input) {
    temp << static_cast<char>(data[offset + input + 2]);
    ++index;
  }
  if ((index % 2) != 0) {
    ++index;
  }
  dest = temp.str();
  return index;
}

template <>
size_t Asap3Helper::FromMc3Value(std::vector<uint8_t> &data, size_t offset,
                                 const std::string &source) {
  auto text_size = source.size();
  if ((text_size % 2) != 0) {
    ++text_size;
  }
  if (data.size() < offset + text_size + sizeof(uint16_t)) {
    data.resize(offset + text_size + sizeof(uint16_t));
  }
  const auto length = static_cast<uint16_t>(source.size());

  boost::endian::endian_buffer<boost::endian::order::big, uint16_t,
                               sizeof(length) * 8>
      buff(length);
  memcpy(data.data() + offset, buff.data(), sizeof(length));
  size_t count = 0;
  for (const char input : source) {
    data[offset + 2 + count] = static_cast<uint8_t>(input);
    ++count;
  }
  while (count < text_size) {
    data[offset + 2 + count] = 0;
    ++count;
  }

  return text_size + sizeof(uint16_t);
}

template <>
size_t Asap3Helper::FromMc3Value(std::vector<uint8_t> &data, size_t offset,
                                 const std::vector<uint8_t> &source) {
  auto source_size = source.size();
  if ((source_size % 2) != 0) {
    ++source_size;
  }
  if (data.size() < offset + source_size) {
    data.resize(offset + source_size, 0);
  }
  memcpy(data.data() + offset, source.data(), source.size());
  return source_size;
}

}  // namespace asap3