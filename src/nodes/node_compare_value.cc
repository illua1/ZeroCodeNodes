
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::compare_value {

const static std::vector<std::string> options = {"Равно", "Больше", "Меньше", "Не равно"};

class Math : public Node {
  mutable int op_index = 0;
 public:
  Math() = default;

  ~Math() override = default;

  void declare(DeclarationContext &decl) const override
  {
    op_index = decl.add_selector("Операция", op_index, options);

    decl.add_input<float>("А");
    decl.add_input<float>("Б");

    decl.add_output<int>("Результат");
  }

  void execute(ExecutionContext &context) const override
  {
    const float a = context.get_input<float>("А");
    const float b = context.get_input<float>("Б");
    
    if (options[op_index] == "Равно") {
      context.set_output<int>("Результат", a == b);
      return;
    }
    
    if (options[op_index] == "Больше") {
      context.set_output<int>("Результат", a < b);
      return;
    }
    
    if (options[op_index] == "Меньше") {
      context.set_output<int>("Результат", a > b);
      return;
    }
    
    if (options[op_index] == "Не равно") {
      context.set_output<int>("Результат", a != b);
      return;
    }

    context.set_output("Результат", 0);
  }

  void visit_data(DataVisitor &visitor) const override
  {
    visitor.visit_int(op_index, "op_index");
  }
};

}

namespace zcn {

void register_node_compare_value_node_type()
{
  register_node_type("Сравнить числа", []() -> NodePtr {
    return std::make_unique<node::compare_value::Math>();
  });
}

}