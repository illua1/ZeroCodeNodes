
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::output_node {

const static std::vector<std::string> options = {"Число", "Значение", "Текст"};

class Output : public InterfaceOutputNode {
  mutable int data_type_index_ = 0;

 public:
  Output() = default;

  ~Output() override = default;

  void declare(DeclarationContext &decl) const override
  {
    data_type_index_ = decl.add_selector("Тип", data_type_index_, options);

    if (options[data_type_index_] == "Число") {
      decl.add_input<int>("Данные");
    } else if (options[data_type_index_] == "Значение") {
      decl.add_input<float>("Данные");
    } else if (options[data_type_index_] == "Текст") {
      decl.add_input<std::string>("Данные");
    }
  }

  void execute(ExecutionContext &context) const override
  {
    const auto *name_of = context.context_provider<const NodeNameProvider *>();
    if (name_of == nullptr) {
      return;
    }
    
    auto *in_tree = context.context_provider<SubTreeExecutionProvider *>();
    if (in_tree == nullptr) {
      return;
    }

    in_tree->set_output(name_of->name, context.get_input(this->data_type(), "Данные"));
  }

  void visit_data(DataVisitor &visitor) const override
  {
    visitor.visit_int(data_type_index_, "data_type_index_");
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

void register_node_output_node_type()
{
  register_node_type("Вывод", []() -> NodePtr {
    return std::make_unique<node::output_node::Output>();
  });
}

}