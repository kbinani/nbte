#pragma once

namespace nbte {

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
  LookAndFeel() {
    juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::UDEVGothic35_Regular_ttf, BinaryData::UDEVGothic35_Regular_ttfSize);
    if (typeface) {
      setDefaultSansSerifTypeface(typeface);
    }
  }
};

} // namespace nbte
