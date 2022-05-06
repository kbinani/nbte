#pragma once

namespace nbte {

DirectoryContents::DirectoryContents(Path const &dir, std::shared_ptr<Node> parent) : fDir(dir) {
  using namespace std;
  namespace fs = std::filesystem;
  error_code ec;
  auto iterator = fs::directory_iterator(dir, ec);
  if (ec) {
    return;
  }
  map<String, Path> directories;
  map<String, Path> files;
  for (auto const &it : iterator) {
    Path p = it.path();
    if (it.is_directory()) {
      directories[p.filename().u8string()] = p;
    } else if (it.is_regular_file()) {
      files[p.filename().u8string()] = p;
    }
  }
  for (auto const &it : directories) {
    fValue.push_back(Node::DirectoryUnopened(it.second, parent));
  }
  for (auto const &it : files) {
    fValue.push_back(Node::FileUnopened(it.second, parent));
  }
}

String DirectoryContents::save(TemporaryDirectory &temp) {
  for (auto &it : fValue) {
    auto err = it->save(temp);
    if (!err.empty()) {
      return err;
    }
  }
  return u8"";
}

} // namespace nbte
