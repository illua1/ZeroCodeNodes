
#include "../ZCN_node.hh"

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
    // context.set_output<std::string>("Текст", this->value_);
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