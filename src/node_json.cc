
#include "ZCN_node.hh"
#include "ZCN_node_json.hh"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace zcn {

template<typename Tree, typename Func>
static void tree_and_json(Tree &tree, const Func &func)
{
  func(tree.total_uid, "total_uid");
  func(tree.nodes_uid, "nodes_uid");
  func(tree.node_sockets_uid, "node_sockets_uid");
  func(tree.node_sockets_uid, "node_sockets_uid");
  func(tree.node_labels, "node_labels");
  func(tree.links_uid, "links_uid");
  func(tree.links, "links");
  func(tree.values, "values");
}

template<typename T>
using clean_type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;

std::string tree_to_json(const TreePtr &tree)
{
  json obj;

  tree_and_json(*tree, [&](auto &in, std::string name) {
    if constexpr (std::is_same_v<clean_type<decltype(TreePtr::element_type::values)>, clean_type<decltype(in)>>) {
      json values_list = json::object();
      for (const auto &value : in) {
        std::visit([&](const auto &typed_value) {
          values_list[value.first] = typed_value;
        }, value.second);
      }
      obj[name] = std::move(values_list);
    } else {
      obj[name] = in;
    }
  });

  return obj.dump(); 
}

TreePtr tree_from_json(const std::string &json_text)
{
  const json obj = json::parse(json_text);
  
  TreePtr raw_tree = std::make_unique<NodeTree>();

  tree_and_json(*raw_tree, [&](auto &in, std::string name) {
    if constexpr (std::is_same_v<clean_type<decltype(TreePtr::element_type::values)>, clean_type<decltype(in)>>) {

    } else {
      in = obj[name].get<clean_type<decltype(in)>>();
    }
  });
  
  return raw_tree;
}

}