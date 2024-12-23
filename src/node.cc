
#include "ZCN_node.hh"

#include <cassert>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

namespace zcn {

class CreationDeclarationContext : public DeclarationContext {
  int &total_uid_;
  int index = 0;
  std::vector<int> &uids_;
 public:
  CreationDeclarationContext(int &total_uid, std::vector<int> &uids) : total_uid_(total_uid), uids_(uids) {}
  ~CreationDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    const int input_uid = index++;
    uids_.push_back(input_uid + 150);
  }

  void add_output(const DataType type, std::string name) override
  {
    const int input_uid = index++;
    uids_.push_back(input_uid);
  }

  void add_data(const DataType /*type*/, std::string /*name*/) override {}
};

static std::string new_node_name(const std::vector<std::string> &names, std::string start) {
  if (start == "") {
    return new_node_name(names, "AAAAAAAAAA");
  }
  if (std::find(names.begin(), names.end(), start) != names.end()) {
    return std::move(start);
  }
  const std::string separator = (start.back() == '.' ? "" : ".");
  int iter = 1;
  while (std::find(names.begin(), names.end(), start + separator + std::to_string(iter)) != names.end()) {
    iter++;
  }
  return start + separator + std::to_string(iter);
};

int add_node_to_tree(NodeTree &tree, const std::string node_type)
{
  NodePtr new_node = create_node_by_name(node_type);
  const int node_uid = tree.total_uid;
  tree.total_uid += 300;

  tree.nodes.push_back(std::move(new_node));
  tree.nodes_uid.push_back(node_uid);
  tree.node_labels.push_back(node_type);
  tree.node_names.push_back(new_node_name(tree.node_names, node_type));

  std::vector<int> socket_uid;
  CreationDeclarationContext node_decl(tree.total_uid, socket_uid);
  tree.nodes.back()->declare(node_decl);
  tree.node_sockets_uid.push_back(std::move(socket_uid));

  return tree.nodes_uid.size() - 1;
}

void drop_node_from_tree(NodeTree &tree, const int node_index)
{
  tree.nodes.erase(tree.nodes.begin() + node_index);
  tree.nodes_uid.erase(tree.nodes_uid.begin() + node_index);
  tree.node_labels.erase(tree.node_labels.begin() + node_index);
  tree.node_names.erase(tree.node_names.begin() + node_index);
}

int add_link_from_tree(NodeTree &tree, std::pair<int, int> link)
{
  const int link_uid = tree.total_uid++;
  tree.links_uid.push_back(link_uid);
  tree.links.push_back(link);
  return link_uid;
}

void drop_link_from_tree(NodeTree &tree, int link_uid)
{
  const int link_i = std::distance(tree.links_uid.begin(), std::find(tree.links_uid.begin(), tree.links_uid.end(), link_uid));
  assert(link_i < tree.links_uid.size());
  tree.links.erase(tree.links.begin() + link_i);
  tree.links_uid.erase(tree.links_uid.begin() + link_i);
}

TreePtr new_tree()
{
  return std::make_shared<NodeTree>();
}

}

namespace zcn {

void add_nodes_for_path(NodeTree &tree, const std::string path)
{
  // add_node_to_tree(tree, "")
}

}

namespace zcn {

static std::unordered_map<std::string, std::function<NodePtr()>> &node_types()
{
  static std::unordered_map<std::string, std::function<NodePtr()>> types;
  return types;
}

void register_node_type(std::string type_name, std::function<NodePtr()> construct)
{
  assert(node_types().find(type_name) == node_types().end());
  node_types()[std::move(type_name)] = std::move(construct);
}

NodePtr create_node_by_name(const std::string type_name)
{
  return node_types().at(type_name)();
}

class ExistSocketsDeclarationContext : public DeclarationContext {
  NodeTree &tree_;
  const int node_uid_;
  std::unordered_set<int> &exist_sockets_;
  int index_ = 0;
 public:
  ExistSocketsDeclarationContext(NodeTree &tree, const int node_uid, std::unordered_set<int> &exist_sockets) : tree_(tree), node_uid_(node_uid), exist_sockets_(exist_sockets) {}

  ~ExistSocketsDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    index_++;
    exist_sockets_.insert(node_uid_ + 150 + index_);
  }

  void add_output(const DataType type, std::string name) override
  {
    index_++;
    exist_sockets_.insert(node_uid_ + index_);
  }

  void add_data(const DataType /*type*/, std::string /*name*/) override {}
};

void validate_links(NodeTree &tree)
{
  std::unordered_set<int> exist_sockets;

  for (int node_index = 0; node_index < tree.nodes.size(); node_index++) {
    NodePtr &node = tree.nodes[node_index];
    ExistSocketsDeclarationContext declaration(tree, tree.nodes_uid[node_index], exist_sockets);
    node->declare(declaration);
  }

  std::vector<bool> links_to_delete;
  links_to_delete.resize(tree.links.size());
  std::transform(tree.links.begin(), tree.links.end(), links_to_delete.begin(), [&](const std::pair<int, int> link) {
    if (exist_sockets.find(link.first) == exist_sockets.end()) {
      return false;
    }
    if (exist_sockets.find(link.second) == exist_sockets.end()) {
      return false;
    }
    return true;
  });

  tree.links.erase(std::remove_if(tree.links.begin(), tree.links.end(), [&](auto &item) {
    return !links_to_delete[std::distance(&tree.links[0], &item)];
  }), tree.links.end());

  tree.links_uid.erase(std::remove_if(tree.links_uid.begin(), tree.links_uid.end(), [&](auto &item) {
    return !links_to_delete[std::distance(&tree.links_uid[0], &item)];
  }), tree.links_uid.end());
}

void register_node_types()
{
  register_node_file_read_node_type();
  register_node_file_write_node_type();
  register_node_foreach_file_node_type();

  register_node_int_input_node_type();
  register_node_float_input_node_type();
  register_node_string_input_node_type();
  register_node_context_file_input_node_type();

  register_node_math_node_type();
  register_node_compare_value_node_type();

  register_node_string_up_register_node_type();
  register_node_string_down_register_node_type();
  register_node_string_regax_node_type();
  register_node_string_replace_node_type();
  register_node_string_slice_node_type();
  register_node_string_find_node_type();
  register_node_string_length_node_type();
  register_node_string_line_break_node_type();
  register_node_string_concatination_node_type();
  register_node_string_compare_node_type();

  register_node_value_to_text_node_type();

  register_node_execute_other_node_type();
  register_node_loop_node_type();
  register_node_switch_node_type();

  register_node_input_node_type();
  register_node_output_node_type();

  register_node_question_node_type();
  register_node_question_button_node_type();
  
  register_node_end_node_type();
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

namespace zcn {

static const std::unordered_map<std::string, TreePtr> *&context_forest_map_ptr()
{
  static const std::unordered_map<std::string, TreePtr> *ptr = nullptr;
  return ptr;
}

void set_trees_context(const std::unordered_map<std::string, TreePtr> *forest)
{
  context_forest_map_ptr() = forest;
}

const TreePtr find_tree(const std::string &name)
{
  if (name == "") {
    return nullptr;
  }
  if (context_forest_map_ptr() != nullptr) {
    if (context_forest_map_ptr()->find(name) == context_forest_map_ptr()->end()) {
      return nullptr;
    }
    return context_forest_map_ptr()->at(name);
  }
  return nullptr;
}

}

namespace zcn {

std::string node_socket_to_path(const int node_uid, const int socket_uid)
{
  return "nodes[" + std::to_string(node_uid) + "].[" + std::to_string(socket_uid) + "]";
}

std::string node_value_to_path(const int node_uid, const std::string &name)
{
  return "nodes[" + std::to_string(node_uid) + "]" + "." + name;
}

}