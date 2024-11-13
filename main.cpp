#include <iostream>

#include <algorithm>
#include <string>

#include "src/ZCN_node.hh"

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

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version.c_str());

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

    ImNodes::BeginNodeEditor();
    

    ImNodes::BeginNode(0);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Halo ainge ladu");
    ImNodes::EndNodeTitleBar();

    ImGui::Text("AAAAAA");
    ImGui::Dummy(ImVec2(80.0f, 45.0f));

    ImNodes::BeginInputAttribute(1);
    ImGui::Text("AAA");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(2);
    ImGui::Text("output pin");
    ImNodes::EndOutputAttribute();
    
    ImNodes::EndNode();

    ImNodes::BeginNode(1);
    ImGui::Text("AAAAAA");
    ImGui::Dummy(ImVec2(80.0f, 45.0f));

    ImNodes::BeginInputAttribute(3);
    ImGui::Text("AAA");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(4);
    ImGui::Text("output pin");
    ImNodes::EndOutputAttribute();
    
    ImNodes::EndNode();
    
    ImNodes::EndNodeEditor();


    glfw::pollEvents();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    window.swapBuffers();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImNodes::DestroyContext();
  ImGui::DestroyContext();

  return 0;
}