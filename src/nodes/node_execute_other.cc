
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::execute_other {

class ExecuteOther : public Node {
 public:
  ExecuteOther() = default;

  ~ExecuteOther() override = default;

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

void register_node_execute_other_node_type()
{
  register_node_type("Функция", []() -> NodePtr {
    return std::make_unique<node::execute_other::ExecuteOther>();
  });
}

}