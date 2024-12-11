
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <algorithm>
#include <string>
#include <memory>

namespace zcn::node::loop {

class LoopNode : public Node {
  struct IterfaceItem {
    DataType type;
    std::string value_name;
  };

 public:
  mutable std::string loop_body_tree_name_ = "";
  LoopNode() = default;

  ~LoopNode() override = default;

  void declare(DeclarationContext &decl) const override
  {
    loop_body_tree_name_ = std::move(decl.add_tree_selector("Тело цикла", std::move(loop_body_tree_name_)));
    const TreePtr tree = find_tree(loop_body_tree_name_);
    
    decl.add_input<int>("Повторы");
    
    if (!tree) {
      return;
    }
    for (const IterfaceItem &item : this->tree_to_inputs(tree)) {
      decl.add_input(item.type, item.value_name);
    }
    for (const IterfaceItem &item : this->tree_to_outputs(tree)) {
      decl.add_output(item.type, item.value_name);
    }
  }

  void execute(ExecutionContext &context) const override
  {
    const TreePtr tree = find_tree(loop_body_tree_name_);
    if (!tree) {
      return;
    }
    std::unordered_map<std::string, RData> inputs;
    const std::vector<IterfaceItem> input_items = this->tree_to_inputs(tree);
    for (const IterfaceItem &item : input_items) {
      inputs[item.value_name] = context.get_input(item.type, item.value_name);
    }

    const int repeats = context.get_input<int>("Повторы");

    const std::vector<IterfaceItem> output_items = this->tree_to_outputs(tree);
    if (repeats < 1) {
      for (int index = 0; index < output_items.size(); index++) {
        context.set_output(output_items[index].value_name, defult_value(output_items[index].type));
      }
      return;
    }

    std::unordered_map<std::string, RData> outputs;
    SubTreeExecutionProvider execute_context(inputs, outputs);
    
    std::vector<BaseProvider *> context_stack = context.context_providers();
    context_stack.insert(context_stack.begin(), &execute_context);


    std::unordered_map<std::string, RData> next_inputs;
    for (int index = 0; index < repeats; index++) {
      ExecuteLog log;
      zcn::execute(tree, log, context_stack);
      for (int index = 0; index < std::min(input_items.size(), output_items.size()); index++) {
        next_inputs[input_items[index].value_name] = outputs[output_items[index].value_name];
      }
      std::swap(next_inputs, inputs);
    }

    for (int index = 0; index < output_items.size(); index++) {
      context.set_output(output_items[index].value_name, outputs.at(output_items[index].value_name));
    }
  }

  void visit_data(DataVisitor &visitor) const override
  {
    visitor.visit_text(loop_body_tree_name_, "loop_body_tree_name_");
  }

 private:
  std::vector<IterfaceItem> tree_to_inputs(const TreePtr tree) const
  {
    std::vector<IterfaceItem> list;
    for (int node_index = 0; node_index < tree->nodes.size(); node_index++) {
      if (const auto *input_node = dynamic_cast<const InterfaceInputNode *>(tree->nodes[node_index].get()); input_node != nullptr) {
        list.push_back({input_node->data_type(), tree->node_names[node_index]});
      }
    }

    std::sort(list.begin(), list.end(), [&](const IterfaceItem &a, const IterfaceItem &b) {
      return a.value_name < b.value_name;
    });
    return list;
  }

  std::vector<IterfaceItem> tree_to_outputs(const TreePtr tree) const
  {
    std::vector<IterfaceItem> list;
    for (int node_index = 0; node_index < tree->nodes.size(); node_index++) {
      if (const auto *input_node = dynamic_cast<const InterfaceOutputNode *>(tree->nodes[node_index].get()); input_node != nullptr) {
        list.push_back({input_node->data_type(), tree->node_names[node_index]});
      }
    }

    std::sort(list.begin(), list.end(), [&](const IterfaceItem &a, const IterfaceItem &b) {
      return a.value_name < b.value_name;
    });

    return list;
  }
};

}

namespace zcn {

void register_node_loop_node_type()
{
  register_node_type("Повтор", []() -> NodePtr {
    return std::make_unique<node::loop::LoopNode>();
  });
}

}