#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <MIDI.h>

#include <algorithm>

#include "config.h"

Adafruit_USBD_MIDI usbMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMidi, MIDI);

// A key goes Idle -> Moving when it passes START_THRESHOLD, plays its note when it reaches
// NOTE_ON_THRESHOLD (velocity = time spent between the two), and stops below RELEASE_THRESHOLD.
enum class KeyState : uint8_t { Idle, Moving, On };

// Per-key mapping and sensing state. Key index = column * 8 + row, same order as the
// Lumatone's 0-55 key numbering inside a section.
// TODO: load the mapping and calibration from flash, written by the editor over SysEx (see README.md)
struct Key {
    uint8_t note;
    uint8_t channel;
    uint32_t colour;

    int32_t rest16;     // rest reading x16, so slow drift can be followed with integers
    int32_t span;       // ADC distance from rest to bottom-out, learned while playing
    int32_t position;   // 0 .. TRAVEL_FULL
    KeyState state;
    uint32_t moveStartUs;
    uint8_t pressure;   // last aftertouch value sent
    uint32_t pressureUs;
};

Key keys[KEY_COUNT];

// velocity v needs START -> NOTE_ON in at most velocityThresholdUs[v] (slot 0 unused)
uint32_t velocityThresholdUs[128];

// ---------------------------------------------------------------------------
// LEDs run on core 1: updating 56 LEDs blocks for ~1.7 ms, which would stall the key scan
// ---------------------------------------------------------------------------
Adafruit_NeoPixel leds(KEY_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
const uint32_t PRESSED_COLOUR = Adafruit_NeoPixel::Color(255, 255, 255);
volatile uint32_t ledColours[KEY_COUNT];
volatile bool ledsDirty = true;

// TODO: LED chain order follows the PCB routing, map it once the board is drawn
static void setLed(uint8_t index, uint32_t colour) {
    ledColours[index] = colour;
    ledsDirty = true;
}

void setup1() {
    leds.begin();
    leds.setBrightness(LED_MAX_BRIGHTNESS);
}

void loop1() {
    if (!ledsDirty)
        return;
    ledsDirty = false;
    for (uint8_t i = 0; i < KEY_COUNT; ++i)
        leds.setPixelColor(i, ledColours[i]);
    leds.show();
}

// ---------------------------------------------------------------------------
// Sensors
// ---------------------------------------------------------------------------
static void selectChannel(uint8_t mux, uint8_t channel) {
    for (uint8_t m = 0; m < MUX_COUNT; ++m)
        if (ENABLE_PINS[m] != NO_PIN)
            digitalWrite(ENABLE_PINS[m], m == mux ? LOW : HIGH);
    for (uint8_t bit = 0; bit < sizeof(SELECT_PINS); ++bit)
        digitalWrite(SELECT_PINS[bit], (channel >> bit) & 1);
    delayMicroseconds(MUX_SETTLE_US);
}

static int32_t readKey(uint8_t index) {
    selectChannel(index / MUX_CHANNELS, index % MUX_CHANNELS);
    return analogRead(ADC_PIN);
}

static void calibrateRest() {
    int32_t sums[KEY_COUNT] = {};
    for (uint16_t sample = 0; sample < REST_SAMPLES; ++sample)
        for (uint8_t i = 0; i < KEY_COUNT; ++i)
            sums[i] += readKey(i);
    for (uint8_t i = 0; i < KEY_COUNT; ++i) {
        keys[i].rest16 = sums[i] * 16 / REST_SAMPLES;
        keys[i].span = DEFAULT_SPAN;
    }
}

// Logarithmic curve, computed once so the scan loop stays in integers
static void buildVelocityCurve() {
    const float ratio = (float)VELOCITY_FAST_US / (float)VELOCITY_SLOW_US;
    for (int v = 1; v <= 127; ++v)
        velocityThresholdUs[v] = (uint32_t)(VELOCITY_SLOW_US * powf(ratio, (float)(v - 1) / 126.0f));
}

static uint8_t velocityFromTime(uint32_t us) {
    for (int v = 127; v > 1; --v)
        if (us <= velocityThresholdUs[v])
            return (uint8_t)v;
    return 1;
}

// ---------------------------------------------------------------------------
// Key logic
// ---------------------------------------------------------------------------
static void updateAftertouch(Key& key, uint32_t now) {
#if ENABLE_POLY_AFTERTOUCH
    const int32_t depth = key.position - AFTERTOUCH_START;
    const uint8_t pressure =
        depth <= 0 ? 0 : (uint8_t)std::min<int32_t>(127, depth * 127 / (TRAVEL_FULL - AFTERTOUCH_START));
    const uint8_t change = pressure > key.pressure ? pressure - key.pressure : key.pressure - pressure;
    const bool reachedEnd = change > 0 && (pressure == 0 || pressure == 127);
    if ((change >= AFTERTOUCH_MIN_STEP || reachedEnd) && now - key.pressureUs >= AFTERTOUCH_INTERVAL_US) {
        MIDI.sendAfterTouch(key.note, pressure, key.channel);
        key.pressure = pressure;
        key.pressureUs = now;
    }
#else
    (void)key;
    (void)now;
#endif
}

static void updateKey(uint8_t index, int32_t raw, uint32_t now) {
    Key& key = keys[index];

    // Works with either magnet polarity: only the distance from rest matters
    const int32_t deviation = abs(raw - key.rest16 / 16);
    if (deviation > key.span)
        key.span = deviation; // learn the real bottom-out
    key.position = std::min<int32_t>(TRAVEL_FULL, deviation * TRAVEL_FULL / key.span);

    switch (key.state) {
    case KeyState::Idle:
        if (key.position >= START_THRESHOLD) {
            key.state = KeyState::Moving;
            key.moveStartUs = now;
        } else {
            key.rest16 += (raw * 16 - key.rest16) >> REST_DRIFT_SHIFT;
        }
        break;

    case KeyState::Moving:
        if (key.position >= NOTE_ON_THRESHOLD) {
            MIDI.sendNoteOn(key.note, velocityFromTime(now - key.moveStartUs), key.channel);
            key.state = KeyState::On;
            key.pressure = 0;
            key.pressureUs = now;
            setLed(index, PRESSED_COLOUR);
        } else if (key.position < START_THRESHOLD - THRESHOLD_HYSTERESIS) {
            key.state = KeyState::Idle; // touched but not played
        }
        break;

    case KeyState::On:
        if (key.position < RELEASE_THRESHOLD) {
            if (key.pressure != 0)
                MIDI.sendAfterTouch(key.note, 0, key.channel);
            MIDI.sendNoteOff(key.note, 0, key.channel);
            // Released only part way: a new press is timed from here
            key.state = key.position >= START_THRESHOLD ? KeyState::Moving : KeyState::Idle;
            key.moveStartUs = now;
            setLed(index, key.colour);
        } else {
            updateAftertouch(key, now);
        }
        break;
    }
}

static void loadDefaultMapping() {
    for (uint8_t i = 0; i < KEY_COUNT; ++i) {
        keys[i].note = i;
        keys[i].channel = MIDI_CHANNEL;
        keys[i].colour = Adafruit_NeoPixel::ColorHSV(uint16_t(i * 65536UL / KEY_COUNT), 255, 80);
        keys[i].state = KeyState::Idle;
        setLed(i, keys[i].colour);
    }
}

#ifdef DEBUG_PRINT_KEYS
static void printKeys(uint32_t now) {
    static uint32_t lastPrintUs = 0;
    if (now - lastPrintUs < 100000)
        return;
    lastPrintUs = now;
    for (uint8_t i = 0; i < KEY_COUNT; ++i)
        Serial.printf("%2u rest %4ld span %4ld pos %4ld | ", i, (long)(keys[i].rest16 / 16), (long)keys[i].span,
                      (long)keys[i].position);
    Serial.println();
}
#endif

// ---------------------------------------------------------------------------
void setup() {
    char productName[32];
    snprintf(productName, sizeof(productName), "OSSLumatone Section %d", SECTION_INDEX + 1);
    TinyUSBDevice.setManufacturerDescriptor("OSSLumatone");
    TinyUSBDevice.setProductDescriptor(productName);

    usbMidi.setStringDescriptor(productName);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();

    for (uint8_t pin : SELECT_PINS)
        pinMode(pin, OUTPUT);
    for (uint8_t pin : ENABLE_PINS)
        if (pin != NO_PIN) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, HIGH);
        }
    analogReadResolution(12);

    Serial.begin(115200);

    buildVelocityCurve();
    loadDefaultMapping();
    calibrateRest(); // keys must be up at power-on
}

void loop() {
    // Drain incoming MIDI (future: SysEx configuration from the editor)
    while (MIDI.read()) {
    }

    // Scan as fast as possible (~3 kHz for 56 keys): velocity timing depends on it
    for (uint8_t i = 0; i < KEY_COUNT; ++i) {
        const int32_t raw = readKey(i);
        updateKey(i, raw, micros());
    }

#ifdef DEBUG_PRINT_KEYS
    printKeys(micros());
#endif
}
