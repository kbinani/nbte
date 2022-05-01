#pragma once

namespace nbte {

using Path = std::filesystem::path;

class Region {
};

class Node;

class DirectoryContents {
public:
  DirectoryContents(Path const &dir, std::shared_ptr<Node> parent);
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

  Compound(std::shared_ptr<mcfile::nbt::CompoundTag> const &tag, Format format) : fTag(tag), fFormat(format) {}

  std::shared_ptr<mcfile::nbt::CompoundTag> fTag;
  Format fFormat;
};

class Node : std::enable_shared_from_this<Node> {
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

  Node(Type type, Value &&value, std::shared_ptr<Node> parent);

  DirectoryContents const *directoryContents() const;
  Path const *fileUnopened() const;
  Path const *directoryUnopened() const;
  Compound const *compound() const;

  std::string description() const;

  static std::shared_ptr<Node> OpenDirectory(Path const &path);
  static std::shared_ptr<Node> OpenCompound(Path const &path);

  static std::shared_ptr<Node> DirectoryUnopened(Path const &path, std::shared_ptr<Node> const &parent);
  static std::shared_ptr<Node> FileUnopened(Path const &path, std::shared_ptr<Node> const &parent);

private:
  Type const fType;
  Value fValue;
  std::shared_ptr<Node> const fParent;
};

} // namespace nbte
