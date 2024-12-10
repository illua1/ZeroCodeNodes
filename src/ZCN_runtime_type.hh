#pragma once

#include <string>
#include <variant>

namespace zcn {

enum class DataType : int8_t {
  Text,
  Float,
  Int,
  Bool,
};

template<typename T>
inline DataType static_type_to_type()
{
  if constexpr (std::is_same_v<T, bool>) {
    return DataType::Bool;
  }
  if constexpr (std::is_same_v<T, int>) {
    return DataType::Int;
  }
  if constexpr (std::is_same_v<T, float>) {
    return DataType::Float;
  }
  if constexpr (std::is_same_v<T, std::string>) {
    return DataType::Text;
  }
}

std::string data_type_to_name(DataType type);

DataType name_to_data_type(const std::string &name);

using RData = std::variant<bool, int, float, std::string>;

RData defult_value(DataType type);

RData ensure_type(DataType type, RData source);

}