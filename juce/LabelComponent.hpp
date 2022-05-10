#pragma once

namespace nbte {

class LabelComponent : public juce::Component {
public:
  LabelComponent(juce::String const &text, juce::Image const &icon, bool disabled) : fText(text), fIcon(icon), fDisabled(disabled) {
    setSize(kFrameHeight, kFrameHeight);
  }

  void paint(juce::Graphics &g) override {
    g.saveState();
    g.drawImage(fIcon, juce::Rectangle<float>(kPaddingX, kFrameHeight * 0.5f - fIcon.getHeight() * 0.5f, fIcon.getWidth(), fIcon.getHeight()));

    g.setFont(15.0f);
    if (fDisabled) {
      g.setColour(juce::Colour::fromFloatRGBA(0.60f, 0.60f, 0.60f, 1.00f));
    } else {
      g.setColour(juce::Colours::black);
    }
    int x = kPaddingX + fIcon.getWidth() + kPaddingX;
    g.drawFittedText(fText, x, 0, getWidth() - x, getHeight(), juce::Justification::left, 1);
    g.restoreState();
  }

private:
  juce::String fText;
  juce::Image fIcon;
  bool fDisabled;
};

} // namespace nbte
