/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <any>
#include <cstdint>
#include <string>
#include <vector>
namespace asap3 {
enum class Mc3DataType : uint16_t {
  A_FLOAT32 = 0,
  A_FLOAT64 = 1,
  MC3_STRING = 2,
  A_INT16 = 3,
  A_UINT16 = 4,
  A_INT32 = 5,
  A_UINT32 = 6,
  A_INT64 = 7,
  A_UINT64 = 8,
  NoType = 0xFF
};

enum class Mc3ValueType : uint16_t {
  VALUE = 0,
  CONSTANT = 1,
  OFFSET = 2,
  FACTOR = 3
};

enum class Mc3CharType : uint16_t {
  VALUE = 0,
  CURVE = 1,
  MAP = 2,
  CUBE3D = 3,
  CUBE4D = 4,
  CUBE5D = 5,
  VALBLK_1D = 6,
  VALBLK_2D = 7,
  VALBLK_3D = 8,
  VALBLK_4D = 9,
  VALBLK_5D = 10
};

enum class CommandCode : uint16_t {
  REPEAT_REQUEST = 0x00,
  EMERGENCY = 0x01,
  INIT = 0x02,
  SELECT_DESCRIPTION_FILE_AND_BINARY_FILE = 0x03,
  COPY_BINARY_FILE = 0x04,
  CHANGE_BINARY_FILE = 0x05,
  SELECT_LOOKUP_TABLE = 0x06,
  PUT_LOOKUP_TABLE = 0x07,
  GET_LOOKUP_TABLE = 0x08,
  INCREASE_LOOKUP_TABLE = 0x0A,
  PARAMETER_FOR_VALUE_ACQUISITION = 0x0C,
  SWITCHING_OFFLINE_ONLINE = 0x0D,
  GET_PARAMETER = 0x0E,
  SET_PARAMETER = 0x0F,
  SET_GRAPHIC_MODE = 0x10,
  RESET_DEVICE = 0x11,
  SET_FORMAT = 0x12,
  GET_ONLINE_VALUE = 0x13,
  IDENTIFY = 0x14,
  GET_USER_DEFINED_VALUE = 0x15,
  GET_USER_DEFINED_VALUE_LIST = 0x16,
  DEFINE_DESCRIPTION_FILE_AND_BINARY_FILE = 0x1E,
  EXIT = 0x32,
  DEFINE_RECORDER_PARAMETERS = 0x29,
  DEFINE_TRIGGER_CONDITION = 0x2A,
  ACTIVATE_RECORDER = 0x2B,
  GET_RECODER_STATUS = 0x2C,
  GET_RECORDER_RESULT_HEADER = 0x2D,
  GET_RECORDER_RESULTS = 0x2E,
  SAVE_RECORDER_FILE = 0x2F,
  LOAD_RECORDER_FILE = 0x30,
  SET_CASE_SENSITIVE_LABELS = 0x3D,
  PUT_LOOKUP_TABLE_EV2 = 0x6B,
  GET_LOOKUP_TABLE_EV2 = 0x6D,
  INCREASE_LOOKUP_TABLE_EV2 = 0x6E,
  SELECT_LOOKUP_TABLE_EV2 = 0x6F,
  PARAMETER_FOR_VALUE_ACQUISITION_EV2 = 0x70,
  GET_PARAMETER_EV2 = 0x72,
  SET_PARAMETER_EV2 = 0x73,
  GET_ONLINE_VALUE_EV2 = 0x77,
  GET_RECODER_RESULTS_EV2 = 0x92,
  GET_RECORDER_RESULT_DATA_EV2 = 0x95,
  GET_CALPAGE_INFO = 0xA0,
  GET_CURRENT_CALPAGE = 0xA1,
  GET_MEASUREMENT_INFO = 0xA3,
  GET_RASTER_OVERVIEW = 0xA4,
  GET_CHARACTERISTIC_INFO = 0xA5,
  READ_CHARACTERISTIC = 0xA6,
  READ_CELL_VALUES = 0xA7,
  WRITE_CHARACTERISTIC = 0xA8,
  WRITE_CELL_VALUES = 0xA9,
  SELECT_CHARACTERISTIC = 0xAA,
  QUERY_AVAILABLE_SERVICE = 0xC8,
  GET_SERVICE_INFORMATION = 0xC9,
  EXECUTE_SERVICE = 0xCA,
};

enum class StatusCode : uint16_t {
  STATUS_OK = 0x0000,
  STATUS_SUCCESS = 0x1232,
  STATUS_NOT_PROCESSED = 0x2343,
  STATUS_MEASURING_DATA_CHANGED = 0x2344,
  STATUS_RESERVED = 0x3454,
  STATUS_CMD_NOT_AVAILABLE = 0x5656,
  STATUS_ACK = 0xAAAA,
  STATUS_REPEAT_CMD = 0xEEEE,
  STATUS_ERROR = 0xFFFF,
};

struct DataValue {
  std::string name;
  Mc3DataType type;
  std::any value;
};

using DataValueList = std::vector<DataValue>;

struct Service {
  std::string name;
  std::string info;
};

using ServiceList = std::vector<Service>;

}  // namespace asap3
