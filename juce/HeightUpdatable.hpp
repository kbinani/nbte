#pragma once

namespace nbte {

class HeightUpdatable {
public:
  ~HeightUpdatable() {}
  virtual void updateHeight(int width) = 0;
};

} // namespace nbte
