#pragma once

namespace nbte {

class StringComponent : public juce::Component {
public:
  StringComponent(juce::Image const &icon, juce::String const &label, juce::String const &value) : fIcon(icon), fLabel(label), fValue(value) {
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
    int editorX = kTreeNodeToLabelSpacing + fIcon.getWidth() + kFramePaddingX + fLabelWidth + kFramePaddingX + kFramePaddingX;
    fEditor.setBounds(editorX, kFramePaddingY, juce::jmax(0, width - editorX - kItemInnerSpacing), kFrameHeight);
  }

  void paint(juce::Graphics &g) override {
    g.saveState();
    g.drawImage(fIcon, juce::Rectangle<float>(kTreeNodeToLabelSpacing, kFrameHeightWithSpacing * 0.5f - fIcon.getHeight() * 0.5f, fIcon.getWidth(), fIcon.getHeight()));

    g.setFont(15.0f);
    g.setColour(juce::Colours::black);
    int x = kTreeNodeToLabelSpacing + fIcon.getWidth() + kFramePaddingX;
    g.drawFittedText(fLabel, x, 0, fLabelWidth, getHeight(), juce::Justification::left, 1);
    g.restoreState();
  }

  std::function<void(juce::String const &newValue, juce::String const &oldValue)> onChange;

private:
  void setValue(juce::String const &v, bool updateEditor = true) {
    if (v == fValue) {
      return;
    }
    juce::String old = v;
    fValue = v;
    if (onChange) {
      onChange(v, old);
    }
    if (updateEditor) {
      updateEditorText();
    }
  }

  void updateEditorText() {
    fEditor.setText(fValue);
  }

  void onEditorTextChange() {
    juce::String text = fEditor.getText();
    setValue(text, false);
  }

  void onEditorLostFocus() {
    updateEditorText();
  }

private:
  juce::Image fIcon;
  juce::String fLabel;
  float fLabelWidth;
  juce::String fValue;
  juce::TextEditor fEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringComponent)
};

} // namespace nbte
