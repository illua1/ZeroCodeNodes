
#include "../ZCN_node.hh"

#include <string>
#include <locale>
#include <memory>
#include <cwctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <string>

namespace zcn::node::string_slice {

class StringSlice : public Node {
 public:
  StringSlice() = default;

  ~StringSlice() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Текст");
    decl.add_input<int>("Начало");
    decl.add_input<int>("Длинна");
    decl.add_output<std::string>("Часть");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string text = context.get_input<std::string>("Текст");
    const int start = context.get_input<int>("Начало");
    const int size = context.get_input<int>("Длинна");

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring textw = converter.from_bytes(text);

    const int part_start = std::max(0, std::min<int>(textw.size() - 1, start));
    const int part_size = std::max(0, std::min<int>(textw.size() - part_start, size));

    context.set_output<std::string>("Часть", converter.to_bytes(textw.substr(part_start, part_size)));
  }
};

}

namespace zcn {

void register_node_string_slice_node_type()
{
  register_node_type("Вырезать строку", []() -> NodePtr {
    return std::make_unique<node::string_slice::StringSlice>();
  });
}

}