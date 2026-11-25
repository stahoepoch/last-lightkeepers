"""Put the verbs in the level. There were none.

2026-11-24, after Chris played the opening map himself:

> *"I just want to have a fun game to play, and it's kind of, I don't know,
> not fun. I guess because there's nothing to do. It feels kind of like
> something I'd see in 1996 or something."*

He is describing a fact, and the fact is countable. `LLK_Boswells_OpeningPass`
holds **3,063 actors**. Six of them do anything at all:

| actor | n |
| --- | --- |
| `ALLKLanternPickup` | 1 |
| `ALLKOpeningGrille` | 1 |
| `ALLKExtractionZone` | 1 (a leftover of the extraction raid, not this chapter) |
| `ALLKRelicActor` | 4 |
| `ALLKStoreElevator` | 2 |
| `ALLKSweeper` | 1 |
| **`ALLKBreakerPanel`** | **0** |
| **`ALLKPushableFixture`** | **0** |

So 3,000-odd pieces of shop and ten things to touch, and the two verbs that
would make the floor a place you *act in* rather than walk through are
compiled, tested, documented - and in no level.

**This is the project's own failure shape, one layer up.**
`Tools/verify_boswells_ground_cells.py` named it: *a correct system over an
absent thing, with a clean log.* Every previous instance was a system over
absent geometry. This one is worse, because the absent thing is the gameplay:
`ALLKBreakerPanel` is 170 lines of careful argument about one-way switches,
noise economy and mounting a handle at a child's eye height, and nothing has
ever been able to press E on it.

It also means the last three days of lighting work could not pay off. The
breaker is `docs/ROADMAP.md` goal 5 - *"find a story clue, change part of the
building, and reach Sayers"* - in its most literal form, and the reason the
emergency state has never felt like a choice is that there has been no way to
end it.

## What this places, and why there

**One breaker, in the north service corridor.** Traced, not guessed:
`ROUTE_SERVICE_WALL_W` and `ROUTE_NORTH_WALL_W` enclose a walkable
back-of-house band behind the entrance, and a breaker panel belongs on a
service wall rather than on the selling floor. It sits before the crossing
rather than after it, which makes it a decision instead of a reward: the
keeper can light the floor they are about to walk, and pay for it with the
loudest noise in the game.

`ULLKPowerSubsystem::PowerOnNoiseStuds` is **70** against a pushed fixture's
16. Every machine on the floor hears the building wake up. That is the whole
point of the beat and it needed no new code.

**Three pushable fixtures**, on the ground floor where a 68 cm body has
somewhere to shove them: two stock units in the service corridor, one by the
escalator landing on the selling floor.

## What this does NOT do, deliberately

**It does not gate the breaker behind the climb.** `ALLKBreakerPanel`'s own
header suggests it - *"the level author can raise it to make the player find
something to climb first"* - and the reach model cannot express it. `ReachCm`
is an eye-to-handle sphere, so with the eye at 124 cm and a body that can
stand 40 cm from a wall, every handle below **268 cm** is reachable standing
no matter how high it is authored. Gating on height therefore means either a
handle mounted absurdly high for a shop, or a `ReachCm` short enough (~60 cm)
that the prompt vanishes when the keeper stands slightly wrong. Both are worse
than a panel you stretch up to.

So the handle is authored at **7 studs, 196 cm** - where a real panel is, and
well above a 145 cm child - and the gate is one number away whenever Chris
wants it. A puzzle invented to justify a verb is not a puzzle.

**It does not touch the extraction zone.** `ANCHOR_LOADING_DOCK_EXIT` is an
`ALLKExtractionZone`, which belongs to the co-op raid `AGENTS.md` calls
history. Removing it is a decision about the dock chapter, not a side effect
of placing a breaker.

Idempotent: finds its own actors by label and moves them rather than stacking
a second set. Saves the map, so it needs `-AllowUnsafeMapSave`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
CUBE = "/Engine/BasicShapes/Cube.Cube"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
PANEL_MATERIAL = MATERIAL_DIR + "/MI_LLK_Panel_Breaker"
PANEL_COLOUR = (0.31, 0.32, 0.34)   # grey enamel, the colour of a loadcentre
CRATE_PARENT = MATERIAL_DIR + "/M_LLK_LightFixture"
CRATE_MATERIAL = MATERIAL_DIR + "/MI_LLK_Crate_Stock"
CRATE_COLOUR = (0.38, 0.27, 0.16)   # cardboard, not a hole in the floor

BREAKER_LABEL = "BSW_OPENING_BREAKER_PANEL"

# Authored at a real panel's height, not at a height invented to force a climb.
# See the docstring: the reach model cannot gate on this.
HANDLE_CM = 196.0
BREAKER_REACH_CM = 150.0

# The corridor light. Top of the emergency layer's own 1.3-2.8 cd band, in
# the battery-pack white that pass authored.
CORRIDOR_LIGHT_CD = 2.8
EMERGENCY_TAG = "LLK_Emergency"

# The panel body, in cm. A loadcentre door is about this.
PANEL_W, PANEL_T, PANEL_H = 60.0, 12.0, 90.0
PANEL_CENTRE_Z = 170.0       # so the door spans 125..215 and the handle at 196 is on it

# Where to look for the service wall, and which way its usable face points.
# Traced rather than hardcoded: the wall's y moved twice during the route pass.
BREAKER_PROBE = (-3650.0, 2400.0)
BREAKER_WALL_FAMILY = "ROUTE_SERVICE_WALL"
PANEL_STANDOFF_CM = 8.0      # how far the body sits off the wall face

# Pushables: label -> (x, y, footprint cm, height cm, travel studs, heading deg)
# Heights are all climbable: `LLKClimbFeel::MaxLedgeHeightCm` is the keeper's
# eye at 124 cm, so a 60 cm crate is a step and a 110 cm one is a mantle.
PUSHABLES = {
    "BSW_OPENING_PUSH_STOCK_WEST":  (-3250.0, 2400.0, 90.0, 60.0,  8.0,   0.0),
    "BSW_OPENING_PUSH_STOCK_EAST":  ( -800.0, 2400.0, 90.0, 110.0, 8.0,   0.0),
    "BSW_OPENING_PUSH_ESCALATOR":   (  900.0,  600.0, 90.0, 60.0,  6.0,  90.0),
}

EYE_CM = 124.0
MEZZ_DECK_CM = 650.0          # the upper storey. Two floors, one x and y.
GROUND_HEAD_CM = 400.0        # under the ground floor ceiling tile at 514.


def say(message):
    unreal.log("VERB " + message)
    print("VERB " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("VERB " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
actors_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actors_sub.get_all_level_actors()

cube = unreal.EditorAssetLibrary.load_asset(CUBE)
require(cube is not None, "no cube mesh")
def grey_or_cardboard(path, colour):
    """A dull diffuse instance. Creates it the first time, re-authors it after.

    Both of this tool's objects first went in on borrowed materials - the
    panel on `M_BSW_DarkChrome`, the crate on the service carpet - and both
    came back from the render as pure black rectangles. A black rectangle is
    not a breaker and not a crate; it is the absence of one, which is the
    whole complaint about the upper storey's blockout cubes.
    """
    mat = unreal.EditorAssetLibrary.load_asset(path)
    if mat is None:
        parent = unreal.EditorAssetLibrary.load_asset(CRATE_PARENT)
        require(parent is not None, "no %s to instance" % CRATE_PARENT)
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            path.split("/")[-1], MATERIAL_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew())
        require(mat is not None, "could not create " + path)
        mat.set_editor_property("parent", parent)
        say("created " + path)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        mat, "BaseColour", unreal.LinearColor(*colour, 1.0))
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        mat, "Emissive", 0.0)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    return mat


panel_mat = grey_or_cardboard(PANEL_MATERIAL, PANEL_COLOUR)

crate_mat = grey_or_cardboard(CRATE_MATERIAL, CRATE_COLOUR)

by_label = {}
for actor in actors:
    by_label.setdefault(actor.get_actor_label(), []).append(actor)


def trace(start, end, ignore=None):
    return unreal.SystemLibrary.line_trace_single(
        world, start, end, unreal.TraceTypeQuery.ECC_VISIBILITY, True,
        ignore or [], unreal.DrawDebugTrace.NONE, True)


def mesh_of(actor):
    """The one static mesh component, whatever the class calls it.

    `ALLKBreakerPanel` and `ALLKPushableFixture` both build their own `Body`
    subobject, so neither has `AStaticMeshActor::static_mesh_component` - and
    reaching for that name is how this tool failed on its first run.
    """
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    require(len(comps) == 1, "%s has %d static mesh components, want 1"
            % (actor.get_actor_label(), len(comps)))
    return comps[0]


def dress(actor, mesh, material, scale_cm):
    """A cube is 100 cm, so scale is the size in metres."""
    comp = mesh_of(actor)
    # `SetStaticMesh` answers false when the mesh is already the one asked for,
    # which made the second run of this raise "mesh assignment failed" on an
    # actor that was correct. Assert the end state, not the act - the same
    # correction `Tools/ceiling_boswells_upper_storey.py` had to make.
    if comp.static_mesh != mesh:
        comp.set_static_mesh(mesh)
    require(comp.static_mesh == mesh,
            "%s would not take the mesh" % actor.get_actor_label())
    comp.set_material(0, material)
    actor.set_actor_scale3d(unreal.Vector(scale_cm[0] / 100.0,
                                          scale_cm[1] / 100.0,
                                          scale_cm[2] / 100.0))


def reuse_or_spawn(label, cls, location):
    """Idempotent: one actor per label, moved rather than duplicated."""
    existing = [a for a in by_label.get(label, []) if isinstance(a, cls)]
    stale = [a for a in by_label.get(label, []) if not isinstance(a, cls)]
    for a in stale:
        say("  removing a %s that is squatting on %s"
            % (a.get_class().get_name(), label))
        actors_sub.destroy_actor(a)
    if existing:
        actor = existing[0]
        for extra in existing[1:]:
            actors_sub.destroy_actor(extra)
        actor.set_actor_location(location, False, True)
        return actor, False
    actor = actors_sub.spawn_actor_from_class(cls, location)
    require(actor is not None, "spawn failed for " + label)
    actor.set_actor_label(label)
    return actor, True


# =====================================================================
# 1. Find the service wall the breaker hangs on
# =====================================================================

px, py = BREAKER_PROBE
hit = trace(unreal.Vector(px, py, EYE_CM), unreal.Vector(px, py - 900.0, EYE_CM))
require(hit is not None, "no wall south of the north service corridor at x %.0f, "
        "so there is nowhere in that corridor to hang a breaker" % px)
parts = hit.to_tuple()
wall_actor = parts[9]
wall_y = parts[5].y
require(BREAKER_WALL_FAMILY in wall_actor.get_actor_label().upper(),
        "the first thing south of the service corridor is %s, not a %s - the "
        "corridor is not where this tool thinks it is"
        % (wall_actor.get_actor_label(), BREAKER_WALL_FAMILY))
say("service wall: %s, face at y %.0f" % (wall_actor.get_actor_label(), wall_y))

# Is there floor to stand on in front of it?
stand_y = wall_y + PANEL_STANDOFF_CM + 60.0
floor = trace(unreal.Vector(px, stand_y, 400.0), unreal.Vector(px, stand_y, -200.0))
require(floor is not None, "nothing to stand on in front of the service wall")
floor_z = floor.to_tuple()[4].z
say("floor in front of it: z %.0f" % floor_z)


# =====================================================================
# 2. The breaker
# =====================================================================

# `ALLKBreakerPanel` makes `Body` its ROOT component, so the panel mesh cannot
# be offset up from the actor origin - moving the body moves the actor, which
# is exactly what the first run of this did: it put the origin at 170 and the
# handle at 366 cm, a breaker mounted above the ceiling line. So the origin IS
# the middle of the panel door, and `HandleHeightStuds` carries the short
# offset from there up to the handle.
panel_y = wall_y + PANEL_STANDOFF_CM + PANEL_T * 0.5
breaker, is_new = reuse_or_spawn(
    BREAKER_LABEL, unreal.LLKBreakerPanel,
    unreal.Vector(px, panel_y, floor_z + PANEL_CENTRE_Z))
dress(breaker, cube, panel_mat, (PANEL_W, PANEL_T, PANEL_H))
breaker.set_editor_property("ReachCm", BREAKER_REACH_CM)

# Calibrate rather than hardcode 28 cm to the stud. `LLKUnits::StudsToCm` is a
# C++ constant this script cannot read, and a tool that hardcodes a copy of a
# constant is a tool that silently drifts when the constant changes. Measure it
# off the class itself: set one stud, see where the handle lands.
breaker.set_editor_property("HandleHeightStuds", 1.0)
one_stud_cm = breaker.get_handle_point().z - breaker.get_actor_location().z
require(one_stud_cm > 1.0, "a stud measured %.2f cm, which cannot be right"
        % one_stud_cm)
wanted_above_origin = HANDLE_CM - PANEL_CENTRE_Z
breaker.set_editor_property("HandleHeightStuds",
                            wanted_above_origin / one_stud_cm)
say("one stud is %.1f cm, so the handle is authored %.2f studs above the "
    "panel centre to land at %.0f cm off the floor"
    % (one_stud_cm, wanted_above_origin / one_stud_cm, HANDLE_CM))

handle = breaker.get_handle_point()
require(abs(handle.z - (floor_z + HANDLE_CM)) < 1.0,
        "the handle landed at %.0f cm and should be at %.0f"
        % (handle.z - floor_z, HANDLE_CM))
say("breaker %s at (%.0f %.0f %.0f), door 125..215, handle z %.0f, reach %.0f cm"
    % ("placed" if is_new else "moved", px, panel_y, floor_z, handle.z,
       BREAKER_REACH_CM))

# Assert the thing that matters: a keeper standing in front of it can reach it.
# This is the check that would have caught a panel hung on the wrong side of a
# wall, which is how an interactable ends up in a level and still does nothing.
eye = unreal.Vector(px, stand_y, floor_z + EYE_CM)
reach = (handle - eye).length()
require(reach <= BREAKER_REACH_CM,
        "the handle is %.0f cm from the eye of a keeper standing in front of "
        "the panel and the reach is %.0f, so the prompt would never appear"
        % (reach, BREAKER_REACH_CM))
say("  a keeper standing %.0f cm off the wall has the handle %.0f cm from "
    "their eye, inside the %.0f cm reach" % (stand_y - wall_y, reach,
                                             BREAKER_REACH_CM))

# And that the panel is not buried in the wall or in a rack.
clear = trace(unreal.Vector(px, stand_y + 120.0, floor_z + EYE_CM),
              unreal.Vector(px, panel_y, floor_z + EYE_CM))
if clear is not None:
    front = clear.to_tuple()[9]
    require(front == breaker or front == wall_actor,
            "%s stands between the keeper and the breaker, so the panel is "
            "behind the shop fittings" % front.get_actor_label())


# A breaker in an unlit corridor is a breaker nobody finds. The first render of
# this panel came back as a BLACK FRAME: the north service corridor has no
# light of any kind, so the one interactable that changes the chapter sits in
# a room the keeper cannot see is a room. Real service corridors carry egress
# lighting, and `repair_boswells_emergency_state.py` already decided the egress
# route is the emergency circuit - this corridor was simply missed.
light_label = BREAKER_LABEL + "_EGRESS_LIGHT"
corridor_light, _ = reuse_or_spawn(
    light_label, unreal.PointLight,
    unreal.Vector(px, panel_y + 90.0, floor_z + 300.0))
lc = corridor_light.point_light_component
lc.set_editor_property("intensity", CORRIDOR_LIGHT_CD)
lc.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
lc.set_editor_property("attenuation_radius", 700.0)
lc.set_editor_property("light_color", unreal.Color(158, 189, 179, 255))
lc.set_editor_property("cast_shadows", False)
lc.set_editor_property("visible", True)
corridor_light.set_editor_property("tags", [unreal.Name(EMERGENCY_TAG)])
say("corridor egress light at %.1f cd on %s, so the panel is findable in the "
    "dark" % (CORRIDOR_LIGHT_CD, EMERGENCY_TAG))


# =====================================================================
# 3. The pushables
# =====================================================================

placed = 0
moved = 0
for label, (x, y, foot, height, travel, heading) in sorted(PUSHABLES.items()):
    # From BELOW the mezzanine deck, not above it. The first run of this traced
    # from z 700 and every crate landed at 662 - on top of the upper storey's
    # deck, in the departments, 6.6 m above the floor they were meant to stand
    # on. Boswell's has two floors at the same x and y and only z tells them
    # apart, which is the same trap the ceiling pass fell into with the lights.
    # Ignore the crate this tool put here last time, or the trace lands on its
    # own previous run's lid and the tool refuses to run twice.
    mine = list(by_label.get(label, []))
    down = trace(unreal.Vector(x, y, GROUND_HEAD_CM),
                 unreal.Vector(x, y, -200.0), mine)
    require(down is not None, "no floor under %s at (%.0f %.0f)" % (label, x, y))
    z = down.to_tuple()[4].z
    stood_on = down.to_tuple()[9].get_actor_label()
    # Name the thing it landed on, and insist it is a floor. Twice now this
    # trace has started too high and found a different storey: once the
    # mezzanine deck at 662, once the ground floor's own CEILING tile at 519.
    # "Is this a floor" is the question; "is z small" is a guess at it.
    require("FLOOR" in stood_on.upper(),
            "%s came to rest on %s at z %.0f, which is not a floor"
            % (label, stood_on, z))

    # The cube's pivot is its middle and `Body` is the root, so the actor sits
    # at half the crate's height and the crate's underside lands on the floor.
    # Offsetting the component instead would move the actor, which is how the
    # breaker ended up with its handle at 366 cm on the first run.
    fixture, is_new = reuse_or_spawn(
        label, unreal.LLKPushableFixture,
        unreal.Vector(x, y, z + height * 0.5))
    dress(fixture, cube, crate_mat, (foot, foot, height))
    fixture.set_actor_rotation(unreal.Rotator(0.0, 0.0, heading), False)
    fixture.set_editor_property("TravelStuds", travel)

    # A pushable nobody can climb is a crate. The climb only accepts a ledge
    # at or below the keeper's eye, so a crate taller than that is furniture.
    require(height <= EYE_CM,
            "%s is %.0f cm and the keeper's eye is at %.0f, so its top is not "
            "a ledge the climb will take" % (label, height, EYE_CM))

    # And the top has to be findable by a trace, or the climb cannot see it.
    top = trace(unreal.Vector(x, y, z + height + 80.0),
                unreal.Vector(x, y, z + height * 0.5))  # must FIND it, so no ignore
    require(top is not None and top.to_tuple()[9] == fixture,
            "a trace down onto %s does not land on it, so the climb cannot "
            "find its top" % label)
    ledge = top.to_tuple()[4].z - z
    require(abs(ledge - height) < 3.0,
            "%s reads %.0f cm tall to a trace and was authored %.0f"
            % (label, ledge, height))

    if is_new:
        placed += 1
    else:
        moved += 1
    say("pushable %-32s (%7.0f %7.0f %6.0f)  %.0f cm tall, %.0f studs of travel"
        % (label, x, y, z, height, travel))

say("pushables: %d placed, %d moved" % (placed, moved))


# =====================================================================
# 4. The end state, asserted
# =====================================================================

after = actors_sub.get_all_level_actors()
breakers = [a for a in after if isinstance(a, unreal.LLKBreakerPanel)]
pushables = [a for a in after if isinstance(a, unreal.LLKPushableFixture)]
require(len(breakers) == 1, "%d breakers, want exactly 1" % len(breakers))
require(len(pushables) == len(PUSHABLES),
        "%d pushables, want %d" % (len(pushables), len(PUSHABLES)))
say("the level now contains %d breaker and %d pushable fixtures"
    % (len(breakers), len(pushables)))

saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))
