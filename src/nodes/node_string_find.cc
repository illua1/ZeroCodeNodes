
#include "../ZCN_node.hh"

#include <iostream>

#include <string>
#include <locale>
#include <memory>
#include <cwctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <string>

namespace zcn::node::string_find {

class StringFindToken : public Node {
 public:
  StringFindToken() = default;

  ~StringFindToken() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Текст");
    decl.add_input<std::string>("Токен");
    decl.add_output<int>("Начало");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string text = context.get_input<std::string>("Текст");
    const std::string token = context.get_input<std::string>("Токен");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring textw = converter.from_bytes(text);
    std::wstring tokenw = converter.from_bytes(token);
    
    const int index = textw.find(tokenw);
    context.set_output<int>("Начало", index == -1 ? 0 : index);
  }
};

}

namespace zcn {

void register_node_string_find_node_type()
{
  register_node_type("Найти в строке", []() -> NodePtr {
    return std::make_unique<node::string_find::StringFindToken>();
  });
}

}