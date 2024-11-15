
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::float_input {

class FloatInputNode : public Node {
  float value_ = 0.0f;

 public:
  FloatInputNode() = default;

  ~FloatInputNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_output<std::string>("Зачение");
  }

  void draw(DrawContext &context) override
  {
    context.value_input(this->value_);
  }

  void execute(ExecutionContext &context) const override
  {
    context.set_output<float>("Значение", this->value_);
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