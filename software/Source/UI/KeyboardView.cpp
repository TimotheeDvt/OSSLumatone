#include "KeyboardView.h"

KeyboardView::KeyboardView(const KeyboardLayout &l) : layout(l) {
  for (int flat = 0; flat < Layout::keyCount; ++flat) {
    const auto centre = Layout::keyCentreMm(Layout::KeyId::fromFlat(flat));
    const auto keyBox = juce::Rectangle<float>(Layout::keyCornerRadiusMm * 2.0f, Layout::keyCornerRadiusMm * 2.0f)
                            .withCentre(centre);
    boundsMm = flat == 0 ? keyBox : boundsMm.getUnion(keyBox);
  }
}

void KeyboardView::resized() {
  const auto area = getLocalBounds().toFloat().reduced(10.0f);
  scale = juce::jmin(area.getWidth() / boundsMm.getWidth(), area.getHeight() / boundsMm.getHeight());
  offset = area.getCentre() - juce::Point<float>(boundsMm.getCentreX(), -boundsMm.getCentreY()) * scale;
}

juce::Point<float> KeyboardView::toScreen(juce::Point<float> mm) const {
  // Flip y: row 0 is the front of the instrument, drawn at the bottom
  return offset + juce::Point<float>(mm.x, -mm.y) * scale;
}

juce::Path KeyboardView::hexagonAt(juce::Point<float> centre) const {
  // Flat-top hexagon, a small gap between keys
  const float radius = Layout::keyCornerRadiusMm * scale * 0.94f;
  juce::Path hex;
  for (int corner = 0; corner < 6; ++corner) {
    const float angle = juce::MathConstants<float>::pi / 3.0f * (float)corner;
    const juce::Point<float> p(centre.x + radius * std::cos(angle), centre.y + radius * std::sin(angle));
    if (corner == 0)
      hex.startNewSubPath(p);
    else
      hex.lineTo(p);
  }
  hex.closeSubPath();
  return hex;
}

void KeyboardView::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(0xff15161a));

  for (int flat = 0; flat < Layout::keyCount; ++flat) {
    const auto key = Layout::KeyId::fromFlat(flat);
    const auto &mapping = layout.mapping(key);
    const auto centre = toScreen(Layout::keyCentreMm(key));
    const auto hex = hexagonAt(centre);

    g.setColour(pressedKeys[(size_t)flat] ? juce::Colours::white : mapping.colour);
    g.fillPath(hex);

    const bool selected = selectedKey.has_value() && selectedKey->flat() == flat;
    g.setColour(selected ? juce::Colours::white : juce::Colours::black.withAlpha(0.5f));
    g.strokePath(hex, juce::PathStrokeType(selected ? 2.5f : 1.0f));

    if (scale > 1.5f) {
      g.setColour(juce::Colours::black.withAlpha(0.7f));
      g.setFont(juce::FontOptions(9.0f * scale / 2.0f));
      g.drawText(juce::String(mapping.note),
                 juce::Rectangle<float>(Layout::keyPitchMm * scale, Layout::keyPitchMm * scale).withCentre(centre),
                 juce::Justification::centred);
    }
  }
}

std::optional<Layout::KeyId> KeyboardView::keyAt(juce::Point<float> position) const {
  for (int flat = 0; flat < Layout::keyCount; ++flat) {
    const auto key = Layout::KeyId::fromFlat(flat);
    if (hexagonAt(toScreen(Layout::keyCentreMm(key))).contains(position))
      return key;
  }
  return std::nullopt;
}

void KeyboardView::mouseDown(const juce::MouseEvent &e) {
  if (const auto key = keyAt(e.position)) {
    setSelectedKey(key);
    if (onKeySelected)
      onKeySelected(*key);
  }
}

void KeyboardView::setSelectedKey(std::optional<Layout::KeyId> key) {
  selectedKey = key;
  repaint();
}

void KeyboardView::setKeyPressed(Layout::KeyId key, bool pressed) {
  pressedKeys[(size_t)key.flat()] = pressed;
  repaint();
}
