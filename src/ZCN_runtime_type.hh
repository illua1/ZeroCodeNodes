#pragma once

#include <string>
#include <variant>

namespace zcn {

using RData = std::variant<bool, int, float, std::string>;

}