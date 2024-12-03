
#include "../ZCN_node.hh"

#include <string>
#include <locale>
#include <memory>
#include <cwctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <string>

namespace zcn::node::string_replace {

class EndNode : public Node {
 public:
  EndNode() = default;

  ~EndNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Текст");
    decl.add_input<std::string>("До");
    decl.add_input<std::string>("После");
    decl.add_output<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string text = context.get_input<std::string>("Текст");
    const std::string old_part = context.get_input<std::string>("До");
    const std::string new_part = context.get_input<std::string>("После");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const std::wstring textw = converter.from_bytes(text);
    const std::wstring old_partw = converter.from_bytes(old_part);
    const std::wstring new_partw = converter.from_bytes(new_part);

    if (old_partw.size() == 0) {
      context.set_output<std::string>("Текст", text);
      return;
    }

    std::wstring replaced;

    int prev_position = 0;
    int new_position = textw.find(old_partw);
    while (new_position != std::wstring::npos) {
      replaced += textw.substr(prev_position, new_position - prev_position);
      replaced += new_partw;
      prev_position = new_position + old_partw.size();
      new_position = textw.find(old_partw, prev_position);
    }
    replaced += textw.substr(prev_position, new_position - prev_position);

    context.set_output<std::string>("Текст", converter.to_bytes(replaced));
  }
};

}

namespace zcn {

void register_node_string_replace_node_type()
{
  register_node_type("Заменить", []() -> NodePtr {
    return std::make_unique<node::string_replace::EndNode>();
  });
}

}