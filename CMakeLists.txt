cmake_minimum_required(VERSION 3.16)
project(nbte VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(nbte_product_bundle_identifier com.github.kbinani.nbte)
set(nbte_macos_deployment_target "10.15")
set(nbte_apple_development_team_id "XM695N4A7T")

execute_process(COMMAND git rev-list --all --count OUTPUT_VARIABLE nbte_build_number)
string(STRIP "${nbte_build_number}" nbte_build_number)

include(FetchContent)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
  set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${nbte_apple_development_team_id}")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "${nbte_macos_deployment_target}")
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  add_definitions(
    /DUNICODE
    /D_UNICODE
    /source-charset:utf-8
    /we4828
    /wd4100 # Suppress "unreferenced formal parameter" warnings
    /wd26812 # Suppress "Prefer 'enum class' over 'enum'" warnings
    /DNOMINMAX
  )
  add_compile_options(
    $<$<CONFIG:Debug>:/MTd>
    $<$<CONFIG:Release>:/MT>
    $<$<CONFIG:RelWithDebInfo>:/MT>
    $<$<CONFIG:MinSizeRel>:/MT>)
endif()

add_subdirectory(deps/glfw EXCLUDE_FROM_ALL)
add_subdirectory(deps/libminecraft-file EXCLUDE_FROM_ALL)

if (NOT nbte_busgnag_ready AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  execute_process(COMMAND xcodebuild -project Bugsnag.xcodeproj -target BugsnagStatic -sdk macosx -arch "\$(ARCHS_STANDARD)" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/bugsnag-cocoa)
  set(nbte_busgnag_ready ON CACHE INTERNAL "")
endif()

configure_file(src/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp)

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
file(READ deps/uuid4/LICENSE NBTE_LEGAL_UUID4)
string(STRIP "${NBTE_LEGAL_UUID4}" NBTE_LEGAL_UUID4)
file(READ deps/bugsnag-cocoa/LICENSE.txt NBTE_LEGAL_BUGSNAG_COCOA)
string(STRIP "${NBTE_LEGAL_BUGSNAG_COCOA}" NBTE_LEGAL_BUGSNAG_COCOA)
configure_file(src/render/legal.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/render/legal.hpp)

configure_file(package/Package.appxmanifest.in ${CMAKE_CURRENT_SOURCE_DIR}/package/Package.appxmanifest)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  configure_file(resource/Info.plist.in ${CMAKE_CURRENT_SOURCE_DIR}/resource/Info.plist)
endif()

list(APPEND nbte_files
  src/render/render.hpp
  src/model/state.hpp
  src/string.hpp
  src/version.hpp
  src/version.hpp.in
  src/render/legal.hpp
  src/render/legal.hpp.in
  src/platform.hpp
  src/model/node.hpp
  src/model/node.impl.hpp
  src/model/directory-contents.impl.hpp
  src/model/region.impl.hpp
  src/model/compound.impl.hpp
  src/temporary-directory.hpp
  src/imgui-ext.hpp
  src/texture.hpp
  src/texture-set.hpp
  resource/resource.rc.in
  resource/UDEVGothic35_Regular.ttf
  resource/nbte32.png
  deps/imgui/imgui.cpp
  deps/imgui/imgui_demo.cpp
  deps/imgui/imgui_draw.cpp
  deps/imgui/imgui_tables.cpp
  deps/imgui/imgui_widgets.cpp
  deps/imgui/misc/cpp/imgui_stdlib.cpp
  deps/nativefiledialog/src/nfd_common.c
  deps/stb/stb_image.h
  deps/uuid4/src/uuid4.h
  deps/uuid4/src/uuid4.c)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND nbte_files
    src/main.mm
    src/imgui-ext.mm
    resource/assets.xcassets
    resource/MainMenu.xib
    deps/imgui/backends/imgui_impl_metal.mm
    deps/imgui/backends/imgui_impl_osx.mm
    deps/nativefiledialog/src/nfd_cocoa.m)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  list(APPEND nbte_files
    src/main.cpp
    src/imgui-ext.cpp
    deps/imgui/backends/imgui_impl_dx12.cpp
    deps/imgui/backends/imgui_impl_win32.cpp
    resource/resource.rc
    deps/nativefiledialog/src/nfd_win.cpp)
endif()
list(APPEND nbte_icon_files
  resource/document_attribute_b.png
  resource/document_attribute_d.png
  resource/document_attribute_f.png
  resource/document_attribute_i.png
  resource/document_attribute_l.png
  resource/document_attribute_s.png
  resource/edit_small_caps.png
  resource/box.png
  resource/block.png
  resource/edit_list.png
  resource/folder.png
  resource/document.png
  resource/document_exclamation.png
  resource/edit_code.png)

string(REPLACE "." "," NBTE_FILE_VERSION ${CMAKE_PROJECT_VERSION})
set(NBTE_ICON_FILES "")
foreach (icon_file IN LISTS nbte_icon_files)
  get_filename_component(basename ${icon_file} NAME)
  set(NBTE_ICON_FILES "${NBTE_ICON_FILES}${basename} DATA \"${basename}\"\n")
endforeach()
configure_file(resource/resource.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.rc)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  add_executable(nbte MACOSX_BUNDLE ${nbte_files} ${nbte_icon_files})
  set_target_properties(nbte PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resource/Info.plist"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${nbte_product_bundle_identifier}"
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
    XCODE_ATTRIBUTE_ENABLE_APP_SANDBOX YES
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/resource/codesign.entitlements"
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH $<IF:$<CONFIG:Debug>,YES,NO>
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)
  set_source_files_properties(resource/assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set_source_files_properties(resource/MainMenu.xib PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set_source_files_properties(resource/UDEVGothic35_Regular.ttf PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  foreach(icon_file IN LISTS nbte_icon_files)
    set_source_files_properties(${icon_file} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  endforeach()
  target_link_directories(nbte PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/deps/bugsnag-cocoa/build/Release")
else()
  add_executable(nbte WIN32 ${nbte_files} ${nbte_icon_files})
endif()

list(APPEND nbte_include_directories
  deps/imgui
  deps/imgui/backends
  deps/imgui/misc/cpp
  deps/glfw/include
  deps/nativefiledialog/src/include
  deps/stb
  deps/hwm.task
  deps/uuid4/src)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND nbte_include_directories "${CMAKE_CURRENT_SOURCE_DIR}/deps/bugsnag-cocoa/Bugsnag/include")
endif()
target_include_directories(nbte PRIVATE ${nbte_include_directories})

list(APPEND nbte_link_libraries mcfile)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  list(APPEND nbte_link_libraries
    "-framework MetalKit"
    "-framework GameController"
    "-framework Metal"
    "-framework AppKit"
    BugsnagStatic)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  list(APPEND nbte_link_libraries d3d12.lib d3dcompiler.lib dxgi.lib)
endif()
target_link_libraries(nbte PRIVATE ${nbte_link_libraries})

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${nbte_files})

add_executable(binary_to_compressed_c deps/imgui/misc/fonts/binary_to_compressed_c.cpp)

include_external_msproject(package ${CMAKE_CURRENT_SOURCE_DIR}/package/package.wapproj
  TYPE C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5
  GUID 03182add-54ab-4518-b6a3-cfd896bcd9a4)
