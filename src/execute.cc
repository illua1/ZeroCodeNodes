#pragma once

#include <filesystem>
#include <string>
#include <algorithm>
#include <vector>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>

#include "ZCN_execute.hh"
#include "ZCN_node.hh"
#include "ZCN_runtime_type.hh"

namespace zcn {

class EStateUpdateDeclarationContext : public DeclarationContext {
  int node_uid_;
  std::vector<int> &uids_;
  int index_ = 0;
  int output_ = 0;
 public:
  EStateUpdateDeclarationContext(const int node_uid, std::vector<int> &uids) : node_uid_(node_uid), uids_(uids) {
    uids_.clear();
  }
  ~EStateUpdateDeclarationContext() override
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

static void execute_from(const std::vector<std::vector<int>> &node_to_input_nodes,
                         const int start_node,
                         std::vector<bool> &visited_nodes,
                         std::vector<int> &r_nodes_order)
{
  visited_nodes[start_node] = true;
  
  for (const int input_node_index : node_to_input_nodes[start_node]) {
    if (input_node_index == -1) {
      continue;
    }
    if (visited_nodes[input_node_index]) {
      continue;
    }
    execute_from(node_to_input_nodes, input_node_index, visited_nodes, r_nodes_order);
  }
  
  r_nodes_order.push_back(start_node);
}

BaseExecutionContext::ExecutionDeclarationContext::ExecutionDeclarationContext(std::unordered_map<std::string, int> &input_uids_for_name,
                          std::unordered_map<std::string, int> &output_uids_for_name,
                          const std::vector<int> &socket_uids) : input_uids_for_name_(input_uids_for_name),
                                                                 output_uids_for_name_(output_uids_for_name),
                                                                 socket_uids_(socket_uids) {}

void BaseExecutionContext::ExecutionDeclarationContext::add_input(const DataType /*type*/, std::string name)
{
 input_uids_for_name_[name] = socket_uids_[input_uid++];
}

void BaseExecutionContext::ExecutionDeclarationContext::add_output(const DataType /*type*/, std::string name)
{
 output_uids_for_name_[name] = socket_uids_[input_uid++];
}

void BaseExecutionContext::ExecutionDeclarationContext::add_data(const DataType /*type*/, std::string /*name*/) {}

BaseExecutionContext::BaseExecutionContext(const int node_uid,
                       const std::unordered_map<std::string, RData> &tree_values,
                       const std::unordered_map<int, int> &input_uid_to_output_uid,
                       const std::unordered_map<int, int> &node_index_from_output_uid,
                       const std::vector<int> &node_uids,
                       const std::vector<int> &node_socket_uids,
                       std::unordered_map<std::string, RData> &execution_values,
                       const NodePtr &node,
                       std::vector<BaseProvider *> providers)
                       : node_uid_(node_uid),
                         tree_values_(tree_values),
                         input_uid_to_output_uid_(input_uid_to_output_uid),
                         node_index_from_output_uid_(node_index_from_output_uid),
                         node_uids_(node_uids),
                         execution_values_(execution_values),
                         providers_(std::move(providers))
{
  ExecutionDeclarationContext topology(input_uids_for_name_, output_uids_for_name_, node_socket_uids);
  node->declare(topology);
}

RData BaseExecutionContext::get_input(DataType type, const std::string name)
{
  const int socket_uid = input_uids_for_name_[name];
  if (input_uid_to_output_uid_.find(socket_uid) == input_uid_to_output_uid_.end()) {
    const std::string socket_path = node_socket_to_path(node_uid_, socket_uid);
    
    if (tree_values_.find(socket_path) == tree_values_.end()) {
      execution_values_[socket_path] = defult_value(type);
      return defult_value(type);
    }
    
    execution_values_[socket_path] = tree_values_.at(socket_path);
    
    return tree_values_.at(socket_path);
  }
  const int output_uid = input_uid_to_output_uid_.at(socket_uid);
  if (node_index_from_output_uid_.find(output_uid) == node_index_from_output_uid_.end()) {
    return {};
  }
  const int source_node_index = node_index_from_output_uid_.at(output_uid);
  const int source_node_uid = node_uids_[source_node_index];
  const std::string socket_path = node_socket_to_path(source_node_uid, output_uid);
  
  RData recived_value = ensure_type(type, execution_values_.at(socket_path));
  
  {
    const std::string this_socket_path = node_socket_to_path(node_uid_, socket_uid);
    execution_values_[this_socket_path] = recived_value;
  }
  
  
  return recived_value;
}

RData BaseExecutionContext::get_data(DataType type, const std::string name)
{
  const std::string node_path = node_value_to_path(node_uid_, name);
  return tree_values_.at(node_path);
}

void BaseExecutionContext::set_output(const std::string name, RData value)
{
  const int socket_uid = output_uids_for_name_.at(name);
  const std::string socket_path = node_socket_to_path(node_uid_, socket_uid);
  execution_values_[socket_path] = std::move(value);
}

std::vector<BaseProvider *> &BaseExecutionContext::context_providers()
{
  return providers_;
}

class TopologyDeclarationContext : public DeclarationContext {
  int input_uid = 0;
  std::vector<int> &input_uids_;
  std::vector<int> &outout_uids_;
  const std::vector<int> &socket_uids_;

 public:
  TopologyDeclarationContext(std::vector<int> &input_uids, std::vector<int> &output_uids, const std::vector<int> &socket_uids) :
    input_uids_(input_uids), outout_uids_(output_uids), socket_uids_(socket_uids) {}
  ~TopologyDeclarationContext() override = default;

  void add_input(const DataType /*type*/, std::string /*name*/) override
  {
    input_uids_.push_back(socket_uids_[input_uid++]);
  }

  void add_output(const DataType /*type*/, std::string /*name*/) override
  {
    outout_uids_.push_back(socket_uids_[input_uid++]);
  }

  void add_data(const DataType /*type*/, std::string /*name*/) override {}
};

void execute(const TreePtr &tree, ExecuteLog &log, std::vector<BaseProvider *> providers)
{
  for (int node_index = 0; node_index < tree->nodes_uid.size(); node_index++) {
      EStateUpdateDeclarationContext ensure_socket_uid(tree->nodes_uid[node_index], tree->node_sockets_uid[node_index]);
      tree->nodes[node_index]->declare(ensure_socket_uid);
    }
  
  std::vector<std::vector<int>> nodes_inputs;
  std::vector<std::vector<int>> nodes_outputs;
  nodes_inputs.resize(tree->nodes_uid.size());
  nodes_outputs.resize(tree->nodes_uid.size());

  std::unordered_map<int, int> node_index_from_output_uid;

  for (int node_index = 0; node_index < tree->nodes_uid.size(); node_index++) {
    TopologyDeclarationContext topology(nodes_inputs[node_index], nodes_outputs[node_index], tree->node_sockets_uid[node_index]);
    tree->nodes[node_index]->declare(topology);
    
    for (const int output_uid : nodes_outputs[node_index]) {
      node_index_from_output_uid[output_uid] = node_index;
    }
  }

  std::vector<int> output_nodes;
  for (int node_index = 0; node_index < tree->nodes_uid.size(); node_index++) {
    if (nodes_outputs[node_index].empty()) {
      output_nodes.push_back(node_index);
    }
  }

  std::unordered_map<int, int> input_uid_to_output_uid;
  for (const auto [output, input] : tree->links) {
    input_uid_to_output_uid[input] = output;
  }

  std::vector<std::vector<int>> node_to_input_nodes;
  node_to_input_nodes.resize(tree->nodes_uid.size());
  for (int node_index = 0; node_index < tree->nodes_uid.size(); node_index++) {
    node_to_input_nodes[node_index].resize(nodes_inputs[node_index].size());
    std::transform(nodes_inputs[node_index].begin(), nodes_inputs[node_index].end(), node_to_input_nodes[node_index].begin(), [&](const int input_socket_uid) {
      if (input_uid_to_output_uid.find(input_socket_uid) == input_uid_to_output_uid.end()) {
        return -1;
      }
      const int output_uid = input_uid_to_output_uid[input_socket_uid];
      if (node_index_from_output_uid.find(output_uid) == node_index_from_output_uid.end()) {
        return -1;
      }
      return node_index_from_output_uid[output_uid];
    });
  }

  std::vector<int> ordered_node_indices;
  {
    std::vector<bool> visited_nodes;
    visited_nodes.resize(tree->nodes_uid.size());
    std::generate(visited_nodes.begin(), visited_nodes.end(), [&]() { return false; });
    for (const int output_index : output_nodes) {
      execute_from(node_to_input_nodes, output_index, visited_nodes, ordered_node_indices);
    }
  }

  std::unordered_map<std::string, RData> socket_values;
  for (const int node_index : ordered_node_indices) {
    const int node_uid = tree->nodes_uid[node_index];

    std::vector<BaseProvider *> sub_providers = providers;
    NodeNameProvider node_name = NodeNameProvider{tree->node_names[node_index]};

    

    sub_providers.insert(sub_providers.begin(), &node_name);

    BaseExecutionContext context(node_uid,
                                 tree->values,
                                 input_uid_to_output_uid,
                                 node_index_from_output_uid,
                                 tree->nodes_uid,
                                 tree->node_sockets_uid[node_index],
                                 socket_values,
                                 tree->nodes[node_index],
                                 sub_providers);
    tree->nodes[node_index]->execute(context);
  }

  log.socket_value = std::move(socket_values);
}

std::string normalize_path(const std::string& messyPath) {
    std::filesystem::path path(messyPath);
    std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
    std::string npath = canonicalPath.make_preferred().string();
    return npath;
}

RData FileSystemProvider::get_from_path(std::string path) const
{
  const std::string normalized_path = normalize_path(path);

  if (!std::filesystem::exists(normalized_path)) {
    return "";
  }

  std::stringstream buffer;
  std::ifstream file_reader(path);

  buffer << file_reader.rdbuf();
  return buffer.str();
}

void FileSystemProvider::set_for_path(std::string path, RData data)
{
  const std::string normalized_path = normalize_path(path);

  std::ofstream opened_file(normalized_path);
  if (opened_file.is_open()) {
    opened_file << std::get<std::string>(ensure_type(DataType::Text, data));
    opened_file.close();
  }
}

RData VirtualFileSystemProvider::get_from_path(std::string path) const
{
  const std::string normalized_path = normalize_path(path);

  if (!std::filesystem::exists(normalized_path)) {
    return std::string("");
  }

  if (this->file_overide.find(path) != this->file_overide.end()) {
    return this->file_overide.at(path);
  }

  std::stringstream buffer;
  std::ifstream file_reader(path);

  buffer << file_reader.rdbuf();
  std::string data = buffer.str();

  const_cast<VirtualFileSystemProvider *>(this)->file_overide[path] = data;

  return data;
}

void VirtualFileSystemProvider::set_for_path(std::string path, RData data)
{
  const std::string normalized_path = normalize_path(path);

  if (!std::filesystem::exists(normalized_path)) {
    return;
  }

  const_cast<VirtualFileSystemProvider *>(this)->file_overide[path] = std::get<std::string>(ensure_type(DataType::Text, data));
}

RData SubTreeExecutionProvider::get_input(DataType type, const std::string name) const
{
  return ensure_type(type, input_for_node_name_.at(name));
}

void SubTreeExecutionProvider::set_output(const std::string name, RData value)
{
  output_for_node_name_[name] = std::move(value);
}

// static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
// {
//   ImGui::SetCurrentContext(secondary_context);
// 
//   ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
// 
//   ImGui::SetCurrentContext(g);
// }
// 
// static void set_cursor_pos_callback(GLFWwindow* window, double x, double y)
// {
//   ImGui::SetCurrentContext(secondary_context);
// 
//   ImGui_ImplGlfw_CursorPosCallback(window, x, y);
// 
//   ImGui::SetCurrentContext(main_context);
// }

GUIExecutionProvider::WindowsProvider::WindowsProvider(GUIExecutionProvider &owner, const std::string &name) : owner_(owner),
    data_(glfw::Window(100, 50, name.c_str())), gui_context_(ImGui::CreateContext())
{
  glfw::makeContextCurrent(data_);

  ImGui::SetCurrentContext(gui_context_);

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  ImGui::GetStyle().WindowRounding = 0.0f;

  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

  ImGui_ImplGlfw_InitForOpenGL(data_, true);
  ImGui_ImplOpenGL3_Init();

  static const ImWchar ranges[] =
  {
    0x0020, 0x00FF, // Basic Latin + Latin Supplement
    0x0400, 0x044F, // Cyrillic
    0,
  };

  ImFontConfig font_config; 
  font_config.OversampleH = 1;
  font_config.OversampleV = 1; 
  font_config.PixelSnapH = 1;

  io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 16.0f, &font_config, ranges);

  // glfwSetMouseButtonCallback(window, []);
  // glfwSetCursorPosCallback(window, set_cursor_pos_callback);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("Question Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
}

GUIExecutionProvider::WindowsProvider::~WindowsProvider()
{
  ImGui::End();
  ImGui::EndFrame();
  ImGui::Render();
  glfw::pollEvents();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  const_cast<glfw::Window &>(data_).swapBuffers();
  if (gui_context_ != nullptr) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(gui_context_);
  }

  ImGui::SetCurrentContext(owner_.gui_context_);
}

bool GUIExecutionProvider::WindowsProvider::button_try(const std::string name) const
{
  const std::string label = name + "###" + name + __func__ + ";";
  return ImGui::Button(label.c_str());
}

std::string GUIExecutionProvider::WindowsProvider::text_try(const std::string name) const
{
  const std::string label = name + "###" + name + __func__ + ";";
  
  std::string &value = const_cast<GUIExecutionProvider::WindowsProvider *>(this)->text_map_[name];
  
  char buffer[500];
  std::strncpy(buffer, value.c_str(), std::min<int>(sizeof(buffer), value.size() + 1));
  if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags())) {
    value = std::string(std::string_view(buffer));
  }

  return value;
}

GUIExecutionProvider::WindowsProvider GUIExecutionProvider::get_window(const std::string name) const
{
  return WindowsProvider::WindowsProvider(const_cast<GUIExecutionProvider &>(*this), name);
}

bool GUIExecutionProvider::WindowsProvider::is_open() const
{
  ImGui::End();
  ImGui::EndFrame();

  ImGui::Render();
  glfw::pollEvents();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  const_cast<glfw::Window &>(data_).swapBuffers();

  ImGui::SetCurrentContext(const_cast<ImGuiContext *>(gui_context_));

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  const auto [x, y] = data_.getSize();
  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImVec2(x, y));

  ImGui::Begin("Question Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

  return !data_.shouldClose();
}

}