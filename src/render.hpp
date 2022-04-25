#pragma once

#include <filesystem>
#include <iostream>
#include <nfd.h>
#include <optional>

namespace nbte {

struct State {
  ImVec2 fDisplaySize;
  bool fMainMenuBarFileSelected;
  bool fMainMenuBarFileOpenSelected;
};

static std::optional<std::filesystem::path> OpenFileDialog() {
  using namespace std;
  namespace fs = std::filesystem;

  nfdchar_t *outPath = nullptr;
  if (NFD_OpenDialog(nullptr, nullptr, &outPath) == NFD_OKAY) {
    u8string selected;
    selected.assign((char8_t const *)outPath);
    free(outPath);
    return fs::path(selected);
  } else {
    return nullopt;
  }
}

static void Render(State &s) {
  using namespace std;

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
  ImGui::Begin("main", nullptr, flags);
  ImGui::SetWindowPos(ImVec2(0, 0));
  ImGui::SetWindowSize(s.fDisplaySize);

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File", &s.fMainMenuBarFileSelected)) {
      if (ImGui::MenuItem("Open", nullptr, nullptr)) {
        if (auto selected = OpenFileDialog(); selected) {
          // TODO:
        }
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  ImGui::End();

  ImGui::Render();
}

} // namespace nbte
