#pragma once

namespace nbte {

enum class FilterMode {
  Key,
  Value,
};

template <FilterMode Mode>
struct Cache {
  bool containsSearchTerm(std::variant<std::shared_ptr<mcfile::nbt::Tag>, std::shared_ptr<Node>> const &tag, FilterKey const &key) {
    intptr_t ptr;
    if (tag.index() == 0) {
      ptr = (intptr_t)std::get<0>(tag).get();
    } else {
      ptr = (intptr_t)std::get<1>(tag).get();
    }
    if (auto found = fValue.find(ptr); found != fValue.end()) {
      return found->second;
    }
    bool result;
    if (tag.index() == 0) {
      result = containsTerm(std::get<0>(tag), key);
    } else {
      result = containsTerm(std::get<1>(tag), key);
    }
    fValue[ptr] = result;
    return result;
  }

  void revoke(std::shared_ptr<Node> const &node) {
    fValue.erase((intptr_t)node.get());
  }

private:
  bool containsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, FilterKey const &key) {
    using namespace std;
    using namespace mcfile::nbt;

    assert(!key.fSearch.empty());

    switch (tag->type()) {
    case Tag::Type::Byte:
    case Tag::Type::Short:
    case Tag::Type::Int:
    case Tag::Type::Long:
    case Tag::Type::Float:
    case Tag::Type::Double:
    case Tag::Type::ByteArray:
    case Tag::Type::IntArray:
    case Tag::Type::LongArray:
      return false;
    case Tag::Type::Compound:
      if (auto v = dynamic_pointer_cast<CompoundTag>(tag); v) {
        for (auto const &it : *v) {
          if (Mode == FilterMode::Key) {
            String name = it.first;
            if ((key.fCaseSensitive ? name : ToLower(name)).find(key.fSearch) != String::npos) {
              return true;
            }
          }
          if (containsSearchTerm(it.second, key)) {
            return true;
          }
        }
      }
      return false;
    case Tag::Type::List:
      if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
        for (auto const &it : *v) {
          if (containsSearchTerm(it, key)) {
            return true;
          }
        }
      }
      return false;
    case Tag::Type::String:
      if (Mode == FilterMode::Key) {
        return false;
      } else {
        if (auto v = dynamic_pointer_cast<StringTag>(tag); v) {
          String value = v->fValue;
          return (key.fCaseSensitive ? ToLower(value) : value).find(key.fSearch) != String::npos;
        }
      }
      return false;
    default:
      assert(false);
      return false;
    }
    return false;
  }

  bool containsTerm(std::shared_ptr<Node> const &node, FilterKey const &key) {
    if (!node) {
      return false;
    }
    if (key.fSearch.empty()) {
      return true;
    }
    if (auto r = node->region(); r) {
      if (r->fValue.index() != 0) {
        return false;
      }
      for (auto const &it : std::get<0>(r->fValue)) {
        if (!it) {
          continue;
        }
        if (containsTerm(it, key)) {
          return true;
        }
      }
      return false;
    }
    if (auto c = node->compound(); c) {
      if (key.match(c->name())) {
        return true;
      }
      return containsTerm(c->fTag, key);
    }
    if (auto c = node->directoryContents(); c) {
      for (auto const &it : c->fValue) {
        if (containsTerm(it, key)) {
          return true;
        }
      }
      return false;
    }
    if (auto file = node->fileUnopened(); file) {
      return key.match(file->filename().u8string());
    }
    if (auto directory = node->directoryUnopened(); directory) {
      return key.match(directory->filename().u8string());
    }
    return false;
  }

private:
  std::unordered_map<intptr_t, bool> fValue;
};

template <FilterMode Mode, size_t Size>
struct FilterLruCache {
  using ValueType = std::pair<FilterKey, std::shared_ptr<Cache<Mode>>>;

  bool containsSearchTerm(std::variant<std::shared_ptr<mcfile::nbt::Tag>, std::shared_ptr<Node>> const &tag, FilterKey const &key) {
    using namespace std;
    auto found = find_if(fCache.begin(), fCache.end(), [key](auto const &item) { return item.first == key; });
    if (found != fCache.end()) {
      auto ret = found->second->containsSearchTerm(tag, key);
      auto index = std::distance(fCache.begin(), found);
      if (index + 1 != fCache.size()) {
        ValueType copy = *found;
        fCache.erase(found);
        fCache.push_back(copy);
      }
      return ret;
    }
    auto cache = make_shared<Cache<Mode>>();
    if (fCache.size() + 1 > Size) {
      fCache.erase(fCache.begin());
    }
    fCache.push_back(make_pair(key, cache));
    return cache->containsSearchTerm(tag, key);
  }

  void invalidate() {
    fCache.clear();
  }

  void revoke(std::shared_ptr<Node> const &node) {
    for (auto &it : fCache) {
      it.second->revoke(node);
    }
  }

private:
  std::list<ValueType> fCache;
};

template <size_t Size>
struct FilterCacheSelector {
  bool containsTerm(std::variant<std::shared_ptr<mcfile::nbt::Tag>, std::shared_ptr<Node>> const &tag, FilterKey const *key, FilterMode mode) {
    if (!key) {
      return true;
    }
    switch (mode) {
    case FilterMode::Key:
      return fKeyFilterCache.containsSearchTerm(tag, *key);
    case FilterMode::Value:
      return fValueFilterCache.containsSearchTerm(tag, *key);
    }
  }

  void invalidate() {
    fKeyFilterCache.invalidate();
    fValueFilterCache.invalidate();
  }

  void revokeCache(std::shared_ptr<Node> const &node) {
    fKeyFilterCache.revoke(node);
    fValueFilterCache.revoke(node);
  }

private:
  FilterLruCache<FilterMode::Key, Size> fKeyFilterCache;
  FilterLruCache<FilterMode::Value, Size> fValueFilterCache;
};

} // namespace nbte
