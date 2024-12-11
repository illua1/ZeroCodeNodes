
#include <iostream>

#include <string>
#include <memory>
#include <cwctype>
#include <algorithm>

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
  func(tree.node_labels, "node_labels");
  func(tree.node_names, "node_names");
  func(tree.links_uid, "links_uid");
  func(tree.links, "links");
  func(tree.values, "values");
}

template<typename T>
using clean_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

std::string tree_to_json(const TreePtr &tree)
{
  json obj;

  tree_and_json(*tree, [&](auto &in, std::string name) {
    if constexpr (std::is_same_v<clean_type<decltype(TreePtr::element_type::values)>, clean_type<decltype(in)>>) {
      json values_list = json::object();
      for (const auto &value : in) {
        std::visit([&](const auto &typed_value) {
          using T = clean_type<decltype(typed_value)>;
          json typed_item;
          typed_item["type"] = data_type_to_name(static_type_to_type<T>());
          typed_item["data"] = typed_value;
          values_list[value.first] = std::move(typed_item);
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

  tree_and_json(*raw_tree, [&](auto &in, const std::string name) {
    if constexpr (std::is_same_v<clean_type<decltype(TreePtr::element_type::values)>, clean_type<decltype(in)>>) {
      for (const auto item : obj.at(name).items()) {
        const json &obj_key = item.key();
        const json &obj_value = item.value();
        RData value;
        std::visit([&](const auto typed_dummy) {
          using T = clean_type<decltype(typed_dummy)>;
          value = obj_value.at("data").get<T>();
        }, defult_value(name_to_data_type(obj_value.at("type").get<std::string>())));
        raw_tree->values[obj_key.get<std::string>()] = std::move(value);
      }
    } else {
      in = obj.at(name).get<clean_type<decltype(in)>>();
    }
  });

  for (const std::string &label : raw_tree->node_labels) {
    raw_tree->nodes.push_back(create_node_by_name(label));
  }
  
  return raw_tree;
}

}