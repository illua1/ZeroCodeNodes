
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::float_input {

class FloatInputNode : public Node {
 public:
  FloatInputNode() = default;

  ~FloatInputNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_data(DataType::Float, "Значение");

    decl.add_output<float>("Значение");
  }

  void execute(ExecutionContext &context) const override
  {
    context.set_output<float>("Значение", context.get_data<float>("Значение"));
  }
};

}

namespace zcn {

void register_node_float_input_node_type()
{
  register_node_type("Ввод значения", []() -> NodePtr {
    return std::make_unique<node::float_input::FloatInputNode>();
  });
}

}