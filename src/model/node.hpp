#pragma once

namespace nbte {

using Path = std::filesystem::path;

class Region {
};

class Node;

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
    TypeRegionJava,
    TypeChunkJava,
    TypeCompound,
  };
  using Value = std::variant<DirectoryContents,                  // DirectoryContents
                             Path,                               // FileUnopened
                             Path,                               // DirectoryUnopened
                             Path,                               // UnsupportedFile
                             std::shared_ptr<Region>,            // Region
                             std::shared_ptr<mcfile::je::Chunk>, // ChunkJava
                             Compound                            // Compound
                             >;

  Node(Value &&value, std::shared_ptr<Node> parent);

  void open();

  DirectoryContents const *directoryContents() const;
  Path const *fileUnopened() const;
  Path const *directoryUnopened() const;
  Compound const *compound() const;
  Path const *unsupportedFile() const;

  std::string description() const;
  bool hasParent() const;

  static std::shared_ptr<Node> OpenDirectory(Path const &path);
  static std::shared_ptr<Node> OpenCompound(Path const &path);

  static std::shared_ptr<Node> DirectoryUnopened(Path const &path, std::shared_ptr<Node> const &parent);
  static std::shared_ptr<Node> FileUnopened(Path const &path, std::shared_ptr<Node> const &parent);

private:
  Value fValue;
  std::shared_ptr<Node> const fParent;
};

} // namespace nbte