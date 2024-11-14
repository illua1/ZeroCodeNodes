
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::text_input {

class TextInputNode : public Node {
  std::string value_ = "Текст";

 public:
  TextInputNode() = default;

  ~TextInputNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_output<std::string>("Текст");
  }

  void draw(DrawContext &context) override
  {
    context.text_input(this->value_);
  }

  void execute(ExecutionContext &context) const override
  {
    context.set_output<std::string>("Текст", this->value_);
  }
};

}

namespace zcn {

void register_text_input_node_type()
{
  register_node_type("Text Input", []() -> NodePtr {
    return std::make_unique<node::text_input::TextInputNode>();
  });
}

}