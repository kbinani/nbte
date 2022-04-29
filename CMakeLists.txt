cmake_minimum_required(VERSION 3.16)
project(nbte VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  add_definitions(
    /DUNICODE
    /D_UNICODE
    /we4828
    /wd4100 # Suppress "unreferenced formal parameter" warnings
  )
  add_compile_options(
    $<$<CONFIG:Debug>:/MTd>
    $<$<CONFIG:Release>:/MT>
    $<$<CONFIG:RelWithDebInfo>:/MT>
    $<$<CONFIG:MinSizeRel>:/MT>)
endif()

add_subdirectory(deps/glfw EXCLUDE_FROM_ALL)
add_subdirectory(deps/libminecraft-file EXCLUDE_FROM_ALL)

configure_file(src/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp)
string(REPLACE "." "," NBTE_FILE_VERSION ${CMAKE_PROJECT_VERSION})
configure_file(resource/appicon.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/resource/appicon.rc)

list(APPEND nbte_files
  src/main.cpp
  src/render.hpp
  src/state.hpp
  src/string.hpp
  src/udev_gothic35_regular.h
  src/version.hpp
  src/nbte32.h
  deps/imgui/imgui.cpp
  deps/imgui/imgui_draw.cpp
  deps/imgui/imgui_tables.cpp
  deps/imgui/imgui_widgets.cpp
  deps/imgui/backends/imgui_impl_glfw.cpp
  deps/imgui/backends/imgui_impl_opengl3.cpp
  deps/imgui/misc/cpp/imgui_stdlib.cpp
  deps/nativefiledialog/src/nfd_common.c
  deps/stb/stb_image.h)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND nbte_files
    deps/nativefiledialog/src/nfd_cocoa.m)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  list(APPEND nbte_files
    resource/appicon.rc
    deps/nativefiledialog/src/nfd_win.cpp)
endif()

add_executable(nbte ${nbte_files})

target_include_directories(nbte
  PRIVATE
    deps/imgui
    deps/imgui/backends
    deps/imgui/misc/cpp
    deps/glfw/include
    deps/nativefiledialog/src/include
    deps/stb)

list(APPEND nbte_link_libraries
  glfw
  mcfile)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND nbte_link_libraries "-framework OpenGL")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  list(APPEND nbte_link_libraries opengl32.lib)
endif()
target_link_libraries(nbte PRIVATE ${nbte_link_libraries})

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${nbte_files})

add_executable(binary_to_compressed_c deps/imgui/misc/fonts/binary_to_compressed_c.cpp)
