
#include "../ZCN_node.hh"

#include <string>
#include <memory>

namespace zcn::node::string_line_break {

class StringLineBreak : public Node {
 public:
  StringLineBreak() = default;

  ~StringLineBreak() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_output<std::string>("Перенос Строки");
  }

  void execute(ExecutionContext &context) const override
  {
    context.set_output<std::string>("Перенос Строки", "\n");
  }
};

}

namespace zcn {

void register_node_string_line_break_node_type()
{
  register_node_type("Перенос строки", []() -> NodePtr {
    return std::make_unique<node::string_line_break::StringLineBreak>();
  });
}

}