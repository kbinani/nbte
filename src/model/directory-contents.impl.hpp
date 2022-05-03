#pragma once

namespace nbte {

DirectoryContents::DirectoryContents(Path const &dir, std::shared_ptr<Node> parent) : fDir(dir) {
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

std::string DirectoryContents::save(TemporaryDirectory &temp) {
  for (auto &it : fValue) {
    auto err = it->save(temp);
    if (!err.empty()) {
      return err;
    }
  }
  return "";
}

} // namespace nbte
