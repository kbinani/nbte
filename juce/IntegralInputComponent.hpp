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
    fEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    fEditor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    fEditor.setColour(juce::TextEditor::highlightColourId, juce::Colour::fromFloatRGBA(0.26f, 0.59f, 0.98f, 0.35f));
    fEditor.setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::black);
    fEditor.setText(juce::String(value));
    fEditor.onTextChange = [this]() { onTextChange(); };
    fPlusButton.onClick = [this]() { onPlusButtonClick(); };
    fMinusButton.onClick = [this]() { onMinusButtonClick(); };
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

  std::function<void(T newValue, T oldValue)> onChange;

private:
  void setValue(T v, bool updateEditor = true) {
    if (v == fValue) {
      return;
    }
    T old = v;
    fValue = v;
    if (onChange) {
      onChange(v, old);
    }
    if (updateEditor) {
      fEditor.setText(juce::String(fValue));
    }
  }

  void onTextChange() {
    juce::String text = fEditor.getText();
    try {
      if constexpr (std::is_signed_v<T>) {
        int64_t v = std::stoll(text.toRawUTF8());
        T next = (T)std::min(std::max(v, (int64_t)std::numeric_limits<T>::lowest()), (int64_t)std::numeric_limits<T>::max());
        setValue(next, false);
      } else {
        uint64_t v = std::stoul(text.toRawUTF8());
        T next = (T)std::min(std::max(v, (uint64_t)std::numeric_limits<T>::lowest()), (uint64_t)std::numeric_limits<T>::max());
        setValue(next, false);
      }
    } catch (...) {
    }
  }

  void onPlusButtonClick() {
    if (fValue == std::numeric_limits<T>::max()) {
      return;
    }
    setValue(fValue + 1);
  }

  void onMinusButtonClick() {
    if (fValue == std::numeric_limits<T>::lowest()) {
      return;
    }
    setValue(fValue - 1);
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
