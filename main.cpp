#include <iostream>

#include <algorithm>
#include <string>

/*

#define _USE_MATH_DEFINES
#include <math.h>

*/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfw::pollEvents();
    window.swapBuffers();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  return 0;
}