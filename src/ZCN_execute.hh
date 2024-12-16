#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "ZCN_node.hh"
#include "ZCN_runtime_type.hh"

#include <GL/glew.h>

#include <glfwpp/glfwpp.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace zcn {

struct ExecuteLog {
  std::unordered_map<std::string, RData> socket_value;
};

class SideEffectReciver {
 public:
  virtual ~SideEffectReciver() = default;

  virtual RData get_from_path(std::string path) = 0;
  virtual void set_for_path(std::string path, RData data) = 0;
};

class FileSystemProvider : public SideEffectReciver {
 public:
  ~FileSystemProvider() override = default;

  RData get_from_path(std::string path) override;
  void set_for_path(std::string path, RData data) override;
};

class VirtualFileSystemProvider : public SideEffectReciver {
 public:
  std::unordered_map<std::string, std::string> file_overide;

  ~VirtualFileSystemProvider() override = default;

  RData get_from_path(std::string path) override;
  void set_for_path(std::string path, RData data) override;
};

void execute(const TreePtr &tree, ExecuteLog &log, std::vector<BaseProvider *> providers);

class SubTreeExecutionProvider : public BaseProvider {
  const std::unordered_map<std::string, RData> &input_for_node_name_;
  std::unordered_map<std::string, RData> &output_for_node_name_;

 public:
  SubTreeExecutionProvider(const std::unordered_map<std::string, RData> &input_for_node_name,
                           std::unordered_map<std::string, RData> &output_for_node_name)
                                      : input_for_node_name_(input_for_node_name), output_for_node_name_(output_for_node_name) {}
  ~SubTreeExecutionProvider() override = default;

  RData get_input(DataType type, const std::string name) const;
  void set_output(const std::string name, RData value);
};

class NodeNameProvider : public BaseProvider {
 public:
  NodeNameProvider(std::string node_name) : name(std::move(node_name)) {}
  ~NodeNameProvider() override = default;

  std::string name;
};

class BaseExecutionContext : public ExecutionContext {
  const int node_uid_;
  
  const std::unordered_map<std::string, RData> &tree_values_;
  const std::unordered_map<int, int> &input_uid_to_output_uid_;
  const std::unordered_map<int, int> &node_index_from_output_uid_;

  const std::vector<int> &node_uids_;

  std::unordered_map<std::string, RData> &execution_values_;

  std::unordered_map<std::string, int> input_uids_for_name_;
  std::unordered_map<std::string, int> output_uids_for_name_;

  std::vector<BaseProvider *> providers_;

   class ExecutionDeclarationContext : public DeclarationContext {
     int input_uid = 0;
     std::unordered_map<std::string, int> &input_uids_for_name_;
     std::unordered_map<std::string, int> &output_uids_for_name_;
     const std::vector<int> &socket_uids_;

    public:
     ExecutionDeclarationContext(std::unordered_map<std::string, int> &input_uids_for_name,
                                std::unordered_map<std::string, int> &output_uids_for_name,
                                const std::vector<int> &socket_uids);
     ~ExecutionDeclarationContext() override = default;

     void add_input(const DataType /*type*/, std::string name) override;

     void add_output(const DataType /*type*/, std::string name) override;

     void add_data(const DataType /*type*/, std::string /*name*/) override;
   };

 public:
  BaseExecutionContext(const int node_uid,
                       const std::unordered_map<std::string, RData> &tree_values,
                       const std::unordered_map<int, int> &input_uid_to_output_uid,
                       const std::unordered_map<int, int> &node_index_from_output_uid,
                       const std::vector<int> &node_uids,
                       const std::vector<int> &node_socket_uids,
                       std::unordered_map<std::string, RData> &execution_values,
                       const NodePtr &node,
                       std::vector<BaseProvider *> providers);
 
  ~BaseExecutionContext() override = default;

  RData get_input(DataType type, const std::string name) override;

  RData get_data(DataType type, const std::string name) override;

  void set_output(const std::string name, RData value) override;
  
  std::vector<BaseProvider *> &context_providers() override;
};

class TreeExecutionContext {
 public:
  virtual RData get_input(DataType type, const std::string name) = 0;
  virtual void set_output(const std::string name, RData value) = 0;
};

class CacheProvider : public BaseProvider {
 public:
  std::unordered_map<std::string, RData> data;
};

class GUIExecutionProvider : public BaseProvider {
  glfw::Window &context_window_;
  ImGuiContext *gui_context_;
 public:

  class WindowsProvider {
    GUIExecutionProvider &owner_;
    glfw::Window data_;
    ImGuiContext *gui_context_;
   public:
    WindowsProvider(GUIExecutionProvider &owner, const std::string &name);
    WindowsProvider(WindowsProvider &&other) : owner_(other.owner_), data_(std::move(other.data_)), gui_context_(other.gui_context_) { other.gui_context_ = nullptr; }
    ~WindowsProvider();
    bool is_open() const;
    bool button_try(const std::string name) const;
  };

  GUIExecutionProvider(glfw::Window &context_window, ImGuiContext *gui_context) : context_window_(context_window), gui_context_(gui_context) {};
  ~GUIExecutionProvider() override = default;

  WindowsProvider get_window(const std::string name) const;
};

}