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

include(FetchContent)

# GLFW C++ wrapper
set(glfwpp_patch_1 git apply --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/deps_modyfications/glfwpp_cmake_glfw_path.diff)
set(GLFWPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(GLFWPP
  GIT_REPOSITORY https://github.com/janekb04/glfwpp
  GIT_TAG 17b0ca06799fec64f63f5006fe45209ce33aace0
  PATCH_COMMAND ${glfwpp_patch_1}
)

# ImGUI-base nodes
set(IMNODES_STANDALONE_PROJECT OFF CACHE BOOL "" FORCE)
FetchContent_Declare(imnodes
  GIT_REPOSITORY https://github.com/Nelarius/imnodes
  GIT_TAG 8563e1655bd9bb1f249e6552cc6274d506ee788b
)

FetchContent_MakeAvailable(GLFWPP imnodes)

set(EXTERNAL_DEPS OpenGL::GL GLEW::GLEW imgui::imgui Mathter::Mathter GLFWPP imnodes)

set(INTERNAL_LIB ${CMAKE_SOURCE_DIR}/src/ZCN_runtime_type.hh ${CMAKE_SOURCE_DIR}/src/runtime_type.cc)
add_library(internal_lib STATIC ${INTERNAL_LIB})

set(INTERNAL_NODES ${CMAKE_SOURCE_DIR}/src/ZCN_node.hh
                   ${CMAKE_SOURCE_DIR}/src/ZCN_node_draw.hh
                   ${CMAKE_SOURCE_DIR}/src/node.cc
                   ${CMAKE_SOURCE_DIR}/src/node_draw.cc)
add_library(internal_nodes STATIC ${INTERNAL_NODES})
target_link_libraries(internal_nodes PRIVATE internal_lib)
target_link_libraries(internal_nodes PRIVATE ${EXTERNAL_DEPS})

set(NODES_LIB ${CMAKE_SOURCE_DIR}/src/nodes/node_file_read.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_float_input.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_text_concatination.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_text_input.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_value_to_text.cc)
add_library(nodes_lib STATIC ${NODES_LIB})
target_link_libraries(nodes_lib PRIVATE internal_nodes)

target_link_libraries(Main PRIVATE ${EXTERNAL_DEPS} internal_lib nodes_lib)