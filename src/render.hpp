#pragma once

namespace nbte {

enum class Type {
  CompoundTag,
  //Directory,
  //Anvil,
};

struct State {
  ImVec2 fDisplaySize;
  bool fMainMenuBarFileSelected = false;
  bool fMainMenuBarFileOpenSelected = false;
  Type fOpenedType;
  std::variant<std::shared_ptr<mcfile::nbt::CompoundTag>, std::nullopt_t> fOpened = std::nullopt;
  std::string fError;

  void open(std::filesystem::path const &selected) {
    using namespace std;
    namespace fs = std::filesystem;
    fError.clear();

    static std::set<mcfile::Endian> const sEndians = {mcfile::Endian::Big, mcfile::Endian::Little};

    if (fs::is_regular_file(selected)) {
      for (auto endian : sEndians) {
        if (auto tag = mcfile::nbt::CompoundTag::Read(selected, endian); tag) {
          fOpened = tag;
          fOpenedType = Type::CompoundTag;
          return;
        }
      }
      for (auto endian : sEndians) {
        if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(selected, endian); tag) {
          fOpened = tag;
          fOpenedType = Type::CompoundTag;
          return;
        }
      }
      for (auto endian : sEndians) {
        auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(selected);
        if (auto tag = mcfile::nbt::CompoundTag::Read(stream, endian); tag) {
          fOpened = tag;
          fOpenedType = Type::CompoundTag;
          return;
        }
      }
    }
    fError = "Can't open file";
  }
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
  using namespace ImGui;

  ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
  Begin("main", nullptr, flags);
  SetWindowPos(ImVec2(0, 0));
  SetWindowSize(s.fDisplaySize);

  if (BeginMenuBar()) {
    if (BeginMenu("File", &s.fMainMenuBarFileSelected)) {
      if (MenuItem("Open", nullptr, nullptr)) {
        if (auto selected = OpenFileDialog(); selected) {
          s.open(*selected);
        }
      }
      EndMenu();
    }
    EndMenuBar();
  }

  if (!s.fError.empty()) {
    OpenPopup("Error");
    if (BeginPopupModal("Error", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
      TextUnformatted(s.fError.c_str());
      if (Button("OK")) {
        s.fError.clear();
        CloseCurrentPopup();
      }
      EndPopup();
    }
  }

  End();

  ImGui::Render();
}

} // namespace nbte
