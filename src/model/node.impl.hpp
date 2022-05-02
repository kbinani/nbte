#pragma once

namespace nbte {

std::shared_ptr<Node> Node::OpenDirectory(Path const &path, hwm::task_queue &queue) {
  using namespace std;
  auto ret = DirectoryUnopened(path, nullptr);
  ret->open(queue);
  return ret;
}

std::shared_ptr<Node> Node::OpenFile(Path const &path, hwm::task_queue &queue) {
  using namespace std;
  auto ret = FileUnopened(path, nullptr);
  ret->open(queue);
  return ret;
}

std::shared_ptr<Node> Node::DirectoryUnopened(Path const &path, std::shared_ptr<Node> const &parent) {
  using namespace std;
  return shared_ptr<Node>(new Node(Value(in_place_index<TypeDirectoryUnopened>, path), parent));
}

std::shared_ptr<Node> Node::FileUnopened(Path const &path, std::shared_ptr<Node> const &parent) {
  using namespace std;
  return shared_ptr<Node>(new Node(Value(in_place_index<TypeFileUnopened>, path), parent));
}

static std::shared_ptr<mcfile::nbt::CompoundTag> ReadCompound(Path const &path, Compound::Format *format) {
  using namespace mcfile::nbt;

  static std::set<mcfile::Endian> const sEndians = {mcfile::Endian::Big, mcfile::Endian::Little};

  if (auto tag = mcfile::nbt::CompoundTag::Read(path, mcfile::Endian::Little); tag) {
    *format = Compound::Format::RawLittleEndian;
    return tag;
  } else if (auto tag = mcfile::nbt::CompoundTag::Read(path, mcfile::Endian::Big); tag) {
    *format = Compound::Format::RawBigEndian;
    return tag;
  } else if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(path, mcfile::Endian::Little); tag) {
    *format = Compound::Format::DeflatedLittleEndian;
    return tag;
  } else if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(path, mcfile::Endian::Big); tag) {
    *format = Compound::Format::DeflatedBigEndian;
    return tag;
  }
  {
    auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(path);
    if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Big); tag) {
      *format = Compound::Format::GzippedBigEndian;
      return tag;
    }
  }
  {
    auto stream = std::make_shared<mcfile::stream::GzFileInputStream>(path);
    if (auto tag = mcfile::nbt::CompoundTag::Read(stream, mcfile::Endian::Little); tag) {
      *format = Compound::Format::GzippedLittleEndian;
      return tag;
    }
  }
  return nullptr;
}

Node::Node(Node::Value &&value, std::shared_ptr<Node> parent) : fValue(value), fParent(parent) {}

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

Path const *Node::unsupportedFile() const {
  if (fValue.index() != TypeUnsupportedFile) {
    return nullptr;
  }
  return &std::get<TypeUnsupportedFile>(fValue);
}

Region *Node::region() {
  if (fValue.index() != TypeRegion) {
    return nullptr;
  }
  return &std::get<TypeRegion>(fValue);
}

UnopenedChunk const *Node::unopenedChunk() const {
  if (fValue.index() != TypeUnopenedChunk) {
    return nullptr;
  }
  return &std::get<TypeUnopenedChunk>(fValue);
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

bool Node::hasParent() const {
  return !!fParent;
}

void Node::open(hwm::task_queue &queue) {
  using namespace std;

  if (auto unopened = directoryUnopened(); unopened) {
    DirectoryContents contents(*unopened, shared_from_this());
    fValue = Value(std::in_place_index<TypeDirectoryContents>, contents);
    return;
  }
  if (auto unopened = fileUnopened(); unopened) {
    Compound::Format format;
    if (auto tag = ReadCompound(*unopened, &format); tag) {
      fValue = Value(std::in_place_index<TypeCompound>, Compound(unopened->filename().string(), tag, format));
      return;
    }

    if (auto pos = mcfile::je::Region::RegionXZFromFile(*unopened); pos) {
      fValue = Value(std::in_place_index<TypeRegion>, Region(queue, pos->fX, pos->fZ, *unopened, shared_from_this()));
      return;
    }

    fValue = Value(std::in_place_index<TypeUnsupportedFile>, *unopened);
    return;
  }
  if (auto unopened = unopenedChunk(); unopened) {
    auto stream = make_shared<mcfile::stream::FileInputStream>(unopened->fFile);
    mcfile::stream::InputStreamReader reader(stream, mcfile::Endian::Big);
    if (!reader.seek(unopened->fOffset + sizeof(uint32_t))) {
      return;
    }
    uint8_t compressionType;
    if (!reader.read(&compressionType)) {
      return;
    }
    if (compressionType != 2) {
      return;
    }
    vector<uint8_t> buffer(unopened->fSize - 1);
    if (!reader.read(buffer)) {
      return;
    }
    if (auto tag = mcfile::nbt::CompoundTag::ReadCompressed(buffer, mcfile::Endian::Big); tag) {
      fValue = Value(std::in_place_index<TypeCompound>, Compound(unopened->name(), tag, Compound::Format::DeflatedBigEndian));
      return;
    }
  }
}

} // namespace nbte
