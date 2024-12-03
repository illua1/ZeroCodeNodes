
#include "../ZCN_node.hh"

#include <string>
#include <locale>
#include <memory>
#include <cwctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <string>

namespace zcn::node::string_down {

class AllDown : public Node {
 public:
  AllDown() = default;

  ~AllDown() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Текст");
    decl.add_output<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    std::string text = context.get_input<std::string>("Текст");
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring testw = converter.from_bytes(text);
    std::transform(testw.begin(), testw.end(), testw.begin(), [](const std::wint_t c){ return std::tolower(c, std::locale("")); });
    context.set_output<std::string>("Текст", converter.to_bytes(testw));
  }
};

}

namespace zcn {

void register_node_string_down_register_node_type()
{
  register_node_type("Маленькие буквы", []() -> NodePtr {
    return std::make_unique<node::string_down::AllDown>();
  });
}

}