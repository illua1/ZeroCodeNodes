
#include <cassert>
#include <unordered_set>
#include <unordered_map>

#include "ZCN_node.hh"

#include "imgui.h"

#include "imnodes.h"

namespace zcn::nodes {

class ImNodesDeclarationContext : public DeclarationContext {
  int counter_ = 0;
  NodeTree &tree_;
  const int node_index_;
  const std::unordered_set<int> &linked_sockets_;
  std::unordered_map<int, std::pair<float, float>> &r_socket_positions_;

 public:
  ImNodesDeclarationContext(NodeTree &tree,
                            const int node_index,
                            const std::unordered_set<int> &linked_sockets,
                            std::unordered_map<int, std::pair<float, float>> &r_socket_positions) : tree_(tree),
                                                                                                    node_index_(node_index),
                                                                                                    linked_sockets_(linked_sockets),
                                                                                                    r_socket_positions_(r_socket_positions)
  {
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(tree_.node_labels[node_index].c_str());
    ImNodes::EndNodeTitleBar();
  }

  ~ImNodesDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    const int socket_uid = tree_.node_sockets_uid[node_index_][counter_++];
    
    const std::string socket_path = node_socket_to_path(tree_.nodes_uid[node_index_], socket_uid);

    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginInputAttribute(socket_uid, pin_for_type(type));
    // ImGui::TextUnformatted(name.c_str());
    
    if (linked_sockets_.find(socket_uid) == linked_sockets_.end()) {
      switch (type) {
        case DataType::Int: {
          auto [value, _] = tree_.values.try_emplace(socket_path, RData(int(0)));
          if (value == tree_.values.end()) {
            break;
          }
          [[maybe_unused]] auto &typed_value = std::get<int>(value->second);
          printf("Not implemented yet.\n");
          break;
        }
        case DataType::Float: {
          auto [value, _] = tree_.values.try_emplace(socket_path, RData(float(0)));
          if (value == tree_.values.end()) {
            break;
          }
          auto &typed_value = std::get<float>(value->second);

          ImGui::DragFloat(name.c_str(), &typed_value, 1.0f, 0.0f, 0.0f);
          break;
        }
        case DataType::Text: {
          auto [value, _] = tree_.values.try_emplace(socket_path, RData(std::string()));
          if (value == tree_.values.end()) {
            break;
          }
          auto &typed_value = std::get<std::string>(value->second);

          char buffer[100];
          std::strncpy(buffer, typed_value.c_str(), std::min<int>(sizeof(buffer), typed_value.size() + 1));
          if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags(), nullptr, nullptr)) {
            typed_value = std::move(std::string(std::string_view(buffer)));
          }
          break;
        }
      }
    } else {
      ImGui::TextUnformatted(name.c_str());
    }
    
    r_socket_positions_[socket_uid] = std::make_pair(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
    
    ImNodes::EndInputAttribute();
    ImNodes::PopColorStyle();
  }

  void add_output(const DataType type, std::string name) override
  {
    const int socket_uid = tree_.node_sockets_uid[node_index_][counter_++];

    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginOutputAttribute(socket_uid, pin_for_type(type));
    ImGui::TextUnformatted(name.c_str());
    
    r_socket_positions_[socket_uid] = std::make_pair(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
    
    ImNodes::EndOutputAttribute();
    ImNodes::PopColorStyle();
  }

  void add_data(const DataType type, const std::string name) override
  {
    const std::string node_path = node_value_to_path(tree_.nodes_uid[node_index_], name);
    switch (type) {
      case DataType::Int: {
        auto [value, _] = tree_.values.try_emplace(node_path, RData(int(0)));
        if (value == tree_.values.end()) {
          break;
        }
        [[maybe_unused]] auto &typed_value = std::get<int>(value->second);
        printf("Not implemented yet.\n");
        break;
      }
      case DataType::Float: {
        auto [value, _] = tree_.values.try_emplace(node_path, RData(float(0)));
        if (value == tree_.values.end()) {
          break;
        }
        auto &typed_value = std::get<float>(value->second);
        
        ImGui::DragFloat(name.c_str(), &typed_value, 1.0f, 0.0f, 100.0f);
        break;
      }
      case DataType::Text: {
        auto [value, _] = tree_.values.try_emplace(node_path, RData(std::string()));
        if (value == tree_.values.end()) {
          break;
        }
        auto &typed_value = std::get<std::string>(value->second);
        
        char buffer[100];
        std::strncpy(buffer, typed_value.c_str(), std::min<int>(sizeof(buffer), typed_value.size() + 1));
        if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags(), nullptr, nullptr)) {
          typed_value = std::move(std::string(std::string_view(buffer)));
        }
        break;
      }
    }
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

void draw(NodeTree &tree, std::unordered_map<int, std::pair<float, float>> &r_socket_positions)
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
    for (const std::string node_type : zcn::all_node_types()) {
      if (ImGui::MenuItem(node_type.c_str())) {
        const int node_index = zcn::add_node_to_tree(tree, node_type);
        ImNodes::SetNodeScreenSpacePos(tree.nodes_uid[node_index], ImGui::GetMousePos());
      }
    }

    ImGui::EndPopup();
  }

  std::unordered_set<int> linked_sockets;
  for (const auto link : tree.links) {
    linked_sockets.insert(link.first);
    linked_sockets.insert(link.second);
  }

  for (int node_index = 0; node_index < tree.nodes.size(); node_index++) {
    NodePtr &node = tree.nodes[node_index];

    ImNodes::BeginNode(tree.nodes_uid[node_index]);
    ImGui::PushItemWidth(120.0f);

    ImNodesDeclarationContext declaration(tree, node_index, linked_sockets, r_socket_positions);
    node->declare(declaration);
    
    ImGui::PopItemWidth();
    ImNodes::EndNode();
  }

  for (int link_index = 0; link_index < tree.links.size(); link_index++) {
    const std::pair<int, int> link = tree.links[link_index];
    ImNodes::Link(tree.links_uid[link_index], link.first, link.second);
  }

  ImNodes::EndNodeEditor();

  if (std::pair<int, int> link; ImNodes::IsLinkCreated(&link.first, &link.second)){
    zcn::add_link_from_tree(tree, link);
  }

  if (int link_uid; ImNodes::IsLinkHovered(&link_uid) && ImGui::IsKeyPressed(ImGuiKey_MouseRight)) {
    zcn::drop_link_from_tree(tree, link_uid);
  }
}

}