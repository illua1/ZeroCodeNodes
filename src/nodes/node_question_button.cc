
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::question_button {

class QuestionButtonNode : public Node {
 public:
  QuestionButtonNode() = default;

  ~QuestionButtonNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Вопрос");
    decl.add_output<int>("Ответ");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string question_label = context.get_input<std::string>("Вопрос");
    
    
    auto *cache = context.context_provider<CacheProvider *>();
    if (cache != nullptr) {
      if (cache->data.find(question_label) != cache->data.end()) {
        context.set_output("Ответ", ensure_type(DataType::Int, cache->data.at(question_label)));
        return;
      }
    }

    auto *windows_owner = context.context_provider<GUIExecutionProvider *>();
    if (windows_owner == nullptr) {
      return;
    }

    auto window = windows_owner->get_window("Вопросик");
    
    while (window.is_open()) {
      if (window.button_try(question_label)) {
        context.set_output<int>("Ответ", 1);

        if (cache != nullptr) {
          cache->data[question_label] = 1;
        }

        return;
      }
    }

    if (cache != nullptr) {
      cache->data[question_label] = 0;
    }

    context.set_output<int>("Ответ", 0);
  }
};

}

namespace zcn {

void register_node_question_button_node_type()
{
  register_node_type("Кнопка", []() -> NodePtr {
    return std::make_unique<node::question_button::QuestionButtonNode>();
  });
}

}