
#include "../ZCN_node.hh"

#include <string>
#include <memory>
#include <sstream>

namespace zcn::node::value_to_text {

class ValueToTextNode : public Node {
 public:
  ValueToTextNode() = default;

  ~ValueToTextNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<float>("Значение");
    decl.add_input<int>("Знаки");
    decl.add_output<std::string>("Текст");
  }

  void execute(ExecutionContext &context) const override
  {
    std::ostringstream stream;
    stream.precision(std::max(0, context.get_input<int>("Знаки")));
    stream << std::fixed << context.get_input<float>("Значение");

    context.set_output<std::string>("Текст", std::move(stream).str());
  }
};

}

namespace zcn {

void register_node_value_to_text_node_type()
{
  register_node_type("Значение в текст", []() -> NodePtr {
    return std::make_unique<node::value_to_text::ValueToTextNode>();
  });
}

}