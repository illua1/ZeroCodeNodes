#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "ZCN_node.hh"
#include "ZCN_execute.hh"

namespace zcn::nodes {

void draw(NodeTree &tree, const std::vector<std::string> &trees_names, std::unordered_map<int, std::pair<float, float>> &r_socket_positions);

void draw_log_overlay(const NodeTree &tree, const ExecuteLog &log, const std::unordered_map<int, std::pair<float, float>> &socket_positions);

}