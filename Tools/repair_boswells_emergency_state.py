"""Put every light in Boswell's on a circuit, so the dark state is dark.

Chris, 2026-11-23, having opened `LLK_Boswells_OpeningPass` and looked at it
himself: *"the lighting is still to bright and not emergency looking."*

He is right, and the cause is not an intensity anywhere. It is a rule that was
written in C++ and then not applied to the map.

## The rule the building already has

`ULLKPowerSubsystem` is explicit, in Chris's own words at the top of its
header: *"Emergency is the default and always will be."* It finds lights **by
tag**: `LLK_Mains` goes dark when the breaker is off, `LLK_Emergency` stays
lit, and the header says why tags rather than a list - *"a renamed light stops
being a mains light, which is correct."*

What neither the header nor any tool says out loud is the other half:

> **An untagged light is on in both states.**

`ApplyToLights()` only ever touches actors carrying `LLK_Mains`. A light with
no tag is never switched, never reported, and never counted - it simply burns,
in the emergency state and in the mains state alike.

## What the map actually had

`Tools/audit_boswells_clearance_and_mood.py` read every light below the
mezzanine deck and found 138, of which **79 are lit at boot**. Of those 79,
exactly **22 carry `LLK_Emergency`**, and they run at **1.3 to 2.8 candela**.
The other fifty-odd carry no tag at all:

| Family | n | intensity | tagged |
| --- | --- | --- | --- |
| `LLK_DOCK_CHAPTER_WING_POOL_*` | 13 | **31-46** | no |
| `BSW_EGRESS_LOWER_WING/MAIN_*` | 17 | **34** | no |
| `GALLERIA_NEAR_ROUTE_LIGHT_*` | 10 | 19 | no |
| `BSW_DEPT_ROUTE_POOL_*` | 7 | 28 | no |
| `BSW_ESC_DETAIL_*_ROUTE` | 4 | 23 | no |
| `BSW_OPENING_ENTRY_POOL_*` | 2 | **55** | no |
| `BSW_EMERGENCY_FILL_*`, `BSW_EASTFILL_*` | 22 | **1.3-2.8** | `LLK_Emergency` |
| `BSW_MAINS_*` | 27 | 26 | `LLK_Mains`, **off** |

**The emergency state is brighter than the mains state.** The mains are 26
candela and dark; the floor is lit instead by fifty untagged pools at 19 to
55, and the layer actually called emergency contributes about one twentieth of
the light in the room. Throwing the breaker would barely change the picture,
which is the whole chapter beat from `ROADMAP.md` goal 5 - *"change part of
the building"* - quietly not working.

This is the shape this project keeps producing and
`Tools/verify_boswells_ground_cells.py` names in its own header: **a correct
system over an absent thing, with a clean log.** The subsystem is right. Every
tool that placed a light is right on its own terms. Nothing ever asked whether
the two agreed, and `OnWorldBeginPlay` cheerfully reports *"Boswell's is on
emergency power: 22 emergency light(s) lit, 27 mains light(s) dark"* while
fifty lights it has never heard of light the room.

## What this changes

**1. Every light inside the building joins a circuit.** Classified by what the
light is for, and the classification is printed before it is applied:

- **Emergency** - the egress route, and only that. In a real building the
  emergency layer exists to show you the way out and does nothing else, which
  is also exactly the brief `docs/FEAR-AND-LIGHT.md` sets: shapes, not
  contents.
- **Mains** - everything else inside the plate: department pools, escalator
  detail, the wing pools, shopfronts. These are selling light, and selling
  light is what a dead building does not have.
- **Left alone** - anything this tool cannot place is reported as
  unclassified and **not touched**. A tool that guesses in silence is how a
  floor ends up with fifty lights nobody chose.

**2. The emergency layer stops being warm.** It was `rgb(232, 178, 150)` -
orange, the colour of a living-room lamp, which is most of why a dim room
still did not read as an emergency. Emergency heads and exit signage are cold
and slightly green; that is what a battery pack looks like and what a player
has seen in every stairwell they have ever been in.

**3. Intensities are NOT touched.** They are already a 12-to-26x ratio between
the egress pools and the fill, which is the pools-and-dark shape an emergency
floor should have - it was simply buried under fifty untagged lights. Taking
those out is one change, and the measurement comes after it rather than beside
it. If the floor is still too bright, that is the next change and it will be
made against a photograph.

Idempotent: it sets tags and colours rather than adding actors. Saves the map,
so it needs `-AllowUnsafeMapSave`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"

PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
MEZZ_DECK = 650.0

MAINS_TAG = "LLK_Mains"
EMERGENCY_TAG = "LLK_Emergency"

# The egress layer: the way out, and nothing else. Named rather than inferred,
# because "is it on the way out" is a design fact about this floor and not a
# property any transform carries.
EMERGENCY_FAMILIES = (
    "BSW_EMERGENCY_FILL_",      # the 1500 cm grid, 1.3-2.8 cd
    "BSW_EASTFILL_",            # the east wall extension of that same grid
    "ANCHOR_EMERGENCY_",        # the original anchor layer
    "BSW_EGRESS_",              # the marked escape routes
    "GALLERIA_NEAR_ROUTE_LIGHT_",  # the mall's own egress outside the doors
)

# Selling light. Everything a working store switches on in the morning.
MAINS_FAMILIES = (
    "BSW_MAINS_",               # already tagged; listed so the count is honest
    "BSW_DEPT_ROUTE_POOL_",     # department pools
    "BSW_ESC_DETAIL_",          # escalator detail lighting
    "BSW_OPENING_ENTRY_POOL_",  # the entrance wash, the brightest on the floor
    "LLK_DOCK_CHAPTER_WING_",   # the sales wings' general illumination
    "GALLERIA_SHOP_",           # shopfronts in the mall
)

# Scripted or belonging to another chapter's staging. Not on either circuit,
# and deliberately not touched - a storm flash is a cue, not a lamp.
LEAVE_ALONE = (
    "BSW_CAPTURE_",             # the opening capture's own staging
    "LLK_DOCK_CHAPTER_DOCK_",   # the loading dock, outside the sales plate
    "LLK_DOCK_CHAPTER_WEST_SERVICE", "LLK_DOCK_CHAPTER_EAST_SERVICE",
    "LLK_DOCK_CHAPTER_STAFF_", "LLK_DOCK_CHAPTER_FAR_",
)

# A dead battery pack is cold and a little green. The old value was
# rgb(232, 178, 150) - orange - and no amount of dimming makes orange read as
# emergency, because the association the player brings is a table lamp.
EMERGENCY_COLOUR = (158, 189, 179)


def say(message):
    unreal.log("CIRCUIT " + message)
    print("CIRCUIT " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("CIRCUIT " + message)


def classify(label, loc):
    if label.startswith(LEAVE_ALONE):
        return None
    if label.startswith(EMERGENCY_FAMILIES):
        return EMERGENCY_TAG
    if label.startswith(MAINS_FAMILIES):
        return MAINS_TAG
    return "?"


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

lights = []
for actor in actors:
    if not isinstance(actor, unreal.Light):
        continue
    comp = actor.light_component
    if comp is None:
        continue
    lights.append((actor, comp, actor.get_actor_location()))
say("lights in the map: %d" % len(lights))


# =====================================================================
# 1. Classify, and say so before changing anything
# =====================================================================

plan = {EMERGENCY_TAG: [], MAINS_TAG: [], "?": [], None: []}
for actor, comp, loc in lights:
    plan[classify(actor.get_actor_label(), loc)].append((actor, comp, loc))

for bucket, name in ((EMERGENCY_TAG, "EMERGENCY - stays lit"),
                     (MAINS_TAG, "MAINS - dark until the breaker"),
                     (None, "LEFT ALONE - scripted or another chapter"),
                     ("?", "UNCLASSIFIED - NOT TOUCHED")):
    rows = plan[bucket]
    lit = sum(1 for a, c, l in rows
              if c.get_editor_property("intensity") > 0.0
              and c.get_editor_property("visible"))
    say("%-44s n=%-4d lit now=%d" % (name, len(rows), lit))

if plan["?"]:
    say("the %d lights below match no rule in this tool. They keep whatever "
        "they have, which means they stay on in BOTH power states. Add them "
        "to a family above or accept that the breaker does not move them:"
        % len(plan["?"]))
    for actor, comp, loc in plan["?"][:40]:
        say("  ? %-46s %.0f cd at (%.0f,%.0f,%.0f)"
            % (actor.get_actor_label(),
               comp.get_editor_property("intensity"), loc.x, loc.y, loc.z))


# =====================================================================
# 2. Apply the tags
# =====================================================================

def retag(actor, wanted):
    """One tag, not two. A light carrying both is read as mains by
    `OnWorldBeginPlay` - the `else if` means the mains branch wins - so a
    double-tagged emergency light would go dark with the breaker and nobody
    would see why."""
    tags = [str(t) for t in actor.tags]
    keep = [t for t in tags if t not in (MAINS_TAG, EMERGENCY_TAG)]
    if keep + [wanted] == tags:
        return False
    actor.set_editor_property("tags", [unreal.Name(t) for t in keep + [wanted]])
    return True


moved_mains = 0
moved_emerg = 0
for actor, comp, loc in plan[MAINS_TAG]:
    if retag(actor, MAINS_TAG):
        moved_mains += 1
for actor, comp, loc in plan[EMERGENCY_TAG]:
    if retag(actor, EMERGENCY_TAG):
        moved_emerg += 1
say("tags: %d joined the mains, %d joined the emergency layer, "
    "%d already correct"
    % (moved_mains, moved_emerg,
       len(plan[MAINS_TAG]) + len(plan[EMERGENCY_TAG])
       - moved_mains - moved_emerg))
require(moved_mains > 0 or moved_emerg > 0,
        "nothing was retagged - either this has already run, or the light "
        "families have been renamed and this tool's premise is gone")


# =====================================================================
# 3. A mains light must be AUTHORED off, not only switched off
# =====================================================================
#
# `OnWorldBeginPlay` calls `ApplyToLights()` at boot so the runtime is right
# either way. But a map saved with a mains light visible is a map that is lit
# in the editor, in a thumbnail, and in any capture taken without PIE - which
# is every screenshot this project has, because Chris is remote and renders
# are how he sees the game. The saved state has to agree with the boot state.
authored_off = 0
for actor, comp, loc in plan[MAINS_TAG]:
    if comp.get_editor_property("visible"):
        comp.set_editor_property("visible", False)
        authored_off += 1
say("mains authored dark in the saved map: %d newly, %d of %d total"
    % (authored_off, len(plan[MAINS_TAG]), len(plan[MAINS_TAG])))


# =====================================================================
# 4. The emergency layer stops being orange
# =====================================================================

recoloured = 0
for actor, comp, loc in plan[EMERGENCY_TAG]:
    before = comp.get_editor_property("light_color")
    was = (before.r, before.g, before.b)
    colour = unreal.Color(r=EMERGENCY_COLOUR[0], g=EMERGENCY_COLOUR[1],
                          b=EMERGENCY_COLOUR[2], a=255)
    comp.set_editor_property("light_color", colour)
    comp.set_editor_property("use_temperature", False)
    if was != EMERGENCY_COLOUR:
        recoloured += 1
say("emergency colour: %d of %d lights moved off warm orange onto the cold "
    "green-white a battery pack actually gives"
    % (recoloured, len(plan[EMERGENCY_TAG])))


saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))
