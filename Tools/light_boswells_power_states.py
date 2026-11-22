"""Two lighting states for Boswell's: emergency, and the mains coming back on.

Chris, 2026-11-22, after seeing the store lit for the first time:

    "What would it look like with only emergency lights on. That's the vibe
    we want. Only later can we turn the power on to see everything. So we
    need both... I envision being able to turn on the power at some point
    that we will need to."

That turns the lighting from a setting into a mechanic, and it is
`ROADMAP.md` chapter goal 5 - *"find a story clue, change part of the
building"* - in its most literal form. It also replaces
`light_boswells_departments.py`, whose single fill layer was tuned to make
the store readable and therefore looked like a shop with the power on.

## The two layers

**`BSW_EMERGENCY_FILL_*`, tagged `LLK_Emergency`, on by default.** Very dim
and cold, 1.0 to 3.0 candela against the existing emergency spots' ~16. Its
whole job is to lift the departments off *absolute* black so a player can see
that there is racking there and move between it - `ROADMAP.md` slice 2 wants
the chapter navigable with the lantern off - without showing them anything.
Shapes, not contents. You can tell something is hanging there; you cannot
tell what colour it is.

**`BSW_MAINS_*`, tagged `LLK_Mains`, off by default.** The ceiling troffers a
working department store actually runs: bright, neutral-white, even. When
these come on the building stops being a ruin for a moment, which is the
point - the payoff has to be worth crossing the store for, and the contrast
is the reward.

Both are unshadowed and movable. The emergency layer fades with distance from
the mall entrance glass, the only honest ambient source an enclosed anchor
store has; the mains do not, because mains lighting is designed to be even.

## Why tags rather than two sub-levels

A runtime switch has to find these without the level being authored in two
halves, and `AActor::Tags` survives a save, costs nothing and is readable from
C++ with one call. Sub-levels would mean a streaming decision and a second
place for the map to be wrong.

Idempotent: both prefixes are removed before placing, so a retune is one run.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"

EMERGENCY_PREFIX = "BSW_EMERGENCY_FILL_"
MAINS_PREFIX = "BSW_MAINS_"
EMERGENCY_TAG = "LLK_Emergency"
MAINS_TAG = "LLK_Mains"

# Sales floor, from the census: centre (0,0), extent 4350 x 2700 cm.
FLOOR_HALF_X = 4350.0
FLOOR_HALF_Y = 2700.0
GLASS_Y = -2700.0

# Emergency: just enough to read mass by.
EMERGENCY_NEAR = 3.0
EMERGENCY_FAR = 1.0
EMERGENCY_Z = 1150.0
EMERGENCY_RADIUS = 4200.0
EMERGENCY_COLOUR = unreal.Color(150, 178, 232, 255)

# Mains: a working shop. Neutral white, even, hung at the ceiling.
MAINS_INTENSITY = 26.0
MAINS_Z = 1380.0
MAINS_RADIUS = 2600.0
MAINS_COLOUR = unreal.Color(255, 248, 236, 255)

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("POWERSTATES map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

removed = 0
for actor in subsystem.get_all_level_actors():
    label = actor.get_actor_label()
    # The superseded single-layer fill goes too, or the store keeps its
    # power-on look while pretending to be on emergency.
    if (label.startswith(EMERGENCY_PREFIX) or label.startswith(MAINS_PREFIX)
            or label.startswith("BSW_DEPT_FILL_")):
        subsystem.destroy_actor(actor)
        removed += 1
print("POWERSTATES removed_existing=%d" % removed)


def place(prefix, tag, x, y, z, intensity, radius, colour, visible):
    light = subsystem.spawn_actor_from_class(
        unreal.PointLight, unreal.Vector(x, y, z))
    light.set_actor_label("%s%d_%d" % (prefix, int(x), int(y)))
    light.set_editor_property("tags", [tag])
    comp = light.point_light_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_editor_property("intensity", intensity)
    comp.set_editor_property("attenuation_radius", radius)
    comp.set_editor_property("light_color", colour)
    comp.set_editor_property("source_radius", 400.0)
    comp.set_editor_property("cast_shadows", False)
    # Visibility rather than affects_world: the runtime toggles visibility,
    # and a light authored off one way and switched on another is a light
    # that will one day be on in the editor and off in the game.
    comp.set_visibility(visible)
    return light


emergency = mains = 0

x = -FLOOR_HALF_X + 750.0
while x <= FLOOR_HALF_X - 750.0:
    y = -FLOOR_HALF_Y + 650.0
    while y <= FLOOR_HALF_Y - 650.0:
        reach = min(1.0, abs(y - GLASS_Y) / (FLOOR_HALF_Y * 2.0))
        intensity = EMERGENCY_NEAR + (EMERGENCY_FAR - EMERGENCY_NEAR) * reach
        place(EMERGENCY_PREFIX, EMERGENCY_TAG, x, y, EMERGENCY_Z,
              intensity, EMERGENCY_RADIUS, EMERGENCY_COLOUR, True)
        emergency += 1
        y += 1300.0
    x += 1500.0

# Mains on a tighter grid, because even coverage is what makes a shop read as
# open rather than as a cave with a torch in it.
x = -FLOOR_HALF_X + 600.0
while x <= FLOOR_HALF_X - 600.0:
    y = -FLOOR_HALF_Y + 600.0
    while y <= FLOOR_HALF_Y - 600.0:
        place(MAINS_PREFIX, MAINS_TAG, x, y, MAINS_Z,
              MAINS_INTENSITY, MAINS_RADIUS, MAINS_COLOUR, False)
        mains += 1
        y += 1200.0
    x += 1200.0

print("POWERSTATES emergency=%d (%.1f..%.1f cd, on) mains=%d (%.1f cd, off)" % (
    emergency, EMERGENCY_FAR, EMERGENCY_NEAR, mains, MAINS_INTENSITY))

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("POWERSTATES saved=True")
