
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
    decl.add_input<std::string>("Кнопка");
    decl.add_output<std::string>("Ответ");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string question_label = context.get_input<std::string>("Вопрос");
    const std::string button_label = context.get_input<std::string>("Кнопка");
    
    
    auto *cache = context.context_provider<CacheProvider *>();
    if (cache != nullptr) {
      if (cache->data.find(question_label) != cache->data.end()) {
        context.set_output("Ответ", ensure_type(DataType::Text, cache->data.at(question_label)));
        return;
      }
    }

    auto *windows_owner = context.context_provider<GUIExecutionProvider *>();
    if (windows_owner == nullptr) {
      return;
    }

    auto window = windows_owner->get_window("Вопросик");

    while (window.is_open()) {
      
      std::string user_input = window.text_try(question_label);
      
      if (window.button_try(question_label)) {
        context.set_output<std::string>("Ответ", user_input);

        if (cache != nullptr) {
          cache->data[question_label] = user_input;
        }

        return;
      }
    }

    if (cache != nullptr) {
      cache->data[question_label] = 0;
    }

    context.set_output<std::string>("Ответ", "");
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