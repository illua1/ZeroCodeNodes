
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::value_to_text {

class ValueToTextNode : public Node {
 public:
  ValueToTextNode() = default;

  ~ValueToTextNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<float>("Значение");
    decl.add_output<std::string>("Текст");
  }

  void draw(DrawContext &/*context*/) override {}

  void execute(ExecutionContext &context) const override
  {
    context.set_output<std::string>("Текст", std::to_string(context.get_input<float>("Значение")));
  }
};

}

namespace zcn {

void register_node_value_to_text_node_type()
{
  register_node_type("Значение в текст", []() -> NodePtr {
    return std::make_unique<node::value_to_text::ValueToTextNode>();
  });
}

}