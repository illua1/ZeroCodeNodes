
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
    decl.add_input<std::string>("Путь");
    decl.add_output<std::string>("Текст");
  }

  void draw(DrawContext &/*context*/) override {}

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
    return std::make_unique<node::text_input::TextInputNode>();
  });
}

}