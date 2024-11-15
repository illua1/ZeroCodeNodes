
#include "ZCN_node.hh"

#include <unordered_map>

namespace zcn {

static std::unordered_map<std::string, std::function<NodePtr()>> &node_types()
{
  static std::unordered_map<std::string, std::function<NodePtr()>> types;
  return types;
}

void register_node_type(std::string type_name, std::function<NodePtr()> construct)
{
  node_types()[std::move(type_name)] = std::move(construct);
}

NodePtr create_node_by_name(const std::string type_name)
{
  return node_types()[type_name]();
}

void register_node_types()
{
  register_node_file_read_node_type();
  register_node_float_input_node_type();
  register_node_text_concatination_node_type();
  register_node_text_input_node_type();
  register_node_value_to_text_node_type();
}

std::vector<std::string> all_node_types()
{
  std::vector<std::string> list;
  for (const auto item : node_types()) {
    list.push_back(item.first);
  }
  return list;
}

}