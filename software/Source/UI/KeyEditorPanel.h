#pragma once

#include "../DATA/KeyboardLayout.h"
#include <JuceHeader.h>

// Side panel editing the note, channel and colour of the selected key
class KeyEditorPanel : public juce::Component, private juce::ChangeListener {
public:
  explicit KeyEditorPanel(KeyboardLayout &layout);
  ~KeyEditorPanel() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  void showKey(Layout::KeyId key);

  // Called after any edit, so the keyboard view can repaint
  std::function<void()> onMappingChanged;

private:
  void changeListenerCallback(juce::ChangeBroadcaster *) override { applyEdits(); }
  void applyEdits();

  KeyboardLayout &layout;
  std::optional<Layout::KeyId> currentKey;

  juce::Label titleLabel;
  juce::Label noteLabel{{}, "Note"};
  juce::Slider noteSlider{juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft};
  juce::Label channelLabel{{}, "Channel"};
  juce::Slider channelSlider{juce::Slider::IncDecButtons, juce::Slider::TextBoxLeft};
  juce::ColourSelector colourSelector{juce::ColourSelector::showColourspace | juce::ColourSelector::showColourAtTop};
};
