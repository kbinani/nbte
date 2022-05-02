#pragma once

namespace nbte {

class Node;

class Region {
public:
  Region(hwm::task_queue &queue, int x, int z, Path const &path, std::shared_ptr<Node> const &parent);

  bool wait();

  using ValueType = std::vector<std::shared_ptr<Node>>;

  int fX;
  int fZ;
  std::variant<ValueType, std::shared_ptr<std::future<std::optional<ValueType>>>> fValue;
};

class UnopenedChunk {
public:
  UnopenedChunk(Path file, uint64_t offset, uint64_t size, int cx, int cz, int localX, int localZ) : fFile(file), fOffset(offset), fSize(size), fChunkX(cx), fChunkZ(cz), fLocalChunkX(localX), fLocalChunkZ(localZ) {}

  std::string name() const {
    using namespace std;
    return "chunk " + to_string(fChunkX) + " " + to_string(fChunkZ) + " [" + to_string(fLocalChunkX) + " " + to_string(fLocalChunkZ) + " in region]";
  }

  Path fFile;
  uint64_t fOffset;
  uint64_t fSize;
  int fChunkX;
  int fChunkZ;
  int fLocalChunkX;
  int fLocalChunkZ;
};

class DirectoryContents {
public:
  DirectoryContents(Path const &dir, std::shared_ptr<Node> parent);
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

  Compound(std::string const &name, std::shared_ptr<mcfile::nbt::CompoundTag> const &tag, Format format) : fName(name), fTag(tag), fFormat(format) {}

  std::string fName;
  std::shared_ptr<mcfile::nbt::CompoundTag> fTag;
  Format fFormat;
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

  DirectoryContents const *directoryContents() const;
  Path const *fileUnopened() const;
  Path const *directoryUnopened() const;
  Compound const *compound() const;
  Path const *unsupportedFile() const;
  Region *region();

  std::string description() const;
  bool hasParent() const;

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
