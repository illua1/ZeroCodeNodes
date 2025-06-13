cmake_minimum_required(VERSION 3.29)

# set(CMAKE_BUILD_TYPE "DEBUG")
set(CMAKE_BUILD_TYPE "RELEASE")

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Windows)

# set(VCPKG_BUILD_TYPE debug)
set(VCPKG_BUILD_TYPE release)

include("E:/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(ZeroCodeNodes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-DO2 /EHsc")
add_compile_options(/Wall)

set(MSVC_ASAN ON)


add_executable(Main main.cpp)

find_package(Mathter CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)

set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)
find_package(nlohmann_json CONFIG REQUIRED)

message(STATUS "Mathter found----------: ${Mathter_FOUND}")
message(STATUS "OpenGL found---------: ${OpenGL_FOUND}")
message(STATUS "imgui found----------: ${imgui_FOUND}")
message(STATUS "GLEW found-: ${GLEW_FOUND}")

include(FetchContent)

# GLFW C++ wrapper
set(glfwpp_patch_1 git apply --whitespace=fix ${CMAKE_CURRENT_SOURCE_DIR}/deps_modyfications/glfwpp_cmake_glfw_path.diff)

FetchContent_Declare(GLFWPP
  GIT_REPOSITORY https://github.com/janekb04/glfwpp
  GIT_TAG 17b0ca06799fec64f63f5006fe45209ce33aace0
  PATCH_COMMAND ${glfwpp_patch_1}
)

set(GLFWPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(GLFWPP)

# ImGUI-base nodes
FetchContent_Declare(imnodes
  GIT_REPOSITORY https://github.com/Nelarius/imnodes
  GIT_TAG 8563e1655bd9bb1f249e6552cc6274d506ee788b
)

FetchContent_MakeAvailable(imnodes)
set(IMNODES_STANDALONE_PROJECT OFF CACHE BOOL "" FORCE)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost COMPONENTS regex)
include_directories(${Boost_INCLUDE_DIRS})

set(EXTERNAL_DEPS OpenGL::GL
                  GLEW::GLEW
                  imgui::imgui
                  Mathter::Mathter
                  nlohmann_json::nlohmann_json
                  GLFWPP
                  imnodes
                  ${Boost_LIBRARIES})

set(INTERNAL_LIB ${CMAKE_SOURCE_DIR}/src/runtime_type.cc)
add_library(internal_lib STATIC ${INTERNAL_LIB})

set(INTERNAL_NODES ${CMAKE_SOURCE_DIR}/src/execute.cc
                   ${CMAKE_SOURCE_DIR}/src/node.cc
                   ${CMAKE_SOURCE_DIR}/src/node_json.cc
                   ${CMAKE_SOURCE_DIR}/src/node_draw.cc)
add_library(internal_nodes STATIC ${INTERNAL_NODES})
target_link_libraries(internal_nodes PRIVATE ${EXTERNAL_DEPS})
target_link_libraries(internal_nodes PRIVATE internal_lib)

set(NODES_LIB ${CMAKE_SOURCE_DIR}/src/nodes/node_file_read.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_context_file_input.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_file_write.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_foreach_file.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_int_input.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_input.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_float_input.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_input.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_math.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_output.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_compare_value.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_up.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_down.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_regax.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_replace.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_slice.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_find.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_length.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_line_break.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_concatination.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_string_compare.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_value_to_text.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_execute_other.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_end.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_loop.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_switch.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_question_button.cc
              ${CMAKE_SOURCE_DIR}/src/nodes/node_question.cc
              )
add_library(nodes_lib STATIC ${NODES_LIB})
target_link_libraries(nodes_lib PRIVATE internal_nodes)
target_link_libraries(nodes_lib PRIVATE ${EXTERNAL_DEPS})

target_link_libraries(Main PRIVATE ${EXTERNAL_DEPS}
                                   internal_nodes
                                   internal_lib
                                   nodes_lib)