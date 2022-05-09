#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace nbte {

class StackComponent : public juce::Component {
public:
  StackComponent() {}

  void updateHeight(int width) {
    int y = 0;
    for (int i = 0; i < getNumChildComponents(); i++) {
      auto child = getChildComponent(i);
      if (!child->isVisible()) {
        continue;
      }
      int height = child->getHeight();
      child->setBounds(0, y, width, height);
      y += height;
    }
    setSize(width, y);
  }

  void childrenChanged() override {
    updateHeight(getWidth());
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StackComponent)
};

} // namespace nbte
