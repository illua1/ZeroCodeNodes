
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::file_write {

class FileWrite : public Node {
 public:
  FileWrite() = default;

  ~FileWrite() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Путь");
    decl.add_input<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    auto *file_context = context.context_provider<SideEffectReciver *>();
    if (file_context == nullptr) {
      return;
    }

    file_context->set_for_path(context.get_input<std::string>("Путь"), context.get_input<std::string>("Текст"));
  }
};

}

namespace zcn {

void register_node_file_write_node_type()
{
  register_node_type("Записать в", []() -> NodePtr {
    return std::make_unique<node::file_write::FileWrite>();
  });
}

}