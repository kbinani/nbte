#pragma once

namespace nbte {

class Node;

class Region {
public:
  Region(hwm::task_queue &queue, int x, int z, Path const &path, std::shared_ptr<Node> const &parent);

  bool wait();
  std::string save();

  using ValueType = std::vector<std::shared_ptr<Node>>;

  Path fFile;
  int fX;
  int fZ;
  std::variant<ValueType, std::shared_ptr<std::future<std::optional<ValueType>>>> fValue;
};

class DirectoryContents {
public:
  DirectoryContents(Path const &dir, std::shared_ptr<Node> parent);
  std::string save();

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

  Compound(std::variant<std::string, Path> const &name, std::shared_ptr<mcfile::nbt::CompoundTag> const &tag, Format format) : fName(name), fTag(tag), fFormat(format) {}

  std::string save(Path const &file);
  std::string save();
  std::string name() const;

  std::variant<std::string, Path> fName;
  std::shared_ptr<mcfile::nbt::CompoundTag> fTag;
  Format fFormat;
  bool fEdited = false;
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
  std::string save();

  DirectoryContents const *directoryContents() const;
  DirectoryContents *directoryContents();
  Path const *fileUnopened() const;
  Path const *directoryUnopened() const;
  Compound const *compound() const;
  Compound *compound();
  Path const *unsupportedFile() const;
  Region *region();
  Region const *region() const;

  std::string description() const;
  bool hasParent() const;
  bool isDirty() const;
  void clearDirty();

  static std::shared_ptr<Node> OpenDirectory(Path const &path, hwm::task_queue &queue);
  static std::shared_ptr<Node> OpenFile(Path const &path, hwm::task_queue &queue);

  static std::shared_ptr<Node> DirectoryUnopened(Path const &path, std::shared_ptr<Node> const &parent);
  static std::shared_ptr<Node> FileUnopened(Path const &path, std::shared_ptr<Node> const &parent);

private:
  Value fValue;

public:
  std::shared_ptr<Node> const fParent;
};

} // namespace nbte
