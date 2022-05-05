#pragma once

namespace nbte {

using String = std::u8string;

static String ToLower(String const &s) {
  String ret = s;
  std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
  return ret;
}

static String ToString(int v) {
  auto s = std::to_string(v);
  String ret((char8_t const *)s.c_str());
  return ret;
}

} // namespace nbte
