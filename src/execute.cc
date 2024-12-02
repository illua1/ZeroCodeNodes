#pragma once

#include <iostream>

#include <string>
#include <algorithm>
#include <vector>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "ZCN_execute.hh"
#include "ZCN_node.hh"
#include "ZCN_runtime_type.hh"

namespace zcn {

static void execute_from(const std::vector<std::vector<int>> &node_to_input_nodes,
                         const int start_node,
                         std::vector<bool> &visited_nodes,
                         std::vector<int> &r_nodes_order)
{
  visited_nodes[start_node] = true;
  
  for (const int input_node_index : node_to_input_nodes[start_node]) {
    if (input_node_index == -1) {
      continue;
    }
    if (visited_nodes[input_node_index]) {
      continue;
    }
    execute_from(node_to_input_nodes, input_node_index, visited_nodes, r_nodes_order);
  }
  
  r_nodes_order.push_back(start_node);
}

class BaseExecutionContext : public ExecutionContext {
  const int node_uid_;
  
  const std::unordered_map<std::string, RData> &tree_values_;
  const std::unordered_map<int, int> &input_uid_to_output_uid_;
  const std::unordered_map<int, int> &node_index_from_output_uid_;

  const std::vector<int> &node_uids_;

  std::unordered_map<std::string, RData> &execution_values_;

  std::unordered_map<std::string, int> input_uids_for_name_;
  std::unordered_map<std::string, int> output_uids_for_name_;
  
   class ExecutionDeclarationContext : public DeclarationContext {
     int input_uid = 0;
     std::unordered_map<std::string, int> &input_uids_for_name_;
     std::unordered_map<std::string, int> &output_uids_for_name_;
     const std::vector<int> &socket_uids_;

    public:
     ExecutionDeclarationContext(std::unordered_map<std::string, int> &input_uids_for_name,
                                std::unordered_map<std::string, int> &output_uids_for_name,
                                const std::vector<int> &socket_uids) : input_uids_for_name_(input_uids_for_name),
                                                                       output_uids_for_name_(output_uids_for_name),
                                                                       socket_uids_(socket_uids) {}
     ~ExecutionDeclarationContext() override = default;

     void add_input(const DataType /*type*/, std::string name) override
     {
       input_uids_for_name_[name] = socket_uids_[input_uid++];
     }

     void add_output(const DataType /*type*/, std::string name) override
     {
       output_uids_for_name_[name] = socket_uids_[input_uid++];
     }

     void add_data(const DataType /*type*/, std::string /*name*/) override {}
   };

 public:
  BaseExecutionContext(const int node_uid,
                       const std::unordered_map<std::string, RData> &tree_values,
                       const std::unordered_map<int, int> &input_uid_to_output_uid,
                       const std::unordered_map<int, int> &node_index_from_output_uid,
                       const std::vector<int> &node_uids,
                       const std::vector<int> &node_socket_uids,
                       std::unordered_map<std::string, RData> &execution_values,
                       const NodePtr &node)
                       : node_uid_(node_uid),
                         tree_values_(tree_values),
                         input_uid_to_output_uid_(input_uid_to_output_uid),
                         node_index_from_output_uid_(node_index_from_output_uid),
                         node_uids_(node_uids),
                         execution_values_(execution_values)
  {
    ExecutionDeclarationContext topology(input_uids_for_name_, output_uids_for_name_, node_socket_uids);
    node->declare(topology);
  }
 
  ~BaseExecutionContext() override = default;

  RData get_input(const std::string name) override
  {
    const int socket_uid = input_uids_for_name_[name];
    if (input_uid_to_output_uid_.find(socket_uid) == input_uid_to_output_uid_.end()) {
      const std::string socket_path = node_socket_to_path(node_uid_, socket_uid);
      return tree_values_.at(socket_path);
    }
    const int output_uid = input_uid_to_output_uid_.at(socket_uid);
    const int source_node_index = node_index_from_output_uid_.at(output_uid);
    const int source_node_uid = node_uids_[source_node_index];
    const std::string socket_path = node_socket_to_path(source_node_uid, output_uid);
    return execution_values_.at(socket_path);
  }

  RData get_data(const std::string name) override
  {
    const std::string node_path = node_value_to_path(node_uid_, name);
    return tree_values_.at(node_path);
  }

  void set_output(const std::string name, RData value) override
  {
    const int socket_uid = output_uids_for_name_.at(name);
    const std::string socket_path = node_socket_to_path(node_uid_, socket_uid);
    execution_values_[socket_path] = std::move(value);
  }

};

class TopologyDeclarationContext : public DeclarationContext {
  int input_uid = 0;
  std::vector<int> &input_uids_;
  std::vector<int> &outout_uids_;
  const std::vector<int> &socket_uids_;

 public:
  TopologyDeclarationContext(std::vector<int> &input_uids, std::vector<int> &output_uids, const std::vector<int> &socket_uids) :
    input_uids_(input_uids), outout_uids_(output_uids), socket_uids_(socket_uids) {}
  ~TopologyDeclarationContext() override = default;

  void add_input(const DataType /*type*/, std::string /*name*/) override
  {
    input_uids_.push_back(socket_uids_[input_uid++]);
  }

  void add_output(const DataType /*type*/, std::string /*name*/) override
  {
    outout_uids_.push_back(socket_uids_[input_uid++]);
  }

  void add_data(const DataType /*type*/, std::string /*name*/) override {}
};

void execute(const TreePtr &tree, ExecuteLog &log, SideEffectReciver &reciver)
{
  std::vector<std::vector<int>> nodes_inputs;
  std::vector<std::vector<int>> nodes_outputs;
  nodes_inputs.resize(tree->nodes_uid.size());
  nodes_outputs.resize(tree->nodes_uid.size());

  std::unordered_map<int, int> node_index_from_output_uid;

  for (int node_index = 0; node_index < tree->nodes_uid.size(); node_index++) {
    TopologyDeclarationContext topology(nodes_inputs[node_index], nodes_outputs[node_index], tree->node_sockets_uid[node_index]);
    tree->nodes[node_index]->declare(topology);
    
    for (const int output_uid : nodes_outputs[node_index]) {
      node_index_from_output_uid[output_uid] = node_index;
    }
  }

  std::vector<int> output_nodes;
  for (int node_index = 0; node_index < tree->nodes_uid.size(); node_index++) {
    if (nodes_outputs[node_index].empty()) {
      output_nodes.push_back(node_index);
    }
  }

  std::unordered_map<int, int> input_uid_to_output_uid;
  for (const auto [output, input] : tree->links) {
    input_uid_to_output_uid[input] = output;
  }

  std::vector<std::vector<int>> node_to_input_nodes;
  node_to_input_nodes.resize(tree->nodes_uid.size());
  for (int node_index = 0; node_index < tree->nodes_uid.size(); node_index++) {
    node_to_input_nodes[node_index].resize(nodes_inputs[node_index].size());
    std::transform(nodes_inputs[node_index].begin(), nodes_inputs[node_index].end(), node_to_input_nodes[node_index].begin(), [&](const int input_socket_uid) {
      if (input_uid_to_output_uid.find(input_socket_uid) == input_uid_to_output_uid.end()) {
        return -1;
      }
      const int output_uid = input_uid_to_output_uid[input_socket_uid];
      if (node_index_from_output_uid.find(output_uid) == node_index_from_output_uid.end()) {
        return -1;
      }
      return node_index_from_output_uid[output_uid];
    });
  }

  std::vector<int> ordered_node_indices;
  {
    std::vector<bool> visited_nodes;
    visited_nodes.resize(tree->nodes_uid.size());
    std::generate(visited_nodes.begin(), visited_nodes.end(), [&]() { return false; });
    for (const int output_index : output_nodes) {
      execute_from(node_to_input_nodes, output_index, visited_nodes, ordered_node_indices);
    }
  }

  std::unordered_map<std::string, RData> socket_values;
  for (const int node_index : ordered_node_indices) {
    const int node_uid = tree->nodes_uid[node_index];

    BaseExecutionContext context(node_uid,
                                 tree->values,
                                 input_uid_to_output_uid,
                                 node_index_from_output_uid,
                                 tree->nodes_uid,
                                 tree->node_sockets_uid[node_index],
                                 socket_values,
                                 tree->nodes[node_index]);
    tree->nodes[node_index]->execute(context);
  }

std::cout << "\n";
  for (const auto item : socket_values) {
    std::cout << item.first << ", ";
    std::visit([&](auto value) {
      std::cout << value << ";\n";
    }, item.second);
  }

  log.socket_value = std::move(socket_values);
}

RData FileSystemProvider::get_from_path(std::string path)
{
  return int(1);
}

void FileSystemProvider::set_for_path(std::string path, RData data)
{
  
}

RData VirtualFileSystemProvider::get_from_path(std::string path)
{
  return int(1);
}

void VirtualFileSystemProvider::set_for_path(std::string path, RData data)
{
  
}

}