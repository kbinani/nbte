#pragma once

namespace nbte {

DirectoryContents::DirectoryContents(Path const &dir, std::shared_ptr<Node> parent) {
  namespace fs = std::filesystem;
  std::error_code ec;
  auto iterator = fs::directory_iterator(dir, ec);
  if (ec) {
    return;
  }
  for (auto const &it : iterator) {
    if (it.is_directory()) {
      fValue.push_back(Node::DirectoryUnopened(it.path(), parent));
    } else if (it.is_regular_file()) {
      fValue.push_back(Node::FileUnopened(it.path(), parent));
    }
  }
}

std::shared_ptr<Node> Node::OpenFolder(Path const &path) {
  using namespace std;
  DirectoryContents contents(path, nullptr);
  return shared_ptr<Node>(new Node(TypeDirectoryContents,
                                   Value(in_place_index<TypeDirectoryContents>, contents),
                                   nullptr));
}

std::shared_ptr<Node> Node::DirectoryUnopened(Path const &path, std::shared_ptr<Node> const &parent) {
  using namespace std;
  return shared_ptr<Node>(new Node(TypeDirectoryUnopened,
                                   Value(in_place_index<TypeDirectoryUnopened>, path),
                                   parent));
}

std::shared_ptr<Node> Node::FileUnopened(Path const &path, std::shared_ptr<Node> const &parent) {
  using namespace std;
  return shared_ptr<Node>(new Node(TypeFileUnopened,
                                   Value(in_place_index<TypeFileUnopened>, path),
                                   parent));
}

std::shared_ptr<Node> Node::OpenCompound(Path const &path) {
  using namespace std;
  static std::set<mcfile::Endian> const sEndians = {mcfile::Endian::Big, mcfile::Endian::Little};

  if (auto tag = mcfile::nbt::CompoundTag::Read(path, mcfile::Endian::Little); tag) {
    return shared_ptr<Node>(new Node(TypeCompound,
                                     Value(in_place_index<TypeCompound>, Compound(tag, Compound::Format::RawLittleEndian)),
                                     nullptr));
  } else if (auto tag = mcfile::nbt::CompoundTag::Read(path, mcfile::Endian::Big); tag) {
    return shared_ptr<Node>(new Node(TypeCompound,
                                     Value(in_place_index<TypeCompound>, Compound(tag, Compound::Format::RawBigEndian)),
                                     nullptr));
  } else if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(path, mcfile::Endian::Little); tag) {
    return shared_ptr<Node>(new Node(TypeCompound,
                                     Value(in_place_index<TypeCompound>, Compound(tag, Compound::Format::DeflatedLittleEndian)),
                                     nullptr));
  } else if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(path, mcfile::Endian::Big); tag) {
    return shared_ptr<Node>(new Node(TypeCompound,
                                     Value(in_place_index<TypeCompound>, Compound(tag, Compound::Format::DeflatedBigEndian)),
                                     nullptr));
  }
  {
    auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(path);
    if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Big); tag) {
      return shared_ptr<Node>(new Node(TypeCompound,
                                       Value(in_place_index<TypeCompound>, Compound(tag, Compound::Format::GzippedBigEndian)),
                                       nullptr));
    }
  }
  {
    auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(path);
    if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Little); tag) {
      return shared_ptr<Node>(new Node(TypeCompound,
                                       Value(in_place_index<TypeCompound>, Compound(tag, Compound::Format::GzippedLittleEndian)),
                                       nullptr));
    }
  }
  return nullptr;
}

Node::Node(Type type, Node::Value &&value, std::shared_ptr<Node> parent) : fType(type), fValue(value), fParent(parent) {}

DirectoryContents const *Node::directoryContents() const {
  if (fValue.index() != TypeDirectoryContents) {
    return nullptr;
  }
  return &std::get<TypeDirectoryContents>(fValue);
}

Path const *Node::fileUnopened() const {
  if (fValue.index() != TypeFileUnopened) {
    return nullptr;
  }
  return &std::get<TypeFileUnopened>(fValue);
}

Path const *Node::directoryUnopened() const {
  if (fValue.index() != TypeDirectoryUnopened) {
    return nullptr;
  }
  return &std::get<TypeDirectoryUnopened>(fValue);
}

Compound const *Node::compound() const {
  if (fValue.index() != TypeCompound) {
    return nullptr;
  }
  return &std::get<TypeCompound>(fValue);
}

std::string Node::description() const {
  if (auto compound = this->compound(); compound) {
    switch (compound->fFormat) {
    case Compound::Format::RawLittleEndian:
      return "Raw NBT (LittleEndian)";
    case Compound::Format::RawBigEndian:
      return "Raw NBT (BigEndian)";
    case Compound::Format::DeflatedLittleEndian:
      return "Deflated NBT (LittleEndian)";
    case Compound::Format::DeflatedBigEndian:
      return "Deflated NBT (BigEndian)";
    case Compound::Format::GzippedBigEndian:
      return "Gzipped NBT (BigEndian)";
    case Compound::Format::GzippedLittleEndian:
      return "Gzipped NBT (LittleEndian)";
    }
  }
  return "Unknown";
}

} // namespace nbte
