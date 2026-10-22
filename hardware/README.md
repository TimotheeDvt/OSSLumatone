# OSSLumatone Hardware
### DIY Hex MIDI Controller — a budget Lumatone

Enclosure, key plates, keycaps, PCB outline, parts list and build notes. See the
[top-level README](../README.md) for how this fits into the full project,
[firmware/README.md](../firmware/README.md) for the code running on the Picos and
[software/README.md](../software/README.md) for the desktop editor.

A 280-key hexagonal (isomorphic) USB-MIDI controller with the **Lumatone's dimensions**, built
from keyboard-hobby parts: Hall-effect (magnetic) switches, printed hex keycaps, per-key RGB LEDs
and Raspberry Pi Pico microcontrollers. Like the Lumatone, the keys are grouped in
**5 sections of 56 keys**, so you can build and test one section first.

Each key is a **Moyu Studio Yurui HE** magnetic switch: the switch only carries a magnet, and a
linear Hall sensor on the PCB under it measures how far the key is pressed. That gives
**velocity** (from how fast the key travels) and **polyphonic aftertouch** (from how deep it is
held), like the Lumatone.

| File | What it is |
|---|---|
| `README.md` | This document: specification, parts, prices, build notes |
| `hex_midi_controller.scad` | Parametric OpenSCAD model: enclosure tiles, deck, switch plates, keycaps, kickstand, PCB outline |
| `hex_prototype_6key.scad` | 6-key test build (3 columns × 2 keys, sensors on a perfboard + one multiplexer, no custom PCB) plus test prints for the plate cutout and keycap stem |

---

## 1. Hardware specification

### Taken from the Lumatone

| Item | Lumatone spec | This build |
|---|---|---|
| Dimensions | 92 × 35.5 × 4.6 cm | 920 × 355 × 46 mm (same) |
| Kickstand angle | 15° | 15° (two fold-out legs) |
| Height with kickstand | 12.5 cm at the highest point | ≈ 14.1 cm (see note) |
| Key width (across flats) | 21 mm | 21 mm printed caps |
| Key diameter (across corners) | 24 mm | 24.2 mm |
| Key spacing (centre to centre) | 24.15 mm | 24.15 mm (same) |
| Number of keys | 280 | 280 (same) |
| Key arrangement | 35 vertical columns of 8 keys | 35 columns of 8 = 5 sections × 7 columns |

*Height note:* a flat-bottomed 46 mm box tilted 15° reaches about 14 cm at its back edge, so the
Lumatone's 12.5 cm suggests its back edge is thinner or rounded. Lower `kickstand_angle` in the
model if you want a lower back.

*Layout note:* the column count (35 × 8) comes from Lumatone's help centre. The exact outline of
each Lumatone section isn't published in the sources I found, so the stepped layout here is
"Lumatone-style" rather than a measured copy. Keys sit on a true hexagonal lattice, so any
isomorphic mapping (Bosanquet, Wicki-Hayden, microtonal) works.

### Full specification

| Item | Specification |
|---|---|
| Layout | Flat-top hexagons; keys in a column touch along their flats; columns 20.91 mm apart (24.15 × √3/2), each column offset half a key. Each section steps down by half a key (`section_drop`), giving the Lumatone-like slope |
| Key field | 735.3 × 250.4 mm, centred on the body |
| Switches | Moyu Studio Yurui HE Linear: magnetic (Hall-effect), MX-style standard profile, 5-pin, 3.6 ± 0.2 mm travel, 37 g actuation / 42 g bottom-out, factory lubed, N-pole down, 630 ± 40 G at bottom-out. Plate-mounted, nothing to solder (no electrical contact) |
| Key sensing | 1 × linear **bipolar** Hall sensor per key, SMD (SOT-23) on the PCB top, under the switch's magnet. Bipolar because the magnet's north pole faces down (a south-only sensor such as the DRV5056 would see nothing) |
| Multiplexing | 4 × CD74HC4067 (16-channel analog mux) per section, 64 channels for 56 keys, all read by one Pico ADC pin |
| Expression | Velocity (time between two travel thresholds) and polyphonic aftertouch (travel depth), computed in firmware |
| Switch plate | 5 identical plates, 146.4 × 221.1 × 1.5 mm, 56 × 14.0 mm cutouts each (MX standard). Print them, or order them from JLCPCB in FR4 using the exported DXF |
| Switch stack | MX standard: PCB top 5.0 mm below the plate top, so the PCB hangs 3.5 mm under the plate. The 46 mm body still fits: PCB underside 39.4 mm above the floor, ribs 23 mm high |
| Keycaps | Printed 21 mm flat-top hex caps with an MX cross socket, 14.8 mm above the deck at rest (11.2 mm pressed) |
| Lighting | 1 × SK6812MINI-E addressable RGB LED per key, reverse-mounted under the switch's LED window |
| PCB | 5 identical boards, 142.5 × 198.1 mm, 2 layers, with a 3.3 V LDO for the sensors |
| Microcontroller | 1 × Raspberry Pi Pico (RP2040) per section, screwed to standoffs under its PCB |
| Interface | USB-MIDI class compliant (no drivers on Windows / macOS / Linux / iPad) |
| MIDI mapping | Each section is its own USB-MIDI device on its own channel (1–5), like the Lumatone's per-section channels, so 280 keys can exceed the 128-note MIDI limit for microtonal tunings |
| Connection | The 5 Picos go to a USB hub inside the right end of the body; one USB-C cable goes out the back, next to a DC jack for the hub's power supply |
| Power | 5 V 6 A supply. About 1.2 A per section at most (sensors + capped LEDs), see §5 |
| Enclosure | 10 bolt-together trays + 10 flat deck pieces, all ≤ 240.4 × 177.5 mm (fits a 250 × 210 mm bed), no support material |

### Pico pin plan (per section)

| Pico pins | Use |
|---|---|
| GP0 – GP3 | Mux select S0–S3, shared by the 4 multiplexers |
| GP4 – GP7 | Mux enable EN0–EN3 (active low, one mux on at a time) |
| GP26 (ADC0) | Common output of the 4 muxes, tied together (a disabled 4067 is high-impedance) |
| GP27, GP28 (ADC1, ADC2) | Free: expression pedal, breath sensor… |
| GP15 | LED data → 74AHCT125 level shifter (3.3 V → 5 V) → first LED |
| ADC_VREF | Optional: drive it from the sensor LDO, so readings follow the sensor supply (ratiometric) |
| VSYS | 5 V from the internal supply rail through a Schottky diode (see §5) |
| GND | Ground (keep the sensor/ADC ground return away from the LED current) |

Key `i` of a section (`i = column × 8 + row`, 0–55, the Lumatone's numbering) is on mux `i / 16`,
channel `i mod 16`: columns 0–1 on mux 0, 2–3 on mux 1, 4–5 on mux 2, column 6 on mux 3 (Y0–Y7).

Key centre positions, for placing footprints in KiCad (column `c` = 0…6 inside a section,
key `k` = 0…7 from the front, origin = plate corner): print them from the model's `key_xy()`
function, or use `x = c × 20.914`, `y = k × 24.15 − (c mod 2) × 12.075` plus the plate offset.

---

## 2. Bill of materials

Prices checked on **20 October 2026**, including VAT, **shipping not included**.
Rows marked **(est.)** are my estimates, not checked prices. Stock changes often, so check before ordering.
Quantities include a few spares.

### Full build (280 keys)

| # | Part | Qty | Unit price | Subtotal | Where to buy |
|---|---|---|---|---|---|
| 1 | Moyu Studio Yurui HE Linear switches | 290 | $0.24 | $69.60 (≈ €60, conversion est.) | [Divinikey](https://keeb-finder.com/switches/moyu-studio-yurui-he-linear-switches) (US, ships to France), or [MaxGaming](https://us.maxgaming.com/us/switches/moyu-studio-yurui-he-linear-switch) at $0.29 ($84.10) |
| 2 | SK6812MINI-E RGB LEDs, bundle of 66 | 5 bundles | €11.95 | €59.75 | [splitkb.com](https://splitkb.com/collections/keyboard-parts/products/sk6812mini-e-rgb-leds) (NL) |
| 3 | Raspberry Pi Pico (RP2040) | 5 | €4.33 | €21.65 | [RS Components France](https://fr.rs-online.com/web/p/raspberry-pi/2122162) — or [LDLC](https://www.ldlc.com/fr-lu/fiche/PB00627355.html) at €6.90 |
| 4 | Linear bipolar Hall sensor, SOT-23 (e.g. TI DRV5055A3, about ±85 mT, or a 49E-type SOT-23) | 290 | ≈ €0.30 (est.) | €87.00 | Mouser / LCSC (pick from the JLCPCB parts library if they assemble the boards). 49E-type parts cost a few cents |
| 5 | CD74HC4067 16-channel analog mux (SSOP-24 / TSSOP-24) | 22 | ≈ €0.80 (est.) | €17.60 | Mouser / LCSC |
| 6 | Power parts per section: 3.3 V LDO ≥ 800 mA (e.g. AP7361C-33), Schottky diode (VSYS), bulk capacitors | 5 sets | ≈ €1.50 (est.) | €7.50 | Mouser / LCSC |
| 7 | 100 nF 0603 capacitors (one per sensor + one per LED), pack of 600 | 1 | ≈ €8 (est.) | €8.00 | Any shop |
| 8 | 74AHCT125 level shifter (SOIC-14) | 5 | ≈ €1.00 (est.) | €5.00 | [RS Components France](https://fr.rs-online.com/) — search "74AHCT125" |
| 9 | Custom PCB, 2-layer, 142.5 × 198.1 mm, 5 pcs | 1 order | ≈ €80 (est.) | €80.00 | [JLCPCB](https://cart.jlcpcb.com/quote) — upload your Gerbers for an exact quote |
| 10 | PLA or PETG filament, ~3 kg (trays, decks, plates, legs) | 3 spools | ≈ €20 (est.) | €60.00 | Any shop; or a printing service |
| 11 | Clear resin, ~0.5 L, for the 280 keycaps (or 1 more filament spool) | 1 | ≈ €25 (est.) | €25.00 | Any shop; or a printing service |
| 12 | USB hub, 7 ports, small enough for the end bay | 1 | ≈ €25 (est.) | €25.00 | Any shop |
| 13 | 5 V 6 A power supply with barrel plug (8 mm panel jack) | 1 | ≈ €18 (est.) | €18.00 | Any shop |
| 14 | USB-C panel-mount extension cable | 1 | ≈ €8 (est.) | €8.00 | Any shop |
| 15 | M2 self-tapping screws, 6 mm (≈ 50: plates + Picos) | 1 set | ≈ €8 (est.) | €8.00 | Any shop |
| 16 | M3 × 12 bolts + nuts (≈ 45, to join the trays) | 1 set | ≈ €8 (est.) | €8.00 | Any shop |
| 17 | M4 × 30 bolts + nylon lock nuts (kickstand hinges) | 2 | ≈ €1.50 (est.) | €3.00 | Any shop |
| 18 | Glue for the deck pieces (CA or epoxy) | 1 | ≈ €5 (est.) | €5.00 | Any shop |
| 19 | Micro-USB cables, short (Pico → hub) | 5 | ≈ €2 (est.) | €10.00 | Any shop |
| 20 | Hook-up wire (Pico → PCB, 5 V rail) | 1 | ≈ €5 (est.) | €5.00 | Any shop |

| | |
|---|---|
| Checked prices (rows 1–3) | **≈ €141** (the switches are priced in USD) |
| Estimated items (rows 4–20) | **€380.10** |
| **Total, full 280-key build** | **≈ €520** + shipping |

Compared with the earlier Kailh Choc version (≈ €675): the Choc switches (≈ €182), the bought HEX
keycaps (€153.72) and the diodes (€28.50) are gone. The HE switches, Hall sensors, muxes and
resin cost less than that.

### First section (56 keys, no enclosure)

60 switches ($14.40 ≈ €12) + 60 Hall sensors (≈ €18) + 5 muxes (≈ €4) + power parts and
capacitors (≈ €10) + 1 LED bundle (€11.95) + 1 Pico (€4.33) + level shifter (€1)
+ PCB order (€80, you get 5 boards anyway, enough for the full build) + 1 filament spool for the
plate (€20) + resin for 56 caps (≈ €10) + screws, cable, wire (€15) = **≈ €187** + shipping.

Test the section as plate + PCB on the table before printing the 20 enclosure pieces.

### 6-key prototype (`hex_prototype_6key.scad`)

10 switches (≈ $3) + 6 SS49E through-hole Hall sensors (≈ €3) + 1 × 74HC4051 (≈ €0.50)
+ a 70 × 50 mm perfboard (≈ €1) + 1 Pico (€4.33) + filament and resin you already have
= **≈ €12**. No LEDs. It is the cheapest way to check the plate cutout, the keycap fit, the sensor
signal and the velocity curve before ordering PCBs.

### Ways to make it cheaper

- **Have JLCPCB solder the sensors, muxes and LEDs** (PCB assembly service): costs more per board
  but saves over 1000 tiny SMD solder joints per section. Pick parts from their parts library when
  designing, a 49E-type sensor from there is much cheaper than the DRV5055.
- **Skip the LEDs and level shifter**: saves ~€65 and a lot of soldering.
- Buy switches in bulk (MaxGaming sells up to 1000 at a time) and compare HE switch prices on
  [keeb-finder](https://keeb-finder.com/switches/moyu-studio-yurui-he-linear-switches).

### Tools (not included in the total)

Soldering iron with a fine tip, solder, flux, tweezers (for SMD parts), multimeter,
a 3D printer with a bed of at least 250 × 210 mm (or a printing service), small screwdrivers.

---

## 3. Ready-made alternatives (compare before you build)

| Project | Notes |
|---|---|
| [HexBoard](https://shapingthesilence.com/tech/hexboard-midi-controller/) by Jared DeCook | RP2040, RGB keys, USB-C + TRS MIDI, open firmware. Sold as a kit, good reference design |
| [0x33.board](https://lectronz.com/products/beta-0x33-board-midi-controller) by s-ol | 48 keys, open-source hardware, Choc switches with s-ol's HEX keycaps |
| [Adafruit Hexpad guide](https://learn.adafruit.com/hexpad) | 7 keys, fully documented learning project (CircuitPython) |

---

## 4. 3D-printed parts (`hex_midi_controller.scad`)

Open the file in [OpenSCAD](https://openscad.org/), open **Window → Customizer**, choose a part
(and `tile_x` / `tile_y` for tiles), press **F6** (render), then **File → Export**.

| `part` value | What | How many |
|---|---|---|
| `assembly` | Preview of the whole instrument (trays, deck, plates, PCBs, switches, coloured keys, kickstand) | — |
| `case_tile` | Open tray with floor, walls, joining ribs, plate bosses, Pico standoffs; end trays also carry the front feet and kickstand hinges, and the right back tray has the USB-C and DC openings → **STL** | 10 (`tile_x` 0–4 × `tile_y` 0–1) |
| `deck_tile` | Flat 3 mm top surface around the plates, glued on top of the trays → **STL** | 10 (same numbering) |
| `plate` | Switch plate, 1.5 mm, 56 × 14.0 mm MX cutouts → **STL** | 5 (all identical) |
| `plate_dxf` | Same plate as 2D → **DXF**: order it from JLCPCB in FR4 for a stiffer plate | — |
| `pcb_outline` | 2D board outline → **DXF**, import into KiCad on Edge.Cuts. Same origin as the plate | — |
| `keycap` | One 21 mm hex keycap with an MX cross socket → **STL** | test piece |
| `keycap_print_batch` | 16 keycaps, upside down (socket up), ready to print → **STL** | 18 batches for 280 caps |
| `kickstand_leg` | Fold-out leg, 77.7 mm from hinge to foot (gives exactly 15°) → **STL** | 2 |

Command line examples:
```
openscad -D 'part="case_tile"' -D tile_x=0 -D tile_y=1 -o tray_0_1.stl hex_midi_controller.scad
openscad -D 'part="deck_tile"' -D tile_x=0 -D tile_y=1 -o deck_0_1.stl hex_midi_controller.scad
openscad -D 'part="plate_dxf"' -o plate.dxf hex_midi_controller.scad
```

The OpenSCAD console prints all the key sizes (key field, plate, PCB, tile sizes, leg length,
height on the kickstand). **Everything is parametric**: body size, key spacing, number of
sections and columns, section slope (`section_drop`), wall thickness, kickstand angle, etc.

**Print settings:** PLA or PETG, 0.2 mm layers. Trays: 3 walls, 20 % infill, print as they come
(floor on the bed). Deck pieces and legs: flat on the bed. Plates: 100 % infill, or FR4 from JLCPCB.
Keycaps: clear resin gives the best result (LEDs shine through, and the cross socket comes out
accurate); for FDM use 0.1 mm layers and print a `stem_test` first.

**Assembly order:**
1. Bolt the 10 trays together with M3 bolts through the ribs (5 across, front + back).
2. Install the USB hub in the right end bay, mount the USB-C panel cable and the DC jack.
3. Per section: clip 56 switches into the plate, then fit the PCB under it: the switches' plastic
   posts go through the PCB holes, there is nothing to solder (the switches only carry a magnet).
   Screw the Pico onto its standoffs and wire it to the PCB and to the 5 V rail.
4. Drop each plate/PCB into its window and screw it down (6 × M2 into the bosses).
5. Glue the 10 deck pieces on top of the walls and ribs.
6. Fit the kickstand legs on M4 bolts with lock nuts (tighten until the legs hold by friction).
7. Keycaps on.

**Values to check on a real switch** (marked VERIFY in the files; the Yurui HE datasheet doesn't
give dimensions): `switch_cut` (14.0 mm, print the prototype's `plate_test`), `pcb_to_plate`
(3.5 mm, MX standard), `housing_h` and `stem_rise` (where the keycap sits), the cross size and
`stem_tol` (print the prototype's `stem_test`). The MX cross is symmetric, so the hex caps stay
flat-top whichever way the switches are turned.

Because the switches are MX height, the caps stand 14.8 mm above the deck instead of the few
millimetres of the Choc version. The body height (46 mm) did not need to change.

---

## 5. PCB and firmware notes

**PCB (KiCad, free):**
- Import `pcb_outline.dxf` as the board edge and `plate_dxf` on a user layer to align footprints.
  Place one footprint per key with the formula in §1 (24.15 mm spacing).
- **Switch footprint:** holes for the switch's plastic posts only (MX-style: 4 mm centre post, 1.7 mm
  side pegs). No pads, the switch has no electrical contact. **VERIFY on a real switch** where the
  magnet sits and whether the bottom housing has a centre post: the Hall sensor must sit right
  under the magnet, which is the switch centre on most HE switches.
- **Hall sensor:** SOT-23 on the PCB top, centred under the magnet, 100 nF to ground right next to
  it. It sits under the switch's bottom housing, so keep it low (SOT-23 is about 1.1 mm).
- **Muxes:** 4 × CD74HC4067, best on the PCB underside. Keep the analog traces (sensor → mux →
  ADC) short and away from the LED data and LED power traces; use a ground pour.
- **LEDs:** one SK6812MINI-E per switch, reverse-mounted in a cutout under the switch's LED window
  (on MX-style switches, about 5 mm from the centre on one side: check the real switch),
  chained data-out → data-in, 100 nF next to each LED.
- Keep everything inside the outline: the plate screws and their bosses are outside it.

**Firmware:** the project firmware lives in [`../firmware`](../firmware/README.md) (C++, PlatformIO +
Arduino-Pico core + Adafruit TinyUSB). It reads every key through the muxes, calibrates each key
(rest and bottom-out readings), computes velocity and polyphonic aftertouch.

**Power:**
- **Sensors:** a linear Hall sensor draws about 6 mA (10 mA max), so 56 sensors need 0.35–0.56 A
  at 3.3 V. That is more than the Pico's own 3.3 V regulator should supply (about 300 mA), so
  each PCB has its own 3.3 V LDO (≥ 800 mA) fed from 5 V.
- **LEDs:** an SK6812 draws up to about 36 mA at full white, so 56 LEDs can draw about 2 A, and
  all 280 about 10 A. Cap brightness in firmware so each section stays under about 0.6 A
  (max brightness 25 %).
- **Total:** about 1.2 A per section at most, 6 A for the instrument: use a 5 V 6 A supply, or
  lower the LED cap. Feed each section's 5 V directly from the supply rail, not through the USB
  hub ports (hub ports are rated 0.5–0.9 A), and power the Pico through VSYS with a Schottky
  diode, as the Pico datasheet describes, so USB and the supply don't fight.

**Microtonal use:** send plain notes per channel and retune in software: Surge XT, Pianoteq, or any
synth supporting Scala (`.scl`/`.kbm`) or MTS-ESP.

---

## 6. Upgrades

- **Faster scanning:** read the ADC with DMA and its round-robin mode, so the CPU only does the
  velocity maths.
- **No hub:** one "master" Pico talks to the others over I2C or UART and is the only USB device.
- **5-pin/TRS MIDI out:** add a TRS jack and two resistors on a spare Pico UART pin.

---

## Sources

- [Lumatone help – How many keys does Lumatone actually have?](https://help.lumatone.io/en/what-is-lumatone/how-many-keys-does-lumatone-actually-have) (8 keys per vertical row)
- [MaxGaming – Moyu Studio Yurui HE Linear](https://us.maxgaming.com/us/switches/moyu-studio-yurui-he-linear-switch) (specs, magnet flux, $0.29)
- [keeb-finder – Moyu Studio Yurui HE Linear](https://keeb-finder.com/switches/moyu-studio-yurui-he-linear-switches) (5-pin, N-pole down, Divinikey $0.24)
- [splitkb.com – SK6812MINI-E RGB LEDs](https://splitkb.com/collections/keyboard-parts/products/sk6812mini-e-rgb-leds)
- [RS Components – Raspberry Pi Pico](https://fr.rs-online.com/web/p/raspberry-pi/2122162)
- [Lectronz – SK68xx mini-E (5 V / level-shifter note)](https://lectronz.com/products/rgb-led-sk68xx-mini-e)
- [JLCPCB – ordering](https://api.jlcpcb.com/help/article/how-do-i-place-an-order)
- [Raspberry Pi – HexBoard article](https://www.raspberrypi.com/news/hexboard-midi-controller-strikes-a-chord-with-rp2040/)
- [Adafruit – Hexpad guide](https://learn.adafruit.com/hexpad)
