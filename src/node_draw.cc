
#include <cassert>
#include <unordered_map>

#include "ZCN_node.hh"

#include "imgui.h"

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

  void value_input(float &value) override
  {
    
  }

  void list_input(const std::vector<std::string> list, int &index) override
  {
    
  }
};

class ImNodesDeclarationContext : public DeclarationContext {
  int counter_ = 0;
  const std::vector<int> &socket_uids_;
 public:
  ImNodesDeclarationContext(const std::vector<int> &socket_uids) : socket_uids_(socket_uids) {}
  ~ImNodesDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    const int socket_uid = socket_uids_[counter_++];
    
    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginInputAttribute(socket_uid, pin_for_type(type));
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndInputAttribute();
    ImNodes::PopColorStyle();
  }

  void add_output(const DataType type, std::string name) override
  {
    const int socket_uid = socket_uids_[counter_++];

    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginOutputAttribute(socket_uid, pin_for_type(type));
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
    return {};
  }
  
  static uint32_t color_for_type(const DataType type)
  {
    switch (type) {
      case DataType::Int: return IM_COL32(11, 109, 191, 255);
      case DataType::Float: return IM_COL32(109, 11, 191, 255);
      case DataType::Text: return IM_COL32(191, 109, 11, 255);
    }
    assert(false);
    return {};
  }
};

void draw(NodeTree &tree)
{

  if (ImGui::BeginMenuBar()) {

    bool openpopuptemp = false;
    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("About"))
        openpopuptemp = true;
      ImGui::EndMenu();
    }

    if (openpopuptemp == true) {
      ImGui::OpenPopup("About");
      openpopuptemp = false;
    }

    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("I'm a popup!");
      if (ImGui::Button("Close", ImVec2(60, 0)))
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    ImGui::EndMenuBar();
  }

  ImNodes::BeginNodeEditor();

  const bool open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImNodes::IsEditorHovered() && ImGui::IsKeyPressed(ImGuiKey_A);
  
  if (!ImGui::IsAnyItemHovered() && open_popup) {
    ImGui::OpenPopup("add node");
  }

  if (ImGui::BeginPopup("add node")) {
    const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
    for (const std::string node_type : zcn::all_node_types()) {
      if (ImGui::MenuItem(node_type.c_str())) {
        const int node_index = zcn::add_node_to_tree(tree, node_type);
        ImNodes::SetNodeScreenSpacePos(tree.nodes_uid[node_index], ImGui::GetMousePos());
      }
    }

    ImGui::EndPopup();
  }

  for (int node_index = 0; node_index < tree.nodes.size(); node_index++) {
    NodePtr &node = tree.nodes[node_index];

    ImNodes::BeginNode(tree.nodes_uid[node_index]);
    ImGui::PushItemWidth(120.0f);

    ImNodesDrawContext draw_context(tree.node_labels[node_index]);
    node->draw(draw_context);

    ImNodesDeclarationContext declaration(tree.node_sockets_uid[node_index]);
    node->declare(declaration);
    
    ImGui::PopItemWidth();
    ImNodes::EndNode();
  }

  ImNodes::EndNodeEditor();
}

}