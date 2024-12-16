#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "ZCN_node.hh"
#include "ZCN_runtime_type.hh"

namespace zcn {

class GUIExecutionProvider : public BaseProvider {
 public:
  GUIExecutionProvider() = default;
  ~GUIExecutionProvider() override = default;

  bool button_try(const std::string name) const;
};

}