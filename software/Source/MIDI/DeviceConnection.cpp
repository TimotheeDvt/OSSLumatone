#include "DeviceConnection.h"

DeviceConnection::~DeviceConnection() { disconnect(); }

int DeviceConnection::connect() {
  disconnect();

  for (const auto &device : juce::MidiInput::getAvailableDevices()) {
    if (!device.name.startsWith(deviceNamePrefix))
      continue;
    if (auto input = juce::MidiInput::openDevice(device.identifier, this)) {
      input->start();
      inputs.push_back(std::move(input));
    }
  }

  for (const auto &device : juce::MidiOutput::getAvailableDevices()) {
    if (!device.name.startsWith(deviceNamePrefix))
      continue;
    if (auto output = juce::MidiOutput::openDevice(device.identifier))
      outputs.push_back(std::move(output));
  }

  return (int)inputs.size();
}

void DeviceConnection::disconnect() {
  for (auto &input : inputs)
    input->stop();
  inputs.clear();
  outputs.clear();
}

void DeviceConnection::handleIncomingMidiMessage(juce::MidiInput *, const juce::MidiMessage &message) {
  if (!message.isNoteOnOrOff())
    return;

  const int channel = message.getChannel();
  const int note = message.getNoteNumber();
  const bool pressed = message.isNoteOn();

  // MIDI thread -> message thread
  juce::MessageManager::callAsync([self = juce::WeakReference<DeviceConnection>(this), channel, note, pressed] {
    if (self != nullptr && self->onNote)
      self->onNote(channel, note, pressed);
  });
}
