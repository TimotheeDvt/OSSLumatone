# OSSLumatone - an Open-Source Lumatone

**OSSLumatone** is an open-source take on the [Lumatone](https://www.lumatone.io/): a 280-key
hexagonal, isomorphic MIDI keyboard. The Lumatone is a great instrument for microtonal and
isomorphic playing, but it costs several thousand euros and its hardware, firmware and software
are closed. This project rebuilds all three in the open, from affordable keyboard-hobby parts,
so anyone can build, repair, modify and extend their own.

The goal is to match the Lumatone where it matters for playing (same size, same key spacing, same
280-key layout split into 5 sections, velocity and polyphonic aftertouch, per-key RGB lighting,
per-key note/channel mapping), for roughly **€520 in parts** for the full instrument, about
**€190** for a first 56-key section, or **≈ €12** for a 6-key test build.

Every key is a magnetic (Hall-effect) switch: a sensor under it measures how far and how fast the
key moves, which gives **velocity** and **polyphonic aftertouch** like the Lumatone.

---

## Project Overview

The instrument is made of three parts:

**Hardware** - 3D-printed enclosure, switch plates and keycaps (parametric OpenSCAD), Moyu Yurui
HE magnetic switches, one linear Hall sensor and one RGB LED per key, and 5 identical PCBs, one per
56-key section.

**Firmware** - C++ on a Raspberry Pi Pico (RP2040), one per section. It reads the key sensors
through analog multiplexers, sends USB-MIDI notes with velocity and polyphonic aftertouch, and
drives the LEDs. Every section is a class-compliant USB-MIDI device on
its own channel (1–5), like the Lumatone, so no drivers are needed.

**Software** - OSSLumatone Editor, a JUCE desktop app (the Lumatone Editor equivalent) to edit
the note, channel and colour of every key and send the map to the instrument.

```
[ Instrument ]
  └─ 5 sections × 56 keys (HE switches + Hall sensors + SK6812 LEDs)
       └─ 1 Raspberry Pi Pico per section
            │  USB-MIDI  (notes out, configuration SysEx in)
            ▼  internal USB hub → one USB-C cable
[ Host Computer ]
  ├─ OSSLumatone Editor   - key map editing, live key view
  └─ Any DAW / synth      - plays the notes (retune with Scala / MTS-ESP for microtonal use)
```

---

## Repository Structure

```
.
├── hardware/               # Physical instrument
│   ├── README.md                   # Specification, bill of materials, printing and assembly, PCB notes
│   ├── hex_midi_controller.scad    # Full 280-key parametric model (trays, deck, plates, keycaps, kickstand)
│   └── hex_prototype_6key.scad     # 6-key perfboard test build + fit test prints
│
├── firmware/               # PlatformIO project for the Raspberry Pi Pico (RP2040)
│   ├── README.md                   # Pins, build/flash steps, roadmap
│   ├── platformio.ini              # One environment per section (section1 ... section5) + prototype6
│   ├── include/config.h            # Pins, muxes, thresholds, velocity curve, LED brightness cap
│   └── src/main.cpp                # Sensor scan, calibration, velocity/aftertouch, USB-MIDI, LEDs
│
└── software/               # JUCE CMake project (desktop editor)
    ├── README.md                   # Build/run steps, code layout, roadmap
    ├── CMakeLists.txt
    ├── Makefile                    # Convenience wrapper around CMake
    ├── JUCE/                       # JUCE library (cloned locally, not tracked)
    └── Source/                     # Layout data, MIDI connection, keyboard view, key editor
```

---

## Hardware

Lumatone-size body (920 × 355 × 46 mm) split into printable tiles, 280 Moyu Yurui HE switches on a
24.15 mm hex lattice with a Hall sensor under each, SK6812MINI-E LEDs, one PCB and one Pico per
section. See
[`hardware/README.md`](hardware/README.md) for the full specification, parts list with prices,
printing and assembly steps.

---

## Firmware

Analog key scanning, per-key calibration, velocity and aftertouch, USB-MIDI and LED control for
one 56-key section on an RP2040, built with PlatformIO and the Arduino-Pico core. See [`firmware/README.md`](firmware/README.md) for pins,
build/flash steps and the roadmap.

---

## Software

JUCE desktop app to view and edit the 280-key map. See [`software/README.md`](software/README.md)
for build steps and the roadmap.

---

## Status

Early prototype: the hardware model is complete, firmware and software are first skeletons.
Nothing has been tested on a real section yet.

---

## Credits

Instrument design, firmware, and software by **Timothée D.**.

Inspired by the [Lumatone](https://www.lumatone.io/) (not affiliated). Built with
[OpenSCAD](https://openscad.org/), [JUCE](https://juce.com/), [PlatformIO](https://platformio.org/),
[Arduino-Pico](https://github.com/earlephilhower/arduino-pico) and
[Adafruit TinyUSB / NeoPixel](https://github.com/adafruit).
