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

  void fillTextEditorBackground(juce::Graphics &g, int width, int height, juce::TextEditor &) override {
    g.setColour(juce::Colours::white);
    g.fillRect(0, 0, width, height);
  }

  void drawTextEditorOutline(juce::Graphics &, int width, int height, juce::TextEditor &) override {}
};

} // namespace nbte
