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

  std::shared_ptr<Node> fOpened;
  std::filesystem::path fOpenedPath;

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
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;
    namespace fs = std::filesystem;

    fError.clear();
    auto file = fOpenedPath;

    if (!fOpened) {
      return;
    }
    if (auto compound = fOpened->compound(); compound) {
      Endian endian = Endian::Big;
      switch (compound->fFormat) {
      case Compound::Format::RawLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Compound::Format::RawBigEndian: {
        auto stream = make_shared<FileOutputStream>(file);
        OutputStreamWriter writer(stream, endian);
        if (!compound->fTag->writeAsRoot(writer)) {
          fError = "IO Error";
        }
        break;
      }
      case Compound::Format::DeflatedLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Compound::Format::DeflatedBigEndian: {
        auto stream = make_shared<FileOutputStream>(file);
        if (!CompoundTag::WriteCompressed(*compound->fTag, *stream, endian)) {
          fError = "IO Error";
        }
        break;
      }
      case Compound::Format::GzippedLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Compound::Format::GzippedBigEndian: {
        auto stream = make_shared<GzFileOutputStream>(file);
        OutputStreamWriter writer(stream, endian);
        if (!compound->fTag->writeAsRoot(writer)) {
          fError = "IO Error";
        }
        break;
      }
      }
    } else if (auto r = fOpened->region(); r) {
      // TODO:
      fError = "Unimplemented yet";
    } else if (auto contents = fOpened->directoryContents(); contents) {
      // TODO:
      fError = "Unimplemented yet";
    }

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
