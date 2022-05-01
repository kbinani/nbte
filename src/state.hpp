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

  bool fEdited = false;

  void open(std::filesystem::path const &selected) {
    fError.clear();

    if (auto node = Node::OpenCompound(selected); node) {
      fOpened = node;
      fOpenedPath = selected;
      return;
    }
    fError = "Can't open file";
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
      case Compound::Format::CompoundTagRawLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Compound::Format::CompoundTagRawBigEndian: {
        auto stream = make_shared<FileOutputStream>(file);
        OutputStreamWriter writer(stream, endian);
        if (!compound->fTag->writeAsRoot(writer)) {
          fError = "IO Error";
        }
        break;
      }
      case Compound::Format::CompoundTagDeflatedLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Compound::Format::CompoundTagDeflatedBigEndian: {
        auto stream = make_shared<FileOutputStream>(file);
        if (!CompoundTag::WriteCompressed(*compound->fTag, *stream, endian)) {
          fError = "IO Error";
        }
        break;
      }
      case Compound::Format::CompoundTagGzippedLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Compound::Format::CompoundTagGzippedBigEndian: {
        auto stream = make_shared<GzFileOutputStream>(file);
        OutputStreamWriter writer(stream, endian);
        if (!compound->fTag->writeAsRoot(writer)) {
          fError = "IO Error";
        }
        break;
      }
      }
    }

    if (fError.empty()) {
      fEdited = false;
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
    if (fEdited) {
      title += " *";
    }
    return title;
  }
};

} // namespace nbte
