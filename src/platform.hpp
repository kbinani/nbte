#pragma once

namespace nbte {

using Path = std::filesystem::path;

static std::optional<std::filesystem::path> OpenFileDialog() {
  using namespace std;
  namespace fs = std::filesystem;

  nfdchar_t *outPath = nullptr;
  if (NFD_OpenDialog(nullptr, nullptr, &outPath) == NFD_OKAY) {
    String selected;
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
    String selected;
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

static String DecorateModCtrl(String const &pair) {
#if defined(__APPLE__)
  return u8"Cmd+" + pair;
#else
  return u8"Ctrl+" + pair;
#endif
}

static String QuitMenuShortcut() {
#if defined(__APPLE__)
  return u8"Cmd+Q";
#else
  return u8"Alt+F4";
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

static Path TemporaryDirectoryRoot() {
#if defined(_MSC_VER)
  wchar_t buffer[2048] = {0};
  GetTempPathW(sizeof(buffer) / sizeof(buffer[0]), buffer);
  return Path(buffer);
#else
  return std::filesystem::temp_directory_path();
#endif
}

static String UuidString() {
  char data[37] = {0};
  uuid4_generate(data);
  String ret;
  ret.assign(data, data + 36);
  return ret;
}

struct Resource {
  Resource(void *data, size_t size, bool systemOwned) : fData(data), fSize(size), fSystemOwned(systemOwned) {}

  Resource(Resource const &) = delete;
  Resource &operator=(Resource const &) = delete;

  ~Resource() {
    if (!fSystemOwned) {
      free(fData);
    }
  }

  void *fData;
  size_t fSize;
  bool const fSystemOwned;
};

static std::unique_ptr<Resource> LoadNamedResource(char const *name) {
  using namespace std;
  namespace fs = std::filesystem;
#if defined(_MSC_VER)
  HINSTANCE self = GetModuleHandle(nullptr);
  HRSRC info = FindResourceA(self, name, "DATA");
  if (!info) {
    return nullptr;
  }
  HANDLE rc = LoadResource(self, info);
  if (!rc) {
    return nullptr;
  }
  void *address = LockResource(rc);
  if (!address) {
    return nullptr;
  }
  int size = SizeofResource(self, info);
  if (size == 0) {
    return nullptr;
  }
  return make_unique<Resource>(address, size, true);
#else
  @autoreleasepool {
    NSString *fileName = [NSString stringWithUTF8String:name];
    NSString *baseName = [fileName stringByDeletingPathExtension];
    NSString *extension = [fileName pathExtension];
    NSURL *url = [[NSBundle mainBundle] URLForResource:baseName withExtension:extension];
    if (!url) {
      return nullptr;
    }
    fs::path path([[url path] cStringUsingEncoding:NSUTF8StringEncoding]);
    auto stream = make_shared<mcfile::stream::FileInputStream>(path);
    vector<uint8_t> buffer;
    mcfile::stream::InputStream::ReadUntilEos(*stream, buffer);
    if (buffer.empty()) {
      return nullptr;
    }
    void *copy = malloc(buffer.size());
    memcpy(copy, buffer.data(), buffer.size());
    return std::make_unique<Resource>(copy, buffer.size(), false);
  }
#endif
}

std::optional<Texture> LoadTexture(char const *name, void *devicePtr) {
#if defined(_MSC_VER)
#if 1
  //TODO:
  return std::nullopt;
#else
  auto resource = LoadNamedResource(name);
  if (!resource) {
    return std::nullopt;
  }

  // Load from file
  int width, height, components;
  unsigned char *img = stbi_load_from_memory((stbi_uc const *)resource->fData, resource->fSize, &width, &height, &components, 4);
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
  ret.fTexture = (ImTextureID)(intptr_t)image_texture;
  ret.fWidth = width;
  ret.fHeight = height;
  return ret;
#endif
#else
  @autoreleasepool {
    id<MTLDevice> device = (__bridge id<MTLDevice>)devicePtr;
    NSImage *image = [NSImage imageNamed:[NSString stringWithUTF8String:name]];
    NSSize size = image.size;
    NSRect rect = NSMakeRect(0, 0, size.width, size.height);
    CGImageRef imageRef = [image CGImageForProposedRect:&rect context:nil hints:nil];

    NSUInteger width = CGImageGetWidth(imageRef);
    NSUInteger height = CGImageGetHeight(imageRef);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

    NSUInteger components = 4;
    void *data = calloc(width * height * components, 1);
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;

    uint32_t bitmapInfo = kCGImageAlphaPremultipliedLast | kCGImageByteOrder32Big;
    CGContextRef ctx = CGBitmapContextCreate(data,
                                             width,
                                             height,
                                             bitsPerComponent,
                                             bytesPerRow,
                                             colorSpace,
                                             bitmapInfo);
    CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), imageRef);

    CGColorSpaceRelease(colorSpace);
    CGContextRelease(ctx);

    MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                 width:width
                                                                                                height:height
                                                                                             mipmapped:YES];
    id<MTLTexture> texture = [device newTextureWithDescriptor:textureDescriptor];

    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    [texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bytesPerRow];

    Texture ret;
    ret.fTexture = texture;
    ret.fWidth = width;
    ret.fHeight = height;
    return ret;
  }
#endif
}

} // namespace nbte
