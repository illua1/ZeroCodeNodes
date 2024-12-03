
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::text_input {

class TextInputNode : public Node {
 public:
  TextInputNode() = default;

  ~TextInputNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_data(DataType::Text, "Текст");

    decl.add_output<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    context.set_output<std::string>("Текст", context.get_data<std::string>("Текст"));
  }
};

}

namespace zcn {

void register_node_string_input_node_type()
{
  register_node_type("Текст", []() -> NodePtr {
    return NodePtr(std::make_unique<node::text_input::TextInputNode>());
  });
}

}