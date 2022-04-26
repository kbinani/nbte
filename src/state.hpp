#pragma once

namespace nbte {

enum class Type {
  CompoundTag,
  // Directory,
  // Anvil,
};

struct Storage {
  std::unordered_map<std::shared_ptr<mcfile::nbt::Tag>, int> fInt;
  std::unordered_map<std::shared_ptr<mcfile::nbt::Tag>, std::string> fString;

  void clear() {
    fInt.clear();
    fString.clear();
  }

  int *useInt(std::shared_ptr<mcfile::nbt::IntTag> const &tag) {
    if (auto found = fInt.find(tag); found == fInt.end()) {
      fInt[tag] = tag->fValue;
    }
    return &fInt[tag];
  }

  int *useInt(std::shared_ptr<mcfile::nbt::ByteTag> const &tag) {
    if (auto found = fInt.find(tag); found == fInt.end()) {
      fInt[tag] = tag->fValue;
    }
    return &fInt[tag];
  }

  int *useInt(std::shared_ptr<mcfile::nbt::ShortTag> const &tag) {
    if (auto found = fInt.find(tag); found == fInt.end()) {
      fInt[tag] = tag->fValue;
    }
    return &fInt[tag];
  }

  std::string *useString(std::shared_ptr<mcfile::nbt::LongTag> const &tag) {
    if (auto found = fString.find(tag); found == fString.end()) {
      fString[tag] = std::to_string(tag->fValue);
    }
    return &fString[tag];
  }
};

struct State {
  ImVec2 fDisplaySize;
  bool fMainMenuBarFileSelected = false;
  bool fMainMenuBarFileOpenSelected = false;
  Type fOpenedType;
  std::variant<std::shared_ptr<mcfile::nbt::CompoundTag>, std::nullopt_t> fOpened = std::nullopt;
  std::string fError;
  Storage fStorage;

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
          fStorage.clear();
          return;
        }
      }
      for (auto endian : sEndians) {
        if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(selected, endian); tag) {
          fOpened = tag;
          fOpenedType = Type::CompoundTag;
          fStorage.clear();
          return;
        }
      }
      for (auto endian : sEndians) {
        auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(selected);
        if (auto tag = mcfile::nbt::CompoundTag::Read(stream, endian); tag) {
          fOpened = tag;
          fOpenedType = Type::CompoundTag;
          fStorage.clear();
          return;
        }
      }
    }
    fError = "Can't open file";
  }
};

} // namespace nbte
