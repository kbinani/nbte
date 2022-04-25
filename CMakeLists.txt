cmake_minimum_required(VERSION 3.16)
project(nbte VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(deps/glfw EXCLUDE_FROM_ALL)

list(APPEND nbte_files
  src/main.cpp
  src/render.hpp
  deps/imgui/imgui.cpp
  deps/imgui/imgui_demo.cpp
  deps/imgui/imgui_draw.cpp
  deps/imgui/imgui_tables.cpp
  deps/imgui/imgui_widgets.cpp
  deps/imgui/backends/imgui_impl_glfw.cpp
  deps/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(nbte ${nbte_files})

target_include_directories(nbte
  PRIVATE
    deps/imgui
    deps/imgui/backends)
target_link_libraries(nbte
  PRIVATE
    opengl32.lib
    glfw)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${nbte_files})
