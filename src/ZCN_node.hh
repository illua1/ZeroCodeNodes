#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "ZCN_runtime_type.hh"

namespace zcn {

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

  virtual void add_input(DataType type, std::string name) = 0;
  virtual void add_output(DataType type, std::string name) = 0;
  virtual int add_selector(const std::string name, const int selected, const std::vector<std::string> &options);
  virtual std::string add_tree_selector(const std::string name, std::string tree_name);
  virtual void add_data(DataType type, std::string name) = 0;
};

class BaseProvider {
 public:
  virtual ~BaseProvider() = default;
};

class ExecutionContext {
 public:
  virtual ~ExecutionContext() = default;

  virtual std::vector<BaseProvider *> &context_providers() = 0;

  template<typename T>
  T get_input(const std::string name);

  template<typename T>
  T get_data(const std::string name);

  template<typename T>
  void set_output(const std::string name, T value);

  template<typename T>
  T context_provider();

  virtual RData get_input(DataType type, const std::string name) = 0;
  virtual RData get_data(DataType type, const std::string name) = 0;
  virtual void set_output(const std::string name, RData value) = 0;
};

class DataVisitor {
 public:
  virtual ~DataVisitor() = default;
  virtual void visit_text(std::string &data, const std::string &name) = 0;
  virtual void visit_int(int &data, const std::string &name) = 0;
  virtual void visit_float(float &data, const std::string &name) = 0;
};

class Node {
 public:
  virtual ~Node() = default;
  virtual void declare(DeclarationContext &decl) const = 0;
  virtual void execute(ExecutionContext &context) const = 0;
  virtual void visit_data(DataVisitor &visitor) const;
};

class InterfaceNode : public Node {
 public:
  virtual DataType data_type() const = 0;
};

class InterfaceInputNode : public InterfaceNode {};

class InterfaceOutputNode : public InterfaceNode {};

using NodePtr = std::unique_ptr<Node>;

struct NodeTree {
  int total_uid = 0;
  std::vector<int> nodes_uid;
  std::vector<std::vector<int>> node_sockets_uid;
  std::vector<std::string> node_labels;
  std::vector<std::string> node_names;
  std::vector<NodePtr> nodes;
  
  std::vector<int> links_uid;
  std::vector<std::pair<int, int>> links;

  std::unordered_map<std::string, RData> values;
};

using TreePtr = std::shared_ptr<NodeTree>;

TreePtr new_tree();

void validate_links(NodeTree &tree);

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
inline void DeclarationContext::add_data(std::string name)
{
  this->add_data(static_type_to_type<T>(), std::move(name));
}

template<typename T>
inline void DeclarationContext::add_input(std::string name)
{
  this->add_input(static_type_to_type<T>(), std::move(name));
}

template<typename T>
inline void DeclarationContext::add_output(std::string name)
{
  this->add_output(static_type_to_type<T>(), std::move(name));
}


inline int DeclarationContext::add_selector(const std::string /*name*/, const int selected, const std::vector<std::string> &/*options*/)
{
  return selected;
}

inline std::string DeclarationContext::add_tree_selector(const std::string /*name*/, std::string tree_name)
{
  return tree_name;
}

template<typename T>
inline T ExecutionContext::get_input(std::string name)
{
  return *std::get_if<T>(&this->get_input(static_type_to_type<T>(), name));
}

template<typename T>
inline T ExecutionContext::get_data(std::string name)
{
  return *std::get_if<T>(&this->get_data(static_type_to_type<T>(), name));
}

template<typename T>
inline void ExecutionContext::set_output(std::string name, T value)
{
  this->set_output(name, RData(std::move(value)));
}


  template<typename T>
inline T ExecutionContext::context_provider()
{
  std::vector<BaseProvider *> &list = this->context_providers();
  const auto result = std::find_if(list.begin(), list.end(), [&](BaseProvider *base_ptr) {
    return dynamic_cast<T>(base_ptr) != nullptr;
  });
  if (result == list.end()) {
    return nullptr;
  }
  return dynamic_cast<T>(*result);
}

}

namespace zcn {

void register_node_type(std::string type_name, std::function<NodePtr()> construct);

NodePtr create_node_by_name(const std::string type_name);

std::vector<std::string> all_node_types();

void register_node_file_read_node_type();
void register_node_file_write_node_type();
void register_node_foreach_file_node_type();
void register_node_context_file_input_node_type();

void register_node_int_input_node_type();
void register_node_float_input_node_type();
void register_node_string_input_node_type();

void register_node_math_node_type();
void register_node_compare_value_node_type();

void register_node_string_up_register_node_type();
void register_node_string_down_register_node_type();
void register_node_string_regax_node_type();
void register_node_string_replace_node_type();
void register_node_string_slice_node_type();
void register_node_string_find_node_type();
void register_node_string_length_node_type();
void register_node_string_line_break_node_type();
void register_node_string_concatination_node_type();
void register_node_string_compare_node_type();

void register_node_value_to_text_node_type();

void register_node_execute_other_node_type();
void register_node_loop_node_type();
void register_node_switch_node_type();

void register_node_input_node_type();
void register_node_output_node_type();

void register_node_question_node_type();
void register_node_question_button_node_type();

void register_node_end_node_type();


void register_node_types();

}

namespace zcn {

void set_trees_context(const std::unordered_map<std::string, TreePtr> *forest);
const TreePtr find_tree(const std::string &name);

}

namespace zcn {

inline void Node::visit_data(DataVisitor &/*visitor*/) const
{
  
}

}

namespace zcn {

std::string node_socket_to_path(int node_uid, int socket_uid);
std::string node_value_to_path(int node_uid, const std::string &name);

}