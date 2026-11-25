"""Build the Galleria's upper level, and settle which map is the Galleria.

Chris, asked which of the two maps that model this mall is the real one:

    "The Galleria"

## The decision, with the measurements behind it

There are two. They cannot be reconciled by moving one into the other, and
the numbers say which one is a level and which one is a drawing.

| | `LLK_Boswells_OpeningPass` concourse | `LLK_Galleria_Shell` |
| --- | --- | --- |
| actors | 170 | 578 |
| length modelled | 48 m | 680 m |
| long axis | **Y** | **X** |
| lights | 16 point, 6 spot | **none at all** |
| Boswell's | the playable store, 87 x 54 m, two floors | a 45 x 48 m box, three floors |
| dressing | shopfronts, neon fascia, kiosks, planters, signage | untextured shells |
| connected to the store | yes, through `MALL_GLASS_ENTRANCE` | no |

**The two disagree about which way the mall runs and about how big
Boswell's is.** The shell's anchor is roughly half the store Chris has been
walking, at a different origin, with a third floor the playable store does not
have. So the shell cannot be imported, and the playable concourse cannot be
moved into the shell's frame without abandoning everything built on top of it.

**So the opening map is the Galleria, and `LLK_Galleria_Shell` is a plan.**
That map is another writer's uncommitted work and this tool does not open,
modify or delete it. Its value is what it knows about the mall's *layout* -
and the first thing it knows that the playable map does not is that **the
Galleria has two levels.**

## Why that matters right now, not later

`docs/FINDINGS-THE-STORE-HAD-NO-OUTSIDE.md` put a mall entrance at the store's
deck level, because Chris said the mall entrance has glass "on both top and
bottom". **It opens onto nothing.** The concourse has one floor, so that
doorway is a sheet of glass over a 650 cm drop.

And the concourse's own geometry has been saying the upper level exists the
whole time: `MALL_GLASS_ENTRANCE` is 1468 cm tall with a horizontal mullion at
**660** - a two-storey mall screen, divided at the floor level of a storey
nobody built.

This tool builds that storey.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
CUBE = "/Engine/BasicShapes/Cube.Cube"
PREFIX = "GALLERIA_UPPER_"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
FIXTURE_PARENT = MATERIAL_DIR + "/M_LLK_LightFixture"

# Read off the concourse rather than typed: the public floor runs x -900..900
# between shop frontages, the outer walls stand at x +-2200, and the roof is
# at 1490.
PUBLIC_FLOOR = "GALLERIA_NEAR_PUBLIC_FLOOR"
OUTER_W, OUTER_E = "GALLERIA_NEAR_W_OUTER_WALL", "GALLERIA_NEAR_E_OUTER_WALL"
MALL_SCREEN = "MALL_GLASS_ENTRANCE"
UPPER_DOOR = "BSW_EXT_MALL_UPPER_GLASS"

DECK_Z = 650.0            # the store's own upper floor, so they are one level
DECK_T = 26.0
RAIL_H = 110.0
ROOF_Z = 1490.0
OUTER_WALL_TOP = 1200.0   # measured; the gap to the roof is a hole

SHUTTER_H = 300.0
FASCIA_H = 80.0
BAY_LENGTH = 1550.0

STAIR_RISER = 18.05
STAIR_TREAD = 30.0
STAIR_W = 320.0

EMERGENCY_TAG = "LLK_Emergency"
BALCONY_LIGHT_CD = 2.8

NEW_MATERIALS = {
    "MI_LLK_Galleria_Shutter": (0.21, 0.22, 0.23),
    "MI_LLK_Galleria_Fascia":  (0.12, 0.13, 0.15),
}

# The first render of this balcony was a dark mass over the shopfronts: it was
# there and it did not read as a floor, because nothing on it was lit. What
# sells a mall's upper level from the ground is the line of light along its
# edge, which is exactly what the lower level already does with its teal
# fascia strips. 0.40 is the value
# docs/FINDINGS-THE-FITTINGS-WERE-NOT-THE-PROBLEM.md settled on - above it a
# strip clips to white before anyone sees its colour.
EDGE_EMISSIVE = 0.40
EDGE_COLOUR = (0.36, 0.86, 0.84)


def require(condition, message):
    if not condition:
        raise RuntimeError("UPPER " + message)


def say(message):
    unreal.log("UPPER| " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None, "could not load " + MAP)
actors_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube = unreal.EditorAssetLibrary.load_asset(CUBE)
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")

by_label = {}
for actor in actors_sub.get_all_level_actors():
    by_label.setdefault(actor.get_actor_label(), []).append(actor)


def bounds_of(label):
    group = by_label.get(label, [])
    require(len(group) == 1, "%s is not in the level exactly once" % label)
    return group[0].get_actor_bounds(False)


floor_o, floor_e = bounds_of(PUBLIC_FLOOR)
WELL_X = floor_e.x                     # the open void over the lower concourse
CONCOURSE_Y0 = floor_o.y - floor_e.y   # far end, south
CONCOURSE_Y1 = floor_o.y + floor_e.y   # near end, at the store
outer_o, outer_e = bounds_of(OUTER_W)
OUTER_X = abs(outer_o.x)
say("concourse: well half-width %.0f, outer wall at %.0f, y %.0f..%.0f"
    % (WELL_X, OUTER_X, CONCOURSE_Y0, CONCOURSE_Y1))

screen_o, screen_e = bounds_of(MALL_SCREEN)
require(abs((screen_o.z + screen_e.z) - 1468.0) < 30.0,
        "the mall screen's head is at %.0f, not the 1468 this tool was "
        "measured against" % (screen_o.z + screen_e.z))

door_o, door_e = bounds_of(UPPER_DOOR)
require(abs(door_o.z - (DECK_Z + 155.0)) < 60.0,
        "the store's upper mall entrance sits at z %.0f, which is not the "
        "deck this balcony is being built at" % door_o.z)
say("the store's upper mall entrance is at y %.0f, x %.0f..%.0f"
    % (door_o.y, door_o.x - door_e.x, door_o.x + door_e.x))


# ---------------------------------------------------------------- materials

def ensure_material(name, colour):
    path = MATERIAL_DIR + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = unreal.EditorAssetLibrary.load_asset(path)
        if isinstance(existing, unreal.MaterialInstanceConstant):
            return existing
    parent = unreal.EditorAssetLibrary.load_asset(FIXTURE_PARENT)
    require(isinstance(parent, unreal.Material), "parent material missing")
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset(name, MATERIAL_DIR,
                             unreal.MaterialInstanceConstant,
                             unreal.MaterialInstanceConstantFactoryNew())
    require(mat is not None, "could not create " + path)
    unreal.MaterialEditingLibrary.set_material_instance_parent(mat, parent)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        mat, "BaseColour", unreal.LinearColor(colour[0], colour[1], colour[2], 1.0))
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        mat, "Emissive", 0.0)
    unreal.MaterialEditingLibrary.update_material_instance(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    say("created material %s" % name)
    return mat


MATS = {name: ensure_material(name, colour)
        for name, colour in NEW_MATERIALS.items()}


def ensure_edge_material():
    name = "MI_LLK_Galleria_EdgeBand"
    path = MATERIAL_DIR + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = unreal.EditorAssetLibrary.load_asset(path)
        if isinstance(existing, unreal.MaterialInstanceConstant):
            return existing
    parent = unreal.EditorAssetLibrary.load_asset(FIXTURE_PARENT)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset(name, MATERIAL_DIR,
                             unreal.MaterialInstanceConstant,
                             unreal.MaterialInstanceConstantFactoryNew())
    require(mat is not None, "could not create " + path)
    unreal.MaterialEditingLibrary.set_material_instance_parent(mat, parent)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        mat, "EmissiveColour",
        unreal.LinearColor(EDGE_COLOUR[0], EDGE_COLOUR[1], EDGE_COLOUR[2], 1.0))
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        mat, "BaseColour", unreal.LinearColor(0.05, 0.07, 0.07, 1.0))
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        mat, "Emissive", EDGE_EMISSIVE)
    unreal.MaterialEditingLibrary.update_material_instance(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    say("created material %s" % name)
    return mat


MATS["edge"] = ensure_edge_material()
for name in ("M_BSW_StoneTile", "M_BSW_MallMullionTeal", "M_BSW_DarkChrome",
             "MI_LLK_Soffit_Structure", "MI_LLK_Fixture_Emergency"):
    asset = unreal.EditorAssetLibrary.load_asset(MATERIAL_DIR + "/" + name)
    require(isinstance(asset, unreal.MaterialInterface), "missing " + name)
    MATS[name] = asset

TILE = MATS["M_BSW_StoneTile"]
RAIL = MATS["M_BSW_MallMullionTeal"]
STRUCTURE = MATS["MI_LLK_Soffit_Structure"]
SHUTTER = MATS["MI_LLK_Galleria_Shutter"]
FASCIA = MATS["MI_LLK_Galleria_Fascia"]


# ---------------------------------------------------------------- placing

placed = []


def clear_previous():
    gone = 0
    for label, group in by_label.items():
        if label.startswith(PREFIX):
            for actor in group:
                if actors_sub.destroy_actor(actor):
                    gone += 1
    return gone


def mesh_of(actor):
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    require(comps, "%s has no static mesh component" % actor.get_actor_label())
    return comps[0]


def box(label, x0, x1, y0, y1, z0, z1, material, collision=True):
    require(x1 > x0 and y1 > y0 and z1 > z0, "degenerate box " + label)
    full = PREFIX + label
    actor = actors_sub.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector((x0 + x1) / 2.0, (y0 + y1) / 2.0, (z0 + z1) / 2.0),
        unreal.Rotator())
    require(actor is not None, "could not spawn " + full)
    actor.set_actor_label(full)
    comp = mesh_of(actor)
    comp.set_mobility(unreal.ComponentMobility.STATIC)
    if comp.static_mesh != cube:
        require(comp.set_static_mesh(cube), "mesh assignment failed for " + full)
    actor.set_actor_scale3d(unreal.Vector((x1 - x0) / 100.0, (y1 - y0) / 100.0,
                                          (z1 - z0) / 100.0))
    comp.set_material(0, material)
    if not collision:
        actor.set_actor_enable_collision(False)
        comp.set_collision_profile_name("NoCollision")
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    placed.append(full)
    return actor


gone = clear_previous()
say("cleared %d actor(s) from a previous run" % gone)


# ---------------------------------------------------------------- the decks

# A balcony each side, over the lower shop bays, with the middle left open so
# the two levels can see each other. That well is the whole reason a mall
# atrium exists and it is the one thing a stealth floor gets free from it.
for side, sign in (("W", -1.0), ("E", +1.0)):
    x_inner = sign * WELL_X
    x_outer = sign * OUTER_X
    box("DECK_" + side, min(x_inner, x_outer), max(x_inner, x_outer),
        CONCOURSE_Y0, CONCOURSE_Y1, DECK_Z - DECK_T, DECK_Z, TILE)
    # the rail along the well, on the deck's inner edge
    rail_a, rail_b = x_inner, x_inner - sign * 14.0
    box("RAIL_" + side, min(rail_a, rail_b), max(rail_a, rail_b),
        CONCOURSE_Y0, CONCOURSE_Y1, DECK_Z, DECK_Z + RAIL_H, RAIL)
    # the lit edge, on the face a keeper sees from the floor below
    band_a, band_b = x_inner + sign * 3.0, x_inner + sign * 9.0
    box("EDGE_BAND_" + side, min(band_a, band_b), max(band_a, band_b),
        CONCOURSE_Y0, CONCOURSE_Y1, DECK_Z - DECK_T - 10.0, DECK_Z - DECK_T + 8.0,
        MATS["edge"], collision=False)

# The crossing at the near end, which is what the store's upper doorway opens
# onto. Without it that doorway is glass over a 650 cm drop.
box("DECK_CROSSING", -OUTER_X, OUTER_X, CONCOURSE_Y1, door_o.y - 10.0,
    DECK_Z - DECK_T, DECK_Z, TILE)
box("RAIL_CROSSING_W", -WELL_X - 14.0, -WELL_X, CONCOURSE_Y1, door_o.y - 10.0,
    DECK_Z, DECK_Z + RAIL_H, RAIL)
box("RAIL_CROSSING_E", WELL_X, WELL_X + 14.0, CONCOURSE_Y1, door_o.y - 10.0,
    DECK_Z, DECK_Z + RAIL_H, RAIL)
# The well-head rail, in two pieces: the stair arrives through the middle of
# it and a rail across the top of a staircase is a wall.
box("RAIL_WELL_HEAD_W", -WELL_X, -STAIR_W / 2.0 - 20.0,
    CONCOURSE_Y1 - 14.0, CONCOURSE_Y1, DECK_Z, DECK_Z + RAIL_H, RAIL)
box("RAIL_WELL_HEAD_E", STAIR_W / 2.0 + 20.0, WELL_X,
    CONCOURSE_Y1 - 14.0, CONCOURSE_Y1, DECK_Z, DECK_Z + RAIL_H, RAIL)

# The far end is closed: the mall goes on past this, and telling the player it
# is shut is honest where pretending it continues is not.
box("FAR_BULKHEAD", -OUTER_X, OUTER_X, CONCOURSE_Y0 - 30.0, CONCOURSE_Y0,
    DECK_Z, ROOF_Z, STRUCTURE)


# ------------------------------------------------- the hole over the shops

# The outer walls stop at 1200 and the roof is at 1490. That 290 cm gap was
# invisible while the sky was black; it is a strip of open sky now.
for side, sign in (("W", -1.0), ("E", +1.0)):
    x = sign * OUTER_X
    box("OUTER_INFILL_" + side, min(x - 14.0, x + 14.0), max(x - 14.0, x + 14.0),
        CONCOURSE_Y0, CONCOURSE_Y1, OUTER_WALL_TOP, ROOF_Z, STRUCTURE)


# ---------------------------------------------------------------- the shops

# Upper tenancies: frontage only, shuttered. An empty upper level with open
# units would be a lie about what is behind them.
unit = 0
for side, sign in (("W", -1.0), ("E", +1.0)):
    x_face = sign * (OUTER_X - 380.0)
    y = CONCOURSE_Y0 + 120.0
    while y + BAY_LENGTH <= CONCOURSE_Y1 - 120.0:
        box("SHOP_%s_%d_SHUTTER" % (side, unit),
            min(x_face, x_face + sign * 16.0), max(x_face, x_face + sign * 16.0),
            y + 60.0, y + BAY_LENGTH - 60.0, DECK_Z, DECK_Z + SHUTTER_H,
            SHUTTER)
        box("SHOP_%s_%d_FASCIA" % (side, unit),
            min(x_face - sign * 10.0, x_face + sign * 20.0),
            max(x_face - sign * 10.0, x_face + sign * 20.0),
            y + 40.0, y + BAY_LENGTH - 40.0,
            DECK_Z + SHUTTER_H, DECK_Z + SHUTTER_H + FASCIA_H, FASCIA)
        box("SHOP_%s_%d_PIER" % (side, unit),
            min(x_face, x_face + sign * 40.0), max(x_face, x_face + sign * 40.0),
            y, y + 60.0, DECK_Z, DECK_Z + SHUTTER_H + FASCIA_H, STRUCTURE)
        unit += 1
        y += BAY_LENGTH
say("%d shuttered upper tenancies" % unit)


# ---------------------------------------------------------------- the stair

# One way up from the lower concourse, so the upper level is reachable
# without going through the store.
#
# It rises NORTH onto the crossing deck, not up the middle of the well. The
# first version put it in the centre of the concourse rising southward, where
# its top tread arrived at 650 cm over the open well with no floor anywhere
# near it - a staircase to nothing, which nothing in this file would have
# caught because every assert was about the balcony and none about the stair.
steps = int(round(DECK_Z / STAIR_RISER))
stair_top_y = CONCOURSE_Y1
stair_y = stair_top_y - steps * STAIR_TREAD
for index in range(steps):
    z = index * STAIR_RISER
    box("STAIR_TREAD_%02d" % index,
        -STAIR_W / 2.0, STAIR_W / 2.0,
        stair_y + index * STAIR_TREAD, stair_y + (index + 1) * STAIR_TREAD + 4.0,
        max(z - 10.0, 0.0), z + STAIR_RISER, TILE)
# The rail follows the stair. A single box from the bottom step to the top at
# one height is not a handrail - it is a beam, and in the first render of this
# court it read as two diagonals crossing the mall entrance's glass.
for index in range(steps):
    z = index * STAIR_RISER
    for side, sign in (("W", -1.0), ("E", +1.0)):
        a, b = sign * (STAIR_W / 2.0), sign * (STAIR_W / 2.0 + 12.0)
        box("STAIR_RAIL_%s_%02d" % (side, index), min(a, b), max(a, b),
            stair_y + index * STAIR_TREAD,
            stair_y + (index + 1) * STAIR_TREAD + 4.0,
            z + 24.0, z + 24.0 + RAIL_H, RAIL)
say("%d stair treads rising %.0f cm to the balcony" % (steps, DECK_Z))


# ---------------------------------------------------------------- the light

# On the emergency circuit, tagged, at this project's own emergency band of
# 1.3 to 2.8 cd. An untagged light burns in both states, which is the defect
# docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md is entirely about.
lights = 0
y = CONCOURSE_Y0 + 700.0
while y < CONCOURSE_Y1:
    for sign in (-1.0, +1.0):
        label = PREFIX + "EGRESS_LIGHT_%s_%d" % ("W" if sign < 0 else "E", lights)
        light = actors_sub.spawn_actor_from_class(
            unreal.PointLight,
            unreal.Vector(sign * (OUTER_X - 700.0), y, DECK_Z + 300.0),
            unreal.Rotator())
        require(light is not None, "could not spawn " + label)
        light.set_actor_label(label)
        light.set_editor_property("tags", [EMERGENCY_TAG])
        comp = light.get_component_by_class(unreal.PointLightComponent)
        comp.set_editor_property("intensity", BALCONY_LIGHT_CD)
        comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
        comp.set_editor_property("attenuation_radius", 1100.0)
        comp.set_editor_property("light_color", unreal.Color(158, 189, 179, 255))
        comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
        placed.append(label)
        lights += 1
    y += 950.0

# And over the stair, which came back from its first render as a black wedge.
# The one route between the two levels being the darkest thing in the court is
# the breaker-in-an-unlit-corridor mistake again.
for index, frac in enumerate((0.25, 0.75)):
    label = PREFIX + "STAIR_LIGHT_%d" % index
    light = actors_sub.spawn_actor_from_class(
        unreal.PointLight,
        unreal.Vector(0.0, stair_y + (stair_top_y - stair_y) * frac,
                      # just above the handrail at that point on the run, not
                      # above the whole flight: the first pair hung at the top
                      # and lit nothing but the rail caps
                      DECK_Z * frac + 210.0),
        unreal.Rotator())
    require(light is not None, "could not spawn " + label)
    light.set_actor_label(label)
    light.set_editor_property("tags", [EMERGENCY_TAG])
    comp = light.get_component_by_class(unreal.PointLightComponent)
    comp.set_editor_property("intensity", BALCONY_LIGHT_CD)
    comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    comp.set_editor_property("attenuation_radius", 1200.0)
    comp.set_editor_property("light_color", unreal.Color(158, 189, 179, 255))
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    placed.append(label)
    lights += 1

say("%d balcony and stair egress lights, all on %s" % (lights, EMERGENCY_TAG))


# ------------------------------------------------- the roof that was not there

# A visibility trace straight up the middle of the concourse reported OPEN SKY
# for its whole 48 m, because the roof glass carries no collision. It renders
# and it occludes, but nothing can measure it - the same defect the kids
# ceiling had in docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md, where a ceiling
# nobody can measure is a ceiling nobody can prove is there.
roofed = 0
for label, group in by_label.items():
    if "GALLERIA_NEAR_ROOF" not in label:
        continue
    for actor in group:
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            if comp.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION:
                comp.set_collision_profile_name("BlockAll")
                comp.set_collision_enabled(
                    unreal.CollisionEnabled.QUERY_AND_PHYSICS)
                roofed += 1
say("gave collision to %d roof piece(s) that had none" % roofed)


# ---------------------------------------------------------------- checks

after = {}
for actor in actors_sub.get_all_level_actors():
    after.setdefault(actor.get_actor_label(), []).append(actor)
for label in placed:
    require(len(after.get(label, [])) == 1,
            "%s is not in the level exactly once after the run" % label)

# The whole point: the store's upper doorway must now open onto a floor.
probe_x = door_o.x
probe_y = door_o.y - 120.0
down = unreal.SystemLibrary.line_trace_single(
    world, unreal.Vector(probe_x, probe_y, DECK_Z + 150.0),
    unreal.Vector(probe_x, probe_y, DECK_Z - 400.0),
    unreal.TraceTypeQuery.ECC_VISIBILITY, False, [], unreal.DrawDebugTrace.NONE,
    True)
require(down is not None,
        "there is still nothing under the store's upper mall entrance")
landing = down.to_tuple()[9].get_actor_label()
require(landing.startswith(PREFIX),
        "the upper mall entrance now stands over %s rather than this balcony"
        % landing)
say("the upper mall entrance opens onto %s" % landing)

# A staircase has to arrive somewhere. This is the check the first version did
# not have, and it would have caught a stair topping out over the open well.
arrival = unreal.SystemLibrary.line_trace_single(
    world, unreal.Vector(0.0, stair_top_y + 90.0, DECK_Z + 140.0),
    unreal.Vector(0.0, stair_top_y + 90.0, DECK_Z - 300.0),
    unreal.TraceTypeQuery.ECC_VISIBILITY, False, [], unreal.DrawDebugTrace.NONE,
    True)
require(arrival is not None,
        "the stair tops out at z %.0f with nothing under the landing" % DECK_Z)
arrival_label = arrival.to_tuple()[9].get_actor_label()
require(arrival_label.startswith(PREFIX),
        "the stair lands on %s rather than this level" % arrival_label)
say("the stair lands on %s" % arrival_label)

# And the balcony must have headroom a keeper can stand in.
up = unreal.SystemLibrary.line_trace_single(
    world, unreal.Vector(-OUTER_X + 500.0, (CONCOURSE_Y0 + CONCOURSE_Y1) / 2.0,
                         DECK_Z + 30.0),
    unreal.Vector(-OUTER_X + 500.0, (CONCOURSE_Y0 + CONCOURSE_Y1) / 2.0,
                  DECK_Z + 900.0),
    unreal.TraceTypeQuery.ECC_VISIBILITY, False, [], unreal.DrawDebugTrace.NONE,
    True)
head = (up.to_tuple()[4].z - DECK_Z) if up else 900.0
require(head > 200.0,
        "the west balcony has %.0f cm of headroom, which a keeper cannot "
        "stand in" % head)
say("balcony headroom %.0f cm" % head)

say("placed %d actor(s)" % len(placed))
saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))
