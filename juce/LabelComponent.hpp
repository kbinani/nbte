#pragma once

namespace nbte {

class LabelComponent : public juce::Component {
public:
  LabelComponent(juce::Image const &icon, juce::String const &text) : fIcon(icon), fText(text) {
    setSize(kFrameHeightWithSpacing, kFrameHeightWithSpacing);
  }

  void paint(juce::Graphics &g) override {
    g.saveState();
    g.drawImage(fIcon, juce::Rectangle<float>(kTreeNodeToLabelSpacing, kFrameHeightWithSpacing * 0.5f - fIcon.getHeight() * 0.5f, fIcon.getWidth(), fIcon.getHeight()));

    g.setFont(15.0f);
    if (isEnabled()) {
      g.setColour(juce::Colours::black);
    } else {
      g.setColour(juce::Colour::fromFloatRGBA(0.60f, 0.60f, 0.60f, 1.00f));
    }
    int x = kTreeNodeToLabelSpacing + fIcon.getWidth() + kFramePaddingX;
    g.drawFittedText(fText, x, 0, getWidth() - x, getHeight(), juce::Justification::left, 1);
    g.restoreState();
  }

private:
  juce::String fText;
  juce::Image fIcon;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabelComponent)
};

} // namespace nbte
