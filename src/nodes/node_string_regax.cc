
#include "../ZCN_node.hh"

#include <boost/regex.hpp> 

#include <string>
#include <locale>
#include <memory>
#include <cwctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <string>
#include <regex>
#include <sstream>

namespace zcn::node::string_regax {

class StringRegax : public Node {
 public:
  StringRegax() = default;

  ~StringRegax() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Текст");
    decl.add_input<std::string>("Паттерн");
    decl.add_input<std::string>("Замена");
    decl.add_output<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string text = context.get_input<std::string>("Текст");
    const std::string pattern = context.get_input<std::string>("Паттерн");
    const std::string new_part = context.get_input<std::string>("Замена");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring textw = converter.from_bytes(text);
    std::wstring patterw = converter.from_bytes(pattern);
    std::wstring new_partw = converter.from_bytes(new_part);

    std::basic_stringstream<wchar_t> stream;
    try {
      context.set_output<std::string>("Текст", converter.to_bytes(boost::regex_replace(textw, boost::wregex(patterw), new_partw)));
      return;
    }
    catch (const boost::regex_error &/*e*/) {}

    context.set_output<std::string>("Текст", "");
  }
};

}

namespace zcn {

void register_node_string_regax_node_type()
{
  register_node_type("Регулярные выражения", []() -> NodePtr {
    return std::make_unique<node::string_regax::StringRegax>();
  });
}

}