#pragma once

namespace nbte {

class MainWindow : public juce::DocumentWindow {
public:
  explicit MainWindow(juce::String name)
      : juce::DocumentWindow(name, juce::Colour::fromFloatRGBA(0.94f, 0.94f, 0.94f, 1.00f), juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton) {
    setUsingNativeTitleBar(true);
    fViewport.reset(new juce::Viewport());
    fStackComponent.reset(new StackComponent());
    fLabelComponent.reset(new LabelComponent("Sample", TextureSet::Get().fIconDocument));
    fStackComponent->addAndMakeVisible(fLabelComponent.get());
    fViewport->setViewedComponent(fStackComponent.get(), false);
    setContentNonOwned(fViewport.get(), false);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    setResizable(true, true);
    centreWithSize(1280, 720);
#endif

    setVisible(true);
  }

  void resized() override {
    if (fViewport) {
      fViewport->setBounds(0, 0, getWidth(), getHeight());
      fStackComponent->updateHeight(getWidth());
    }
  }

  void closeButtonPressed() override {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }

private:
  std::unique_ptr<juce::Viewport> fViewport;
  std::unique_ptr<StackComponent> fStackComponent;
  std::unique_ptr<LabelComponent> fLabelComponent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace nbte
