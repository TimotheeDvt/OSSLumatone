#pragma once

#include "DATA/KeyboardLayout.h"
#include "MIDI/DeviceConnection.h"
#include "UI/KeyEditorPanel.h"
#include "UI/KeyboardView.h"
#include <JuceHeader.h>

class MainComponent : public juce::Component {
public:
  MainComponent();

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  void toggleConnection();

  KeyboardLayout layout;
  DeviceConnection connection;

  juce::Label titleLabel;
  juce::TextButton connectButton{"CONNECT"};
  juce::TextButton resetButton{"RESET MAP"};
  juce::Label statusLabel;

  KeyboardView keyboardView{layout};
  KeyEditorPanel keyEditor{layout};
};
