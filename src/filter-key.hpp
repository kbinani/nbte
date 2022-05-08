#pragma once

namespace nbte {

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

} // namespace nbte
