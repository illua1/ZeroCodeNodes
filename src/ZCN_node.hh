#pragma once

#include <string>
#include <vector>

#include "ZCN_runtime_type.hh"

namespace zcn {

enum class DataType : int8_t {
  Text,
  Float,
  Int,
};

template<typename T>
inline DataType static_type_to_type();

class NodeDeclaration {
 protected:
  std::vector<std::pair<DataType, std::string>> inputs_;
  std::vector<std::pair<DataType, std::string>> outputs_;
  std::string ui_name_;
  
 public:
  void add_input(const DataType type, std::string name);

  template<typename T>
  void add_input(std::string name);

  void add_output(const DataType type, std::string name);

  template<typename T>
  void add_output(std::string name);

  void set_name(std::string name);
};

class NodeContext {
  
};

class Node {
 public:
  virtual ~Node() = 0;
  virtual void declaration(NodeDeclaration &decl) const = 0;
  virtual void execute() const = 0;
};

template<typename T>
inline DataType static_type_to_type()
{
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

void NodeDeclaration::add_input(const DataType type, std::string name)
{
  this->inputs_.push_back(std::pair{type, std::move(name)});
}

template<typename T>
void NodeDeclaration::add_input(std::string name)
{
  this->add_input(static_type_to_type<T>(), std::move(name));
}

void NodeDeclaration::add_output(const DataType type, std::string name)
{
  this->outputs_.push_back(std::pair{type, std::move(name)});
}

template<typename T>
void NodeDeclaration::add_output(std::string name)
{
  this->add_output(static_type_to_type<T>(), std::move(name));
}

void NodeDeclaration::set_name(std::string name)
{
  this->ui_name_ = std::move(name);
}

}