#pragma once

namespace nbte {

enum class FilterMode {
  Key,
  Value,
};

struct FilterKey {
  FilterKey(String const &search, bool caseSensitive) : fSearch(caseSensitive ? search : ToLower(search)), fCaseSensitive(caseSensitive) {}

  bool operator==(FilterKey const &other) const {
    return fSearch == other.fSearch && fCaseSensitive == other.fCaseSensitive;
  }

  bool match(String const &target) const {
    return (fCaseSensitive ? target : ToLower(target)).find(fSearch) != String::npos;
  }

  String fSearch;
  bool fCaseSensitive;
};

template <FilterMode Mode>
struct Cache {
  bool containsSearchTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, FilterKey const &key) {
    intptr_t ptr = (intptr_t)tag.get();
    if (auto found = fValue.find(ptr); found != fValue.end()) {
      return found->second;
    }
    bool result = containsTerm(tag, key);
    fValue[ptr] = result;
    return result;
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
            String name = ReinterpretAsU8String(it.first);
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
          String value = ReinterpretAsU8String(v->fValue);
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

private:
  std::unordered_map<intptr_t, bool> fValue;
};

template <FilterMode Mode, size_t Size>
struct FilterLruCache {
  using ValueType = std::pair<FilterKey, std::shared_ptr<Cache<Mode>>>;

  bool containsSearchTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, FilterKey const &key) {
    using namespace std;
    auto found = find_if(fCache.begin(), fCache.end(), [key](auto const &item) { return item.first == key; });
    if (found != fCache.end()) {
      ValueType copy = *found;
      fCache.erase(found);
      fCache.push_back(copy);
      return copy.second->containsSearchTerm(tag, key);
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

private:
  std::list<ValueType> fCache;
};

template <size_t Size>
struct FilterCacheSelector {
  bool containsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, FilterKey const *key, FilterMode mode) {
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

private:
  FilterLruCache<FilterMode::Key, Size> fKeyFilterCache;
  FilterLruCache<FilterMode::Value, Size> fValueFilterCache;
};

} // namespace nbte
