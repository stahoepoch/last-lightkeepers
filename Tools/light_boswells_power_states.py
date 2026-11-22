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

## Corrected 2026-11-22, hours after it shipped: every light was in the attic

The first version of this file hung both layers at two typed constants,
`EMERGENCY_Z = 1150` and `MAINS_Z = 1380`, across the whole footprint.
**The lower sales floor's ceiling is at 514 and the mezzanine deck above it
at 638**, so all 48 lights were placed above both, in the roof void, lighting
the top of a ceiling nobody can see.

What the store looked like in the screenshots was the original
`ANCHOR_EMERGENCY_LOWER` spots at ~485 doing all the work. The layer added
here contributed nothing to the floor the chapter plays on, and - the part
that matters - **throwing the breaker would have lit the mezzanine and left
the sales floor exactly as dark as before.** The feature read as working
because the only thing that ever checked it counted lights.

So the heights are no longer typed. Each light is hung under the ceiling
that is actually above its own position, read from the map: one under the
lowest lid for the floor you walk on, one under the highest for the
mezzanine. The constant was never a number that could have been right
everywhere, because the building has two storeys and an atrium.

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
EMERGENCY_RADIUS = 4200.0
EMERGENCY_COLOUR = unreal.Color(150, 178, 232, 255)

# Mains: a working shop. Neutral white, even, hung at the ceiling.
MAINS_INTENSITY = 26.0
MAINS_RADIUS = 2600.0
MAINS_COLOUR = unreal.Color(255, 248, 236, 255)

# How far below a ceiling a hung fitting sits. Small, because the point is
# that the light and the thing it comes out of are the same object to a
# player, and `fixture_boswells_lights.py` places the housing at the light.
DROP_CM = 26.0

# What counts as a ceiling. A lid is a big flat thing overhead; a shelf is
# not, and a 1 m sign is not.
#
# **140, not 300.** At 300 this missed the aisle bulkhead entirely, because
# `ceiling_boswells_aisle_soffit.py` lays it as 3 m pieces so it can cut round
# the atrium, and a 3 m piece has a half-span of 150. The aisle therefore got
# a ceiling and no lights under it, and the black trench in the screenshots
# became a dark grey one - the same hole, now with a lid on it. A ceiling made
# of many small pieces is still a ceiling.
LID_MIN_SPAN_CM = 140.0
LID_MAX_THICKNESS_CM = 40.0
LID_MIN_HEIGHT_CM = 300.0

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("POWERSTATES map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# ---- the ceilings this building actually has ----------------------------
lids = []
for actor in subsystem.get_all_level_actors():
    label = actor.get_actor_label()
    if label.startswith(EMERGENCY_PREFIX) or label.startswith(MAINS_PREFIX):
        continue
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not any(c.static_mesh is not None for c in comps):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if (extent.z > LID_MAX_THICKNESS_CM or extent.x < LID_MIN_SPAN_CM
            or extent.y < LID_MIN_SPAN_CM or centre.z < LID_MIN_HEIGHT_CM):
        continue
    lids.append((centre, extent, label))
print("POWERSTATES lids=%d" % len(lids))
if not lids:
    raise RuntimeError("POWERSTATES no ceilings found; every light would be "
                       "hung at a guess, which is the fault this replaces")


def ceiling_heights(x, y):
    """Undersides of every ceiling over this point, lowest first.

    The light goes under the FIRST of these for the floor you walk on and
    under the LAST for the mezzanine. The ones between are plenum - the gap
    between a suspended ceiling and the deck it hangs from - and a light in
    there illuminates insulation.
    """
    over = sorted(c.z - e.z for c, e, _ in lids
                  if abs(x - c.x) <= e.x and abs(y - c.y) <= e.y)
    if not over:
        return []
    return [over[0]] if len(over) == 1 else [over[0], over[-1]]


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
    # The height is in the name. Two lights at one (x, y) on two storeys is
    # the normal case now, and a label that cannot tell them apart is how a
    # later pass deletes the wrong one.
    light.set_actor_label("%s%d_%d_%d" % (prefix, int(x), int(y), int(z)))
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
skipped_no_ceiling = 0
storeys = {}

x = -FLOOR_HALF_X + 750.0
while x <= FLOOR_HALF_X - 750.0:
    y = -FLOOR_HALF_Y + 650.0
    while y <= FLOOR_HALF_Y - 650.0:
        heights = ceiling_heights(x, y)
        if not heights:
            # Open to the sky, or over the atrium void with nothing above.
            # Hanging a light at a guess here is exactly the fault this run
            # exists to undo, so it hangs nothing.
            skipped_no_ceiling += 1
            y += 1300.0
            continue
        reach = min(1.0, abs(y - GLASS_Y) / (FLOOR_HALF_Y * 2.0))
        intensity = EMERGENCY_NEAR + (EMERGENCY_FAR - EMERGENCY_NEAR) * reach
        for under in heights:
            place(EMERGENCY_PREFIX, EMERGENCY_TAG, x, y, under - DROP_CM,
                  intensity, EMERGENCY_RADIUS, EMERGENCY_COLOUR, True)
            emergency += 1
            storeys[int(under)] = storeys.get(int(under), 0) + 1
        y += 1300.0
    x += 1500.0

# Mains on a tighter grid, because even coverage is what makes a shop read as
# open rather than as a cave with a torch in it.
x = -FLOOR_HALF_X + 600.0
while x <= FLOOR_HALF_X - 600.0:
    y = -FLOOR_HALF_Y + 600.0
    while y <= FLOOR_HALF_Y - 600.0:
        for under in ceiling_heights(x, y):
            place(MAINS_PREFIX, MAINS_TAG, x, y, under - DROP_CM,
                  MAINS_INTENSITY, MAINS_RADIUS, MAINS_COLOUR, False)
            mains += 1
        y += 1200.0
    x += 1200.0

print("POWERSTATES emergency=%d (%.1f..%.1f cd, on) mains=%d (%.1f cd, off)" % (
    emergency, EMERGENCY_FAR, EMERGENCY_NEAR, mains, MAINS_INTENSITY))
print("POWERSTATES hung_under_ceilings_at=%s skipped_no_ceiling=%d" % (
    sorted(storeys.items()), skipped_no_ceiling))

# The claim this file now has to be able to make. A mains layer that never
# reaches the floor the chapter plays on is the defect it was written to fix,
# so it refuses to save a map where that is still true.
LOWER_CEILING_MAX_CM = 700.0
lower_mains = sum(1 for a in subsystem.get_all_level_actors()
                  if a.get_actor_label().startswith(MAINS_PREFIX)
                  and a.get_actor_location().z < LOWER_CEILING_MAX_CM)
print("POWERSTATES mains_on_the_sales_floor=%d" % lower_mains)
if lower_mains == 0:
    raise RuntimeError(
        "POWERSTATES every mains light is above %.0f cm, so throwing the "
        "breaker would light the mezzanine and leave the sales floor as dark "
        "as before - which is the exact bug this run exists to fix"
        % LOWER_CEILING_MAX_CM)

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("POWERSTATES saved=True")
