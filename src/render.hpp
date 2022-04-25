#pragma once

namespace nbte {

struct State {
  ImVec2 fDisplaySize;
};

static void Render(State &s) {
  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
  ImGui::Begin("main", nullptr, flags);
  ImGui::SetWindowPos(ImVec2(0, 0));
  ImGui::SetWindowSize(s.fDisplaySize);
  ImGui::End();

  ImGui::Render();
}

} // namespace nbte
