# DIY Hex MIDI Controller — a budget Lumatone

A 280-key hexagonal (isomorphic) USB-MIDI controller with the **Lumatone's dimensions**, built
from keyboard-hobby parts: Kailh Choc switches, hex keycaps, per-key RGB LEDs and Raspberry Pi
Pico microcontrollers. Like the Lumatone, the keys are grouped in **5 sections of 56 keys**,
so you can build and test one section first.

> **Main difference from a real Lumatone:** mechanical switches are on/off, so this build has
> **no velocity sensitivity** (every note plays at the same velocity). See "Upgrades" at the end.

| File | What it is |
|---|---|
| `README.md` | This document: specification, parts, prices, build notes |
| `hex_midi_controller.scad` | Parametric OpenSCAD model: enclosure tiles, deck, switch plates, keycaps, kickstand, PCB outline |

---

## 1. Hardware specification

### Taken from the Lumatone

| Item | Lumatone spec | This build |
|---|---|---|
| Dimensions | 92 × 35.5 × 4.6 cm | 920 × 355 × 46 mm (same) |
| Kickstand angle | 15° | 15° (two fold-out legs) |
| Height with kickstand | 12.5 cm at the highest point | ≈ 14.1 cm (see note) |
| Key width (across flats) | 21 mm | 21 mm printed caps, or 20.96 mm s-ol/FKcaps HEX caps |
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
| Switches | Kailh Choc v1 low-profile (PG1350), plate-mounted, soldered to PCB |
| Switch plate | 5 identical plates, 146.4 × 221.1 × 1.2 mm, 56 cutouts each. Print them, or order them from JLCPCB in FR4 using the exported DXF |
| Lighting | 1 × SK6812MINI-E addressable RGB LED per key, reverse-mounted under the switch |
| Key matrix | 8 rows × 7 columns per section, one 1N4148 diode per key (no ghosting, full polyphony) |
| PCB | 5 identical boards, 142.5 × 198.1 mm, 2 layers |
| Microcontroller | 1 × Raspberry Pi Pico (RP2040) per section, screwed to standoffs under its PCB |
| Interface | USB-MIDI class compliant (no drivers on Windows / macOS / Linux / iPad) |
| MIDI mapping | Each section is its own USB-MIDI device on its own channel (1–5), like the Lumatone's per-section channels, so 280 keys can exceed the 128-note MIDI limit for microtonal tunings |
| Connection | The 5 Picos go to a USB hub inside the right end of the body; one USB-C cable goes out the back, next to a DC jack for the hub's power supply |
| Power | 5 V. LED brightness must be capped in firmware (see §5) |
| Enclosure | 10 bolt-together trays + 10 flat deck pieces, all ≤ 240.4 × 177.5 mm (fits a 250 × 210 mm bed), no support material |

### Pico pin plan (per section)

| Pico pins | Use |
|---|---|
| GP0 – GP7 | Matrix rows 0–7 (key position in the column, outputs) |
| GP8 – GP14 | Matrix columns 0–6 (inputs with pull-down) |
| GP15 | LED data → 74AHCT125 level shifter (3.3 V → 5 V) → first LED |
| VBUS (5 V) | LED power |
| GND | Ground |

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
| 1 | HEX keycaps (20.96 mm ≈ Lumatone's 21 mm), bundle of 10 | 28 bundles | €5.49 | €153.72 | [splitkb.com](https://splitkb.com/products/hex-keycap) (NL) — **sold out on 20/10/2026**; other shops: [FKcaps vendor list](https://fkcaps.com/products/hex-keycaps), or print them (§4) |
| 2 | Kailh Choc v1 switches (e.g. Red, linear), pack of 10 | 29 packs | ≈ €6.28 (£5.46) | ≈ €182.12 | [Mechboards](https://kbd.news/switch/Kailh-Choc-V1-Red-69.html) (UK) — cheaper at Chosfox (CN, $4.28/10), compare on [kbd.news](https://kbd.news/switch/Kailh-Choc-V1-Red-69.html). **Must be v1**, not v2 |
| 3 | SK6812MINI-E RGB LEDs, bundle of 66 | 5 bundles | €11.95 | €59.75 | [splitkb.com](https://splitkb.com/collections/keyboard-parts/products/sk6812mini-e-rgb-leds) (NL) |
| 4 | 1N4148 diodes, SOD-123, pack of 100 | 3 packs | €9.50 | €28.50 | [Opencircuit](https://opencircuit.shop/product/1n4148-smt-sod-123-diodes-100-pack) (NL) |
| 5 | Raspberry Pi Pico (RP2040) | 5 | €4.33 | €21.65 | [RS Components France](https://fr.rs-online.com/web/p/raspberry-pi/2122162) — or [LDLC](https://www.ldlc.com/fr-lu/fiche/PB00627355.html) at €6.90 |
| 6 | 74AHCT125 level shifter (SOIC-14) | 5 | ≈ €1.00 (est.) | €5.00 | [RS Components France](https://fr.rs-online.com/) — search "74AHCT125" |
| 7 | Custom PCB, 2-layer, 142.5 × 198.1 mm, 5 pcs | 1 order | ≈ €80 (est.) | €80.00 | [JLCPCB](https://cart.jlcpcb.com/quote) — upload your Gerbers for an exact quote |
| 8 | PLA or PETG filament, ~3 kg (trays, decks, plates, legs) | 3 spools | ≈ €20 (est.) | €60.00 | Any shop; or a printing service |
| 9 | USB hub, 7 ports, small enough for the end bay | 1 | ≈ €25 (est.) | €25.00 | Any shop |
| 10 | 5 V 4 A power supply with barrel plug (8 mm panel jack) | 1 | ≈ €12 (est.) | €12.00 | Any shop |
| 11 | USB-C panel-mount extension cable | 1 | ≈ €8 (est.) | €8.00 | Any shop |
| 12 | M2 self-tapping screws, 6 mm (≈ 50: plates + Picos) | 1 set | ≈ €8 (est.) | €8.00 | Any shop |
| 13 | M3 × 12 bolts + nuts (≈ 45, to join the trays) | 1 set | ≈ €8 (est.) | €8.00 | Any shop |
| 14 | M4 × 30 bolts + nylon lock nuts (kickstand hinges) | 2 | ≈ €1.50 (est.) | €3.00 | Any shop |
| 15 | Glue for the deck pieces (CA or epoxy) | 1 | ≈ €5 (est.) | €5.00 | Any shop |
| 16 | Micro-USB cables, short (Pico → hub) | 5 | ≈ €2 (est.) | €10.00 | Any shop |
| 17 | Hook-up wire (Pico → PCB) | 1 | ≈ €5 (est.) | €5.00 | Any shop |

| | |
|---|---|
| Checked prices (rows 1–5) | **€445.74** |
| Estimated items (rows 6–17) | **€229.00** |
| **Total, full 280-key build** | **≈ €675** + shipping |

### First prototype (one 56-key section, no enclosure)

6 keycap bundles (€32.94) + 6 switch packs (€37.68) + 1 LED bundle (€11.95) + 1 diode pack (€9.50)
+ 1 Pico (€4.33) + level shifter (€1) + PCB order (€80, you get 5 boards anyway, enough for the full build)
+ 1 filament spool for the plate (€20) + screws, cable, wire (€15) = **≈ €212** + shipping.

Test the section as plate + PCB on the table before printing the 20 enclosure pieces.

### Ways to make it cheaper

- **Have JLCPCB solder the diodes and LEDs** (PCB assembly service): costs more per board but saves
  560 tiny SMD solder joints per section. Pick parts from their parts library when designing.
- **Print the keycaps** instead of buying them (saves ~€150; resin printing works much better than FDM).
- **Skip the LEDs and level shifter**: saves ~€65 and a lot of soldering.
- Buy switches from China (Chosfox, AliExpress): cheapest, but slower delivery.

### Tools (not included in the total)

Soldering iron with a fine tip, solder, flux, tweezers (for SMD parts), multimeter,
a 3D printer with a bed of at least 250 × 210 mm (or a printing service), small screwdrivers.

---

## 3. Ready-made alternatives (compare before you build)

| Project | Notes |
|---|---|
| [HexBoard](https://shapingthesilence.com/tech/hexboard-midi-controller/) by Jared DeCook | RP2040, RGB keys, USB-C + TRS MIDI, open firmware. Sold as a kit, good reference design |
| [0x33.board](https://lectronz.com/products/beta-0x33-board-midi-controller) by s-ol | 48 keys, open-source hardware, uses the same HEX keycaps |
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
| `plate` | Switch plate, 1.2 mm, 56 Choc cutouts → **STL** | 5 (all identical) |
| `plate_dxf` | Same plate as 2D → **DXF**: order it from JLCPCB in FR4 for a stiffer plate | — |
| `pcb_outline` | 2D board outline → **DXF**, import into KiCad on Edge.Cuts. Same origin as the plate | — |
| `keycap` | One 21 mm hex keycap with Choc v1 stem → **STL** | test piece |
| `keycap_print_batch` | 16 keycaps, upside down, ready to print → **STL** | 18 batches for 280 caps |
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
Keycaps: clear resin gives the best result (LEDs shine through); for FDM use 0.1 mm layers.

**Assembly order:**
1. Bolt the 10 trays together with M3 bolts through the ribs (5 across, front + back).
2. Install the USB hub in the right end bay, mount the USB-C panel cable and the DC jack.
3. Per section: clip 56 switches into the plate, put the plate on the PCB and solder the switches
   (the PCB hangs under the plate). Screw the Pico onto its standoffs and wire it to the PCB.
4. Drop each plate/PCB into its window and screw it down (6 × M2 into the bosses).
5. Glue the 10 deck pieces on top of the walls and ribs.
6. Fit the kickstand legs on M4 bolts with lock nuts (tighten until the legs hold by friction).
7. Keycaps on.

**Values to check with a test print** (marked VERIFY in the file): `choc_cut` (13.8 mm plate
cutout), `pcb_to_plate` (2.2 mm), and the keycap stem (`stem_tol`). For flat-top keys, check how
your keycaps sit on the switch: if they come out pointy-top, rotate the switch footprints 90° on
the PCB. The plate cutouts are square, so the plate stays the same.

---

## 5. PCB and firmware notes

**PCB (KiCad, free):**
- Import `pcb_outline.dxf` as the board edge and `plate_dxf` on a user layer to align footprints.
- s-ol's KiCad library has Choc footprints and hex-grid tips:
  [git.s-ol.nu/kb/kicad-lib](https://git.s-ol.nu/kb/kicad-lib/-/). Note that this build uses the
  Lumatone's 24.15 mm spacing, not s-ol's 21.5 mm, so place footprints with the formula in §1.
- One diode per switch (matrix), one SK6812MINI-E per switch with a cutout under the switch window,
  LEDs chained data-out → data-in, a 100 nF capacitor next to each LED is recommended.
- Keep everything inside the outline: the plate screws and their bosses are outside it.

**Firmware options:**
- **Arduino IDE** with the *Arduino-Pico* core + *Adafruit TinyUSB* (USB-MIDI) + *Adafruit NeoPixel*: fast matrix scanning, easy.
- **CircuitPython**, like the Adafruit Hexpad guide: easiest to modify, slower.
- Fork the open **HexBoard firmware**: it already has hex layouts, microtonal support and LED colouring.

**LED power:** an SK6812 draws up to about 36 mA at full white, so 56 LEDs can draw about 2 A,
and all 280 about 10 A. Cap brightness in firmware so each section stays under about 0.6 A
(e.g. max brightness 25 %), which keeps the whole instrument within a 5 V 4 A supply.

**Microtonal use:** send plain notes per channel and retune in software: Surge XT, Pianoteq, or any
synth supporting Scala (`.scl`/`.kbm`) or MTS-ESP.

---

## 6. Upgrades

- **Velocity:** replace the Choc switches with Hall-effect (magnetic) switches read by analog
  multiplexers. It gives velocity and aftertouch, but needs a new PCB and more expensive switches.
- **No hub:** one "master" Pico talks to the others over I2C or UART and is the only USB device.
- **5-pin/TRS MIDI out:** add a TRS jack and two resistors on a spare Pico UART pin.

---

## Sources

- [Lumatone help – How many keys does Lumatone actually have?](https://help.lumatone.io/en/what-is-lumatone/how-many-keys-does-lumatone-actually-have) (8 keys per vertical row)
- [splitkb.com – Hex Keycaps](https://splitkb.com/products/hex-keycap) (price, dimensions, Choc v1 compatibility)
- [splitkb.com – SK6812MINI-E RGB LEDs](https://splitkb.com/collections/keyboard-parts/products/sk6812mini-e-rgb-leds)
- [kbd.news – Kailh Choc (V1) Red prices](https://kbd.news/switch/Kailh-Choc-V1-Red-69.html)
- [Opencircuit – 1N4148 SMT SOD-123 Diodes](https://opencircuit.shop/product/1n4148-smt-sod-123-diodes-100-pack)
- [RS Components – Raspberry Pi Pico](https://fr.rs-online.com/web/p/raspberry-pi/2122162)
- [s-ol – HEX keycaps specs](https://hw.s-ol.nu/HEX-keycaps/)
- [Lectronz – SK68xx mini-E (5 V / level-shifter note)](https://lectronz.com/products/rgb-led-sk68xx-mini-e)
- [JLCPCB – ordering](https://api.jlcpcb.com/help/article/how-do-i-place-an-order)
- [Raspberry Pi – HexBoard article](https://www.raspberrypi.com/news/hexboard-midi-controller-strikes-a-chord-with-rp2040/)
- [Adafruit – Hexpad guide](https://learn.adafruit.com/hexpad)
