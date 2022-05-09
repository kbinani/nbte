#pragma once

#include "BinaryData.h"

namespace nbte {

class TextureSet {
public:
  TextureSet() {
    fIconDocument = juce::PNGImageFormat::loadFrom(BinaryData::document_png, BinaryData::document_pngSize);
  }

  juce::Image fIconDocument;

  static TextureSet const &Get() {
    static TextureSet const sInstance;
    return sInstance;
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextureSet)
};

} // namespace nbte
