
#include "ZCN_node.hh"

#include <cassert>
#include <unordered_map>

namespace zcn {

class CreationDeclarationContext : public DeclarationContext {
  int &total_uid_;
  std::vector<int> &uids_;
 public:
  CreationDeclarationContext(int &total_uid, std::vector<int> &uids) : total_uid_(total_uid), uids_(uids) {}
  ~CreationDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    const int input_uid = total_uid_++;
    uids_.push_back(input_uid);
  }

  void add_output(const DataType type, std::string name) override
  {
    const int input_uid = total_uid_++;
    uids_.push_back(input_uid);
  }
};

int add_node_to_tree(NodeTree &tree, const std::string node_type)
{
  NodePtr new_node = create_node_by_name(node_type);
  const int node_uid = tree.total_uid++;

  tree.nodes.push_back(std::move(new_node));
  tree.nodes_uid.push_back(node_uid);
  tree.node_labels.push_back(node_type);

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