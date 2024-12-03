
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::string_slice {

class EndNode : public Node {
 public:
  EndNode() = default;

  ~EndNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<float>("Значение");
    decl.add_input<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    context.get_input<float>("Значение");
    context.get_input<std::string>("Текст");
  }
};

}

namespace zcn {

void register_node_string_slice_node_type()
{
  register_node_type("Вырезать строку", []() -> NodePtr {
    return std::make_unique<node::string_slice::EndNode>();
  });
}

}