#include <juce_gui_extra/juce_gui_extra.h>

#include "Style.hpp"
#include "StackComponent.hpp"
#include "TextureSet.hpp"
#include "LabelComponent.hpp"
#include "SquareButtonComponent.hpp"
#include "IntegralNumberComponent.hpp"
#include "FloatNumberComponent.hpp"
#include "StringComponent.hpp"
#include "MainWindow.hpp"
#include "LookAndFeel.hpp"

namespace nbte {

class Application : public juce::JUCEApplication {
public:
  Application() {}

  const juce::String getApplicationName() override {
    return JUCE_APPLICATION_NAME_STRING;
  }

  const juce::String getApplicationVersion() override {
    return JUCE_APPLICATION_VERSION_STRING;
  }

  bool moreThanOneInstanceAllowed() override {
    return true;
  }

  void initialise(const juce::String &commandLine) override {
    fLaf.reset(new LookAndFeel());
    juce::LookAndFeel::setDefaultLookAndFeel(fLaf.get());
    fMainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override {
    fMainWindow = nullptr;
  }

  void systemRequestedQuit() override {
    quit();
  }

private:
  std::unique_ptr<LookAndFeel> fLaf;
  std::unique_ptr<MainWindow> fMainWindow;
};

} // namespace nbte

START_JUCE_APPLICATION(nbte::Application)