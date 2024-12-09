#include <string>
#include <variant>
#include <stdexcept>


#include "ZCN_runtime_type.hh"

namespace zcn {

RData defult_value(DataType type)
{
  switch (type) {
    case DataType::Int:
      return int(0);
    case DataType::Float:
      return float(0.0f);
    case DataType::Text:
      return std::string("");
  }
  return {};
}


template<typename OutT, typename InT>
static OutT convert(InT value)
{
  return value;
}

template<> static int convert(bool value)
{
  return int(value);
}

template<> static int convert(float value)
{
  return int(value);
}

template<> static int convert(std::string value)
{
  try {
    return std::stoi(value);
  }
  catch (const std::invalid_argument /*e*/){
    return 0.0f;
  }
  catch (const std::out_of_range /*e*/){
    return 0.0f;
  }
}

template<> static float convert(bool value)
{
  return float(value);
}

template<> static float convert(int value)
{
  return float(value);
}

template<> static float convert(std::string value)
{
  try {
    return std::stof(value);
  }
  catch (const std::invalid_argument /*e*/){
    return 0.0f;
  }
  catch (const std::out_of_range /*e*/){
    return 0.0f;
  }
}

template<> static std::string convert(bool value)
{
  return value ? "Да" : "Нет";
}

template<> static std::string convert(int value)
{
  return std::to_string(value);
}

template<> static std::string convert(float value)
{
  return std::to_string(int(value * 10) * 0.1);
}

template<> static bool convert(int value)
{
  return value > 0;
}

template<> static bool convert(float value)
{
  return value > 1.0f;
}

template<> static bool convert(std::string value)
{
  return true;
}

RData ensure_type(const DataType type, RData source)
{
  RData result_value;
  std::visit([&](auto &&src_typed) {
    std::visit([&](auto &&dst_typed) {
      using SrcT = std::remove_cv_t<std::decay_t<decltype(src_typed)>>;
      using DstT = std::remove_cv_t<std::decay_t<decltype(dst_typed)>>;

      if constexpr (std::is_same_v<SrcT, DstT>) {
        result_value = src_typed;
      } else {
        result_value = convert<DstT>(std::move(src_typed));
      }
    }, defult_value(type));
  }, std::move(source));

  return result_value;
}

}