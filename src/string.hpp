#pragma once

namespace nbte {

using String = std::u8string;

static String ToLower(String const &s) {
  String ret = s;
  std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
  return ret;
}

static std::u8string ReinterpretAsU8String(std::string const &s) {
  return std::u8string((char8_t const *)s.c_str());
}

static std::string ReinterpretAsStdString(std::u8string const &s) {
  return std::string((char const *)s.c_str());
}

template <std::integral T>
static String ToString(T v) {
  return ReinterpretAsU8String(std::to_string(v));
}

} // namespace nbte
