#pragma once

#include <string>

#include "ZCN_node.hh"

namespace zcn {

std::string tree_to_json(const TreePtr &tree);

TreePtr tree_from_json(const std::string &json_text);

}