#include <iostream>

#include <algorithm>
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

int main()
{
  [[maybe_unused]] const auto GLFW = glfw::init();

  glfw::Window window{640, 480, "Zero Code Nodes"};

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

  zcn::register_node_types();

  zcn::NodeTree tree;
  zcn::add_node_to_tree(tree, "Text Input");

  while (!window.shouldClose()) {

    const double time = glfw::getTime();
    glClearColor((sin(time) + 1.0) / 6.0, (cos(time) + 1.0) / 5.0, (-sin(time) + 1.0) / 7.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Main form");
    ImGui::Text("Ohahola cacacola %d", 123);
    ImGui::End();

    zcn::nodes::draw(tree);

    ImGui::Render();
    glfw::pollEvents();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    window.swapBuffers();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImNodes::DestroyContext();
  ImGui::DestroyContext();

  return 0;
}