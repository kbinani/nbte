#pragma once

namespace nbte {

struct TextureSet {
  std::optional<Texture> fIconDocumentAttributeB;
  std::optional<Texture> fIconDocumentAttributeD;
  std::optional<Texture> fIconDocumentAttributeF;
  std::optional<Texture> fIconDocumentAttributeI;
  std::optional<Texture> fIconDocumentAttributeL;
  std::optional<Texture> fIconDocumentAttributeS;
  std::optional<Texture> fIconEditSmallCaps;
  std::optional<Texture> fIconBox;
  std::optional<Texture> fIconEditList;
  std::optional<Texture> fIconFolder;
  std::optional<Texture> fIconBlock;

  void loadTextures(void *device) {
    fIconDocumentAttributeB = LoadTexture("document_attribute_b.png", device);
    fIconDocumentAttributeD = LoadTexture("document_attribute_d.png", device);
    fIconDocumentAttributeF = LoadTexture("document_attribute_f.png", device);
    fIconDocumentAttributeI = LoadTexture("document_attribute_i.png", device);
    fIconDocumentAttributeL = LoadTexture("document_attribute_l.png", device);
    fIconDocumentAttributeS = LoadTexture("document_attribute_s.png", device);
    fIconEditSmallCaps = LoadTexture("edit_small_caps.png", device);
    fIconBox = LoadTexture("box.png", device);
    fIconEditList = LoadTexture("edit_list.png", device);
    fIconFolder = LoadTexture("folder.png", device);
    fIconBlock = LoadTexture("block.png", device);
  }
};

} // namespace nbte
