
#include <cassert>
#include <unordered_set>
#include <numeric>
#include <unordered_map>
#include <algorithm>

#include "ZCN_node.hh"
#include "ZCN_execute.hh"

#include "imgui.h"

#include "imnodes.h"

namespace zcn::nodes {

class StateUpdateDeclarationContext : public DeclarationContext {
  int node_uid_;
  std::vector<int> &uids_;
  int index_ = 0;
  int output_ = 0;
 public:
  StateUpdateDeclarationContext(const int node_uid, std::vector<int> &uids) : node_uid_(node_uid), uids_(uids) {
    uids_.clear();
  }
  ~StateUpdateDeclarationContext() override
  {
    uids_.resize(index_);
  }

  void add_input(const DataType type, std::string name) override
  {
    index_++;
    if (index_ < uids_.size()) {
      return;
    }
    uids_.push_back(node_uid_ + 150 + index_);
  }

  void add_output(const DataType type, std::string name) override
  {
    index_++;
    if (index_ < uids_.size()) {
      return;
    }
    uids_.push_back(node_uid_ + index_);
  }

  void add_data(const DataType /*type*/, std::string /*name*/) override {}
};

class ImNodesDeclarationContext : public DeclarationContext {
  int counter_ = 0;
  NodeTree &tree_;
  const int node_index_;
  const std::unordered_set<int> &linked_sockets_;
  std::unordered_map<int, std::pair<float, float>> &r_socket_positions_;
  const std::vector<std::string> &trees_names_;

 public:
  ImNodesDeclarationContext(NodeTree &tree,
                            const int node_index,
                            const std::unordered_set<int> &linked_sockets,
                            std::unordered_map<int, std::pair<float, float>> &r_socket_positions,
                            const std::vector<std::string> &trees_names) : tree_(tree),
                                                                           node_index_(node_index),
                                                                           linked_sockets_(linked_sockets),
                                                                           r_socket_positions_(r_socket_positions),
                                                                           trees_names_(trees_names)
  {
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(tree_.node_names[node_index].c_str());
    ImNodes::EndNodeTitleBar();
  }

  ~ImNodesDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    {
      StateUpdateDeclarationContext ensure_socket_uid(tree_.nodes_uid[node_index_], tree_.node_sockets_uid[node_index_]);
      tree_.nodes[node_index_]->declare(ensure_socket_uid);
    }
    if (counter_ > 0) {
      ImGui::Dummy(ImVec2(0.0f, 6.0f));
    }

    const int socket_uid = tree_.node_sockets_uid[node_index_][counter_++];
    
    const std::string socket_path = node_socket_to_path(tree_.nodes_uid[node_index_], socket_uid);

    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginInputAttribute(socket_uid, pin_for_type(type));

    r_socket_positions_[socket_uid] = std::make_pair(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);

    if (tree_.values.find(socket_path) == tree_.values.end()) {
      tree_.values[socket_path] = defult_value(type);
    }

    RData value = ensure_type(type, std::move(tree_.values[socket_path]));
    
    ImGui::PushItemWidth(120.0f - ImGui::CalcTextSize(name.c_str()).x);
    if (linked_sockets_.find(socket_uid) == linked_sockets_.end()) {
      switch (type) {
        case DataType::Int: {
          auto &typed_value = std::get<int>(value);

          ImGui::DragInt(name.c_str(), &typed_value, 1.0f, 0, 0);
          break;
        }
        case DataType::Float: {
          auto &typed_value = std::get<float>(value);

          ImGui::DragFloat(name.c_str(), &typed_value, 1.0f, 0.0f, 0.0f);
          break;
        }
        case DataType::Text: {
          auto &typed_value = std::get<std::string>(value);

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
    ImGui::PopItemWidth();

    tree_.values[socket_path] = std::move(value);
    
    ImNodes::EndInputAttribute();
    ImNodes::PopColorStyle();
  }

  void add_output(const DataType type, std::string name) override
  {
    {
      StateUpdateDeclarationContext ensure_socket_uid(tree_.nodes_uid[node_index_], tree_.node_sockets_uid[node_index_]);
      tree_.nodes[node_index_]->declare(ensure_socket_uid);
    }
    if (counter_ > 0) {
      ImGui::Dummy(ImVec2(0.0f, 6.0f));
    }

    const int socket_uid = tree_.node_sockets_uid[node_index_][counter_++];

    ImNodes::PushColorStyle(ImNodesCol_Pin, color_for_type(type));
    ImNodes::BeginOutputAttribute(socket_uid, pin_for_type(type));
    
    r_socket_positions_[socket_uid] = std::make_pair(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
    
    ImGui::Indent(120.f - ImGui::CalcTextSize(name.c_str()).x);
    
    ImGui::TextUnformatted(name.c_str());
    
    ImNodes::EndOutputAttribute();
    ImNodes::PopColorStyle();
  }

  void add_data(const DataType type, const std::string name) override
  {
    const std::string node_path = node_value_to_path(tree_.nodes_uid[node_index_], name);
    
    if (tree_.values.find(node_path) == tree_.values.end()) {
      tree_.values[node_path] = defult_value(type);
    }

    RData value = ensure_type(type, std::move(tree_.values[node_path]));

    switch (type) {
      case DataType::Int: {
        [[maybe_unused]] auto &typed_value = std::get<int>(value);
          ImGui::DragInt(name.c_str(), &typed_value, 1.0f, 0, 0);
        break;
      }
      case DataType::Float: {
        auto &typed_value = std::get<float>(value);
        
        ImGui::DragFloat(name.c_str(), &typed_value, 1.0f, 0.0f, 0.0f);
        break;
      }
      case DataType::Text: {
        auto &typed_value = std::get<std::string>(value);
        
        char buffer[100];
        std::strncpy(buffer, typed_value.c_str(), std::min<int>(sizeof(buffer), typed_value.size() + 1));
        if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags(), nullptr, nullptr)) {
          typed_value = std::move(std::string(std::string_view(buffer)));
        }
        break;
      }
    }

    tree_.values[node_path] = std::move(value);
  }

  int add_selector(const std::string name, const int selected, const std::vector<std::string> &options) override
  {
    std::vector<int> indices;
    indices.resize(options.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](const int a, const int b) {
      return options[a] < options[b];
    });

    std::vector<const char *> raw_options;
    raw_options.resize(options.size());
    std::transform(indices.begin(), indices.end(), raw_options.begin(), [&](const int index) {
      return options[index].c_str();
    });

    ImGui::PushItemWidth(130.0f - ImGui::CalcTextSize(name.c_str()).x);

    int index = std::distance(indices.begin(), std::find(indices.begin(), indices.end(), selected));
    if (ImGui::Combo(name.c_str(), &index, raw_options.data(), raw_options.size())) {
      ImGui::PopItemWidth();
      return indices[index];
    }
    ImGui::PopItemWidth();

    return selected;
  }

  std::string add_tree_selector(const std::string name, std::string tree_name) override
  {
    ImGui::PushItemWidth(130.0f - ImGui::CalcTextSize(name.c_str()).x);

    std::vector<const char *> raw_options;
    raw_options.resize(trees_names_.size());
    std::transform(trees_names_.begin(), trees_names_.end(), raw_options.begin(), [](const std::string &string) {
      return string.c_str();
    });

    int index = std::distance(trees_names_.begin(), std::find(trees_names_.begin(), trees_names_.end(), tree_name));
    if (index == trees_names_.size()) {
      index = std::distance(trees_names_.begin(), std::find(trees_names_.begin(), trees_names_.end(), ""));
    }
    if (ImGui::Combo(name.c_str(), &index, raw_options.data(), raw_options.size())) {
      ImGui::PopItemWidth();
      return trees_names_[index];
    }
    ImGui::PopItemWidth();

    return tree_name;
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

class ExistSocketsDeclarationContext : public DeclarationContext {
  NodeTree &tree_;
  const int node_uid_;
  std::unordered_set<int> &exist_sockets_;
  int index_ = 0;
 public:
  ExistSocketsDeclarationContext(NodeTree &tree, const int node_uid, std::unordered_set<int> &exist_sockets) : tree_(tree), node_uid_(node_uid), exist_sockets_(exist_sockets) {}

  ~ExistSocketsDeclarationContext() override = default;

  void add_input(const DataType type, std::string name) override
  {
    index_++;
    exist_sockets_.insert(node_uid_ + 150 + index_);
  }

  void add_output(const DataType type, std::string name) override
  {
    index_++;
    exist_sockets_.insert(node_uid_ + index_);
  }

  void add_data(const DataType /*type*/, std::string /*name*/) override {}
};


void draw(NodeTree &tree, const std::vector<std::string> &trees_names, std::unordered_map<int, std::pair<float, float>> &r_socket_positions)
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
    std::vector<std::string> list = zcn::all_node_types();
    std::sort(list.begin(), list.end());
    for (const std::string node_type : list) {
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

    ImNodesDeclarationContext declaration(tree, node_index, linked_sockets, r_socket_positions, trees_names);
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

  std::unordered_set<int> exist_sockets;

  for (int node_index = 0; node_index < tree.nodes.size(); node_index++) {
    NodePtr &node = tree.nodes[node_index];
    ExistSocketsDeclarationContext declaration(tree, tree.nodes_uid[node_index], exist_sockets);
    node->declare(declaration);
  }

  std::vector<bool> links_to_delete;
  links_to_delete.resize(tree.links.size());
  std::transform(tree.links.begin(), tree.links.end(), links_to_delete.begin(), [&](const std::pair<int, int> link) {
    if (exist_sockets.find(link.first) == exist_sockets.end()) {
      return false;
    }
    if (exist_sockets.find(link.second) == exist_sockets.end()) {
      return false;
    }
    return true;
  });

  tree.links.erase(std::remove_if(tree.links.begin(), tree.links.end(), [&](auto &item) {
    return !links_to_delete[std::distance(&tree.links[0], &item)];
  }), tree.links.end());

  tree.links_uid.erase(std::remove_if(tree.links_uid.begin(), tree.links_uid.end(), [&](auto &item) {
    return !links_to_delete[std::distance(&tree.links_uid[0], &item)];
  }), tree.links_uid.end());
  
}

static ImVec2 operator + (const ImVec2 &a, const ImVec2 b)
{
  return ImVec2(a.x + b.x, a.y + b.y);
}

static ImVec2 operator - (const ImVec2 &a, const ImVec2 b)
{
  return ImVec2(a.x - b.x, a.y - b.y);
}

class ImNodesOverlayDeclarationContext : public DeclarationContext {
  int counter_ = 0;
  const NodeTree &tree_;
  const int node_index_;
  const ExecuteLog &log_;
  const std::unordered_map<int, std::pair<float, float>> &socket_positions_;
  const std::unordered_set<int> &linked_sockets_;
  ImDrawList &draw_list_;

 public:
  ImNodesOverlayDeclarationContext(const NodeTree &tree,
                                    const int node_index,
                                    const ExecuteLog &log,
                                    const std::unordered_map<int, std::pair<float, float>> &socket_positions,
                                    const std::unordered_set<int> &linked_sockets,
                                    ImDrawList &draw_list) :
                                        tree_(tree),
                                        node_index_(node_index),
                                        log_(log),
                                        socket_positions_(socket_positions),
                                        linked_sockets_(linked_sockets),
                                        draw_list_(draw_list)
  {
  }

  ~ImNodesOverlayDeclarationContext() override = default;

  void add_input(const DataType type, std::string /*name*/) override
  {
    const int socket_uid = tree_.node_sockets_uid[node_index_][counter_++];
    this->show(type, socket_uid, false);
  }

  void add_output(const DataType type, std::string /*name*/) override
  {
    const int socket_uid = tree_.node_sockets_uid[node_index_][counter_++];
    this->show(type, socket_uid, true);
  }

  void add_data(const DataType /*type*/, const std::string /*name*/) override
  {
  }

 private:
  void show(const DataType type, const int socket_uid, const bool dirrection)
  {
    if (linked_sockets_.find(socket_uid) == linked_sockets_.end()) {
      return;
    }
    
    if (socket_positions_.find(socket_uid) == socket_positions_.end()) {
      return;
    }

    const std::string socket_path = node_socket_to_path(tree_.nodes_uid[node_index_], socket_uid);

    if (log_.socket_value.find(socket_path) == log_.socket_value.end()) {
      return;
    }

    const std::pair<float, float> position_pair = socket_positions_.at(socket_uid);
    const RData &log_value = log_.socket_value.at(socket_path);

    constexpr int offset = 6;

    int overlay_alpha = 160;

    int is_socket_uid = -1;
    const bool is_under_cursore = ImNodes::IsPinHovered(&is_socket_uid);
    if (is_under_cursore && is_socket_uid == socket_uid) {
      overlay_alpha = 50;
    }

    const std::string test = [&]() -> std::string {
      switch (type) {
        case DataType::Int: {
          return std::to_string(std::get<int>(log_value));
        }
        case DataType::Float: {
          return std::to_string(std::get<float>(log_value));
        }
        case DataType::Text: {
          return std::get<std::string>(log_value);
        }
      }
      return "..";
    }();

    const ImU32 background_color = [&]() -> ImU32 {
      switch (type) {
        case DataType::Int: {
          return IM_COL32(40, 40, 140, overlay_alpha);
        }
        case DataType::Float: {
          return IM_COL32(140, 40, 40, overlay_alpha);
        }
        case DataType::Text: {
          return IM_COL32(40, 140, 40, overlay_alpha);
        }
      }
      return IM_COL32(0, 0, 0, 256);
    }();

    const ImVec2 node_size = ImNodes::GetNodeDimensions(tree_.nodes_uid[node_index_]);

    const ImVec2 bound = ImGui::CalcTextSize(test.c_str(), nullptr, false);

    const ImVec2 position(position_pair.first - (dirrection ? -node_size.x : bound.x + offset * 2), position_pair.second);

    draw_list_.AddRectFilled(position - ImVec2(offset, offset), position + bound + ImVec2(offset, offset), background_color, float(offset * 2));
    draw_list_.AddText(position, IM_COL32_WHITE, test.c_str());
  }
};

void draw_log_overlay(const NodeTree &tree, const ExecuteLog &log, const std::unordered_map<int, std::pair<float, float>> &socket_positions)
{
  ImDrawList &draw_list = *ImGui::GetWindowDrawList();

  std::unordered_set<int> linked_sockets;
  for (const auto link : tree.links) {
    linked_sockets.insert(link.first);
    linked_sockets.insert(link.second);
  }

  for (int node_index = 0; node_index < tree.nodes.size(); node_index++) {
    const NodePtr &node = tree.nodes[node_index];

    ImNodesOverlayDeclarationContext declaration(tree, node_index, log, socket_positions, linked_sockets, draw_list);
    node->declare(declaration);
  }
}

}