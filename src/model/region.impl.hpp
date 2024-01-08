#pragma once

namespace nbte {

static std::optional<Region::ValueType> ReadRegion(int rx, int rz, Path path, std::shared_ptr<Node> parent) {
  using namespace std;

  Region::ValueType ret;
  ret.resize(1024);

  constexpr uint64_t kSectorSize = 4096;

  auto stream = make_shared<mcfile::stream::FileInputStream>(path);
  mcfile::stream::InputStreamReader sr(stream, mcfile::Endian::Big);

  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      uint64_t const index = (x & 31) + (z & 31) * 32;
      if (!sr.valid()) {
        return nullopt;
      }
      if (!sr.seek(4 * index)) {
        return nullopt;
      }

      uint32_t loc;
      if (!sr.read(&loc)) {
        return nullopt;
      }
      if (loc == 0) {
        // chunk not saved yet
        continue;
      }

      uint64_t sectorOffset = loc >> 8;
      if (!sr.seek(kSectorSize + 4 * index)) {
        return nullopt;
      }

      uint32_t timestamp;
      if (!sr.read(&timestamp)) {
        return nullopt;
      }

      if (!sr.seek(sectorOffset * kSectorSize)) {
        return nullopt;
      }
      uint32_t chunkSize;
      if (!sr.read(&chunkSize)) {
        return nullopt;
      }
      if (chunkSize == 0) {
        // chunk not saved yet
        continue;
      }
#if 0
      UnopenedChunk uc(path, sectorOffset * kSectorSize, chunkSize, rx * 32 + x, rz * 32 + z, x, z);
      auto node = shared_ptr<Node>(new Node(Node::Value(in_place_index<Node::TypeUnopenedChunk>, uc), parent));
#else
      // NOTE: Code for debug: Loading all chunks immediately
      if (!sr.seek(sectorOffset * kSectorSize + sizeof(uint32_t))) {
        return nullopt;
      }
      uint8_t compressionType;
      if (!sr.read(&compressionType)) {
        return nullopt;
      }
      if (compressionType != 2) {
        return nullopt;
      }
      vector<uint8_t> buffer(chunkSize - 1);
      if (!sr.read(buffer)) {
        return nullopt;
      }
      auto tag = mcfile::nbt::CompoundTag::ReadCompressed(buffer, mcfile::Endian::Big);
      if (!tag) {
        return nullopt;
      }
      int cx = rx * 32 + x;
      int cz = rz * 32 + z;
      String name(u8"Chunk " + ToString(cx) + u8" " + ToString(cz) + u8" [" + ToString(x) + u8" " + ToString(z) + u8" in region]");
      auto node = shared_ptr<Node>(new Node(Node::Value(in_place_index<Node::TypeCompound>, Compound(name, cx, cz, tag, Compound::Format::DeflatedBigEndian)), parent));
#endif
      ret[Region::Index(x, z)].swap(node);
    }
  }

  return ret;
}

Region::Region(hwm::task_queue &queue, int x, int z, Path const &file, std::shared_ptr<Node> const &owner) : fFile(file), fX(x), fZ(z), fOwner(owner) {
  fValue = std::make_shared<std::future<std::optional<ValueType>>>(queue.enqueue(ReadRegion, x, z, file, owner));
}

bool Region::wait(State &s) {
  using namespace std;
  if (fValue.index() == 0) {
    return true;
  }
  shared_ptr<future<optional<ValueType>>> future = get<1>(fValue);
  auto state = future->wait_for(chrono::seconds(0));
  if (state != future_status::ready) {
    return false;
  }
  if (auto owner = fOwner.lock(); owner) {
    s.revokeFilterCache(owner);
  }
  if (auto v = future->get(); v) {
    fValue = *v;
  } else {
    fValue = ValueType(1024);
  }
  future.reset();
  return true;
}

String Region::save(TemporaryDirectory &tempRoot) {
  using namespace std;
  namespace fs = std::filesystem;
  if (fValue.index() != 0) {
    return u8"";
  }
  auto const &r = get<0>(fValue);
  bool dirty = isDirty();
  if (!dirty) {
    return u8"";
  }

  Path temp = tempRoot.createTempChildDirectory();
  Path backup = temp / fFile.filename();
  error_code ec;
  fs::rename(fFile, backup, ec);
  if (ec) {
    ec.clear();
    fs::remove_all(temp, ec);
    return u8"IO Error";
  }
  ec.clear();

  auto out = make_shared<mcfile::stream::FileOutputStream>(fFile);
  auto region = mcfile::je::Region::MakeRegion(backup);
  if (!region) {
    return u8"IO Error";
  }
  bool ok = mcfile::je::Region::SquashChunksAsMca(*out, [this, &r, region](int x, int z, mcfile::stream::OutputStream &output, bool &stop) {
    for (auto const &it : r) {
      if (!it) {
        continue;
      }
      auto c = it->compound();
      if (!c) {
        continue;
      }
      if (x != c->fChunkX - fX * 32) {
        continue;
      }
      if (z != c->fChunkZ - fZ * 32) {
        continue;
      }
      if (!mcfile::nbt::CompoundTag::WriteCompressed(*c->fTag, output, mcfile::Endian::Big)) {
        stop = true;
        return;
      }
    }
    if (!region->exportToCompressedNbt(fX * 32 + x, fZ * 32 + z, output)) {
      stop = true;
    }
  });
  out.reset();

  if (ok) {
    fs::remove_all(temp, ec);
    return u8"";
  } else {
    fs::remove(fFile, ec);
    ec.clear();
    fs::rename(backup, fFile, ec);
    ec.clear();
    fs::remove_all(temp, ec);
    return u8"IO error";
  }
}

bool Region::isDirty() const {
  if (fValue.index() != 0) {
    return false;
  }
  auto const &r = get<0>(fValue);
  for (auto const &it : r) {
    if (!it) {
      continue;
    }
    if (auto c = it->compound(); c) {
      if (!c) {
        continue;
      }
      if (c->fEdited) {
        return true;
      }
    }
  }
  return false;
}

} // namespace nbte
