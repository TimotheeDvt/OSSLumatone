# OSSLumatone Editor

A desktop **JUCE** app to configure the instrument: the open-source counterpart of the Lumatone
Editor. It shows the 280 keys at their real positions, lets you set each key's note, MIDI channel
and colour, and highlights the keys you play. See the [top-level README](../README.md) for the full
system picture and [firmware/README.md](../firmware/README.md) for the code on the Picos.

> **Status:** first skeleton. Editing works inside the app; sending the map to the instrument is
> not implemented yet (see Roadmap).

## Core Features

* **Real key layout:** the same geometry as `hardware/hex_midi_controller.scad` (5 sections × 7
  columns × 8 keys, 24.15 mm hex lattice, each section stepping down half a key), scaled to the window.
* **Per-key editing:** click a key, then set its note (0–127), channel (1–16) and colour in the side panel.
* **Live view:** **CONNECT** opens every MIDI device whose name starts with `OSSLumatone` (one per
  section) and lights the keys you press on the instrument.
* **Default map:** **RESET MAP** restores the Lumatone-style factory map, key `i` of section `s`
  plays note `i` on channel `s + 1`, same as the firmware default.

---

## Building

### Prerequisites

- **CMake 3.22+**
- **Ninja** build system
- The **mold** linker (`target_link_options` in `CMakeLists.txt`; remove that line to use the default linker)
- A C++17-capable compiler (MSVC 2022 on Windows, Clang/GCC on macOS/Linux)
- JUCE cloned as a subdirectory at `JUCE/` (not tracked here)

  ```bash
  cd software
  git clone https://github.com/juce-framework/JUCE.git
  ```

### Steps

Using the provided `Makefile`:

```bash
cd software
make clean    # remove build directory
make          # configure + build
make build    # build only
make run      # build + launch
```

Build artefacts are written to `build/OSSLumatoneEditor_artefacts/Release/OSSLumatone Editor`.

### Runtime Dependencies

The app is dynamically linked, so you might need to install some system libraries:

```bash
# Debian/Ubuntu
sudo apt install libfontconfig1 libfreetype6 libasound2

# Arch
sudo pacman -S fontconfig freetype2 alsa-lib
```

---

## Code Layout

```
software/
├── CMakeLists.txt          # JUCE GUI app target (juce_add_gui_app)
├── Makefile                # Convenience wrapper around CMake
├── JUCE/                   # JUCE library (cloned locally, not tracked)
└── Source/
    ├── Main.cpp                    # JUCEApplication + main window
    ├── MainComponent.{h,cpp}       # Header bar, keyboard view, key editor panel
    ├── DATA/
    │   └── KeyboardLayout.{h,cpp}  # Key geometry (mm) and the 280-key note/channel/colour map
    ├── MIDI/
    │   └── DeviceConnection.{h,cpp}# Opens the "OSSLumatone Section N" MIDI ports
    └── UI/
        ├── KeyboardView.{h,cpp}    # Draws and hit-tests the hex keys
        └── KeyEditorPanel.{h,cpp}  # Note / channel / colour editor for the selected key
```

It is a plain GUI app (`juce_add_gui_app`) rather than a plugin like REMORA: it only talks MIDI to
the instrument and makes no sound.

---

## Roadmap

- **Send the map to the instrument** over SysEx, one message stream per section. The format will be
  defined together with the firmware and documented in [firmware/README.md](../firmware/README.md).
- **Save / load maps**, including import and export of Lumatone `.ltn` files so existing layouts
  can be reused.
- **Layout generators:** fill the board from an isomorphic layout (Wicki-Hayden, Bosanquet,
  Harmonic Table…) and a scale (Scala `.scl`), like the Lumatone Editor's isomorphic tool.
- **Multi-key editing:** select several keys and change them together, colour by pitch class.
- **Built-in test synth**, so the instrument can be tried without a DAW.
