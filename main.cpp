#include <iostream>

#include <algorithm>
#include <unordered_set>
#include <numeric>
#include <string>
#include <iostream>
#include <fstream>

#include "src/ZCN_execute.hh"
#include "src/ZCN_node.hh"
#include "src/ZCN_node_json.hh"
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

#include <nlohmann/json.hpp>

#include "shlobj.h"
#include "BaseTsd.h"

static const std::string glsl_version = "#version 130";

struct MetaTree {
  zcn::TreePtr tree = zcn::new_tree();
  std::string name;
  std::string internal_name;
  ImNodesEditorContext *context = ImNodes::EditorContextCreate();

  zcn::ExecuteLog view_log;

  MetaTree(std::string custom_name, std::string custom_internal_name): name(std::move(custom_name)), internal_name(std::move(custom_internal_name)) {}

  MetaTree(MetaTree &&other) : tree(std::move(other.tree)), name(std::move(other.name)), internal_name(std::move(other.internal_name)), context(other.context), view_log(std::move(other.view_log))
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

static std::string local_storage_path()
{
  TCHAR szPath[MAX_PATH];

  if(SUCCEEDED(SHGetFolderPath(NULL, 
                               CSIDL_APPDATA | CSIDL_FLAG_CREATE, 
                               NULL, 
                               0, 
                               szPath))) 
  {
    return std::string(szPath) + "\\" + "zero_code_nodes_registry.txt";
  }
  return "";
}

static void try_to_reg_ext(const std::string ext = ".zcn")
{
  std::string desc = "my file type";

  CHAR exe_path[MAX_PATH];
  GetModuleFileName(NULL, exe_path, MAX_PATH);

  std::string app = std::string(exe_path) + " %1";
  std::string action = "Open with my program";

  std::string sub = "\\shell\\";

  std::string path = ext + sub + action + "\\" + "command\\";

  HKEY hkey;
  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, ext.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS) {
    std::cerr << "Error 1\n";
    return;
  }
  RegSetValueEx(hkey, "", 0, REG_SZ, reinterpret_cast<const BYTE *>(desc.c_str()), desc.size());
  RegCloseKey(hkey);

  if(RegCreateKeyEx(HKEY_CLASSES_ROOT, path.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) != ERROR_SUCCESS) {
    std::cerr << "Error 2\n";
    return;
  }
  RegSetValueEx(hkey, "", 0, REG_SZ, reinterpret_cast<const BYTE *>(app.c_str()), app.size());

  RegCloseKey(hkey);
}

int main(int argc, char *argv[])
{
  std::cout << argc << ";\n";
  if (argc > 1) {
    std::cout << ": " << argv[1] << ";\n";
  } 

  try_to_reg_ext(".zcn");

  [[maybe_unused]] const auto GLFW = glfw::init();

  glfw::Window window(640, 480, "Zero Code Nodes");
  glfw::makeContextCurrent(window);

  IMGUI_CHECKVERSION();

  ImGuiContext *gui_context = ImGui::CreateContext();
  ImGui::SetCurrentContext(gui_context);
  ImNodesContext *nodes_context = ImNodes::CreateContext();

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

  ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick | ImNodesAttributeFlags_EnableLinkCreationOnSnap);

  std::unordered_set<std::string> tree_names;
  tree_names.insert("");
  std::unordered_set<std::string> internal_tree_names;

  std::unordered_set<std::string> local_storage_tree_names;
  local_storage_tree_names.insert("");
  std::unordered_set<std::string> local_storage_internal_tree_names;

  const auto new_tree_name = [](std::unordered_set<std::string> &tree_names, std::string start) -> std::string {
    start = start == "" ? "AAAA" : start;
    if (tree_names.find(start) == tree_names.end()) {
      tree_names.insert(start);
      return std::move(start);
    }
    const std::string separator = (start.back() == '.' ? "" : ".");
    int iter = 1;
    while (tree_names.find(start + separator + std::to_string(iter)) != tree_names.end()) {
      iter++;
    }
    tree_names.insert(start + separator + std::to_string(iter));
    return start + separator + std::to_string(iter);
  };

  zcn::register_node_types();
  std::vector<MetaTree> session;
  
  std::vector<MetaTree> local_storage_session;

  {
    std::ifstream autosave_file("autosave.zcn");
    if (autosave_file.is_open()) {
      std::string value;
      autosave_file >> value;
      nlohmann::json load = nlohmann::json::parse(value);

      for (auto item : load) {
        const std::string name = item["name"].get<std::string>();
        const std::string internal_name = item["internal_name"].get<std::string>();
        tree_names.insert(name);
        internal_tree_names.insert(internal_name);
        MetaTree tree(name, internal_name);
        tree.tree = std::move(zcn::tree_from_json(item["topology"].dump()));
        session.push_back(std::move(tree));
      }

      std::cout << load;

      autosave_file.close();

    } else {
      session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));
      session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));
      session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));
    }
  }

  {
    std::ifstream autosave_file(local_storage_path());
    if (autosave_file.is_open()) {
      std::string value;
      autosave_file >> value;
      nlohmann::json load = nlohmann::json::parse(value);

      for (auto item : load) {
        const std::string name = item["name"].get<std::string>();
        const std::string internal_name = item["internal_name"].get<std::string>();
        local_storage_tree_names.insert(name);
        local_storage_internal_tree_names.insert(internal_name);
        MetaTree tree(name, internal_name);
        tree.tree = std::move(zcn::tree_from_json(item["topology"].dump()));
        local_storage_session.push_back(std::move(tree));
      }

      std::cout << load;

      autosave_file.close();

    }
  }

  window.dropEvent.setCallback([&](glfw::Window &/*window*/, const std::vector<const char*> &list) {
    for (const char *path : list) {
      // zcn::add_nodes_for_path(tree, std::string(path));
    }
  });

  zcn::CacheProvider execution_ui_cache;

  while (!window.shouldClose()) {

    const double time = glfw::getTime();
    glClearColor((sin(time) + 1.0) / 6.0, (cos(time) + 1.0) / 5.0, (-sin(time) + 1.0) / 7.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Реестр", nullptr, ImGuiWindowFlags_MenuBar)) {
      const MetaTree *tree_to_delete = nullptr;
      const MetaTree *tree_to_copy = nullptr;

      if (ImGui::BeginMenuBar()) {
        if (ImGui::Button("Загрузить")) {
        }
        ImGui::EndMenuBar();
      }

      if (ImGui::BeginListBox("###Список программ", {-1.0f, -1.0f})) {
        for (MetaTree &tree : local_storage_session) {

          const auto item_draw_func = [&]() {
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
          };

          ImGui::BeginGroup();
          item_draw_func();
          ImGui::EndGroup();

          if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            const MetaTree *ptr = &tree;
            ImGui::SetDragDropPayload("ИзРег##Tree", &ptr, sizeof(&ptr), ImGuiCond_Once);
            item_draw_func();
            ImGui::EndDragDropSource();
          }

          ImGui::Separator();
        }
        ImGui::EndListBox();
      }

      if (ImGui::BeginDragDropTarget()) {
        const auto payload = ImGui::AcceptDragDropPayload("ВРег##Tree");
        if (payload != nullptr) {
          const MetaTree *dragged_tree = *(MetaTree **)payload->Data;
          zcn::TreePtr copy = zcn::tree_from_json(zcn::tree_to_json(dragged_tree->tree));
          MetaTree meta_copy(new_tree_name(local_storage_tree_names, dragged_tree->name), new_tree_name(local_storage_internal_tree_names, dragged_tree->internal_name));
          meta_copy.tree = std::move(copy);

          local_storage_session.push_back(std::move(meta_copy));
        }
        ImGui::EndDragDropTarget();
      }

      if (tree_to_delete != nullptr) {
        tree_names.erase(tree_to_delete->name);
        local_storage_session.erase(local_storage_session.begin() + std::distance<const MetaTree *>(&local_storage_session[0], tree_to_delete));
      }

      ImGui::End();
    }

    const MetaTree *new_tree = nullptr;
    if (ImGui::Begin("Программы", nullptr, ImGuiWindowFlags_MenuBar)) {
      const MetaTree *tree_to_delete = nullptr;
      const MetaTree *tree_to_copy = nullptr;

      if (ImGui::BeginMenuBar()) {
        if (ImGui::Button("Добавить")) {
          session.push_back(std::move(MetaTree(new_tree_name(tree_names, "Дерево"), new_tree_name(internal_tree_names, "Tree"))));
          new_tree = &session.back();
        }
        if (ImGui::Button("Загрузить")) {
        }
        ImGui::EndMenuBar();
      }

      if (ImGui::BeginListBox("###Список программ", {-1.0f, -1.0f})) {
        for (MetaTree &tree : session) {

          const auto item_draw_func = [&]() {
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

            if (const std::string delete_button_label = "Копировать###Copy" + tree.internal_name; ImGui::Button(delete_button_label.c_str())) {
              tree_to_copy = &tree;
            }

            if (const std::string delete_button_label = "Сохранить###Save" + tree.internal_name; ImGui::Button(delete_button_label.c_str())) {
              // std::cout << zcn::tree_to_json(tree.tree) << ";\n";
            }
          };

          ImGui::BeginGroup();
          item_draw_func();
          ImGui::EndGroup();
          if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            const MetaTree *ptr = &tree;
            ImGui::SetDragDropPayload("ВРег##Tree", &ptr, sizeof(&ptr), ImGuiCond_Once);
            item_draw_func();
            ImGui::EndDragDropSource();
          }

          ImGui::Separator();
        }
        ImGui::EndListBox();
      }

      if (ImGui::BeginDragDropTarget()) {
        const auto payload = ImGui::AcceptDragDropPayload("ИзРег##Tree");
        if (payload != nullptr) {
          const MetaTree *dragged_tree = *(MetaTree **)payload->Data;
          zcn::TreePtr copy = zcn::tree_from_json(zcn::tree_to_json(dragged_tree->tree));
          MetaTree meta_copy(new_tree_name(tree_names, dragged_tree->name), new_tree_name(internal_tree_names, dragged_tree->internal_name));
          meta_copy.tree = std::move(copy);

          session.push_back(std::move(meta_copy));
          new_tree = &session.back();
        }
        ImGui::EndDragDropTarget();
      }

      if (tree_to_delete != nullptr) {
        tree_names.erase(tree_to_delete->name);
        session.erase(session.begin() + std::distance<const MetaTree *>(&session[0], tree_to_delete));
      }

      if (tree_to_copy != nullptr) {
        zcn::TreePtr copy = zcn::tree_from_json(zcn::tree_to_json(tree_to_copy->tree));
        MetaTree meta_copy(new_tree_name(tree_names, tree_to_copy->name),
                           new_tree_name(internal_tree_names, tree_to_copy->internal_name));
        meta_copy.tree = std::move(copy);

        session.push_back(std::move(meta_copy));
        new_tree = &session.back();
      }

      ImGui::End();
    }

    std::vector<std::string> trees_names = {""};
    std::unordered_map<std::string, zcn::TreePtr> trees_by_name;
    for (MetaTree &tree : session) {
      trees_names.push_back(tree.name);
      trees_by_name[tree.name] = tree.tree;
    }
    std::sort(trees_names.begin(), trees_names.end());
    zcn::set_trees_context(&trees_by_name);

    for (MetaTree &tree : session) {
      if (ImGui::Begin(tree.internal_name.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar)) {
        if (new_tree == &tree) {
          ImGui::SetWindowPos({500, 500});
          ImGui::SetWindowSize({500, 500});
        }

        if (ImGui::BeginMenuBar()) {
          char buffer[100];
          ImGui::Dummy(ImVec2(90.0f, 0.0f));
          std::strncpy(buffer, tree.name.c_str(), std::min<int>(sizeof(buffer), tree.name.size() + 1));
          if (ImGui::InputText("", buffer, sizeof(buffer), ImGuiInputTextFlags(), nullptr, nullptr)) {
            tree_names.erase(tree.name);
            tree.name = new_tree_name(tree_names, std::string(std::string_view(buffer)));
          }
          ImGui::Dummy(ImVec2(90.0f, 0.0f));
          ImGui::EndMenuBar();
        }

        ImNodes::EditorContextSet(tree.context);

        std::unordered_map<int, std::pair<float, float>> socket_positions;
        zcn::nodes::draw(*tree.tree, trees_names, socket_positions);
        zcn::nodes::draw_log_overlay(*tree.tree, tree.view_log, socket_positions);

        ImGui::End();
      }
    }

    ImGui::Render();
    glfw::pollEvents();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    window.swapBuffers();

    for (MetaTree &tree : session) {
      zcn::VirtualFileSystemProvider side_effect_provider;
      zcn::GUIExecutionProvider gui_provider(window, gui_context);
      std::vector<zcn::BaseProvider *> providers = {&gui_provider, &execution_ui_cache};
      zcn::execute(tree.tree, tree.view_log, providers);
    }

    glfw::makeContextCurrent(window);
    ImGui::SetCurrentContext(gui_context);
    ImNodes::SetCurrentContext(nodes_context);
  }

  {
    std::ofstream autosave_file("autosave.zcn");
    if (autosave_file.is_open()) {
      nlohmann::json save;
      for (MetaTree &tree : session) {
        nlohmann::json tree_info;
        tree_info["name"] = tree.name;
        tree_info["internal_name"] = tree.internal_name;
        tree_info["topology"] = nlohmann::json::parse(zcn::tree_to_json(tree.tree));
        save.push_back(tree_info);
      }
      autosave_file << save.dump();
      autosave_file.close();

    }
  }
  
  {
    std::ofstream autosave_file(local_storage_path());
    if (autosave_file.is_open()) {
      nlohmann::json save;
      for (MetaTree &tree : local_storage_session) {
        nlohmann::json tree_info;
        tree_info["name"] = tree.name;
        tree_info["internal_name"] = tree.internal_name;
        tree_info["topology"] = nlohmann::json::parse(zcn::tree_to_json(tree.tree));
        save.push_back(tree_info);
      }
      autosave_file << save.dump();
      autosave_file.close();

    }
  }

  ImNodes::PopAttributeFlag();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImNodes::DestroyContext();
  ImGui::DestroyContext();

  return 0;
}