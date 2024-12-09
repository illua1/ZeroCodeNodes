
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::file_input {

class FileInputNode : public Node {
 public:
  FileInputNode() = default;

  ~FileInputNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Путь");
    decl.add_output<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    FileSystemProvider *file_context = nullptr;
    if (file_context = dynamic_cast<FileSystemProvider *>(&context); file_context == nullptr) {
      return;
    }

    const RData text = file_context->get_from_path(context.get_input<std::string>("Путь"));
    context.set_output("Текст", text);
  }
};

}

namespace zcn {

void register_node_file_read_node_type()
{
  register_node_type("Прочитать файл", []() -> NodePtr {
    return std::make_unique<node::file_input::FileInputNode>();
  });
}

}