
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::switch_node {

class Switch : public Node {
 public:
  Switch() = default;

  ~Switch() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<int>("Номер");
    decl.add_input<std::string>("Текст 1");
    decl.add_input<std::string>("Текст 2");
    decl.add_input<float>("Значение 1");
    decl.add_input<float>("Значение 2");
    
    decl.add_output<std::string>("Текс");
    decl.add_output<float>("Значение");
  }

  void execute(ExecutionContext &context) const override
  {
  }
};

}

namespace zcn {

void register_node_switch_node_type()
{
  register_node_type("Выбор", []() -> NodePtr {
    return std::make_unique<node::switch_node::Switch>();
  });
}

}