#pragma once

namespace nbte {

class Node;
class State;

class Region {
public:
  Region(hwm::task_queue &queue, int x, int z, Path const &path, std::shared_ptr<Node> const &owner);

  bool wait(State &s);
  String save(TemporaryDirectory &temp);
  bool isDirty() const;

  using ValueType = std::vector<std::shared_ptr<Node>>;

  Path fFile;
  int fX;
  int fZ;
  std::variant<ValueType, std::shared_ptr<std::future<std::optional<ValueType>>>> fValue;
  std::weak_ptr<Node> fOwner;
};

class DirectoryContents {
public:
  DirectoryContents(Path const &dir, std::shared_ptr<Node> parent);
  String save(TemporaryDirectory &temp);
  bool dirtyFiles(std::vector<Path> *buffer = nullptr) const;

  Path fDir;
  std::vector<std::shared_ptr<Node>> fValue;
};

class Compound {
public:
  enum class Format {
    RawLittleEndian,
    RawBigEndian,
    DeflatedLittleEndian,
    DeflatedBigEndian,
    GzippedLittleEndian,
    GzippedBigEndian,
  };

  Compound(Path const &name, std::shared_ptr<mcfile::nbt::CompoundTag> const &tag, Format format) : fName(name), fTag(tag), fFormat(format) {}
  Compound(String const &name, int cx, int cz, std::shared_ptr<mcfile::nbt::CompoundTag> const &tag, Format format) : fName(name), fTag(tag), fFormat(format), fChunkX(cx), fChunkZ(cz) {}

  String save(Path const &file);
  String save();
  String name() const;
  std::optional<Path> filePathIfEdited() const;

  std::variant<String, Path> fName;
  std::shared_ptr<mcfile::nbt::CompoundTag> fTag;
  Format fFormat;
  bool fEdited = false;
  int fChunkX = 0;
  int fChunkZ = 0;
};

class Node : public std::enable_shared_from_this<Node> {
public:
  enum Type : int {
    TypeDirectoryContents = 0,
    TypeFileUnopened,
    TypeDirectoryUnopened,
    TypeUnsupportedFile,
    TypeRegion,
    TypeCompound,
  };
  using Value = std::variant<DirectoryContents, // DirectoryContents
                             Path,              // FileUnopened
                             Path,              // DirectoryUnopened
                             Path,              // UnsupportedFile
                             Region,            // Region
                             Compound           // Compound
                             >;

  Node(Value &&value, std::shared_ptr<Node> parent);

  void load(hwm::task_queue &queue);
  String save(TemporaryDirectory &temp);

  DirectoryContents const *directoryContents() const;
  DirectoryContents *directoryContents();
  Path const *fileUnopened() const;
  Path const *directoryUnopened() const;
  Compound const *compound() const;
  Compound *compound();
  Path const *unsupportedFile() const;
  Region *region();
  Region const *region() const;

  String description() const;
  bool hasParent() const;
  void clearDirty();
  bool dirtyFiles(std::vector<Path> *buffer = nullptr) const;

  static std::shared_ptr<Node> OpenDirectory(Path const &path, hwm::task_queue &queue);
  static std::shared_ptr<Node> OpenFile(Path const &path, hwm::task_queue &queue);

  static std::shared_ptr<Node> DirectoryUnopened(Path const &path, std::shared_ptr<Node> const &parent);
  static std::shared_ptr<Node> FileUnopened(Path const &path, std::shared_ptr<Node> const &parent);

private:
  Value fValue;

public:
  std::weak_ptr<Node> const fParent;
};

} // namespace nbte
