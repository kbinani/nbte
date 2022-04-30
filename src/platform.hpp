#pragma once

namespace nbte {

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

static int GetModCtrlKeyIndex() {
#if defined(__APPLE__)
  return ImGui::GetKeyIndex(ImGuiKey_ModSuper);
#else
  return ImGui::GetKeyIndex(ImGuiKey_ModCtrl);
#endif
}

static std::string DecorateModCtrl(std::string const &pair) {
#if defined(__APPLE__)
  return "Cmd+" + pair;
#else
  return "Ctrl+" + pair;
#endif
}

static std::string QuitMenuShortcut() {
#if defined(__APPLE__)
  return "Cmd+Q";
#else
  return "Alt+F4";
#endif
}

} // namespace nbte
