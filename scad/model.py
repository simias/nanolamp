# import openscad
from solid2 import cube, text, cylinder, set_global_fn

# set the number of faces for curved shapes
set_global_fn(100)

brick_c = 190
brick_thickness = 81
slot_depth = 10
lip_thickness = 10
base_w = 135
base_h = 55

base = cube(brick_thickness + 2 * lip_thickness, base_w, base_h)

# Slot for the glass brick
slot = cube(brick_thickness, base_w + 10, base_h)
slot = slot.translate(lip_thickness, -5, base_h - slot_depth)

# Brick glass support
brick_s = cube(brick_thickness - 2 * 5, base_w - 2 * 8, base_h)
brick_s = brick_s.translate(lip_thickness + 5, 8, base_h - slot_depth - 5)

diff_margin = 1
diff_thickness = 2

diff = cube(brick_thickness - 2 * 5 - diff_margin * 2, base_w - 2 * 8 -
            diff_margin * 2,
            diff_thickness)
diff = diff.translate(lip_thickness + 5 + diff_margin, 8 + diff_margin, base_h -
                      slot_depth - 5 + 0.01)

# Diffuser support
diff_s = cube(brick_thickness - 4 * 5, base_w - 2 * 8 - 2*5, base_h)
diff_s = diff_s.translate(lip_thickness + 5 + 5, 8 + 5, -1)

base = base - slot - brick_s - diff_s

# Circular pot/switch
pot_rad = 24 / 2
pot_depth = 20
pot_hole_rad = 9 / 2
pot_hole_depth = 20
pot_cap_rad = 14 / 2
pot_cap_len = 16

pot = cylinder(pot_depth, pot_rad, pot_rad);
pot += cylinder(pot_hole_depth + 1,
                pot_hole_rad,
                pot_hole_rad).translate(0,
                                        0,
                                        pot_hole_depth - 1);
cap = cylinder(pot_cap_len, pot_cap_rad, pot_cap_rad)
cap = cap.translate(0, 0, 30);

pot = pot + cap.debug()

pot = pot.rotate(0, 90, 180)
pot = pot.translate(pot_depth + 9, 105, pot_rad + 5 );

# Slide pot
slide_w = 60
slide_h = 10
slide_d = 7
slide_m = 0.5

slide = cube(slide_w + slide_m * 2, slide_h + slide_m * 2, slide_d + 1)
slide += cube(slide_h + slide_m * 2, slide_h + slide_m * 2, 30).translate(0, 0, -25)
slide += cube(slide_h + slide_m * 2, slide_h + slide_m * 2, 30).translate(slide_w - slide_h + slide_m *2, 0, -25)
slide += cube(slide_w, slide_h, slide_d + 1+ slide_m).translate(slide_m, slide_m,
                                                   slide_m).debug()
slide += cube(5, 1, 15).translate(20, slide_m + slide_h / 2 - 0.5, slide_d).debug()
slide = slide.rotate(-90, 180, 90)
slide = slide.translate(slide_d, 80, 11.5)

brick = cube(brick_thickness - 1, brick_c, brick_c)
brick = brick.translate(lip_thickness + 0.5, - (brick_c - base_w) / 2, base_h -
                        slot_depth + 0.01)

model = base + diff.background() - pot - slide + brick.background()

# save your model for use in OpenSCAD
model.save_as_scad("model.scad")
