#pragma once

namespace nbte {

struct Texture {
#if defined(__APPLE__)
  id<MTLTexture> fTexture;
#else
  ImTextureID fTexture;
#endif
  int fWidth;
  int fHeight;
};

std::optional<Texture> LoadTexture(char const *name, void *devicePtr);

} // namespace nbte
