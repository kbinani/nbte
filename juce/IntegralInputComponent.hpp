#pragma once

namespace nbte {

template <std::integral T>
class IntegralInputComponent : public juce::Component {
public:
  IntegralInputComponent(juce::Image const &icon, juce::String const &label, T value) : fIcon(icon), fLabel(label), fValue(value), fPlusButton("+"), fMinusButton("-") {
    juce::Font font(kFontSize);
    fLabelWidth = font.getStringWidthFloat(label);
    addAndMakeVisible(&fPlusButton);
    addAndMakeVisible(&fMinusButton);
    addAndMakeVisible(&fEditor);

    setSize(kFrameHeightWithSpacing, kFrameHeightWithSpacing);
  }

  void resized() override {
    int width = getWidth();
    int buttonSize = kFrameHeight;
    int buttonX = width - kItemInnerSpacing - buttonSize;
    fPlusButton.setBounds(buttonX, kPaddingY, kFrameHeight, kFrameHeight);
    buttonX = buttonX - kItemInnerSpacing - buttonSize;
    fMinusButton.setBounds(buttonX, kPaddingY, kFrameHeight, kFrameHeight);
    int editorX = kPaddingX + fIcon.getWidth() + kPaddingX + fLabelWidth + kPaddingX + kPaddingX;
    fEditor.setBounds(editorX, kPaddingY, juce::jmax(0, buttonX - editorX - kItemInnerSpacing), kFrameHeight);
  }

  void paint(juce::Graphics &g) override {
    g.saveState();
    g.drawImage(fIcon, juce::Rectangle<float>(kPaddingX, kFrameHeightWithSpacing * 0.5f - fIcon.getHeight() * 0.5f, fIcon.getWidth(), fIcon.getHeight()));

    g.setFont(15.0f);
    g.setColour(juce::Colours::black);
    int x = kPaddingX + fIcon.getWidth() + kPaddingX;
    g.drawFittedText(fLabel, x, 0, fLabelWidth, getHeight(), juce::Justification::left, 1);
    g.restoreState();
  }

private:
  juce::Image fIcon;
  juce::String fLabel;
  float fLabelWidth;
  T fValue;
  juce::TextEditor fEditor;
  SquareButtonComponent fPlusButton;
  SquareButtonComponent fMinusButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntegralInputComponent)
};

} // namespace nbte
