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
  std::shared_ptr<std::future<std::string>> fSaveTask;

  std::string fError;

  bool fFilterBarOpened = false;
  std::string fFilter;
  FilterMode fFilterMode = FilterMode::Key;
  bool fFilterCaseSensitive = false;
  bool fFilterBarGotFocus = false;

  std::optional<Path> fMinecraftSaveDirectory;

  std::unique_ptr<hwm::task_queue> fPool;
  std::unique_ptr<hwm::task_queue> fSaveQueue;
  TextureSet fTextures;

  State() : fPool(new hwm::task_queue(std::thread::hardware_concurrency())), fSaveQueue(new hwm::task_queue(1)) {
  }

  void loadTextures(void *device) {
    fTextures.loadTextures(device);
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

  bool canSave() const {
    if (!fOpened) {
      return false;
    }
    if (fSaveTask) {
      return false;
    }
    return true;
  }

  void save() {
    using namespace std;

    fError.clear();
    if (!fOpened) {
      return;
    }
    fSaveTask = make_shared<future<string>>(fSaveQueue->enqueue([](Node &node, TemporaryDirectory &temp) { return node.save(temp); }, *fOpened, ref(fTempRoot)));
  }

  void retrieveSaveTask() {
    using namespace std;
    if (!fSaveTask) {
      return;
    }
    auto state = fSaveTask->wait_for(chrono::seconds(0));
    if (state != future_status::ready) {
      return;
    }
    auto error = fSaveTask->get();
    fError = error;
    if (fError.empty()) {
      fOpened->clearDirty();
    }
    fSaveTask.reset();
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

  std::u8string winowTitle() const {
    std::u8string title = u8"nbte";
    if (!fOpened) {
      return title;
    }
    if (!fOpenedPath.has_filename()) {
      return title;
    }
    title += u8" - " + fOpenedPath.filename().u8string();
    if (fOpened->isDirty()) {
      title += u8" *";
    }
    return title;
  }
};

} // namespace nbte
