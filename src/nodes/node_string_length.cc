
#include "../ZCN_node.hh"

#include <string>
#include <locale>
#include <memory>
#include <cwctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <string>

namespace zcn::node::string_length {

class StringLength : public Node {
 public:
  StringLength() = default;

  ~StringLength() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Текст");
    decl.add_output<int>("Длинна");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string text = context.get_input<std::string>("Текст");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring textw = converter.from_bytes(text);

    context.set_output<int>("Длинна", textw.size());
  }
};

}

namespace zcn {

void register_node_string_length_node_type()
{
  register_node_type("Длинна текста", []() -> NodePtr {
    return std::make_unique<node::string_length::StringLength>();
  });
}

}