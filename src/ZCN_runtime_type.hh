#pragma once

#include <string>
#include <variant>

namespace zcn {

enum class DataType : int8_t {
  Text,
  Float,
  Int,
};

using RData = std::variant<bool, int, float, std::string>;

RData defult_value(DataType type);

RData ensure_type(DataType type, RData source);

}