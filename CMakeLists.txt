cmake_minimum_required(VERSION 3.31)

include(F:/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(ZeroCodeNodes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-DO3 -DNDEBUG /EHsc")


add_executable(Main main.cpp)

find_package(Mathter REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui REQUIRED)
find_package(GLEW REQUIRED)

# GLFW C++ wrapper
include(FetchContent)
set(glfwpp_patch_1 git apply ${CMAKE_CURRENT_SOURCE_DIR}/deps_modyfications/glfwpp_cmake_glfw_path.diff)
set(GLFWPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(GLFWPP
  GIT_REPOSITORY https://github.com/janekb04/glfwpp
  GIT_TAG 17b0ca06799fec64f63f5006fe45209ce33aace0
  PATCH_COMMAND ${glfwpp_patch_1}
)
FetchContent_MakeAvailable(GLFWPP)

target_link_libraries(Main PRIVATE OpenGL::GL)
target_link_libraries(Main PRIVATE GLEW::GLEW)
target_link_libraries(Main PRIVATE imgui::imgui)
target_link_libraries(Main PRIVATE Mathter::Mathter)
target_link_libraries(Main PRIVATE GLFWPP)