
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::switch_node {

const static std::vector<std::string> options = {"Число", "Значение", "Текст"};

class Switch : public Node {
  mutable int data_type_index_ = 0;
 public:
  Switch() = default;

  ~Switch() override = default;

  void declare(DeclarationContext &decl) const override
  {
    data_type_index_ = decl.add_selector("Тип", data_type_index_, options);

    decl.add_input<int>("Выбор");

    if (options[data_type_index_] == "Число") {
      decl.add_input<int>("А");
      decl.add_input<int>("Б");
      decl.add_output<int>("Вывод");
    } else if (options[data_type_index_] == "Значение") {
      decl.add_input<float>("А");
      decl.add_input<float>("Б");
      decl.add_output<float>("Вывод");
    } else if (options[data_type_index_] == "Текст") {
      decl.add_input<std::string>("А");
      decl.add_input<std::string>("Б");
      decl.add_output<std::string>("Вывод");
    }
  }

  void execute(ExecutionContext &context) const override
  {
    const int index = context.get_input<int>("Выбор");
    if (options[data_type_index_] == "Число") {
      const auto a = context.get_input<int>("А");
      const auto b = context.get_input<int>("Б");
      context.set_output<int>("Вывод", index == 1 ? b : a);
    } else if (options[data_type_index_] == "Значение") {
      const auto a = context.get_input<float>("А");
      const auto b = context.get_input<float>("Б");
      context.set_output<float>("Вывод", index == 1 ? b : a);
    } else if (options[data_type_index_] == "Текст") {
      const auto a = context.get_input<std::string>("А");
      const auto b = context.get_input<std::string>("Б");
      context.set_output<std::string>("Вывод", index == 1 ? b : a);
    }
  }
};

}

namespace zcn {

void register_node_switch_node_type()
{
  register_node_type("Выбор", []() -> NodePtr {
    return std::make_unique<node::switch_node::Switch>();
  });
}

}