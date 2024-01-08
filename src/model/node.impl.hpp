#pragma once

namespace nbte {

std::shared_ptr<Node> Node::OpenDirectory(Path const &path, hwm::task_queue &queue) {
  using namespace std;
  auto ret = DirectoryUnopened(path, nullptr);
  ret->load(queue);
  return ret;
}

std::shared_ptr<Node> Node::OpenFile(Path const &path, hwm::task_queue &queue) {
  using namespace std;
  auto ret = FileUnopened(path, nullptr);
  ret->load(queue);
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

  if (auto tag = mcfile::nbt::CompoundTag::ReadFromFile(path, mcfile::Endian::Little); tag) {
    *format = Compound::Format::RawLittleEndian;
    return tag;
  } else if (auto tag = mcfile::nbt::CompoundTag::ReadFromFile(path, mcfile::Endian::Big); tag) {
    *format = Compound::Format::RawBigEndian;
    return tag;
  } else if (auto tag = mcfile::nbt::CompoundTag::ReadCompressedFromFile(path, mcfile::Endian::Little); tag) {
    *format = Compound::Format::DeflatedLittleEndian;
    return tag;
  } else if (auto tag = mcfile::nbt::CompoundTag::ReadCompressedFromFile(path, mcfile::Endian::Big); tag) {
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

DirectoryContents *Node::directoryContents() {
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

Compound *Node::compound() {
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

Region const *Node::region() const {
  if (fValue.index() != TypeRegion) {
    return nullptr;
  }
  return &std::get<TypeRegion>(fValue);
}

String Node::description() const {
  if (auto compound = this->compound(); compound) {
    switch (compound->fFormat) {
    case Compound::Format::RawLittleEndian:
      return u8"Raw NBT (LittleEndian)";
    case Compound::Format::RawBigEndian:
      return u8"Raw NBT (BigEndian)";
    case Compound::Format::DeflatedLittleEndian:
      return u8"Deflated NBT (LittleEndian)";
    case Compound::Format::DeflatedBigEndian:
      return u8"Deflated NBT (BigEndian)";
    case Compound::Format::GzippedBigEndian:
      return u8"Gzipped NBT (BigEndian)";
    case Compound::Format::GzippedLittleEndian:
      return u8"Gzipped NBT (LittleEndian)";
    default:
      return u8"Unknown";
    }
  }
  return u8"";
}

bool Node::hasParent() const {
  return fParent.use_count() != 0;
}

void Node::clearDirty() {
  if (auto contents = directoryContents(); contents) {
    for (auto const &it : contents->fValue) {
      it->clearDirty();
    }
  } else if (auto r = region(); r) {
    if (r->fValue.index() == 0) {
      for (auto const &it : std::get<0>(r->fValue)) {
        if (!it) {
          continue;
        }
        it->clearDirty();
      }
    }
  } else if (auto c = compound(); c) {
    c->fEdited = false;
  }
}

void Node::load(hwm::task_queue &queue) {
  using namespace std;

  if (auto unopened = directoryUnopened(); unopened) {
    DirectoryContents contents(*unopened, shared_from_this());
    fValue = Value(std::in_place_index<TypeDirectoryContents>, contents);
    return;
  }
  if (auto unopened = fileUnopened(); unopened) {
    Compound::Format format;
    if (auto tag = ReadCompound(*unopened, &format); tag) {
      fValue = Value(std::in_place_index<TypeCompound>, Compound(*unopened, tag, format));
      return;
    }

    if (auto pos = mcfile::je::Region::RegionXZFromFile(*unopened); pos) {
      fValue = Value(std::in_place_index<TypeRegion>, Region(queue, pos->fX, pos->fZ, *unopened, shared_from_this()));
      return;
    }

    fValue = Value(std::in_place_index<TypeUnsupportedFile>, *unopened);
    return;
  }
}

String Node::save(TemporaryDirectory &temp) {
  if (auto r = region(); r) {
    return r->save(temp);
  } else if (auto contents = directoryContents(); contents) {
    return contents->save(temp);
  } else if (auto c = compound(); c) {
    return c->save();
  }

  return u8"";
}

bool Node::dirtyFiles(std::vector<Path> *buffer) const {
  if (auto r = region(); r) {
    if (r->isDirty()) {
      if (buffer) {
        buffer->push_back(r->fFile);
      }
      return true;
    }
  } else if (auto contents = directoryContents(); contents) {
    return contents->dirtyFiles(buffer);
  } else if (auto c = compound(); c) {
    if (auto editedFile = c->filePathIfEdited(); editedFile) {
      if (buffer) {
        buffer->push_back(*editedFile);
      }
      return true;
    } else {
      return false;
    }
  }
  return false;
}

} // namespace nbte
