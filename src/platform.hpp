#pragma once

namespace nbte {

using Path = std::filesystem::path;

static std::optional<std::filesystem::path> OpenFileDialog() {
  using namespace std;
  namespace fs = std::filesystem;

  nfdchar_t *outPath = nullptr;
  if (NFD_OpenDialog(nullptr, nullptr, &outPath) == NFD_OKAY) {
    u8string selected;
    selected.assign((char8_t const *)outPath);
    free(outPath);
    return fs::path(selected);
  } else {
    return nullopt;
  }
}

static std::optional<std::filesystem::path> OpenDirectoryDialog() {
  using namespace std;
  namespace fs = std::filesystem;

  nfdchar_t *outPath = nullptr;
  if (NFD_PickFolder(nullptr, &outPath) == NFD_OKAY) {
    u8string selected;
    selected.assign((char8_t const *)outPath);
    free(outPath);
    return fs::path(selected);
  } else {
    return nullopt;
  }
}

static int GetModCtrlKeyIndex() {
#if defined(__APPLE__)
  return ImGui::GetKeyIndex(ImGuiKey_ModSuper);
#else
  return ImGui::GetKeyIndex(ImGuiKey_ModCtrl);
#endif
}

static std::string DecorateModCtrl(std::string const &pair) {
#if defined(__APPLE__)
  return "Cmd+" + pair;
#else
  return "Ctrl+" + pair;
#endif
}

static std::string QuitMenuShortcut() {
#if defined(__APPLE__)
  return "Cmd+Q";
#else
  return "Alt+F4";
#endif
}

#if defined(_MSC_VER)
static std::optional<Path> MinecraftSaveDirectory() {
  namespace fs = std::filesystem;
  wchar_t path[MAX_PATH * 2];
  if (!SHGetSpecialFolderPathW(nullptr, path, CSIDL_APPDATA, FALSE)) {
    return std::nullopt;
  }
  Path appData(path);
  Path saves = appData / ".minecraft" / "saves";
  std::error_code ec;
  if (fs::exists(saves, ec)) {
    return saves;
  } else {
    return std::nullopt;
  }
}
#endif

struct Texture {
#if defined(_MSC_VER)
  GLuint fTexture;
#else
#endif
  int fWidth;
  int fHeight;
};

static std::optional<Texture> LoadTexture(unsigned char const *data, unsigned int size) {
  // Load from file
  int width, height, components;
  unsigned char *img = stbi_load_from_memory(data, size, &width, &height, &components, 4);
  if (img == NULL) {
    return std::nullopt;
  }

  // Create a OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

  // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
  stbi_image_free(img);

  Texture ret;
  ret.fTexture = image_texture;
  ret.fWidth = width;
  ret.fHeight = height;
  return ret;
}

} // namespace nbte
