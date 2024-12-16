
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::question {

class QuestionNode : public Node {
 public:
  QuestionNode() = default;

  ~QuestionNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Вопрос");
    decl.add_output<int>("Ответ");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string question_label = context.get_input<std::string>("Вопрос");
    
    const auto *windows_owner = context.context_provider<GUIExecutionProvider *>();
    if (windows_owner == nullptr) {
      return;
    }

    auto window = windows_owner->get_window("Вопросик");
    
    while (window.is_open()) {
      if (window.button_try(question_label)) {
        context.set_output<int>("Ответ", 1);
        return;
      }
    }

    context.set_output<int>("Ответ", 0);
  }
};

}

namespace zcn {

void register_node_question_node_type()
{
  register_node_type("Вопрос", []() -> NodePtr {
    return std::make_unique<node::question::QuestionNode>();
  });
}

}