#pragma once

namespace nbte {

template <std::floating_point T>
class FloatNumberComponent : public juce::Component {
public:
  FloatNumberComponent(juce::Image const &icon, juce::String const &label, T value) : fIcon(icon), fLabel(label), fValue(value) {
    juce::Font font(kFontSize);
    fLabelWidth = font.getStringWidthFloat(label);
    addAndMakeVisible(&fEditor);
    fEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    fEditor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    fEditor.setColour(juce::TextEditor::highlightColourId, juce::Colour::fromFloatRGBA(0.26f, 0.59f, 0.98f, 0.35f));
    fEditor.setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::black);
    updateEditorText();
    fEditor.onTextChange = [this]() { onEditorTextChange(); };
    fEditor.onFocusLost = [this]() { onEditorLostFocus(); };
    setSize(kFrameHeightWithSpacing, kFrameHeightWithSpacing);
  }

  void resized() override {
    int width = getWidth();
    int editorX = kPaddingX + fIcon.getWidth() + kPaddingX + fLabelWidth + kPaddingX + kPaddingX;
    fEditor.setBounds(editorX, kPaddingY, juce::jmax(0, width - editorX - kItemInnerSpacing), kFrameHeight);
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
      updateEditorText();
    }
  }

  void updateEditorText() {
    fEditor.setText(juce::String::formatted("%.6f", fValue));
  }

  void onEditorTextChange() {
    juce::String text = fEditor.getText();
    try {
      double v = std::stod(text.toRawUTF8());
      T next = (T)std::min(std::max(v, (double)std::numeric_limits<T>::lowest()), (double)std::numeric_limits<T>::max());
      setValue(next, false);
    } catch (...) {
    }
  }

  void onEditorLostFocus() {
    updateEditorText();
  }

private:
  juce::Image fIcon;
  juce::String fLabel;
  float fLabelWidth;
  T fValue;
  juce::TextEditor fEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatNumberComponent)
};

} // namespace nbte
