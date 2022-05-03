#pragma once

namespace nbte {

static std::optional<Region::ValueType> ReadRegion(int rx, int rz, Path path, std::shared_ptr<Node> parent) {
  using namespace std;

  Region::ValueType ret;

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
      string name("chunk " + to_string(cx) + " " + to_string(cz) + " [" + to_string(x) + " " + to_string(z) + " in region]");
      auto node = shared_ptr<Node>(new Node(Node::Value(in_place_index<Node::TypeCompound>, Compound(name, cx, cz, tag, Compound::Format::DeflatedBigEndian)), parent));
      ret.push_back(node);
    }
  }

  return ret;
}

Region::Region(hwm::task_queue &queue, int x, int z, Path const &file, std::shared_ptr<Node> const &parent) : fFile(file), fX(x), fZ(z) {
  fValue = std::make_shared<std::future<std::optional<ValueType>>>(queue.enqueue(ReadRegion, x, z, file, parent));
}

bool Region::wait() {
  using namespace std;
  if (fValue.index() == 0) {
    return true;
  }
  shared_ptr<future<optional<ValueType>>> future = get<1>(fValue);
  auto state = future->wait_for(chrono::seconds(0));
  if (state != future_status::ready) {
    return false;
  }
  if (auto v = future->get(); v) {
    fValue = *v;
  } else {
    fValue = ValueType();
  }
  future.reset();
  return true;
}

std::string Region::save(TemporaryDirectory &tempRoot) {
  using namespace std;
  namespace fs = std::filesystem;
  if (fValue.index() != 0) {
    return "";
  }
  bool dirty = false;
  auto &r = get<0>(fValue);
  for (auto &it : r) {
    if (auto c = it->compound(); c) {
      if (c->fEdited) {
        dirty = true;
        break;
      }
    } else {
      continue;
    }
  }
  if (!dirty) {
    return "";
  }

  Path temp = tempRoot.createTempChildDirectory();
  Path backup = temp / "backup.mca";
  error_code ec;
  fs::rename(fFile, backup, ec);
  if (ec) {
    ec.clear();
    fs::remove_all(temp, ec);
    return "IO Error";
  }
  ec.clear();

  auto out = make_shared<mcfile::stream::FileOutputStream>(fFile);
  bool ok = mcfile::je::Region::SquashChunksAsMca(*out, [this, &r](int x, int z, bool &stop) -> shared_ptr<mcfile::nbt::CompoundTag> {
    for (auto const &it : r) {
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
      return c->fTag;
    }
    return nullptr;
  });
  out.reset();

  if (ok) {
    fs::remove_all(temp, ec);
    return "";
  } else {
    fs::remove(fFile, ec);
    ec.clear();
    fs::rename(backup, fFile, ec);
    ec.clear();
    fs::remove_all(temp, ec);
    return "IO error";
  }
}

} // namespace nbte