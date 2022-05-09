#pragma once

namespace nbte {

class LabelComponent : public juce::Component {
public:
  LabelComponent(juce::String const &text, juce::Image const &icon) : fText(text), fIcon(icon) {
    setSize(kFrameHeight, kFrameHeight);
  }

  void paint(juce::Graphics &g) override {
    g.drawImage(fIcon, juce::Rectangle<float>(kPadding, kFrameHeight * 0.5f - fIcon.getHeight() * 0.5f, fIcon.getWidth(), fIcon.getHeight()));

    g.setFont(15.0f);
    g.setColour(juce::Colours::black);
    int x = kPadding + fIcon.getWidth() + kPadding;
    g.drawFittedText(fText, x, 0, getWidth() - x, getHeight(), juce::Justification::left, 1);
  }

private:
  juce::String fText;
  juce::Image fIcon;
};

} // namespace nbte
