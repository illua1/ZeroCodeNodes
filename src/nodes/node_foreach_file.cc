
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::foreach_file {

class ForeachFile : public Node {
 public:
  ForeachFile() = default;

  ~ForeachFile() override = default;

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

void register_node_foreach_file_node_type()
{
  register_node_type("По файлам", []() -> NodePtr {
    return std::make_unique<node::foreach_file::ForeachFile>();
  });
}

}