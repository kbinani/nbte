#pragma once

namespace nbte {

static std::string ToLower(std::string const &s) {
  std::string ret = s;
  std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
  return ret;
}

} // namespace nbte
