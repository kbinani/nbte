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
  using Value = std::variant<DirectoryContents,                        // DirectoryContents
                             Path,                                     // FileUnopened
                             Path,                                     // DirectoryUnopened
                             Path,                                     // UnsupportedFile
                             std::shared_ptr<Region>,                  // Region
                             std::shared_ptr<mcfile::je::Chunk>,       // ChunkJava
                             std::shared_ptr<mcfile::nbt::CompoundTag> // Compound
                             >;

  Type const fType;

  Node(Type type, Value &&value, std::shared_ptr<Node> parent);

  static std::shared_ptr<Node> OpenFolder(Path const &path);
  static std::shared_ptr<Node> DirectoryUnopened(Path const &path, std::shared_ptr<Node> const &parent);
  static std::shared_ptr<Node> FileUnopened(Path const &path, std::shared_ptr<Node> const &parent);

private:
  Value fValue;
  std::shared_ptr<Node> const fParent;
};

} // namespace nbte
