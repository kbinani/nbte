#pragma once

namespace nbte {

class SquareButtonComponent : public juce::Component, juce::Timer {
  enum State {
    Idle,
    Hovered,
    Active,
  };

  enum {
    kKeyRepeatDelay = 275,
    kKeyRepeatRate = 50,
  };

public:
  explicit SquareButtonComponent(juce::String const &label) : fLabel(label), fState(Idle) {
  }

  void paint(juce::Graphics &g) override {
    g.saveState();
    if (fState == Active) {
      g.setColour(juce::Colour::fromFloatRGBA(0.06f, 0.53f, 0.98f, 1.00f));
    } else if (fState == Hovered) {
      g.setColour(juce::Colour::fromFloatRGBA(0.26f, 0.59f, 0.98f, 1.00f));
    } else {
      g.setColour(juce::Colour::fromFloatRGBA(0.26f, 0.59f, 0.98f, 0.40f));
    }
    g.fillRect(getLocalBounds());
    g.setColour(juce::Colours::black);
    g.drawFittedText(fLabel, getLocalBounds(), juce::Justification::centred, 1);
    g.restoreState();
  }

  void mouseEnter(juce::MouseEvent const &e) override {
    fState = Hovered;
    repaint();
  }

  void mouseExit(juce::MouseEvent const &e) override {
    fState = Idle;
    repaint();
  }

  void mouseDown(juce::MouseEvent const &e) override {
    fState = Active;
    repaint();
    stopTimer();
    startTimer(kKeyRepeatDelay);
  }

  void mouseUp(juce::MouseEvent const &e) override {
    fState = Idle;
    repaint();
    stopTimer();
    if (onClick) {
      onClick();
    }
  }

  void timerCallback() override {
    if (getTimerInterval() != kKeyRepeatRate) {
      stopTimer();
      startTimer(kKeyRepeatRate);
    }
    if (onClick) {
      onClick();
    }
  }

public:
  std::function<void()> onClick;

private:
  juce::String fLabel;
  State fState;
};

} // namespace nbte
