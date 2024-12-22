
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::context_file_input_node {

class CFInput : public Node {
 public:
  CFInput() = default;

  ~CFInput() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_output<std::string>("Путь");
  }

  void execute(ExecutionContext &context) const override
  {
    const auto *path = context.context_provider<const ContextFilePathProvier *>();
    if (path == nullptr) {
      context.set_output<std::string>("Путь", "");
      return;
    }

    context.set_output("Путь", path->data);
  }
};

}

namespace zcn {

void register_node_context_file_input_node_type()
{
  register_node_type("О файле", []() -> NodePtr {
    return std::make_unique<node::context_file_input_node::CFInput>();
  });
}

}