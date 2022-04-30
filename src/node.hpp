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

Node::Node(Type type, Node::Value &&value, std::shared_ptr<Node> parent) : fType(type), fValue(value), fParent(parent) {}

} // namespace nbte
