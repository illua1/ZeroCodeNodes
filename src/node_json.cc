
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
  func(tree.nodes, "nodes");
}

class JSONSerealizeVisitor : public DataVisitor {
 public:
  json &dst;

  JSONSerealizeVisitor(json &dst_in) : dst(dst_in) {}

  ~JSONSerealizeVisitor() override = default;

  void visit_text(std::string &data, const std::string &name) override
  {
    dst[name] = data;
  }

  void visit_int(int &data, const std::string &name) override
  {
    dst[name] = data;
  }

  void visit_float(float &data, const std::string &name) override
  {
    dst[name] = data;
  }
};

template<typename T>
using clean_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

class JSONDeserealizeVisitor : public DataVisitor {
 public:
  const json &src;

  JSONDeserealizeVisitor(const json &src_in) : src(src_in) {}

  ~JSONDeserealizeVisitor() override = default;

  void visit_text(std::string &data, const std::string &name) override
  {
    data = src.at(name).get<clean_type<decltype(data)>>();
  }

  void visit_int(int &data, const std::string &name) override
  {
    data = src.at(name).get<clean_type<decltype(data)>>();
  }

  void visit_float(float &data, const std::string &name) override
  {
    data = src.at(name).get<clean_type<decltype(data)>>();
  }
};

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
    } else if constexpr (std::is_same_v<clean_type<decltype(TreePtr::element_type::nodes)>, clean_type<decltype(in)>>) {
      json values_list = json::array();
      for (const auto &value : in) {
        json value_item;
        JSONSerealizeVisitor serealizer(value_item);
        value->visit_data(serealizer);
        values_list.push_back(value_item);
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

  tree_and_json(*raw_tree, [&](auto &out, const std::string name) {
    if constexpr (std::is_same_v<clean_type<decltype(TreePtr::element_type::values)>, clean_type<decltype(out)>>) {
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
    } else if constexpr (std::is_same_v<clean_type<decltype(TreePtr::element_type::nodes)>, clean_type<decltype(out)>>) {
      json values_list = json::object();
      int index = 0;
      for (const auto &value : obj.at(name)) {
        raw_tree->nodes.push_back(create_node_by_name(raw_tree->node_labels[index++]));
        JSONDeserealizeVisitor deserealizer(value);
        raw_tree->nodes.back()->visit_data(deserealizer);
      }
    } else{
      out = obj.at(name).get<clean_type<decltype(out)>>();
    }
  });

  return raw_tree;
}

}