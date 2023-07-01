/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include <mutex>
#include <string>

#include "asap/asap3def.h"

namespace asap3 {

class A3Parameter {
 public:
  void Name(const std::string& name) { name_ = name; }
  [[nodiscard]] const std::string& Name() const { return name_; }

  void DisplayName(const std::string& display_name) {
    display_name_ = display_name;
  }
  [[nodiscard]] const std::string& DisplayName() const { return display_name_; }

  void Description(const std::string& description) {
    description_ = description;
  }
  [[nodiscard]] const std::string& Description() const { return description_; }

  void Unit(const std::string& unit) { unit_ = unit; }
  [[nodiscard]] const std::string& Unit() const { return unit_; }

  void Device(const std::string& device) { device_ = device; }
  [[nodiscard]] const std::string& Device() const { return device_; }

  void Identity(const std::string& identity) { identity_ = identity; }
  [[nodiscard]] const std::string& Identity() const { return identity_; }

  void Signal(const std::string& signal) { signal_ = signal; }
  [[nodiscard]] const std::string& Signal() const { return signal_; }

  void Type(Mc3DataType type) { type_ = type; }
  [[nodiscard]] Mc3DataType Type() const { return type_; }

  void Max(double max) { max_ = max; }
  [[nodiscard]] double Max() const { return max_; }

  void Min(double min) { min_ = min; }
  [[nodiscard]] double Min() const { return min_; }

  void SetPoint(bool set_point) { set_point_ = set_point; }
  [[nodiscard]] bool SetPoint() const { return set_point_; }

  void NofDecimals(uint8_t nof_decimals) { nof_decimals_ = nof_decimals; }
  [[nodiscard]] uint8_t NofDecimals() const { return nof_decimals_; }

  void CycleTime(int cycle_time) { cycle_time_ = cycle_time; }
  [[nodiscard]] int CycleTime() const { return cycle_time_; }

  void LunNo(uint16_t lun) { lun_ = lun; }
  [[nodiscard]] uint16_t LunNo() const { return lun_; }

  void Exist(bool exist) { exist_ = exist; }
  [[nodiscard]] bool Exist() const { return exist_; }

  void ValueIndex(size_t index) {
    value_index_ = index;
  }
  [[nodiscard]] size_t ValueIndex() const {
    return value_index_;
  }

 private:
  std::string name_;
  std::string unit_;
  std::string description_;
  std::string device_; ///< Test equipment reference
  std::string signal_; ///< Signal or channel name
  std::string identity_;
  std::string display_name_;

  bool set_point_ = false;
  bool exist_ = true;
  size_t value_index_ = 0;

  uint8_t nof_decimals_ = 2;  ///< Number of decimals for floating point values
  int cycle_time_ = 0;        ///< Cycle time in ms
  double min_ = 0;
  double max_ = 0;
  uint16_t lun_ = 0;
  Mc3DataType type_ = Mc3DataType::A_FLOAT32;
};

using A3ParameterList = std::vector<A3Parameter>;
}  // namespace asap3
