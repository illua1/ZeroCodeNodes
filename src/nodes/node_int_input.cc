
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::int_input {

class IntInput : public Node {
 public:
  IntInput() = default;

  ~IntInput() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_data<int>("Число");
    decl.add_output<int>("Число");
  }

  void execute(ExecutionContext &context) const override
  {
    context.set_output<int>("Число", context.get_data<int>("Число"));
  }
};

}

namespace zcn {

void register_node_int_input_node_type()
{
  register_node_type("Число", []() -> NodePtr {
    return std::make_unique<node::int_input::IntInput>();
  });
}

}