
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::input_node {

const static std::vector<std::string> options = {"Число", "Значение", "Текст"};

class Input : public InterfaceInputNode {
  mutable int data_type_index_ = 0;

 public:
  Input() = default;

  ~Input() override = default;

  void declare(DeclarationContext &decl) const override
  {
    data_type_index_ = decl.add_selector("Тип", data_type_index_, options);

    if (options[data_type_index_] == "Число") {
      decl.add_output<int>("Данные");
    } else if (options[data_type_index_] == "Значение") {
      decl.add_output<float>("Данные");
    } else if (options[data_type_index_] == "Текст") {
      decl.add_output<std::string>("Данные");
    }
  }

  void execute(ExecutionContext &context) const override
  {
    const auto *name_of = context.context_provider<const NodeNameProvider *>();
    if (name_of == nullptr) {
      context.set_output("Данные", defult_value(this->data_type()));
      return;
    }
    
    const auto *in_tree = context.context_provider<const SubTreeExecutionProvider *>();
    if (in_tree == nullptr) {
      context.set_output("Данные", defult_value(this->data_type()));
      return;
    }

    context.set_output("Данные", in_tree->get_input(this->data_type(), name_of->name));
  }

  DataType data_type() const override
  {
    if (options[data_type_index_] == "Число") {
      return DataType::Int;
    } else if (options[data_type_index_] == "Значение") {
      return DataType::Float;
    } else if (options[data_type_index_] == "Текст") {
      return DataType::Text;
    }
  }
};

}

namespace zcn {

void register_node_input_node_type()
{
  register_node_type("Ввод", []() -> NodePtr {
    return std::make_unique<node::input_node::Input>();
  });
}

}