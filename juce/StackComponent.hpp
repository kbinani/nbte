#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace nbte {

class StackComponent : public juce::Component, public HeightUpdatable {
public:
  StackComponent() {}

  template <class T, class = std::enable_if<std::is_base_of_v<juce::Component, T>>>
  T *addChildOwned(T *child, bool visible = true) {
    child->setVisible(visible);
    addChildComponent(child);
    fOwnedChildren.emplace_back(child);
    return child;
  }

  void updateHeight(int width) override {
    int y = 0;
    for (auto &child : fOwnedChildren) {
      if (!child->isVisible()) {
        continue;
      }
      if (auto hu = dynamic_cast<HeightUpdatable *>(child.get()); hu) {
        hu->updateHeight(width);
      }
      int height = child->getHeight();
      child->setBounds(0, y, width, height);
      y += height;
    }
    setSize(width, y);
  }

  void childBoundsChanged(juce::Component *) override {
    updateHeight(getWidth());
  }

private:
  std::vector<std::unique_ptr<juce::Component>> fOwnedChildren;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StackComponent)
};

} // namespace nbte
