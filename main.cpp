#include <iostream>

#include <algorithm>
#include <unordered_set>
#include <string>

#include "src/ZCN_node.hh"
#include "src/ZCN_node_draw.hh"

/*

#define _USE_MATH_DEFINES
#include <math.h>

*/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "imnodes.h"

#include <GL/glew.h>

#include <glfwpp/glfwpp.h>

/*

#undef MATHTER_ENABLE_SIMD

#include <Mathter/Vector.hpp>
#include <Mathter/Matrix.hpp>

namespace math = mathter;

using float2 = math::Vector<float, 2>;
using float3 = math::Vector<float, 3>;

using float3x3 = math::Matrix<float, 3, 3>;

*/

static const std::string glsl_version = "#version 130";

struct MetaTree {
  zcn::TreePtr tree = zcn::new_tree();
  std::string name;
  std::string internal_name;
  ImNodesEditorContext *context = ImNodes::EditorContextCreate();

  MetaTree(std::string custom_name, std::string custom_internal_name): name(std::move(custom_name)), internal_name(std::move(custom_internal_name)) {}

  MetaTree(MetaTree &&other) : tree(std::move(other.tree)), name(std::move(other.name)), internal_name(std::move(other.internal_name)), context(other.context)
  {
    other.context = nullptr;
  }
  
  MetaTree &operator =(MetaTree &&other)
  {
    this->~MetaTree();
    new(this) MetaTree(std::move(other));
    return *this;
  }
  
  ~MetaTree()
  {
    if (context != nullptr) {
      ImNodes::EditorContextFree(context);
    }
  }
};

int main()
{
  [[maybe_unused]] const auto GLFW = glfw::init();

  glfw::Window window(640, 480, "Zero Code Nodes");

  glfw::makeContextCurrent(window);

  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  ImNodes::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  ImFontConfig font_config; 
  font_config.OversampleH = 1;
  font_config.OversampleV = 1; 
  font_config.PixelSnapH = 1;

  static const ImWchar ranges[] =
  {
    0x0020, 0x00FF, // Basic Latin + Latin Supplement
    0x0400, 0x044F, // Cyrillic
    0,
  };

  io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 16.0f, &font_config, ranges);

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version.c_str());

  ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);

  std::unordered_set<std::string> tree_names;
  std::unordered_set<std::string> internal_tree_names;

  const auto new_tree_name = [](std::unordered_set<std::string> &tree_names, std::string start) -> std::string {
    if (tree_names.insert(start).second) {
      return std::move(start);
    }
    const std::string separator = (start.back() == '.' ? "" : ".");
    int iter = 1;
    while (!tree_names.insert(start + separator + std::to_string(iter)).second) {
      iter++;
    }
    return start + separator + std::to_string(iter);
  };

  zcn::register_node_types();
  std::vector<MetaTree> session;
  session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));
  session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));
  session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));

  window.dropEvent.setCallback([&](glfw::Window &/*window*/, const std::vector<const char*> &list) {
    for (const char *path : list) {
      // zcn::add_nodes_for_path(tree, std::string(path));
    }
  });

  while (!window.shouldClose()) {

    const double time = glfw::getTime();
    glClearColor((sin(time) + 1.0) / 6.0, (cos(time) + 1.0) / 5.0, (-sin(time) + 1.0) / 7.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Программы", nullptr, ImGuiWindowFlags_MenuBar);
    
    const MetaTree *tree_to_delete = nullptr;
    const MetaTree *new_tree = nullptr;

    if (ImGui::BeginListBox("Список программ"))
    {
      if (ImGui::Button("Добавить")) {
        session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));
        new_tree = &session.back();
      }

      for (MetaTree &tree : session) {

        char buffer[100];
        std::strncpy(buffer, tree.name.c_str(), std::min<int>(sizeof(buffer), tree.name.size() + 1));
        if (const std::string name_input_label = "###Name" + tree.internal_name;
            ImGui::InputText(name_input_label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags())) {
          tree_names.erase(tree.name);
          tree.name = new_tree_name(tree_names, std::string(std::string_view(buffer)));
        }

        if (const std::string delete_button_label = "Удалить###Delete" + tree.internal_name; ImGui::Button(delete_button_label.c_str())) {
          tree_to_delete = &tree;
        }

        ImGui::Separator();
      }
      ImGui::EndListBox();
    }

    ImGui::End();

    for (MetaTree &tree : session) {
      ImGui::Begin(tree.internal_name.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
      if (new_tree == &tree) {
        ImGui::SetWindowPos({500, 500});
        ImGui::SetWindowSize({500, 500});
      }

      ImGui::BeginMenuBar();
      char buffer[100];
      std::strncpy(buffer, tree.name.c_str(), std::min<int>(sizeof(buffer), tree.name.size() + 1));
      if (ImGui::InputText("", buffer, sizeof(buffer), ImGuiInputTextFlags(), nullptr, nullptr)) {
        tree_names.erase(tree.name);
        tree.name = new_tree_name(tree_names, std::string(std::string_view(buffer)));
      }
      ImGui::EndMenuBar();

      ImNodes::EditorContextSet(tree.context);
      zcn::nodes::draw(*tree.tree);
      ImGui::End();
    }

    if (tree_to_delete != nullptr) {
      tree_names.erase(tree_to_delete->name);
      session.erase(session.begin() + std::distance<const MetaTree *>(&session[0], tree_to_delete));
    }

    ImGui::Render();
    glfw::pollEvents();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    window.swapBuffers();
  }

  ImNodes::PopAttributeFlag();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImNodes::DestroyContext();
  ImGui::DestroyContext();

  return 0;
}