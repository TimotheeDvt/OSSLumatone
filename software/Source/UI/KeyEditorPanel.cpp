#include "KeyEditorPanel.h"

KeyEditorPanel::KeyEditorPanel(KeyboardLayout &l) : layout(l) {
  titleLabel.setText("Select a key", juce::dontSendNotification);
  titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));

  noteSlider.setRange(0, 127, 1);
  channelSlider.setRange(1, 16, 1);
  noteSlider.onValueChange = [this] { applyEdits(); };
  channelSlider.onValueChange = [this] { applyEdits(); };

  colourSelector.setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
  colourSelector.addChangeListener(this);

  for (auto *c : std::initializer_list<juce::Component *>{&titleLabel, &noteLabel, &noteSlider, &channelLabel,
                                                           &channelSlider, &colourSelector})
    addAndMakeVisible(c);

  setEnabled(false);
}

KeyEditorPanel::~KeyEditorPanel() { colourSelector.removeChangeListener(this); }

void KeyEditorPanel::paint(juce::Graphics &g) { g.fillAll(juce::Colour(0xff26282e)); }

void KeyEditorPanel::resized() {
  auto area = getLocalBounds().reduced(10);
  titleLabel.setBounds(area.removeFromTop(28));
  area.removeFromTop(8);

  auto noteRow = area.removeFromTop(26);
  noteLabel.setBounds(noteRow.removeFromLeft(70));
  noteSlider.setBounds(noteRow);
  area.removeFromTop(6);

  auto channelRow = area.removeFromTop(26);
  channelLabel.setBounds(channelRow.removeFromLeft(70));
  channelSlider.setBounds(channelRow);
  area.removeFromTop(10);

  colourSelector.setBounds(area.removeFromTop(juce::jmin(area.getHeight(), 260)));
}

void KeyEditorPanel::showKey(Layout::KeyId key) {
  // Clear first so loading the new values does not write them into the previous key
  currentKey.reset();

  const auto &mapping = layout.mapping(key);
  titleLabel.setText("Section " + juce::String(key.section + 1) + "  -  key " + juce::String(key.index),
                     juce::dontSendNotification);
  noteSlider.setValue(mapping.note, juce::dontSendNotification);
  channelSlider.setValue(mapping.channel, juce::dontSendNotification);
  colourSelector.setCurrentColour(mapping.colour, juce::dontSendNotification);

  currentKey = key;
  setEnabled(true);
}

void KeyEditorPanel::applyEdits() {
  if (!currentKey.has_value())
    return;

  auto &mapping = layout.mapping(*currentKey);
  mapping.note = (int)noteSlider.getValue();
  mapping.channel = (int)channelSlider.getValue();
  mapping.colour = colourSelector.getCurrentColour();

  if (onMappingChanged)
    onMappingChanged();
}
