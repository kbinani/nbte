#pragma once

namespace nbte {

enum class FilterMode {
  Key,
  Value,
};

struct CacheKey {
  CacheKey(String const &search, bool caseSensitive) : fSearch(caseSensitive ? search : ToLower(search)), fCaseSensitive(caseSensitive) {}

  bool operator==(CacheKey const &other) const {
    return fSearch == other.fSearch && fCaseSensitive == other.fCaseSensitive;
  }

  String fSearch;
  bool fCaseSensitive;
};

template <FilterMode Mode>
struct Cache {
  bool containsSearchTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, String const &search, bool caseSensitive) {
    intptr_t key = (intptr_t)tag.get();
    if (auto found = fValue.find(key); found != fValue.end()) {
      return found->second;
    }
    bool result = containsTerm(tag, search, caseSensitive);
    fValue[key] = result;
    return result;
  }

private:
  bool containsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag,
                    String const &filter,
                    bool caseSensitive) {
    using namespace std;
    using namespace mcfile::nbt;

    if (filter.empty()) {
      return true;
    }

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
            String key((char8_t const *)it.first.c_str());
            if ((caseSensitive ? key : ToLower(key)).find(filter) != String::npos) {
              return true;
            }
          }
          if (containsSearchTerm(it.second, filter, caseSensitive)) {
            return true;
          }
        }
      }
      return false;
    case Tag::Type::List:
      if (auto v = dynamic_pointer_cast<ListTag>(tag); v) {
        for (auto const &it : *v) {
          if (containsSearchTerm(it, filter, caseSensitive)) {
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
          String value((char8_t const *)v->fValue.c_str());
          return value.find(filter) != String::npos;
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
  using ValueType = std::pair<CacheKey, std::shared_ptr<Cache<Mode>>>;

  bool containsSearchTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag, String const &search, bool caseSensitive) {
    using namespace std;
    CacheKey key(search, caseSensitive);
    auto found = find_if(fCache.begin(), fCache.end(), [&key](auto const &item) { return item.first == key; });
    if (found != fCache.end()) {
      ValueType copy = *found;
      fCache.erase(found);
      fCache.push_back(copy);
      return copy.second->containsSearchTerm(tag, key.fSearch, key.fCaseSensitive);
    }
    auto cache = make_shared<Cache<Mode>>();
    if (fCache.size() + 1 > Size) {
      fCache.erase(fCache.begin());
    }
    fCache.push_back(make_pair(key, cache));
    return cache->containsSearchTerm(tag, key.fSearch, key.fCaseSensitive);
  }

  void invalidate() {
    fCache.clear();
  }

private:
  std::list<ValueType> fCache;
};

template <size_t Size>
struct FilterCacheSelector {
  bool containsTerm(std::shared_ptr<mcfile::nbt::Tag> const &tag,
                    String const &filter,
                    FilterMode mode,
                    bool caseSensitive) {
    if (filter.empty()) {
      return true;
    }
    switch (mode) {
    case FilterMode::Key:
      return fKeyFilterCache.containsSearchTerm(tag, filter, caseSensitive);
    case FilterMode::Value:
      return fValueFilterCache.containsSearchTerm(tag, filter, caseSensitive);
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
