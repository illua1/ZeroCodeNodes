#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "ZCN_runtime_type.hh"

namespace zcn::nodes {

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
