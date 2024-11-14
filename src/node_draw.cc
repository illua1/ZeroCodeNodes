
#include <cassert>
#include <unordered_map>

#include "ZCN_node.hh"

#include "imnodes.h"

namespace zcn::nodes {

class ImNodesDrawContext : public DrawContext {
 public:
  ImNodesDrawContext(const std::string &label) {
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(label.c_str());
    ImNodes::EndNodeTitleBar();

  }

  ~ImNodesDrawContext() override = default;

  void text_input(std::string &text) override
  {
    char buffer[100];
    std::strncpy(buffer, text.c_str(), std::min<int>(sizeof(buffer), text.size() + 1));
    if (ImGui::InputText("", buffer, sizeof(buffer), ImGuiInputTextFlags(), nullptr, nullptr)) {
      text = std::move(std::string(std::string_view(buffer)));
    }
  }

  void value_input(int &value) override
  {
    
  }

  void list_input(const std::vector<std::string> list, int &index) override
  {
    
  }
};

class ImNodesDeclarationContext : public DeclarationContext {
 public:
  ~ImNodesDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginInputAttribute(1, pin_for_type(type));
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndInputAttribute();
    ImNodes::PopColorStyle();
  }

  void add_output(const DataType type, std::string name) override
  {
    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginOutputAttribute(1, pin_for_type(type));
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndOutputAttribute();
    ImNodes::PopColorStyle();
  }

 protected:
  static ImNodesPinShape pin_for_type(const DataType type)
  {
    switch (type) {
      case DataType::Int: return ImNodesPinShape_Circle;
      case DataType::Float: return ImNodesPinShape_Triangle;
      case DataType::Text: return ImNodesPinShape_Quad;
    }
    assert(false);
  }
  
  static uint32_t color_for_type(const DataType type)
  {
    switch (type) {
      case DataType::Int: return IM_COL32(11, 109, 191, 255);
      case DataType::Float: return IM_COL32(109, 11, 191, 255);
      case DataType::Text: return IM_COL32(191, 109, 11, 255);
    }
    assert(false);
  }
};

void draw(NodeTree &tree)
{
  ImNodes::BeginNodeEditor();

  for (int node_index = 0; node_index < tree.nodes.size(); node_index++) {
    NodePtr &node = tree.nodes[node_index];


    ImNodes::BeginNode(tree.nodes_uid[node_index]);
    ImGui::PushItemWidth(120.0f);

    ImNodesDrawContext draw_context(tree.node_labels[node_index]);
    node->draw(draw_context);

    ImNodesDeclarationContext declaration;
    node->declare(declaration);
    
    ImGui::PopItemWidth();
    ImNodes::EndNode();
  }

  ImNodes::EndNodeEditor();
}

}