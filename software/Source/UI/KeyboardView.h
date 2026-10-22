#pragma once

#include "../DATA/KeyboardLayout.h"
#include <JuceHeader.h>
#include <bitset>

// Draws the 280 hex keys at their real positions, scaled to fit.
// Click a key to select it; keys held on the instrument are highlighted.
class KeyboardView : public juce::Component {
public:
  explicit KeyboardView(const KeyboardLayout &layout);

  void paint(juce::Graphics &) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent &) override;

  void setSelectedKey(std::optional<Layout::KeyId> key);
  void setKeyPressed(Layout::KeyId key, bool pressed);

  std::function<void(Layout::KeyId)> onKeySelected;

private:
  juce::Path hexagonAt(juce::Point<float> centre) const;
  juce::Point<float> toScreen(juce::Point<float> mm) const;
  std::optional<Layout::KeyId> keyAt(juce::Point<float> position) const;

  const KeyboardLayout &layout;
  std::optional<Layout::KeyId> selectedKey;
  std::bitset<Layout::keyCount> pressedKeys;

  juce::Rectangle<float> boundsMm; // key field bounding box, in mm
  float scale = 1.0f;              // pixels per mm
  juce::Point<float> offset;
};
