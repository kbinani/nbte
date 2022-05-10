#pragma once

#include "BinaryData.h"

namespace nbte {

class TextureSet {
public:
  TextureSet() {
    using namespace juce;
    using namespace BinaryData;

    fIconDocumentAttributeB = PNGImageFormat::loadFrom(document_attribute_b_png, document_attribute_b_pngSize);
    fIconDocumentAttributeD = PNGImageFormat::loadFrom(document_attribute_d_png, document_attribute_d_pngSize);
    fIconDocumentAttributeF = PNGImageFormat::loadFrom(document_attribute_f_png, document_attribute_f_pngSize);
    fIconDocumentAttributeI = PNGImageFormat::loadFrom(document_attribute_i_png, document_attribute_i_pngSize);
    fIconDocumentAttributeL = PNGImageFormat::loadFrom(document_attribute_l_png, document_attribute_l_pngSize);
    fIconDocumentAttributeS = PNGImageFormat::loadFrom(document_attribute_s_png, document_attribute_s_pngSize);
    fIconEditSmallCaps = PNGImageFormat::loadFrom(edit_small_caps_png, edit_small_caps_pngSize);
    fIconBox = PNGImageFormat::loadFrom(box_png, box_pngSize);
    fIconBlock = PNGImageFormat::loadFrom(block_png, block_pngSize);
    fIconEditList = PNGImageFormat::loadFrom(edit_list_png, edit_list_pngSize);
    fIconFolder = PNGImageFormat::loadFrom(folder_png, folder_pngSize);
    fIconDocument = PNGImageFormat::loadFrom(document_png, document_pngSize);
    fIconDocumentExclamation = PNGImageFormat::loadFrom(document_exclamation_png, document_exclamation_pngSize);
    fIconEditCode = PNGImageFormat::loadFrom(edit_code_png, edit_code_pngSize);
  }

  juce::Image fIconDocumentAttributeB;
  juce::Image fIconDocumentAttributeD;
  juce::Image fIconDocumentAttributeF;
  juce::Image fIconDocumentAttributeI;
  juce::Image fIconDocumentAttributeL;
  juce::Image fIconDocumentAttributeS;
  juce::Image fIconEditSmallCaps;
  juce::Image fIconBox;
  juce::Image fIconBlock;
  juce::Image fIconEditList;
  juce::Image fIconFolder;
  juce::Image fIconDocument;
  juce::Image fIconDocumentExclamation;
  juce::Image fIconEditCode;

  static TextureSet const &Get() {
    static TextureSet const sInstance;
    return sInstance;
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextureSet)
};

} // namespace nbte
