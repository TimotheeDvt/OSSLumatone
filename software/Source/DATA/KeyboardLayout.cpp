#include "KeyboardLayout.h"

namespace Layout {

juce::Point<float> keyCentreMm(KeyId key) {
  const int globalColumn = key.section * columnsPerSection + key.column();
  const int columnInSection = key.column();

  // col_y0() in the SCAD model: odd columns sit half a key lower, each section drops further
  const float columnOffset =
      -((float)(columnInSection % 2) * 0.5f + (float)(key.section * sectionDropHalfKeys) * 0.5f) * keyPitchMm;

  return {(float)globalColumn * columnSpacingMm, columnOffset + (float)key.row() * keyPitchMm};
}

} // namespace Layout

KeyboardLayout::KeyboardLayout() { resetToDefault(); }

void KeyboardLayout::resetToDefault() {
  for (int flat = 0; flat < Layout::keyCount; ++flat) {
    const auto key = Layout::KeyId::fromFlat(flat);
    auto &k = keys[(size_t)flat];
    k.note = key.index;
    k.channel = key.section + 1;
    k.colour = juce::Colour::fromHSV((float)key.index / (float)Layout::keysPerSection, 0.7f, 0.8f, 1.0f);
  }
}

std::optional<Layout::KeyId> KeyboardLayout::findKey(int channel, int note) const {
  for (int flat = 0; flat < Layout::keyCount; ++flat) {
    const auto &k = keys[(size_t)flat];
    if (k.channel == channel && k.note == note)
      return Layout::KeyId::fromFlat(flat);
  }
  return std::nullopt;
}
