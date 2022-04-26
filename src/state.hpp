#pragma once

namespace nbte {

enum class Type {
  CompoundTag,
  // Directory,
  // Anvil,
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

} // namespace nbte
