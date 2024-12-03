
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::string_compare {

const static std::vector<std::string> options = {"Равны", "Не равны"};
const static std::vector<std::string> type_options = {"Регистр", "Абсолютно"};

class StringCmp : public Node {
  mutable int op_index = 0;
  mutable int type_op_index = 0;
 public:
  StringCmp() = default;

  ~StringCmp() override = default;

  void declare(DeclarationContext &decl) const override
  {
    op_index = decl.add_selector("Операция", op_index, options);
    type_op_index = decl.add_selector("Метод", type_op_index, type_options);

    decl.add_input<std::string>("А");
    decl.add_input<std::string>("Б");

    decl.add_output<int>("Результат");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string a = context.get_input<std::string>("А");
    const std::string b = context.get_input<std::string>("Б");
    
    if (a.size() != b.size()) {
      context.set_output<int>("Результат", 0 ^ (options[op_index] != "Равны"));
      return;
    }

    bool result = true;
    for (int index = 0; index < a.size(); index++) {
      if (type_options[type_op_index] != "Регистр") {
        result &= a[index] == b[index];
      } else {
        const unsigned char a_c = a[index];
        const unsigned char b_c = b[index];
        result &= std::toupper(a_c) == std::toupper(b_c);
      }
    }
    context.set_output("Результат", result ^ (options[op_index] != "Равны"));
  }
};

}

namespace zcn {

void register_node_string_compare_node_type()
{
  register_node_type("Сравнить текст", []() -> NodePtr {
    return std::make_unique<node::string_compare::StringCmp>();
  });
}

}