#pragma once

namespace nbte {

struct Texture {
  ImTextureID fTexture;
  int fWidth;
  int fHeight;
};

std::optional<Texture> LoadTexture(char const *name, void *devicePtr);

} // namespace nbte
