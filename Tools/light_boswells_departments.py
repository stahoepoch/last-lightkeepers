"""Give the sales floor an ambient floor, so the departments read as a room.

**The measurement this answers.** `Tools/audit_boswells_ambient.py`, 2026-11-22:
the opening map has **no sky light, no directional, no fog and no sky
atmosphere**. The only light in it is 102 emergency spots and 24 ceiling fills,
total authored intensity 1,990, and a post-process volume with exposure locked
at 0.5 so nothing adapts. The first rendered screenshots anybody has taken of
this project show the result: a lit racetrack aisle with a legible escalator,
ceiling soffits and department signs, and **pure black either side of it** -
where all 1,638 fixtures stand.

Chris, 2026-11-22: *"the department store doesn't look like a department
store. It's empty."* It is not empty. It is unlit, which looks identical.

`ROADMAP.md` slice 2 requires that *"the chapter remains navigable with the
lantern off"*. It currently is not, anywhere off the racetrack.

**Why fill lights rather than a sky light.** An `ASkyLight` in an enclosed
interior captures the interior, which is black, and contributes nothing. A
directional would need an aperture to come through. This store does have an
honest ambient source - the mall entrance glass at the south end, which the
screenshots show as the brightest thing in the building - so the fill is laid
as a **gradient that fades with distance from that glass**. Deeper into the
store is darker, which is both what the architecture implies and what the
chapter wants: the lantern matters more the further in you go.

Fills are deliberately dim. The brightest is a fraction of one emergency spot.
The intent is to lift the departments off absolute black so a player can see
that there are racks in them, not to light the store.

Idempotent: every actor it creates is labelled `BSW_DEPT_FILL_*` and any
existing ones are removed first, so running it twice does not double the light.
"""

import math

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_DEPT_FILL_"

# The sales floor, from the census: centre (0,0), extent 4350 x 2700 cm.
FLOOR_HALF_X = 4350.0
FLOOR_HALF_Y = 2700.0

# The racetrack runs up the middle on X; the departments are outboard of it.
# Fills go over the departments, not over the aisle that is already lit.
AISLE_HALF_X = 0.0

# Ceiling underside is about 1487 cm (ANCHOR_FULL_ROOF); hang fills below it.
FILL_Z = 1150.0

# The mall entrance glass is at the south end, most negative Y. Fill brightness
# falls off with distance from it.
GLASS_Y = -2700.0

NEAR_INTENSITY = 14.0
FAR_INTENSITY = 4.5
RADIUS = 4200.0

# A cold daylight leak, not the warm emergency amber, so the two sources stay
# tellable apart by eye - which is also how a player learns which light is
# the building's own and which is a way out.
COLOUR = unreal.Color(158, 184, 235, 255)

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("DEPTFILL map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()

removed = 0
for actor in actors:
    if actor.get_actor_label().startswith(PREFIX):
        subsystem.destroy_actor(actor)
        removed += 1
print("DEPTFILL removed_existing=%d" % removed)

placed = 0
x = -FLOOR_HALF_X + 750.0
while x <= FLOOR_HALF_X - 750.0:
    if abs(x) < AISLE_HALF_X:
        x += 1500.0
        continue
    y = -FLOOR_HALF_Y + 650.0
    while y <= FLOOR_HALF_Y - 650.0:
        # Distance from the entrance glass, normalised over the floor's depth.
        reach = min(1.0, abs(y - GLASS_Y) / (FLOOR_HALF_Y * 2.0))
        intensity = NEAR_INTENSITY + (FAR_INTENSITY - NEAR_INTENSITY) * reach

        light = subsystem.spawn_actor_from_class(
            unreal.PointLight, unreal.Vector(x, y, FILL_Z))
        light.set_actor_label("%s%d_%d" % (PREFIX, int(x), int(y)))
        comp = light.point_light_component
        comp.set_editor_property("intensity", intensity)
        comp.set_editor_property("attenuation_radius", RADIUS)
        comp.set_editor_property("light_color", COLOUR)
        comp.set_editor_property("source_radius", 600.0)
        comp.set_editor_property("cast_shadows", False)
        comp.set_editor_property("affects_world", True)
        # Movable: these are tuning lights and must not need a bake to
        # change. Set on the component - AActor has no set_mobility in the
        # Python API, only USceneComponent does.
        comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        placed += 1
        y += 1300.0
    x += 1500.0

print("DEPTFILL placed=%d intensity=%.2f..%.2f radius=%.0f z=%.0f" % (
    placed, FAR_INTENSITY, NEAR_INTENSITY, RADIUS, FILL_Z))

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("DEPTFILL saved=True")
