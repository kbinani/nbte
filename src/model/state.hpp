#pragma once

namespace nbte {

struct State {
  ImVec2 fDisplaySize;
  bool fMainMenuBarFileSelected = false;
  bool fMainMenuBarFileOpenSelected = false;
  bool fMainMenuBarFindSelected = false;
  bool fMainMenuBarHelpSelected = false;
  bool fMainMenuBarHelpAboutOpened = false;
  bool fMainMenuBarHelpOpenSourceLicensesOpened = false;

  bool fQuitRequested = false;
  bool fQuitAccepted = false;

  TemporaryDirectory fTempRoot;

  std::shared_ptr<Node> fOpened;
  Path fOpenedPath;
  std::shared_ptr<std::future<String>> fSaveTask;

  String fError;

  bool fFilterBarOpened = false;
  String fFilter;
  FilterMode fFilterMode = FilterMode::Key;
  bool fFilterCaseSensitive = false;
  bool fFilterBarGotFocus = false;

#if NBTE_NAVBAR
  bool fNavigateBarOpened = false;
#endif

  std::optional<Path> fMinecraftSaveDirectory;

  std::unique_ptr<hwm::task_queue> fPool;
  std::unique_ptr<hwm::task_queue> fSaveQueue;
  TextureSet fTextures;

  FilterCacheSelector<64> fCacheSelector;

  State() : fPool(new hwm::task_queue(std::thread::hardware_concurrency())), fSaveQueue(new hwm::task_queue(1)) {
  }

  bool containsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, FilterKey const &key, FilterMode mode) {
    return fCacheSelector.containsTerm(tag, key, mode);
  }

  bool containsTerm(std::shared_ptr<Node> const &node, FilterKey const &key, FilterMode mode) {
    if (!node) {
      return false;
    }
    if (auto r = node->region(); r) {
      if (r->fValue.index() != 0) {
        return false;
      }
      for (auto const &it : std::get<0>(r->fValue)) {
        if (containsTerm(it, key, mode)) {
          return true;
        }
      }
      return false;
    }
    if (auto c = node->compound(); c) {
      if (key.match(c->name())) {
        return true;
      }
      return containsTerm(c->fTag, key, mode);
    }
    if (auto c = node->directoryContents(); c) {
      for (auto const &it : c->fValue) {
        if (containsTerm(it, key, mode)) {
          return true;
        }
      }
      return false;
    }
    if (auto file = node->fileUnopened(); file) {
      return key.match(file->filename().u8string());
    }
    if (auto directory = node->directoryUnopened(); directory) {
      return key.match(directory->filename().u8string());
    }
    return false;
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

  FilterKey filterKey() const {
    if (!fFilterBarOpened) {
      return FilterKey({}, false);
    }
    return FilterKey(fFilter, fFilterCaseSensitive);
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
