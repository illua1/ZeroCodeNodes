#pragma once

#include <unordered_map>

#include "ZCN_node.hh"

namespace zcn::nodes {

void draw(NodeTree &tree, std::unordered_map<int, std::pair<float, float>> &r_socket_positions);

}