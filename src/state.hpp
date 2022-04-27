#pragma once

namespace nbte {

enum class Type {
  CompoundTagRawLittleEndian,
  CompoundTagRawBigEndian,
  CompoundTagDeflatedLittleEndian,
  CompoundTagDeflatedBigEndian,
  CompoundTagGzippedLittleEndian,
  CompoundTagGzippedBigEndian,
  // Directory,
  // Anvil,
};

static std::string TypeDescription(Type type) {
  switch (type) {
  case Type::CompoundTagRawLittleEndian:
    return "Raw NBT (LittleEndian)";
  case Type::CompoundTagRawBigEndian:
    return "Raw NBT (BigEndian)";
  case Type::CompoundTagDeflatedLittleEndian:
    return "Deflated NBT (LittleEndian)";
  case Type::CompoundTagDeflatedBigEndian:
    return "Deflated NBT (BigEndian)";
  case Type::CompoundTagGzippedBigEndian:
    return "Gzipped NBT (BigEndian)";
  case Type::CompoundTagGzippedLittleEndian:
    return "Gzipped NBT (LittleEndian)";
  default:
    return "Unknown";
  }
}

struct State {
  ImVec2 fDisplaySize;
  bool fMainMenuBarFileSelected = false;
  bool fMainMenuBarFileOpenSelected = false;

  Type fOpenedType;
  std::variant<std::nullopt_t, std::shared_ptr<mcfile::nbt::CompoundTag>> fOpened = std::nullopt;
  std::filesystem::path fOpenedPath;

  std::string fError;

  void open(std::filesystem::path const &selected) {
    using namespace std;
    namespace fs = std::filesystem;
    fError.clear();

    static std::set<mcfile::Endian> const sEndians = {mcfile::Endian::Big, mcfile::Endian::Little};

    if (fs::is_regular_file(selected)) {
      if (auto tag = mcfile::nbt::CompoundTag::Read(selected, mcfile::Endian::Little); tag) {
        fOpened = tag;
        fOpenedType = Type::CompoundTagRawLittleEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      if (auto tag = mcfile::nbt::CompoundTag::Read(selected, mcfile::Endian::Big); tag) {
        fOpened = tag;
        fOpenedType = Type::CompoundTagRawBigEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(selected, mcfile::Endian::Little); tag) {
        fOpened = tag;
        fOpenedType = Type::CompoundTagDeflatedLittleEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(selected, mcfile::Endian::Big); tag) {
        fOpened = tag;
        fOpenedType = Type::CompoundTagDeflatedBigEndian;
        fOpenedPath = fs::absolute(selected);
        return;
      }
      {
        auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(selected);
        if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Big); tag) {
          fOpened = tag;
          fOpenedType = Type::CompoundTagGzippedBigEndian;
          fOpenedPath = fs::absolute(selected);
          return;
        }
      }
      {
        auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(selected);
        if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Little); tag) {
          fOpened = tag;
          fOpenedType = Type::CompoundTagGzippedLittleEndian;
          fOpenedPath = fs::absolute(selected);
          return;
        }
      }
    }
    fError = "Can't open file";
  }
};

} // namespace nbte
