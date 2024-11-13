#pragma once

#include <string>

namespace zcn {

class Type {
 public:
  
};

template<typename T>
const Type &get_type_of();

extern template const Type &get_type_of<bool>();
extern template const Type &get_type_of<int>();
extern template const Type &get_type_of<float>();
extern template const Type &get_type_of<std::string>();

}