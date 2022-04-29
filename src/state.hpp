#pragma once

namespace nbte {

enum class Format {
  CompoundTagRawLittleEndian,
  CompoundTagRawBigEndian,
  CompoundTagDeflatedLittleEndian,
  CompoundTagDeflatedBigEndian,
  CompoundTagGzippedLittleEndian,
  CompoundTagGzippedBigEndian,
  // Directory,
  // Anvil,
};

static std::string TypeDescription(Format type) {
  switch (type) {
  case Format::CompoundTagRawLittleEndian:
    return "Raw NBT (LittleEndian)";
  case Format::CompoundTagRawBigEndian:
    return "Raw NBT (BigEndian)";
  case Format::CompoundTagDeflatedLittleEndian:
    return "Deflated NBT (LittleEndian)";
  case Format::CompoundTagDeflatedBigEndian:
    return "Deflated NBT (BigEndian)";
  case Format::CompoundTagGzippedBigEndian:
    return "Gzipped NBT (BigEndian)";
  case Format::CompoundTagGzippedLittleEndian:
    return "Gzipped NBT (LittleEndian)";
  default:
    return "Unknown";
  }
}

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

  Format fOpenedFormat;
  std::variant<std::nullopt_t, std::shared_ptr<mcfile::nbt::CompoundTag>> fOpened = std::nullopt;
  std::filesystem::path fOpenedPath;

  std::string fError;

  bool fFilterBarOpened = false;
  std::string fFilter;
  FilterMode fFilterMode = FilterMode::Key;
  bool fFilterCaseSensitive = false;
  bool fFilterBarGotFocus = false;

  bool fEdited = false;

  void open(std::filesystem::path const &selected) {
    using namespace std;
    namespace fs = std::filesystem;
    fError.clear();

    static std::set<mcfile::Endian> const sEndians = {mcfile::Endian::Big, mcfile::Endian::Little};

    if (fs::is_regular_file(selected)) {
      if (auto tag = mcfile::nbt::CompoundTag::Read(selected, mcfile::Endian::Little); tag) {
        fOpened = tag;
        fOpenedFormat = Format::CompoundTagRawLittleEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      if (auto tag = mcfile::nbt::CompoundTag::Read(selected, mcfile::Endian::Big); tag) {
        fOpened = tag;
        fOpenedFormat = Format::CompoundTagRawBigEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(selected, mcfile::Endian::Little); tag) {
        fOpened = tag;
        fOpenedFormat = Format::CompoundTagDeflatedLittleEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(selected, mcfile::Endian::Big); tag) {
        fOpened = tag;
        fOpenedFormat = Format::CompoundTagDeflatedBigEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      {
        auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(selected);
        if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Big); tag) {
          fOpened = tag;
          fOpenedFormat = Format::CompoundTagGzippedBigEndian;
          fOpenedPath = fs::absolute(selected);
          return;
        }
      }
      {
        auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(selected);
        if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Little); tag) {
          fOpened = tag;
          fOpenedFormat = Format::CompoundTagGzippedLittleEndian;
          fOpenedPath = fs::absolute(selected);
          return;
        }
      }
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

    if (fOpened.index() == 1) {
      auto tag = std::get<1>(fOpened);
      if (!tag) {
        return;
      }
      Endian endian = Endian::Big;
      switch (fOpenedFormat) {
      case Format::CompoundTagRawLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Format::CompoundTagRawBigEndian: {
        auto stream = make_shared<FileOutputStream>(file);
        OutputStreamWriter writer(stream, endian);
        if (!tag->writeAsRoot(writer)) {
          fError = "IO Error";
        }
        break;
      }
      case Format::CompoundTagDeflatedLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Format::CompoundTagDeflatedBigEndian: {
        auto stream = make_shared<FileOutputStream>(file);
        if (!CompoundTag::WriteCompressed(*tag, *stream, endian)) {
          fError = "IO Error";
        }
        break;
      }
      case Format::CompoundTagGzippedLittleEndian:
        endian = Endian::Little;
        [[fallthrough]];
      case Format::CompoundTagGzippedBigEndian: {
        auto stream = make_shared<GzFileOutputStream>(file);
        OutputStreamWriter writer(stream, endian);
        if (!tag->writeAsRoot(writer)) {
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
    if (fOpened.index() == 0) {
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
