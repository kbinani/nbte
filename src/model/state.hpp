#pragma once

namespace nbte {

enum class FilterMode {
  Key,
  Value,
};

struct State {
  ImVec2 fDisplaySize;
  bool fMainMenuBarFileSelected = false;
  bool fMainMenuBarFileOpenSelected = false;
  bool fMainMenuBarFindSelected = false;
  bool fMainMenuBarHelpSelected = false;
  bool fMainMenuBarHelpAboutOpened = false;
  bool fMainMenuBarHelpOpenSourceLicensesOpened = false;
  bool fMainMenuBarQuitSelected = false;

  TemporaryDirectory fTempRoot;

  std::shared_ptr<Node> fOpened;
  Path fOpenedPath;

  std::string fError;

  bool fFilterBarOpened = false;
  std::string fFilter;
  FilterMode fFilterMode = FilterMode::Key;
  bool fFilterCaseSensitive = false;
  bool fFilterBarGotFocus = false;

  std::optional<Path> fMinecraftSaveDirectory;

  std::unique_ptr<hwm::task_queue> fPool;

  std::optional<Texture> fIconDocumentAttributeB;
  std::optional<Texture> fIconDocumentAttributeD;
  std::optional<Texture> fIconDocumentAttributeF;
  std::optional<Texture> fIconDocumentAttributeI;
  std::optional<Texture> fIconDocumentAttributeL;
  std::optional<Texture> fIconDocumentAttributeS;
  std::optional<Texture> fIconEditSmallCaps;

  State() : fPool(new hwm::task_queue(std::thread::hardware_concurrency())) {
    fIconDocumentAttributeB = LoadTexture("document_attribute_b.png");
    fIconDocumentAttributeD = LoadTexture("document_attribute_d.png");
    fIconDocumentAttributeF = LoadTexture("document_attribute_f.png");
    fIconDocumentAttributeI = LoadTexture("document_attribute_i.png");
    fIconDocumentAttributeL = LoadTexture("document_attribute_l.png");
    fIconDocumentAttributeS = LoadTexture("document_attribute_s.png");
    fIconEditSmallCaps = LoadTexture("edit_small_caps.png");
  }

  void open(Path const &selected) {
    fError.clear();

    if (auto node = Node::OpenFile(selected, *fPool); node) {
      fOpened = node;
      fOpenedPath = selected;
      return;
    }
    fError = "Can't open file";
  }

  void openDirectory(Path const &path) {
    fError.clear();
    if (auto node = Node::OpenDirectory(path, *fPool); node) {
      fOpened = node;
      fOpenedPath = path;
      return;
    }
    fError = "Can't open directory";
  }

  void save() {
    fError.clear();
    if (!fOpened) {
      return;
    }
    fError = fOpened->save(fTempRoot);
    if (fError.empty()) {
      fOpened->clearDirty();
    }
  }

  std::string filterTerm() const {
    if (!fFilterBarOpened) {
      return {};
    }
    if (fFilterCaseSensitive) {
      return fFilter;
    } else {
      return ToLower(fFilter);
    }
  }

  std::string winowTitle() const {
    std::string title = "nbte";
    if (!fOpened) {
      return title;
    }
    if (!fOpenedPath.has_filename()) {
      return title;
    }
    title += " - " + fOpenedPath.filename().string();
    if (fOpened->isDirty()) {
      title += " *";
    }
    return title;
  }
};

} // namespace nbte
