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

file(READ deps/libminecraft-file/LICENSE NBTE_LEGAL_LIBMINECRAFT_FILE)
string(STRIP "${NBTE_LEGAL_LIBMINECRAFT_FILE}" NBTE_LEGAL_LIBMINECRAFT_FILE)
file(READ deps/nativefiledialog/LICENSE NBTE_LEGAL_NATIVEFILEDIALOG)
string(STRIP "${NBTE_LEGAL_NATIVEFILEDIALOG}" NBTE_LEGAL_NATIVEFILEDIALOG)
file(READ deps/stb/LICENSE NBTE_LEGAL_STB)
string(STRIP "${NBTE_LEGAL_STB}" NBTE_LEGAL_STB)
file(READ deps/GLFW/LICENSE.md NBTE_LEGAL_GLFW)
string(STRIP "${NBTE_LEGAL_GLFW}" NBTE_LEGAL_GLFW)
file(READ deps/imgui/LICENSE.txt NBTE_LEGAL_DEARIMGUI)
string(STRIP "${NBTE_LEGAL_DEARIMGUI}" NBTE_LEGAL_DEARIMGUI)
file(READ deps/libminecraft-file/deps/zlib-ng/LICENSE.md NBTE_LEGAL_ZLIB_NG)
string(STRIP "${NBTE_LEGAL_ZLIB_NG}" NBTE_LEGAL_ZLIB_NG)
configure_file(src/render-legal.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/render-legal.hpp)

configure_file(package/Package.appxmanifest.in ${CMAKE_CURRENT_SOURCE_DIR}/package/Package.appxmanifest)

list(APPEND nbte_files
  src/render.hpp
  src/state.hpp
  src/string.hpp
  src/udev_gothic35_regular.h
  src/version.hpp
  src/nbte32.h
  src/render-legal.hpp
  deps/imgui/imgui.cpp
  deps/imgui/imgui_draw.cpp
  deps/imgui/imgui_tables.cpp
  deps/imgui/imgui_widgets.cpp
  deps/imgui/misc/cpp/imgui_stdlib.cpp
  deps/nativefiledialog/src/nfd_common.c
  deps/stb/stb_image.h)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND nbte_files
    src/main.mm
    deps/imgui/backends/imgui_impl_metal.mm
    deps/imgui/backends/imgui_impl_osx.mm
    deps/nativefiledialog/src/nfd_cocoa.m)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  list(APPEND nbte_files
    src/main.cpp
    deps/imgui/backends/imgui_impl_glfw.cpp
    deps/imgui/backends/imgui_impl_opengl3.cpp
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
  mcfile)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND nbte_link_libraries
    "-framework MetalKit"
    "-framework GameController"
    "-framework Metal"
    "-framework AppKit")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  list(APPEND nbte_link_libraries
    glfw
    opengl32.lib)
endif()
target_link_libraries(nbte PRIVATE ${nbte_link_libraries})

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${nbte_files})

add_executable(binary_to_compressed_c deps/imgui/misc/fonts/binary_to_compressed_c.cpp)

include_external_msproject(package ${CMAKE_CURRENT_SOURCE_DIR}/package/package.wapproj
  TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
  GUID 03182add-54ab-4518-b6a3-cfd896bcd9a4)
