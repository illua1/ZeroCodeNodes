
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::math {

const static std::vector<std::string> options =
  {"Сложить",
   "Вычесть",
   "Умножить",
   "Поделить",
   "Степень",
   "Корень",
   "Синус",
   "Косинус",
   "Тангенс",
   "Арккосинус",
   "Арксинус",
   "Арктангенс",
   "Округлить",
   "Модуль",
   "Абсолютно"};

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

    decl.add_output<float>("Результат");
  }

  void execute(ExecutionContext &context) const override
  {
    const float a = context.get_input<float>("А");
    const float b = context.get_input<float>("Б");
    
    if (options[op_index] == "Сложить") {
      context.set_output("Результат", a + b);
      return;
    }
    
    if (options[op_index] == "Вычесть") {
      context.set_output("Результат", a - b);
      return;
    }
    
    if (options[op_index] == "Умножить") {
      context.set_output("Результат", a * b);
      return;
    }
    
    if (options[op_index] == "Поделить") {
      context.set_output("Результат", a / b);
      return;
    }
    
    if (options[op_index] == "Степень") {
      context.set_output("Результат", std::pow(a, b));
      return;
    }
    
    if (options[op_index] == "Корень") {
      context.set_output("Результат", std::pow(a, 1.0f / b));
      return;
    }
    
    if (options[op_index] == "Синус") {
      context.set_output("Результат", std::sin(a) * b);
      return;
    }
    
    if (options[op_index] == "Косинус") {
      context.set_output("Результат", std::cos(a) * b);
      return;
    }
    
    if (options[op_index] == "Тангенс") {
      context.set_output("Результат", std::tan(a) * b);
      return;
    }
    
    if (options[op_index] == "Арккосинус") {
      context.set_output("Результат", std::acos(a) * b);
      return;
    }
    
    if (options[op_index] == "Арксинус") {
      context.set_output("Результат", std::asin(a) * b);
      return;
    }
    
    if (options[op_index] == "Арктангенс") {
      context.set_output("Результат", std::atan(a) * b);
      return;
    }
    
    if (options[op_index] == "Округлить") {
      context.set_output("Результат", std::floor(a * b) / b);
      return;
    }
    
    if (options[op_index] == "Модуль") {
      context.set_output("Результат", std::fmod(a, b));
      return;
    }
    
    if (options[op_index] == "Абсолютно") {
      context.set_output("Результат", std::abs(a) * b);
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

void register_node_math_node_type()
{
  register_node_type("Математика", []() -> NodePtr {
    return std::make_unique<node::math::Math>();
  });
}

}