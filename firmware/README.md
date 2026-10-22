# OSSLumatone Firmware

C++ firmware for the Raspberry Pi Pico (RP2040) that drives one 56-key section. It reads one
Hall-effect sensor per key through analog multiplexers, turns the key travel into USB-MIDI notes
with **velocity** and **polyphonic aftertouch**, and lights the per-key RGB LEDs. See the
[top-level README](../README.md) for the full system picture and
[hardware/README.md](../hardware/README.md) for the PCB, wiring and parts.

> **Status:** first skeleton, not tested on hardware yet.

## Hardware

| Part | Role |
|---|---|
| Raspberry Pi Pico (RP2040) | One per section, 5 in total, each is its own USB-MIDI device |
| 56 × Moyu Yurui HE switches | Magnetic: the switch only carries a magnet, no electrical contact |
| 56 × linear bipolar Hall sensor | One under each switch, analog output proportional to the field (= key travel) |
| 4 × CD74HC4067 | 16-channel analog muxes, outputs tied together on one ADC pin |
| 56 × SK6812MINI-E | Per-key RGB LED chain |
| 74AHCT125 | Level shifter for the LED data line (3.3 V → 5 V) |

Pins (also in `include/config.h`):

| Pico pins | Use |
|---|---|
| GP0 – GP3 | Mux select S0–S3, shared by the 4 muxes |
| GP4 – GP7 | Mux enable EN0–EN3, active low, one mux on at a time |
| GP26 (ADC0) | Common mux output |
| GP15 | LED data → 74AHCT125 → first LED |

Key numbering inside a section: `index = column × 8 + row` (0–55), the same 0–55 numbering the
Lumatone uses per section. Key `i` is on mux `i / 16`, channel `i mod 16`.

**6-key prototype** (`prototype6` environment, `hardware/hex_prototype_6key.scad`): 6 SS49E
sensors on a perfboard → 1 × 74HC4051 (Y0–Y5), S0–S2 on GP0–GP2, E tied to GND, Z on GP26.
No LEDs.

## Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or the VS Code extension)
- A Raspberry Pi Pico connected over USB (hold **BOOTSEL** while plugging it in for the first flash)

The project uses the [Arduino-Pico core](https://github.com/earlephilhower/arduino-pico) by
Earle Philhower through the [maxgerhardt/platform-raspberrypi](https://github.com/maxgerhardt/platform-raspberrypi)
PlatformIO platform. The first build downloads the toolchain, which takes a while.

## Build / Flash / Monitor

Every section runs the same code. The PlatformIO environment only sets `SECTION_INDEX`, which picks
the MIDI channel (1–5) and the USB device name (`OSSLumatone Section N`).

```bash
cd firmware
pio run -e section1                  # build only
pio run -e section1 -t upload        # build + flash section 1
pio run -e prototype6 -t upload      # 6-key prototype
pio run                              # build everything
pio device monitor
```

**Keep the keys up while the board powers on:** each key's rest value is measured at boot.

## How it works

1. **Scan:** core 0 reads all keys one after the other, as fast as it can (about 3 kHz for 56 keys:
   select the mux channel, wait 2 µs, read the ADC).
2. **Calibration:** each key's rest reading is averaged at boot, then follows slow drift
   (temperature) while the key is up. The distance from rest to bottom-out (`span`) starts at
   `DEFAULT_SPAN` and is learned the first time the key goes deeper. Only the distance from rest
   is used, so it works whichever way the magnet faces the sensor.
3. **Position:** `0` at rest to `1024` at bottom-out.
4. **Notes:** a key starts timing at 10 % travel and plays at 60 %. **Velocity** comes from the time
   between the two: 4 ms or faster = 127, 120 ms or slower = 1, logarithmic in between. The note
   stops when the key comes back up past 40 % travel (hysteresis between the thresholds).
5. **Aftertouch:** while a note is held, pushing past 80 % travel sends polyphonic key pressure
   (0–127), at most 100 messages per second per key.
6. **LEDs:** core 1 drives the LED chain (a 56-LED update takes ~1.7 ms and would stall the
   scan). Rainbow colours by default, white while a key is down, brightness capped at 25 %.

Default mapping, like the Lumatone's factory layout: key `i` plays note `i` on the section's
channel (1–5).

### Tuning

All values are in `include/config.h`: thresholds, velocity curve (`VELOCITY_FAST_US`,
`VELOCITY_SLOW_US`), aftertouch, `DEFAULT_SPAN`, mux settle time, LED brightness.

To measure the real sensor signal, uncomment `DEBUG_PRINT_KEYS`, flash, and run
`pio device monitor`: it prints each key's rest value, span and position 10 times per second.
Press each key to the bottom and set `DEFAULT_SPAN` a bit below the smallest span you see.

## Roadmap

- **Configuration over SysEx:** receive per-key note, channel and colour from the
  [editor](../software/README.md) and store them in flash. The message format is shared with the
  software and will be documented here.
- **LED order:** map the LED chain order to the key order once the PCB routing is fixed.
- **Single USB device:** one master Pico collecting the other sections over UART/I2C, so the
  instrument shows up as one MIDI device instead of five behind a hub.
- **Store the calibration** (rest and span per key) in flash, plus a calibration mode
  ("press every key to the bottom once") started from the editor.
- **Faster scan:** ADC round-robin + DMA, so velocity timing gets finer and the CPU is free.
- **Release velocity** from the time between two thresholds on the way up.

## Dependencies

Declared in `platformio.ini`:

- [Arduino-Pico](https://github.com/earlephilhower/arduino-pico) - RP2040 Arduino core, ships Adafruit TinyUSB (USB-MIDI)
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) - SK6812 LED driver
- [FortySevenEffects MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) - MIDI message handling
