
#include "../ZCN_node.hh"

#include <string>
#include <locale>
#include <memory>
#include <cwctype>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <string>

namespace zcn::node::string_up {

const static std::vector<std::string> options = {"Все", "Первые", "Наоборот"};

const static std::wstring space_sumbol = []() {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(" ");
}();

class UpperCase : public Node {
  mutable int op_index = 0;
 public:
  UpperCase() = default;

  ~UpperCase() override = default;

  void declare(DeclarationContext &decl) const override
  {
    op_index = decl.add_selector("Операция", op_index, options);

    decl.add_input<std::string>("Текст");

    decl.add_output<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    std::string text = context.get_input<std::string>("Текст");
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring testw = converter.from_bytes(text);
    if (options[op_index] == "Все") {
      std::transform(testw.begin(), testw.end(), testw.begin(), [](const std::wint_t c) { return std::toupper(c, std::locale("")); });
    } else if (options[op_index] == "Первые") {
      bool space = false;
      std::transform(testw.begin(), testw.end(), testw.begin(), [&](const std::wint_t c) {
        if (c == space_sumbol[0]) {
          space = true;
          return c;
        }
        if (space) {
          space = false;
          return std::toupper(c, std::locale(""));
        }
        return std::tolower(c, std::locale(""));
      });
    } else if (options[op_index] == "Наоборот") {
      std::transform(testw.begin(), testw.end(), testw.begin(), [](const std::wint_t c) {
        if (c == std::tolower(c, std::locale(""))) {
          return std::toupper(c, std::locale(""));
        }
        return std::tolower(c, std::locale(""));
      });
    }
    context.set_output<std::string>("Текст", converter.to_bytes(testw));
  }

  void visit_data(DataVisitor &visitor) const override
  {
    visitor.visit_int(op_index, "op_index");
  }
};

}

namespace zcn {

void register_node_string_up_register_node_type()
{
  register_node_type("Большие буквы", []() -> NodePtr {
    return std::make_unique<node::string_up::UpperCase>();
  });
}

}