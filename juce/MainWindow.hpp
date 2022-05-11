#pragma once

namespace nbte {

class MainWindow : public juce::DocumentWindow {
public:
  explicit MainWindow(juce::String name)
      : juce::DocumentWindow(name, juce::Colour::fromFloatRGBA(0.94f, 0.94f, 0.94f, 1.00f), juce::DocumentWindow::closeButton | juce::DocumentWindow::maximiseButton | juce::DocumentWindow::minimiseButton) {
    setUsingNativeTitleBar(true);
    setDraggable(false);

    fViewport.reset(new juce::Viewport());
    TextureSet const &icons = TextureSet::Get();
    fStackComponent.reset(new StackComponent());
    fStackComponent->addChildOwned(new LabelComponent(icons.fIconDocument, "Label"));
    fStackComponent->addChildOwned(new LabelComponent(icons.fIconDocumentExclamation, "Label Disabled"))->setEnabled(false);
    fStackComponent->addChildOwned(new IntegralNumberComponent<uint8_t>(icons.fIconDocumentAttributeB, "Byte", 0));
    fStackComponent->addChildOwned(new IntegralNumberComponent<int32_t>(icons.fIconDocumentAttributeI, "Int32", 0));
    fStackComponent->addChildOwned(new FloatNumberComponent<float>(icons.fIconDocumentAttributeF, "Float", 0));
    fStackComponent->addChildOwned(new FloatNumberComponent<double>(icons.fIconDocumentAttributeD, "Double", 0));
    fStackComponent->addChildOwned(new StringComponent(icons.fIconEditSmallCaps, "String", ""));
    auto tree = new TreeComponent(icons.fIconBox, "Data");
    tree->addChildOwned(new FloatNumberComponent<double>(icons.fIconDocumentAttributeD, "BorderCenterX", 0));
    tree->addChildOwned(new LabelComponent(icons.fIconDocument, "Label"));
    auto childTree = new TreeComponent(icons.fIconBlock, "r.0.0.mca");
    childTree->addChildOwned(new TreeComponent(icons.fIconBox, "Chunk 0 0"))->setHideArrow(true);
    tree->addChildOwned(childTree);
    fStackComponent->addChildOwned(tree);
    fViewport->setViewedComponent(fStackComponent.get(), false);
    fViewport->setScrollBarsShown(true, false);
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
      int width = getWidth() - 2 * kWindowPadding;
      fViewport->setBounds(kWindowPadding, kWindowPadding, width, getHeight() - 2 * kWindowPadding);
      fStackComponent->updateHeight(width);
    }
  }

  void closeButtonPressed() override {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
  }

private:
  std::unique_ptr<juce::Viewport> fViewport;
  std::unique_ptr<StackComponent> fStackComponent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace nbte
