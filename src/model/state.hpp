#pragma once

namespace nbte {

class State {
  String fFilterRaw;
  FilterKey fFilter;

public:
  ImVec2 fDisplaySize;
  bool fMainMenuBarFileSelected = false;
  bool fMainMenuBarFileOpenSelected = false;
  bool fMainMenuBarFindSelected = false;
  bool fMainMenuBarHelpSelected = false;
  bool fMainMenuBarHelpAboutOpened = false;
  bool fMainMenuBarHelpOpenSourceLicensesOpened = false;
  bool fDebugOpened = false;

  bool fQuitRequested = false;
  bool fQuitAccepted = false;

  TemporaryDirectory fTempRoot;

  std::shared_ptr<Node> fOpened;
  Path fOpenedPath;
  std::shared_ptr<std::future<String>> fSaveTask;

  String fError;

  bool fFilterBarOpened = false;
  FilterMode fFilterMode = FilterMode::Key;
  bool fFilterBarGotFocus = false;

#if NBTE_NAVBAR
  bool fNavigateBarOpened = false;
#endif

  std::optional<Path> fMinecraftSaveDirectory;

  std::unique_ptr<hwm::task_queue> fPool;
  std::unique_ptr<hwm::task_queue> fSaveQueue;
  TextureSet fTextures;

  FilterCacheSelector<64> fCacheSelector;

  State() : fFilter({}, false), fPool(new hwm::task_queue(std::thread::hardware_concurrency())), fSaveQueue(new hwm::task_queue(1)) {
  }

  bool containsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, FilterKey const *key, FilterMode mode) {
    return fCacheSelector.containsTerm(tag, key, mode);
  }

  bool containsTerm(std::shared_ptr<Node> const &node, FilterKey const *key, FilterMode mode) {
    return fCacheSelector.containsTerm(node, key, mode);
  }

  void loadTextures(void *device) {
    fTextures.loadTextures(device);
  }

  void open(Path const &selected) {
    fError.clear();

    if (auto node = Node::OpenFile(selected, *fPool); node) {
      fOpened = node;
      if (fOpenedPath != selected) {
        fCacheSelector.invalidate();
      }
      fOpenedPath = selected;
      return;
    }
    fError = u8"Can't open file";
  }

  void openDirectory(Path const &selected) {
    fError.clear();

    if (auto node = Node::OpenDirectory(selected, *fPool); node) {
      fOpened = node;
      if (fOpenedPath != selected) {
        fCacheSelector.invalidate();
      }
      fOpenedPath = selected;
      return;
    }
    fError = u8"Can't open directory";
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
    fSaveTask = make_shared<future<String>>(fSaveQueue->enqueue([](Node &node, TemporaryDirectory &temp) { return node.save(temp); }, *fOpened, ref(fTempRoot)));
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

  FilterKey const *filterKey() const {
    if (!fFilterBarOpened) {
      return nullptr;
    }
    if (fFilterRaw.empty()) {
      return nullptr;
    }
    return &fFilter;
  }

  void updateFilter(String const &filterRaw, bool caseSensitive) {
    fFilterRaw = filterRaw;
    fFilter.fSearch = caseSensitive ? filterRaw : ToLower(filterRaw);
    fFilter.fCaseSensitive = caseSensitive;
  }

  String const &filterRaw() const {
    return fFilterRaw;
  }

  bool filterCaseSensitive() const {
    return fFilter.fCaseSensitive;
  }

  String winowTitle() const {
    String title = u8"nbte";
    if (!fOpened) {
      return title;
    }
    if (!fOpenedPath.has_filename()) {
      return title;
    }
    title += u8" - " + fOpenedPath.filename().u8string();
    if (fOpened->dirtyFiles()) {
      title += u8" *";
    }
    return title;
  }

  bool dirtyFiles(std::vector<Path> &buffer) const {
    if (!fOpened) {
      return false;
    }
    return fOpened->dirtyFiles(&buffer);
  }
};

} // namespace nbte
