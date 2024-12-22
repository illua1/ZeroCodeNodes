
#include "../ZCN_node.hh"
#include "../ZCN_execute.hh"

#include <vector>
#include <filesystem>
#include <string>
#include <memory>
#include <algorithm>

namespace zcn::node::foreach_file {

static std::string normalize_path(const std::string& messyPath) {
    std::filesystem::path path(messyPath);
    std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
    std::string npath = canonicalPath.make_preferred().string();
    return npath;
}

class ForeachFile : public Node {
  struct IterfaceItem {
    DataType type;
    std::string value_name;
  };
  
  mutable std::string loop_body_tree_name_ = "";

 public:
  ForeachFile() = default;

  ~ForeachFile() override = default;

  void declare(DeclarationContext &decl) const override
  {
    loop_body_tree_name_ = std::move(decl.add_tree_selector("Тело цикла", std::move(loop_body_tree_name_)));
    const TreePtr tree = find_tree(loop_body_tree_name_);
    
    decl.add_input<std::string>("Корень");

    if (!tree) {
      return;
    }
    for (const IterfaceItem &item : this->tree_to_inputs(tree)) {
      decl.add_input(item.type, item.value_name);
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

    const std::string root_path = normalize_path(context.get_input<std::string>("Корень"));

    std::unordered_map<std::string, RData> outputs;
    SubTreeExecutionProvider execute_context(inputs, outputs);

    if (!std::filesystem::is_directory(root_path)) {
      return;
    }

    for (const std::filesystem::directory_entry &sub_object : std::filesystem::recursive_directory_iterator(root_path)) {
      std::vector<BaseProvider *> context_stack = context.context_providers();
      context_stack.insert(context_stack.begin(), &execute_context);
      
      auto path = sub_object.path();
      ContextFilePathProvier file_path;
      file_path.data = path.make_preferred().string();
      context_stack.insert(context_stack.begin(), &file_path);

      ExecuteLog log;
      zcn::execute(tree, log, context_stack);
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
};

}

namespace zcn {

void register_node_foreach_file_node_type()
{
  register_node_type("По файлам", []() -> NodePtr {
    return std::make_unique<node::foreach_file::ForeachFile>();
  });
}

}