#pragma once

namespace nbte {

class TreeComponent : public juce::Component, public HeightUpdatable {
  enum {
    kIndentSize = kTreeNodeToLabelSpacing + kFramePaddingX + 4,
  };

  enum {
    kIdle,
    kHovered,
    kActive,
  };

public:
  TreeComponent(juce::Image const &icon, juce::String const &title) : fIcon(icon), fTitle(title) {
    setOpen(true, true);
  }

  template <class T, class = std::enable_if<std::is_base_of_v<juce::Component, T>>>
  T *addChildOwned(T *child, bool visible = true) {
    child->setVisible(visible);
    addChildComponent(child);
    fOwnedChildren.emplace_back(child);
    return child;
  }

  void setHideArrow(bool hide) {
    if (fHideArrow == hide) {
      return;
    }
    fHideArrow = hide;
    repaint();
  }

  void updateHeight(int width) override {
    int height = kFrameHeightWithSpacing;
    if (fOpen) {
      for (auto const &child : fOwnedChildren) {
        if (child->isVisible()) {
          if (auto hu = dynamic_cast<HeightUpdatable *>(child.get()); hu) {
            hu->updateHeight(width - kIndentSize);
          }
          height += child->getHeight();
        }
      }
    }
    setSize(width, height);
  }

  void resized() override {
    int width = getWidth();
    int y = kFrameHeightWithSpacing;
    for (auto const &child : fOwnedChildren) {
      if (!child->isVisible()) {
        continue;
      }
      child->setBounds(kIndentSize, y, width - kIndentSize, child->getHeight());
      y += child->getHeight();
    }
  }

  void paint(juce::Graphics &g) override {
    if (fState == kActive) {
      g.setColour(juce::Colour::fromFloatRGBA(0.26f, 0.59f, 0.98f, 1.00f));
      g.fillRect(buttonBounds());
    } else if (fState == kHovered) {
      g.setColour(juce::Colour::fromFloatRGBA(0.26f, 0.59f, 0.98f, 0.80f));
      g.fillRect(buttonBounds());
    }

    if (!fHideArrow) {
      g.setColour(juce::Colours::black);
      g.fillPath(fTriangle);
    }

    g.drawImage(fIcon, juce::Rectangle<float>(kTreeNodeToLabelSpacing, kFrameHeightWithSpacing * 0.5f - fIcon.getHeight() * 0.5f, fIcon.getWidth(), fIcon.getHeight()));

    g.setFont(kFontSize);
    g.setColour(juce::Colours::black);
    int x = kTreeNodeToLabelSpacing + fIcon.getWidth() + kFramePaddingX;
    g.drawFittedText(fTitle, x, 0, getWidth() - x, kFrameHeightWithSpacing, juce::Justification::left, 1);
  }

  void mouseEnter(juce::MouseEvent const &e) override {
    juce::Component::mouseEnter(e);
    auto button = buttonBounds();
    if (button.contains(e.position)) {
      fState = kHovered;
    } else {
      fState = kIdle;
    }
    repaint();
  }

  void mouseExit(juce::MouseEvent const &e) override {
    juce::Component::mouseExit(e);
    fState = kIdle;
    repaint();
  }

  void mouseMove(juce::MouseEvent const &e) override {
    juce::Component::mouseMove(e);
    auto button = buttonBounds();
    if (button.contains(e.position)) {
      if (fState != kHovered) {
        fState = kHovered;
        repaint();
      }
    } else {
      switch (fState) {
      case kActive:
        fState = kHovered;
        repaint();
        break;
      case kHovered:
        fState = kIdle;
        repaint();
        break;
      case kIdle:
        break;
      }
    }
  }

  void mouseDown(juce::MouseEvent const &e) override {
    juce::Component::mouseDown(e);
    auto button = buttonBounds();
    if (button.contains(e.position)) {
      fState = kActive;
      repaint();
    }
  }

  void mouseDrag(juce::MouseEvent const &e) override {
    juce::Component::mouseDrag(e);
    auto button = buttonBounds();
    if (!button.contains(e.position) && fState == kActive) {
      fState = kHovered;
      repaint();
    }
  }

  void mouseUp(juce::MouseEvent const &e) override {
    juce::Component::mouseUp(e);
    auto button = buttonBounds();
    if (button.contains(e.position)) {
      fState = kHovered;
      setOpen(!fOpen);
    } else {
      fState = kIdle;
    }
    repaint();
  }

  void childBoundsChanged(juce::Component *) override {
    updateHeight(getWidth());
  }

private:
  juce::Rectangle<float> buttonBounds() const {
    return juce::Rectangle<float>(kFramePaddingX, kFramePaddingY, getWidth() - 2 * kFramePaddingX, kFrameHeight);
  }

  void setOpen(bool open, bool force = false) {
    using PointF = juce::Point<float>;

    if (open == fOpen && !force) {
      return;
    }
    if (fHideArrow && !force) {
      return;
    }

    fOpen = open;
    float arrowScale = 0.7f;
    float h = kFontSize;
    float r = h * 0.4f * arrowScale;
    PointF center(kFramePaddingX + kTreeNodeToLabelSpacing * 0.5f, kFrameHeightWithSpacing * 0.5f);
    PointF a, b, c;
    if (fOpen) {
      a = PointF(0, 0.75f) * r;
      b = PointF(-0.866f, -0.75f) * r;
      c = PointF(0.866f, -0.75f) * r;
    } else {
      a = PointF(0.750f, 0) * r;
      b = PointF(-0.750f, 0.866f) * r;
      c = PointF(-0.750f, -0.866f) * r;
    }
    juce::Path triangle;
    triangle.addTriangle(center + a, center + b, center + c);
    fTriangle = triangle;

    updateHeight(getWidth());
  }

private:
  std::vector<std::unique_ptr<juce::Component>> fOwnedChildren;
  juce::Image fIcon;
  juce::String fTitle;
  bool fOpen = true;
  juce::Path fTriangle;
  int fState = kIdle;
  bool fHideArrow = false;
};

} // namespace nbte
