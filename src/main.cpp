// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NOMINMAX
#include <windows.h>
#include <shlobj_core.h>

#include <hwm/task/task_queue.hpp>
#include <minecraft-file.hpp>
#include <nfd.h>
extern "C" {
#include <uuid4.h>
}
#include <variant>
#include <list>

#include "version.hpp"
#include "string.hpp"
#include "texture.hpp"
#include "platform.hpp"
#include "texture-set.hpp"
#include "temporary-directory.hpp"
#include "filter-key.hpp"
#include "model/node.hpp"
#include "filter-cache.hpp"
#include "model/node.impl.hpp"
#include "model/directory-contents.impl.hpp"
#include "model/compound.impl.hpp"
#include "model/state.hpp"
#include "model/region.impl.hpp"
#include "imgui-ext.hpp"
#include "render/legal.hpp"
#include "render/render.hpp"

#pragma comment(lib, "opengl32.lib")

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
  namespace fs = std::filesystem;

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    return 1;
  }

  fs::path file;
  if (lstrlenW(lpCmdLine) > 0) {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);
    if (!argv) {
      return 1;
    }
    if (argc > 0) {
      file = argv[0];
    }
    LocalFree(argv);
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(1280, 720, "nbte", NULL, NULL);
  if (window == NULL) {
    return 1;
  }

  if (auto nbte32 = nbte::LoadNamedResource("nbte32.png"); nbte32) {
    int width, height, components;
    unsigned char *img = stbi_load_from_memory((stbi_uc const *)nbte32->fData, nbte32->fSize, &width, &height, &components, 4);
    GLFWimage icon = {width, height, img};
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(img);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.IniFilename = nullptr;

  ImGui::StyleColorsLight();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  if (auto udevFont = nbte::LoadNamedResource("UDEVGothic35_Regular.ttf"); udevFont) {
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    assert(udevFont->fSystemOwned);

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
    builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesThai());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.BuildRanges(&ranges);

    ImFont *font = io.Fonts->AddFontFromMemoryTTF(udevFont->fData, udevFont->fSize, 15.0f, &cfg, ranges.Data);
    io.Fonts->Build();
  }

  uuid4_init();

  nbte::State state;
  state.fMinecraftSaveDirectory = nbte::MinecraftSaveDirectory();

  if (fs::exists(file)) {
    if (fs::is_regular_file(file)) {
      state.open(file);
    } else if (fs::is_directory(file)) {
      state.openDirectory(file);
    }
  }

  state.loadTextures(nullptr);

  // Main loop
  bool done = false;
  while (!done) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    glfwSetWindowTitle(window, (char const *)state.winowTitle().c_str());

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    state.fDisplaySize = io.DisplaySize;
    bool shouldClose = glfwWindowShouldClose(window) != 0;
    state.fQuitRequested = state.fQuitRequested || shouldClose;
    nbte::Render(state);
    if (state.fQuitAccepted) {
      done = true;
    } else {
      glfwSetWindowShouldClose(window, 0);
    }

    ImGui::Render();

    glViewport(0, 0, display_w, display_h);

    ImGuiStyle const &style = ImGui::GetStyle();
    ImVec4 clearColor = style.Colors[ImGuiCol_WindowBg];
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
