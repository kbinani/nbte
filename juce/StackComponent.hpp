#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace nbte {

class StackComponent : public juce::Component {
public:
  StackComponent() {}

  void addChildOwned(juce::Component *child, bool visible = true) {
    child->setVisible(visible);
    addChildComponent(child);
    fOwnedChildren.push_back(std::unique_ptr<juce::Component>(child));
  }

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
  std::vector<std::unique_ptr<juce::Component>> fOwnedChildren;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StackComponent)
};

} // namespace nbte
