#pragma once

#include <JuceHeader.h>

// Opens the MIDI ports of the instrument: every section shows up as its own
// USB-MIDI device called "OSSLumatone Section N" (see firmware/README.md).
class DeviceConnection : private juce::MidiInputCallback {
public:
  ~DeviceConnection() override;

  // Opens every matching input and output, returns how many sections were found
  int connect();
  void disconnect();
  bool isConnected() const { return !inputs.empty(); }

  // Called on the message thread for every note on / off coming from the instrument
  std::function<void(int channel, int note, bool pressed)> onNote;

  // TODO: send the key map to the sections over SysEx once the format is defined with the firmware

private:
  void handleIncomingMidiMessage(juce::MidiInput *source, const juce::MidiMessage &message) override;

  static constexpr const char *deviceNamePrefix = "OSSLumatone";

  std::vector<std::unique_ptr<juce::MidiInput>> inputs;
  std::vector<std::unique_ptr<juce::MidiOutput>> outputs;

  JUCE_DECLARE_WEAK_REFERENCEABLE(DeviceConnection)
};
