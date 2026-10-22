// =====================================================================
//  DIY Hex MIDI Controller  -  "budget Lumatone", Lumatone dimensions
//
//  Lumatone specs used:
//    body 920 x 355 x 46 mm, 15 deg kickstand,
//    key width 21 mm (across flats), key diameter 24 mm (across corners),
//    key spacing 24.15 mm centre-to-centre, 280 keys
//    = 35 vertical columns of 8 keys = 5 sections of 7 columns.
//
//  Parts are built around:
//    - Moyu Studio Yurui HE linear switches (Hall-effect, MX-style,
//      standard profile, 3.6 mm travel), plate mounted. The switch only
//      carries a magnet: a linear Hall sensor on the PCB under each
//      switch measures the key travel (velocity + aftertouch)
//    - printed 21 mm flat-top hex keycaps with an MX cross socket
//    - one custom PCB + one Raspberry Pi Pico per 56-key section
//
//  The 920 x 355 mm body does not fit a 3D printer, so it is split into
//  10 open trays (5 across x 2 deep, each <= 250 x 210 mm) that bolt
//  together with M3 screws through internal ribs, plus 10 flat deck
//  pieces glued on top. Nothing needs support material.
//
//  Usage (OpenSCAD 2021+): Window > Customizer, pick a part, F6, export.
//    openscad -D 'part="case_tile"' -D tile_x=0 -D tile_y=1 -o tile_0_1.stl hex_midi_controller.scad
//  Values marked VERIFY: print a test piece first.
// =====================================================================

/* [Part to render] */
// assembly = preview only; the rest are printable / exportable
part = "assembly"; // [assembly, case_tile, deck_tile, plate, plate_dxf, pcb_outline, keycap, keycap_print_batch, kickstand_leg]
// Which tile for part = case_tile / deck_tile (0 = left ... 4 = right)
tile_x = 0; // [0:4]
// 0 = front half, 1 = back half
tile_y = 0; // [0:1]

/* [Body - Lumatone dimensions] */
body_w = 920;
body_d = 355;
body_h = 46;
corner_r = 15;
kickstand_angle = 15;

/* [Keys - Lumatone dimensions] */
// Key spacing, centre to centre
pitch = 24.15;
// Key width across flats (Lumatone: 21 mm)
key_flat = 21;
sections = 5;
cols_per_section = 7;
keys_per_col = 8;
// Extra drop of each section, in half key spacings (Lumatone-like slope). Must keep the lattice: odd when cols_per_section is odd
section_drop = 1;

/* [Switch plate] */
plate_t = 1.5;            // MX-style clips are made for 1.5 mm
switch_cut = 14.0;        // MX standard cutout, VERIFY with the prototype's plate_test
plate_margin_y = 20;      // plate beyond the first/last key centre (y)

/* [Enclosure] */
wall_t = 3;
floor_t = 3;
deck_t = 3;               // top surface around the plates
pcb_t = 1.6;
pcb_to_plate = 3.5;       // PCB top to plate underside: MX standard is 5.0 mm PCB top to plate top. VERIFY on the real switch
pcb_margin = 8.5;         // PCB edge beyond key centres
rib_t = 4;                // joining rib thickness (per tile)
rib_h = 20;               // rib height under the PCBs
plate_boss_d = 6;
screw_pilot_d = 1.8;      // M2 self-tapping into bosses
screw_clear_d = 2.4;
bolt_d = 3.4;             // M3 bolts joining tiles
bolt_spacing = 50;
window_gap = 0.4;

/* [Kickstand] */
leg_w = 20;
leg_thick = 10;
hinge_from_back = 45;
front_foot_y = 20;
lug_t = 6;

/* [Switch - Moyu Yurui HE, MX-style] */
// Top housing height above the plate top. VERIFY on the real switch
housing_h = 6.6;
// Stem slider top above the housing top, key at rest (~ travel). VERIFY
stem_rise = 3.6;
travel = 3.6;

/* [Keycap (printed, MX cross socket)] */
cap_h = 2.6;
cap_chamfer = 0.6;
boss_d = 5.6;             // socket tube under the cap
boss_h = 2.0;             // tube length below the cap underside
cross_l = 4.0;            // MX stem cross arm length. VERIFY on the real switch
cross_w = 1.2;            // MX stem cross arm width. VERIFY
cross_depth = 3.8;        // socket depth from the bottom of the tube
stem_tol = 0.15;          // added to the cross arms, VERIFY with the prototype's stem_test
batch_x = 4;
batch_y = 4;

/* [Hidden] */
$fn = 32;
eps = 0.01;
colsp = pitch * sqrt(3) / 2;              // horizontal column spacing (20.91)
ncols = sections * cols_per_section;      // 35
assert((section_drop - (cols_per_section - 1) % 2) % 2 == 1,
       "section_drop breaks the hex lattice: use an odd value when cols_per_section is odd, even when it is even");

// ---------------------------------------------------------------------
// Key lattice. Flat-top hexagons: keys in a column touch along flats.
// ---------------------------------------------------------------------
function col_y0(c) = let(s = floor(c / cols_per_section), j = c % cols_per_section)
    -((j % 2) * 0.5 + s * section_drop * 0.5) * pitch;

y0s = [for (c = [0 : ncols - 1]) col_y0(c)];
raw_ymin = min(y0s);
raw_ymax = max(y0s) + (keys_per_col - 1) * pitch;
key_off = [(body_w - (ncols - 1) * colsp) / 2,
           (body_d - (raw_ymax - raw_ymin)) / 2 - raw_ymin];

function key_xy(c, k) = [c * colsp, col_y0(c) + k * pitch] + key_off;

// section extents
function sec_x0(s) = key_xy(s * cols_per_section, 0)[0] - colsp / 2;
function sec_x1(s) = key_xy((s + 1) * cols_per_section - 1, 0)[0] + colsp / 2;
function sec_kymin(s) = min([for (c = [s * cols_per_section : (s + 1) * cols_per_section - 1]) key_xy(c, 0)[1]]);
function sec_kymax(s) = max([for (c = [s * cols_per_section : (s + 1) * cols_per_section - 1]) key_xy(c, keys_per_col - 1)[1]]);
function plate_y0(s) = sec_kymin(s) - plate_margin_y;
function plate_y1(s) = sec_kymax(s) + plate_margin_y;

// plate screws: 3 front, 3 back, outside every keycap
function plate_screws(s) = let(xa = sec_x0(s) + 15, xb = sec_x1(s) - 15, xm = (sec_x0(s) + sec_x1(s)) / 2)
    [for (x = [xa, xm, xb], y = [sec_kymin(s) - 16, sec_kymax(s) + 16]) [x, y]];

// tiles
x_cuts = concat([0], [for (s = [1 : sections - 1]) sec_x0(s)], [body_w]);
y_cuts = [0, body_d / 2, body_d];

// 12-TET colours for the preview (up = +2 semitones, up-right = -5)
function lattice_m(c, k) = round(col_y0(c) / pitch - c / 2) + k;
function pc(c, k) = ((2 * lattice_m(c, k) - 5 * c) % 12 + 12) % 12;
function key_colour(c, k) =
    pc(c, k) == 0 ? "tomato" :
    len(search(pc(c, k), [0, 2, 4, 5, 7, 9, 11])) > 0 ? "white" : [0.25, 0.25, 0.3];

// ---------------------------------------------------------------------
// 2D helpers
// ---------------------------------------------------------------------
module body_outline(inset = 0) {
    offset(r = corner_r - inset) offset(delta = -corner_r)
        square([body_w, body_d]);
}
module plate_rect(s) {
    translate([sec_x0(s), plate_y0(s)])
        square([sec_x1(s) - sec_x0(s), plate_y1(s) - plate_y0(s)]);
}
module windows() {
    for (s = [0 : sections - 1]) offset(delta = window_gap) plate_rect(s);
}
module tile_rect(tx, ty) {
    translate([x_cuts[tx], y_cuts[ty]])
        square([x_cuts[tx + 1] - x_cuts[tx], y_cuts[ty + 1] - y_cuts[ty]]);
}
// flat-top hexagon (flats on top and bottom)
module hex2d(flat) {
    circle(d = flat / cos(30), $fn = 6);
}

// ---------------------------------------------------------------------
// Switch plate (one per section, all five identical)
// ---------------------------------------------------------------------
module plate_2d(s) {
    difference() {
        plate_rect(s);
        for (c = [s * cols_per_section : (s + 1) * cols_per_section - 1], k = [0 : keys_per_col - 1])
            translate(key_xy(c, k)) square(switch_cut, center = true);
        for (p = plate_screws(s)) translate(p) circle(d = screw_clear_d);
    }
}
module plate_local_2d() {   // section 0 moved to the origin
    translate([-sec_x0(0), -plate_y0(0)]) plate_2d(0);
}

// PCB outline, same local origin as plate_local_2d (for KiCad Edge.Cuts)
module pcb_outline_local() {
    translate([-sec_x0(0), -plate_y0(0)])
        translate([key_xy(0, 0)[0] - pcb_margin, sec_kymin(0) - pcb_margin])
            square([(cols_per_section - 1) * colsp + 2 * pcb_margin,
                    sec_kymax(0) - sec_kymin(0) + 2 * pcb_margin]);
}

// ---------------------------------------------------------------------
// Enclosure tile
// ---------------------------------------------------------------------
function pico_pos(s) = [(sec_x0(s) + sec_x1(s)) / 2, body_d / 2 - 35];
pico_holes = [[-5.7, -23.5], [5.7, -23.5], [-5.7, 23.5], [5.7, 23.5]];
hinge_y = body_d - hinge_from_back;
hinge_x = [50, body_w - 50];
lug_h = leg_thick;
leg_len = (hinge_y - front_foot_y) * tan(kickstand_angle);   // hinge to foot centre

module interior_ribs(tx, ty) {
    // ribs along the cut edges, trimmed under the plates so PCBs fit
    difference() {
        intersection() {
            linear_extrude(body_h - deck_t) body_outline(wall_t - eps);
            union() {
                if (tx > 0)
                    translate([x_cuts[tx], y_cuts[ty], 0]) cube([rib_t, y_cuts[ty + 1] - y_cuts[ty], body_h]);
                if (tx < sections - 1)
                    translate([x_cuts[tx + 1] - rib_t, y_cuts[ty], 0]) cube([rib_t, y_cuts[ty + 1] - y_cuts[ty], body_h]);
                if (ty == 0)
                    translate([x_cuts[tx], y_cuts[1] - rib_t, 0]) cube([x_cuts[tx + 1] - x_cuts[tx], rib_t, body_h]);
                else
                    translate([x_cuts[tx], y_cuts[1], 0]) cube([x_cuts[tx + 1] - x_cuts[tx], rib_t, body_h]);
            }
        }
        translate([0, 0, floor_t + rib_h]) linear_extrude(body_h) offset(delta = 3) windows();
    }
}

module rib_bolt_holes(tx, ty) {
    z = floor_t + rib_h / 2;
    // holes through x-cut ribs (bolt axis along x)
    for (xc = [if (tx > 0) x_cuts[tx], if (tx < sections - 1) x_cuts[tx + 1]],
         y = [y_cuts[ty] + 25 : bolt_spacing : y_cuts[ty + 1] - 15])
        translate([xc, y, z]) rotate([0, 90, 0]) cylinder(d = bolt_d, h = 4 * rib_t, center = true);
    // holes through the y-cut rib (bolt axis along y)
    for (x = [x_cuts[tx] + 25 : bolt_spacing : x_cuts[tx + 1] - 15])
        translate([x, y_cuts[1], z]) rotate([90, 0, 0]) cylinder(d = bolt_d, h = 4 * rib_t, center = true);
}

module enclosure_body() {
    // floor
    linear_extrude(floor_t) body_outline();
    // outer wall (the deck sits on top of it)
    linear_extrude(body_h - deck_t) difference() { body_outline(); body_outline(wall_t); }
    // plate bosses
    for (s = [0 : sections - 1], p = plate_screws(s))
        translate(p) cylinder(d = plate_boss_d, h = body_h - plate_t);
    // Pico standoffs
    for (s = [0 : sections - 1], h = pico_holes)
        translate(pico_pos(s) + h) cylinder(d = 4.5, h = floor_t + 2.5);
    // front feet (same height as folded kickstand, so the body sits level)
    for (x = hinge_x) translate([x, front_foot_y, -lug_h]) cylinder(d = 16, h = lug_h + eps);
    // kickstand hinge lugs
    for (x = hinge_x, side = [-1, 1])
        translate([x + side * (leg_w / 2 + 0.5 + lug_t / 2) - lug_t / 2, hinge_y - 8, -lug_h])
            cube([lug_t, 16, lug_h + eps]);
}

module enclosure_holes() {
    // M2 pilot holes in bosses
    for (s = [0 : sections - 1], p = plate_screws(s))
        translate([p[0], p[1], body_h - plate_t - 10]) cylinder(d = screw_pilot_d, h = 11);
    for (s = [0 : sections - 1], h = pico_holes)
        translate([pico_pos(s)[0] + h[0], pico_pos(s)[1] + h[1], floor_t]) cylinder(d = 1.8, h = 10);
    // kickstand hinge axle (M4)
    for (x = hinge_x) translate([x, hinge_y, -lug_h / 2]) rotate([0, 90, 0]) cylinder(d = 4.3, h = leg_w + 4 * lug_t, center = true);
    // electronics bay (right end): USB-C out + DC jack in the back wall
    translate([body_w - 70, body_d - wall_t - 1, floor_t + 10]) cube([12, wall_t + 2, 7]);
    translate([body_w - 40, body_d + 1, floor_t + 13.5]) rotate([90, 0, 0]) cylinder(d = 8, h = wall_t + 2);
}

module case_tile(tx, ty) {
    difference() {
        intersection() {
            union() { enclosure_body(); interior_ribs(tx, ty); }
            translate([0, 0, -lug_h - 1]) linear_extrude(body_h + lug_h + 2) tile_rect(tx, ty);
        }
        enclosure_holes();
        rib_bolt_holes(tx, ty);
    }
}

// Top deck around the plates: separate flat part (prints without supports),
// glued onto the wall tops and ribs. Same cut lines as the trays.
module deck_tile_2d(tx, ty) {
    intersection() {
        tile_rect(tx, ty);
        difference() { body_outline(); windows(); }
    }
}

// ---------------------------------------------------------------------
// Keycap and kickstand leg
// ---------------------------------------------------------------------
module keycap() {
    hull() {
        linear_extrude(eps) hex2d(key_flat);
        translate([0, 0, cap_h - cap_chamfer]) linear_extrude(eps) hex2d(key_flat);
        translate([0, 0, cap_h - eps]) linear_extrude(eps) hex2d(key_flat - 2 * cap_chamfer);
    }
    // MX cross socket in a tube under the cap. The cross is symmetric, so
    // the hex stays flat-top whichever way the switch is turned
    difference() {
        translate([0, 0, -boss_h]) cylinder(d = boss_d, h = boss_h + eps);
        translate([0, 0, -boss_h - eps]) linear_extrude(cross_depth + eps) mx_cross_2d(stem_tol);
    }
}
module mx_cross_2d(tol) {
    square([cross_l + tol, cross_w + tol], center = true);
    square([cross_w + tol, cross_l + tol], center = true);
}
module keycap_print_batch() {
    for (i = [0 : batch_x - 1], j = [0 : batch_y - 1])
        translate([i * (key_flat / cos(30) + 3), j * (key_flat + 4), cap_h])
            rotate([180, 0, 0]) keycap();
}
module kickstand_leg() {
    difference() {
        linear_extrude(leg_w) hull() { circle(d = leg_thick); translate([leg_len, 0]) circle(d = leg_thick); }
        translate([0, 0, -1]) cylinder(d = 4.3, h = leg_w + 2);
    }
}

// ---------------------------------------------------------------------
// Preview
// ---------------------------------------------------------------------
module switch_ghost() {
    color([0.15, 0.15, 0.15]) {
        translate([0, 0, -pcb_to_plate]) linear_extrude(pcb_to_plate + plate_t) square(switch_cut - 0.2, center = true);
        translate([0, 0, plate_t]) linear_extrude(housing_h) square(15.6, center = true);
    }
    color("orangered") translate([0, 0, plate_t + housing_h]) linear_extrude(stem_rise) mx_cross_2d(0);
}
// keycap underside above the plate underside, key at rest
cap_z = plate_t + housing_h + stem_rise + boss_h;
module assembly() {
    for (tx = [0 : sections - 1], ty = [0 : 1]) {
        color("dimgray") case_tile(tx, ty);
        color([0.3, 0.3, 0.32]) translate([0, 0, body_h - deck_t]) linear_extrude(deck_t) deck_tile_2d(tx, ty);
    }
    plate_z = body_h - plate_t;
    for (s = [0 : sections - 1]) {
        color("silver") translate([0, 0, plate_z]) linear_extrude(plate_t) plate_2d(s);
        // every section is identical relative to its own plate corner
        color("darkgreen") translate([sec_x0(s), plate_y0(s), plate_z - pcb_to_plate - pcb_t])
            linear_extrude(pcb_t) pcb_outline_local();
    }
    for (c = [0 : ncols - 1], k = [0 : keys_per_col - 1])
        translate([key_xy(c, k)[0], key_xy(c, k)[1], plate_z]) {
            switch_ghost();
            color(key_colour(c, k)) translate([0, 0, cap_z]) keycap();
        }
    // kickstand legs, deployed
    color("dimgray") for (x = hinge_x)
        translate([x - leg_w / 2, hinge_y, -lug_h / 2]) rotate([0, 90, 0]) kickstand_leg();
}

// ---------------------------------------------------------------------
// Console info
// ---------------------------------------------------------------------
foot_drop = lug_h;
v_back = [body_d - front_foot_y, body_h + foot_drop];
max_height = v_back[0] * sin(kickstand_angle) + v_back[1] * cos(kickstand_angle);
echo(str("Keys: ", ncols * keys_per_col, " = ", ncols, " columns x ", keys_per_col));
echo(str("Key field: ", (ncols - 1) * colsp + key_flat / cos(30), " x ", raw_ymax - raw_ymin + key_flat, " mm"));
echo(str("Plate (per section): ", sec_x1(0) - sec_x0(0), " x ", plate_y1(0) - plate_y0(0), " mm"));
echo(str("PCB (per section): ", (cols_per_section - 1) * colsp + 2 * pcb_margin, " x ",
    sec_kymax(0) - sec_kymin(0) + 2 * pcb_margin, " mm"));
for (tx = [0 : sections - 1])
    echo(str("Tile column ", tx, ": ", x_cuts[tx + 1] - x_cuts[tx], " x ", body_d / 2, " mm (print 2: front + back)"));
echo(str("Switch stack: PCB bottom at ", body_h - plate_t - pcb_to_plate - pcb_t, " mm from the floor underside, rib tops at ",
    floor_t + rib_h, " mm; keycap top ", cap_z + cap_h - plate_t, " mm above the deck (", cap_z + cap_h - plate_t - travel, " mm pressed)"));
echo(str("Kickstand leg length (hinge to foot centre): ", leg_len, " mm"));
echo(str("Highest point on kickstand: ~", max_height, " mm (Lumatone: 125 mm)"));

// ---------------------------------------------------------------------
// Part selector
// ---------------------------------------------------------------------
if (part == "assembly") assembly();
else if (part == "case_tile") case_tile(tile_x, tile_y);
else if (part == "deck_tile") translate([-x_cuts[tile_x], -y_cuts[tile_y]]) linear_extrude(deck_t) deck_tile_2d(tile_x, tile_y);
else if (part == "plate") linear_extrude(plate_t) plate_local_2d();
else if (part == "plate_dxf") plate_local_2d();
else if (part == "pcb_outline") pcb_outline_local();
else if (part == "keycap") keycap();
else if (part == "keycap_print_batch") keycap_print_batch();
else if (part == "kickstand_leg") kickstand_leg();
