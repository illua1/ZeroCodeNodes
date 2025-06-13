
#include <iostream>

#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <string>
#include <memory>

namespace zcn::node::question_button {

class ButtonNode : public Node {
 public:
  ButtonNode() = default;

  ~ButtonNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    decl.add_input<std::string>("Имя");
    decl.add_output<int>("Ответ");
  }

  void execute(ExecutionContext &context) const override
  {
    const std::string question_label = context.get_input<std::string>("Имя");

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

class ButtonOrNode : public Node {
 public:
  ButtonOrNode() = default;

  ~ButtonOrNode() override = default;

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
      const int result = window.button_or_try(question_label);
      if (result != 0) {
        context.set_output<int>("Ответ", result);

        if (cache != nullptr) {
          cache->data[question_label] = result;
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
    return std::make_unique<node::question_button::ButtonNode>();
  });
  register_node_type("Кнопка или нет", []() -> NodePtr {
    return std::make_unique<node::question_button::ButtonOrNode>();
  });
  register_node_type("Вопрос и кнопка", []() -> NodePtr {
    return std::make_unique<node::question_button::QuestionButtonNode>();
  });
}

}