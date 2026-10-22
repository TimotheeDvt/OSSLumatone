#pragma once

#include <stdint.h>

// Set per PlatformIO environment (section1 ... section5, prototype6), see platformio.ini
#ifndef SECTION_INDEX
#define SECTION_INDEX 0
#endif

constexpr uint8_t NO_PIN = 0xFF;

// Every key is a Hall-effect switch: a linear Hall sensor under it gives an analog
// reading of the key travel. The sensors go through analog multiplexers to one ADC pin.
// Key i is on mux i / MUX_CHANNELS, channel i % MUX_CHANNELS.
#ifdef PROTOTYPE_6KEY
// 6-key perfboard prototype (hardware/hex_prototype_6key.scad): 1 x 74HC4051, E tied to GND
constexpr uint8_t KEY_COUNT = 6;
constexpr uint8_t MUX_COUNT = 1;
constexpr uint8_t MUX_CHANNELS = 8;
constexpr uint8_t SELECT_PINS[] = {0, 1, 2};
constexpr uint8_t ENABLE_PINS[MUX_COUNT] = {NO_PIN};
#else
// One 56-key section: 4 x CD74HC4067, outputs tied together on the ADC pin
constexpr uint8_t KEY_COUNT = 56; // 7 columns x 8 keys, index = column * 8 + row
constexpr uint8_t MUX_COUNT = 4;
constexpr uint8_t MUX_CHANNELS = 16;
constexpr uint8_t SELECT_PINS[] = {0, 1, 2, 3};
constexpr uint8_t ENABLE_PINS[MUX_COUNT] = {4, 5, 6, 7}; // active low
#endif

constexpr uint8_t ADC_PIN = 26;          // ADC0, common output of the muxes
constexpr uint32_t MUX_SETTLE_US = 2;    // after changing the select lines

// SK6812MINI-E chain, through a 74AHCT125 level shifter (none on the prototype)
constexpr uint8_t LED_PIN = 15;
// 25 % brightness cap keeps one section's LEDs under ~0.6 A (see hardware/README.md, §5)
constexpr uint8_t LED_MAX_BRIGHTNESS = 64;

// Same as the Lumatone: each section plays on its own channel (1-5)
constexpr uint8_t MIDI_CHANNEL = SECTION_INDEX + 1;

// ---------------------------------------------------------------------------
// Key travel, as a position from 0 (rest) to TRAVEL_FULL (bottom-out)
// ---------------------------------------------------------------------------
constexpr int32_t TRAVEL_FULL = 1024;

// Readings taken at boot to find each key's rest value (keys must not be pressed at boot)
constexpr uint16_t REST_SAMPLES = 64;
// Expected ADC distance between rest and bottom-out (12-bit ADC). Each key learns its real
// value the first time it is pressed further. VERIFY: measure with DEBUG_PRINT_KEYS
constexpr int32_t DEFAULT_SPAN = 500;
// The rest value follows slow drift (temperature) while the key is up: 1 / 2^REST_DRIFT_SHIFT per scan
constexpr uint8_t REST_DRIFT_SHIFT = 6;

// Thresholds, in TRAVEL_FULL units. Velocity = time from START to NOTE_ON
constexpr int32_t START_THRESHOLD = TRAVEL_FULL * 10 / 100;
constexpr int32_t NOTE_ON_THRESHOLD = TRAVEL_FULL * 60 / 100;
constexpr int32_t RELEASE_THRESHOLD = TRAVEL_FULL * 40 / 100; // below this the note stops
constexpr int32_t THRESHOLD_HYSTERESIS = TRAVEL_FULL * 3 / 100;

// Velocity curve: START -> NOTE_ON in VELOCITY_FAST_US or less = 127, VELOCITY_SLOW_US or more = 1,
// logarithmic in between. VERIFY by playing
constexpr uint32_t VELOCITY_FAST_US = 4000;
constexpr uint32_t VELOCITY_SLOW_US = 120000;

// Polyphonic aftertouch from how deep a held key is pushed past AFTERTOUCH_START
#define ENABLE_POLY_AFTERTOUCH 1
constexpr int32_t AFTERTOUCH_START = TRAVEL_FULL * 80 / 100;
constexpr uint8_t AFTERTOUCH_MIN_STEP = 2;          // ignore smaller changes (noise)
constexpr uint32_t AFTERTOUCH_INTERVAL_US = 10000;  // at most 100 messages / s per key

// Prints rest, raw value, span and position of every key over USB serial, to tune the values above
// #define DEBUG_PRINT_KEYS
