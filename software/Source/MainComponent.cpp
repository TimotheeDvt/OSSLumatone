#include "MainComponent.h"

MainComponent::MainComponent() {
  titleLabel.setText("OSSLumatone Editor", juce::dontSendNotification);
  titleLabel.setFont(juce::FontOptions(20.0f, juce::Font::bold));
  statusLabel.setText("Not connected", juce::dontSendNotification);
  statusLabel.setJustificationType(juce::Justification::centredRight);

  connectButton.onClick = [this] { toggleConnection(); };
  resetButton.onClick = [this] {
    layout.resetToDefault();
    keyboardView.repaint();
  };

  keyboardView.onKeySelected = [this](Layout::KeyId key) { keyEditor.showKey(key); };
  keyEditor.onMappingChanged = [this] { keyboardView.repaint(); };

  connection.onNote = [this](int channel, int note, bool pressed) {
    if (const auto key = layout.findKey(channel, note))
      keyboardView.setKeyPressed(*key, pressed);
  };

  for (auto *c : std::initializer_list<juce::Component *>{&titleLabel, &connectButton, &resetButton, &statusLabel,
                                                           &keyboardView, &keyEditor})
    addAndMakeVisible(c);

  setSize(1400, 650);
}

void MainComponent::paint(juce::Graphics &g) { g.fillAll(juce::Colour(0xff1e1f24)); }

void MainComponent::resized() {
  auto area = getLocalBounds();

  auto header = area.removeFromTop(44).reduced(8, 6);
  titleLabel.setBounds(header.removeFromLeft(220));
  connectButton.setBounds(header.removeFromLeft(110));
  header.removeFromLeft(8);
  resetButton.setBounds(header.removeFromLeft(110));
  statusLabel.setBounds(header);

  keyEditor.setBounds(area.removeFromRight(280));
  keyboardView.setBounds(area);
}

void MainComponent::toggleConnection() {
  if (connection.isConnected()) {
    connection.disconnect();
    connectButton.setButtonText("CONNECT");
    statusLabel.setText("Not connected", juce::dontSendNotification);
    return;
  }

  const int sections = connection.connect();
  if (sections == 0) {
    statusLabel.setText("No OSSLumatone section found", juce::dontSendNotification);
    return;
  }

  connectButton.setButtonText("DISCONNECT");
  statusLabel.setText(juce::String(sections) + " / " + juce::String(Layout::sectionCount) + " sections connected",
                      juce::dontSendNotification);
}
