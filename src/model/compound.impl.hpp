#pragma once

namespace nbte {

String Compound::name() const {
  if (fName.index() == 0) {
    return std::get<0>(fName);
  } else {
    assert(fName.index() == 1);
    Path const &path = std::get<1>(fName);
    return path.filename().u8string();
  }
}

String Compound::save() {
  if (fName.index() != 1) {
    return u8"";
  }
  Path const &path = std::get<1>(fName);
  return save(path);
}

String Compound::save(Path const &file) {
  using namespace std;
  using namespace mcfile;
  using namespace mcfile::stream;
  using namespace mcfile::nbt;
  namespace fs = std::filesystem;

  Endian endian = Endian::Big;
  switch (fFormat) {
  case Compound::Format::RawLittleEndian:
    endian = Endian::Little;
    [[fallthrough]];
  case Compound::Format::RawBigEndian: {
    auto stream = make_shared<FileOutputStream>(file);
    OutputStreamWriter writer(stream, endian);
    if (!fTag->writeAsRoot(writer)) {
      return u8"IO Error";
    }
    break;
  }
  case Compound::Format::DeflatedLittleEndian:
    endian = Endian::Little;
    [[fallthrough]];
  case Compound::Format::DeflatedBigEndian: {
    auto stream = make_shared<FileOutputStream>(file);
    if (!CompoundTag::WriteCompressed(*fTag, *stream, endian)) {
      return u8"IO Error";
    }
    break;
  }
  case Compound::Format::GzippedLittleEndian:
    endian = Endian::Little;
    [[fallthrough]];
  case Compound::Format::GzippedBigEndian: {
    auto stream = make_shared<GzFileOutputStream>(file);
    OutputStreamWriter writer(stream, endian);
    if (!fTag->writeAsRoot(writer)) {
      return u8"IO Error";
    }
    break;
  }
  default:
    return u8"Unknown compound tag format";
  }
  return u8"";
}

} // namespace nbte
