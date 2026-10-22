#pragma once

#include <JuceHeader.h>
#include <array>
#include <optional>

// Physical layout and per-key mapping of the whole instrument.
// Same geometry as hardware/hex_midi_controller.scad: 5 sections of 7 columns x 8 keys,
// flat-top hexagons on a 24.15 mm lattice, each section stepping down by half a key.
namespace Layout {

constexpr int sectionCount = 5;
constexpr int columnsPerSection = 7;
constexpr int keysPerColumn = 8;
constexpr int keysPerSection = columnsPerSection * keysPerColumn; // 56
constexpr int keyCount = sectionCount * keysPerSection;           // 280

constexpr float keyPitchMm = 24.15f;                           // centre to centre
constexpr float columnSpacingMm = keyPitchMm * 0.8660254f;     // pitch * sqrt(3) / 2
constexpr float keyCornerRadiusMm = 12.1f;                     // 24.2 mm across corners
constexpr int sectionDropHalfKeys = 1;                         // `section_drop` in the SCAD model

// Identifies a key the way the firmware does: section + index (index = column * 8 + row)
struct KeyId {
  int section = 0;
  int index = 0;

  int column() const { return index / keysPerColumn; }
  int row() const { return index % keysPerColumn; }
  int flat() const { return section * keysPerSection + index; }
  static KeyId fromFlat(int flatIndex) { return {flatIndex / keysPerSection, flatIndex % keysPerSection}; }
};

// Key centre in millimetres, x to the right, y up (row 0 = front of the instrument)
juce::Point<float> keyCentreMm(KeyId key);

} // namespace Layout

struct KeyMapping {
  int note = 0;    // 0 - 127
  int channel = 1; // 1 - 16
  juce::Colour colour;
};

// The full 280-key map edited by the app and sent to the instrument
class KeyboardLayout {
public:
  KeyboardLayout();

  // Lumatone-style factory map: key i of section s plays note i on channel s + 1
  void resetToDefault();

  KeyMapping &mapping(Layout::KeyId key) { return keys[(size_t)key.flat()]; }
  const KeyMapping &mapping(Layout::KeyId key) const { return keys[(size_t)key.flat()]; }

  // Finds the key playing (channel, note), used to light keys from incoming MIDI
  std::optional<Layout::KeyId> findKey(int channel, int note) const;

private:
  std::array<KeyMapping, Layout::keyCount> keys;
};
