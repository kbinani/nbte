#include <juce_gui_extra/juce_gui_extra.h>

namespace nbte {

class MainWindow : public juce::DocumentWindow {
public:
  explicit MainWindow(juce::String name)
      : juce::DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton) {
    setUsingNativeTitleBar(true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    centreWithSize(1280, 720);
#endif

    setVisible(true);
  }

  void closeButtonPressed() override {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace nbte
