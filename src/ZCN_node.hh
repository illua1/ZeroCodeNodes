#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "ZCN_runtime_type.hh"

namespace zcn {

enum class DataType : int8_t {
  Text,
  Float,
  Int,
};

template<typename T>
inline DataType static_type_to_type();

class DeclarationContext {
 public:
  virtual ~DeclarationContext() = default;
 
  template<typename T>
  void add_input(std::string name);
  template<typename T>
  void add_output(std::string name);
  template<typename T>
  void add_data(std::string name);

  virtual void add_input(const DataType type, std::string name) = 0;
  virtual void add_output(const DataType type, std::string name) = 0;
  virtual void add_data(const DataType type, std::string name) = 0;
};

class ExecutionContext {
 public:
  virtual ~ExecutionContext() = default;

  template<typename T>
  T get_input(const std::string name);

  template<typename T>
  void set_output(const std::string name, T value);

  virtual RData get_input(const std::string name) = 0;
  virtual void set_output(const std::string name, RData value) = 0;
};

class Node {
 public:
  virtual ~Node() = default;
  virtual void declare(DeclarationContext &decl) const = 0;
  virtual void execute(ExecutionContext &context) const = 0;
};

using NodePtr = std::unique_ptr<Node>;

struct NodeTree {
  int total_uid = 0;
  std::vector<int> nodes_uid;
  std::vector<std::vector<int>> node_sockets_uid;
  std::vector<std::string> node_labels;
  std::vector<NodePtr> nodes;
  
  std::vector<int> links_uid;
  std::vector<std::pair<int, int>> links;

  std::unordered_map<std::string, RData> values;
};

using TreePtr = std::unique_ptr<NodeTree>;

TreePtr new_tree();

}

namespace zcn {

int add_node_to_tree(NodeTree &tree, std::string node_type);
void drop_node_from_tree(NodeTree &tree, const int node_index);

int add_link_from_tree(NodeTree &tree, std::pair<int, int> link);
void drop_link_from_tree(NodeTree &tree, int link_uid);

}

namespace zcn {

void add_nodes_for_path(NodeTree &tree, std::string path);

}

namespace zcn {

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

template<typename T>
void DeclarationContext::add_data(std::string name)
{
  this->add_data(static_type_to_type<T>(), std::move(name));
}

template<typename T>
void DeclarationContext::add_input(std::string name)
{
  this->add_input(static_type_to_type<T>(), std::move(name));
}

template<typename T>
void DeclarationContext::add_output(std::string name)
{
  this->add_output(static_type_to_type<T>(), std::move(name));
}

template<typename T>
T ExecutionContext::get_input(std::string name)
{
  return *std::get_if<T>(&this->get_input(name));
}

template<typename T>
void ExecutionContext::set_output(std::string name, T value)
{
  this->set_output(name, RData(std::move(value)));
}

}

namespace zcn {

void register_node_type(std::string type_name, std::function<NodePtr()> construct);

NodePtr create_node_by_name(const std::string type_name);

std::vector<std::string> all_node_types();

void register_node_file_read_node_type();
void register_node_float_input_node_type();
void register_node_text_concatination_node_type();
void register_node_text_input_node_type();
void register_node_value_to_text_node_type();

void register_node_types();

}