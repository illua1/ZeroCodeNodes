
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::text_concat {

class TextConcatinationNode : public Node {
 public:
  TextConcatinationNode() = default;

  ~TextConcatinationNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Префикс");
    decl.add_input<std::string>("Суфикс");
    decl.add_output<std::string>("Сумма");
  }

  void draw(DrawContext &/*context*/) override {}

  void execute(ExecutionContext &context) const override
  {
    const std::string a = context.get_input<std::string>("Префикс");
    const std::string b = context.get_input<std::string>("Суфикс");

    context.set_output<std::string>("Текст", a + b);
  }
};

}

namespace zcn {

void register_node_text_concatination_node_type()
{
  register_node_type("Соединение строк", []() -> NodePtr {
    return std::make_unique<node::text_concat::TextConcatinationNode>();
  });
}

}