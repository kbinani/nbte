#pragma once

namespace nbte {

class TemporaryDirectory {
public:
  TemporaryDirectory() {
    fRoot = TemporaryDirectoryRoot() / "nbte" / UuidString();
    std::filesystem::create_directories(fRoot);
  }

  ~TemporaryDirectory() {
    try {
      std::error_code ec;
      std::filesystem::remove_all(fRoot, ec);
    } catch (...) {
    }
  }

  Path createTempChildDirectory() {
    Path ret = fRoot / UuidString();
    std::filesystem::create_directories(ret);
    return ret;
  }

private:
  Path fRoot;
};

} // namespace nbte
