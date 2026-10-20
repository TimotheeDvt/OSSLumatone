// =====================================================================
//  DIY Hex MIDI Controller - 6-KEY PROTOTYPE
//
//  Same key geometry as the full Lumatone-size build
//  (hex_midi_controller.scad): 24.15 mm key spacing, 21 mm flat-top
//  hex keys, Kailh Choc v1 switches on a 1.2 mm plate.
//
//  6 keys = 3 columns x 2 keys, so you can try the vertical neighbour
//  and both diagonal neighbours of a key.
//
//  No custom PCB needed: hand-wire each switch to its own Pico pin.
//    Switch 0..5  ->  GP2..GP7   (other switch pin -> GND,
//                                 use the Pico's internal pull-ups)
//    Key order: column 0 bottom, column 0 top, column 1 bottom, ...
//
//  Also includes two small test prints for the values marked VERIFY
//  in the full model: plate cutout size and keycap stem fit.
//
//  Usage: Window > Customizer, pick a part, F6, File > Export > STL.
//    openscad -D 'part="case"' -o proto_case.stl hex_prototype_6key.scad
// =====================================================================

/* [Part to render] */
part = "assembly"; // [assembly, case, plate, keycap, keycap_set, plate_test, stem_test]

/* [Keys - same as the full build] */
pitch = 24.15;      // Lumatone key spacing
key_flat = 21;      // Lumatone key width across flats
cols = 3;
keys_per_col = 2;

/* [Switch plate] */
plate_t = 1.2;
choc_cut = 13.8;    // VERIFY with plate_test
plate_margin = 10;  // plate beyond the keycap edges

/* [Case] */
wall_t = 2.5;
floor_t = 2;
case_h = 18;        // top of the walls = underside of the plate
corner_r = 4;
boss_d = 6;
boss_inset = 5;     // screw centre from the outer corner
screw_pilot_d = 1.8;   // M2 self-tapping
screw_clear_d = 2.4;
usb_w = 12;
usb_h = 8;
pico_standoff_h = 2.5;
foot_d = 8;         // recesses for stick-on rubber feet (8 mm)
foot_depth = 1;
foot_inset = 9;

/* [Keycap] */
cap_h = 2.6;
cap_chamfer = 0.6;
stem_h = 3.0;
stem_w = 1.2;
stem_l = 3.0;
stem_gap = 5.7;
stem_tol = 0.10;    // VERIFY with stem_test

/* [Test prints] */
cut_sizes = [13.6, 13.7, 13.8, 13.9, 14.0];
stem_tols = [0.0, 0.1, 0.2, 0.3];

/* [Hidden] */
$fn = 40;
eps = 0.01;
colsp = pitch * sqrt(3) / 2;            // 20.91 mm between columns
key_corner_r = key_flat / sqrt(3);       // 12.12 mm (24.2 mm across corners)

// ---------------------------------------------------------------------
// Key positions (flat-top hex lattice, odd columns half a key lower)
// ---------------------------------------------------------------------
function raw_xy(c, k) = [c * colsp, k * pitch - (c % 2) * pitch / 2];
raw = [for (c = [0 : cols - 1], k = [0 : keys_per_col - 1]) raw_xy(c, k)];
rxs = [for (p = raw) p[0]];
rys = [for (p = raw) p[1]];

// outer size: key centres + keycap half size + margin
size_x = max(rxs) - min(rxs) + 2 * (key_corner_r + plate_margin);
size_y = max(rys) - min(rys) + 2 * (key_flat / 2 + plate_margin);
key_off = [key_corner_r + plate_margin - min(rxs), key_flat / 2 + plate_margin - min(rys)];

function key_xy(c, k) = raw_xy(c, k) + key_off;
keys = [for (c = [0 : cols - 1], k = [0 : keys_per_col - 1]) [c, k]];

bosses = [for (x = [boss_inset, size_x - boss_inset], y = [boss_inset, size_y - boss_inset]) [x, y]];

// Pico (21 x 51 mm) along y, USB end at the back wall
pico_c = [size_x / 2, size_y - wall_t - 1 - 25.5];
pico_holes = [[-5.7, -23.5], [5.7, -23.5], [-5.7, 23.5], [5.7, 23.5]];

// 12-TET colours for the preview (up = +2 semitones, up-right = -5),
// same mapping as the full model
function lattice_m(c, k) = k - (c % 2) * 0.5 - c / 2;
function pc(c, k) = ((2 * round(lattice_m(c, k)) - 5 * c) % 12 + 12) % 12;
function key_colour(c, k) =
    pc(c, k) == 0 ? "tomato" :
    len(search(pc(c, k), [0, 2, 4, 5, 7, 9, 11])) > 0 ? "white" : [0.25, 0.25, 0.3];

// ---------------------------------------------------------------------
// 2D helpers
// ---------------------------------------------------------------------
module outline(inset = 0) {
    translate([inset, inset])
        offset(r = max(corner_r - inset, 0.5)) offset(delta = -max(corner_r - inset, 0.5))
            square([size_x - 2 * inset, size_y - 2 * inset]);
}
module hex2d(flat) {             // flat-top hexagon
    circle(d = flat / cos(30), $fn = 6);
}

// ---------------------------------------------------------------------
// Parts
// ---------------------------------------------------------------------
module plate() {
    linear_extrude(plate_t) difference() {
        outline();
        for (kk = keys) translate(key_xy(kk[0], kk[1])) square(choc_cut, center = true);
        for (b = bosses) translate(b) circle(d = screw_clear_d);
    }
}

module case_shell() {
    difference() {
        union() {
            difference() {
                linear_extrude(case_h) outline();
                translate([0, 0, floor_t]) linear_extrude(case_h) outline(wall_t);
            }
            // corner bosses, merged into the walls
            intersection() {
                linear_extrude(case_h) outline();
                for (b = bosses) translate(b) cylinder(d = boss_d, h = case_h);
            }
            for (h = pico_holes)
                translate(pico_c + h) cylinder(d = 4.5, h = floor_t + pico_standoff_h);
        }
        for (b = bosses) translate([b[0], b[1], case_h - 8]) cylinder(d = screw_pilot_d, h = 9);
        for (h = pico_holes) translate([pico_c[0] + h[0], pico_c[1] + h[1], floor_t]) cylinder(d = 1.8, h = 10);
        // USB opening for the Pico, back wall
        translate([pico_c[0] - usb_w / 2, size_y - wall_t - 1, floor_t + pico_standoff_h - 1.5])
            cube([usb_w, wall_t + 2, usb_h]);
        // rubber feet recesses
        for (x = [foot_inset, size_x - foot_inset], y = [foot_inset, size_y - foot_inset])
            translate([x, y, -1]) cylinder(d = foot_d, h = foot_depth + 1);
    }
}

module keycap(tol = stem_tol) {
    hull() {
        linear_extrude(eps) hex2d(key_flat);
        translate([0, 0, cap_h - cap_chamfer]) linear_extrude(eps) hex2d(key_flat);
        translate([0, 0, cap_h - eps]) linear_extrude(eps) hex2d(key_flat - 2 * cap_chamfer);
    }
    for (s = [-1, 1])
        translate([s * stem_gap / 2, 0, -stem_h / 2 + eps])
            cube([stem_w - tol, stem_l - tol, stem_h], center = true);
}

// 6 caps, upside down (top face on the bed)
module keycap_set() {
    for (i = [0 : 2], j = [0 : 1])
        translate([i * (key_flat / cos(30) + 3), j * (key_flat + 4), cap_h])
            rotate([180, 0, 0]) keycap();
}

// Strip with 5 cutout sizes. Notches on the front edge = position 1..5
// (13.6, 13.7, 13.8, 13.9, 14.0). Use the size where a switch clips in firmly.
module plate_test() {
    n = len(cut_sizes);
    w = 20;
    linear_extrude(plate_t) difference() {
        square([n * w, 24]);
        for (i = [0 : n - 1]) {
            translate([i * w + w / 2, 13]) square(cut_sizes[i], center = true);
            for (d = [0 : i]) translate([i * w + 3 + d * 2.5, 0]) circle(d = 1.4);
        }
    }
}

// 4 caps with stem_tol 0.0 / 0.1 / 0.2 / 0.3. Number of side notches = 1..4.
// Use the one that holds firmly without cracking.
module stem_test() {
    for (i = [0 : len(stem_tols) - 1])
        translate([i * (key_flat / cos(30) + 3), 0, cap_h])
            rotate([180, 0, 0]) difference() {
                keycap(stem_tols[i]);
                for (d = [0 : i])
                    translate([-key_flat / cos(30) / 2 + 3.5 + d * 2, -key_flat / 2, cap_h / 2]) cube([1, 1.2, cap_h + 1], center = true);
            }
}

// ---------------------------------------------------------------------
// Preview
// ---------------------------------------------------------------------
module assembly() {
    color("lightsteelblue") case_shell();
    color("green") translate([pico_c[0] - 10.5, pico_c[1] - 25.5, floor_t + pico_standoff_h]) cube([21, 51, 1]);
    color("silver") translate([0, 0, case_h]) plate();
    for (kk = keys) translate([key_xy(kk[0], kk[1])[0], key_xy(kk[0], kk[1])[1], case_h]) {
        color([0.15, 0.15, 0.15]) {
            translate([0, 0, -2.2]) linear_extrude(2.2) square(choc_cut - 0.2, center = true);
            translate([0, 0, plate_t]) linear_extrude(2.2) square(15, center = true);
        }
        color(key_colour(kk[0], kk[1])) translate([0, 0, plate_t + 2.2 + 1.0]) keycap();
    }
}

// ---------------------------------------------------------------------
echo(str("Prototype: ", len(keys), " keys, case ", size_x, " x ", size_y, " x ", case_h + plate_t, " mm"));
for (i = [0 : len(keys) - 1])
    echo(str("Key ", i, " (column ", keys[i][0], ", key ", keys[i][1], ") -> GP", i + 2,
             "  centre ", key_xy(keys[i][0], keys[i][1])));

if (part == "assembly") assembly();
else if (part == "case") case_shell();
else if (part == "plate") plate();
else if (part == "keycap") keycap();
else if (part == "keycap_set") keycap_set();
else if (part == "plate_test") plate_test();
else if (part == "stem_test") stem_test();
