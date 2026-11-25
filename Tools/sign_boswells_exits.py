"""Exit signs, with the amber glow, lit on both circuits.

2026-11-24. Chris, having played the opening map:

> *"Emergency fittings are cool, but I don't have that like the exit signs and
> stuff with that orange glow that you normally see with emergency lights."*

He is right, and the store had **four** things with EXIT in the label: two
faces on the children's deck wearing a material called `burgundy`, a
wayfinding word block, and a dock sign. None of them glowed. The emergency
layer this project authored two days ago is the cold, slightly green white of
a battery pack - `rgb(158, 189, 179)`, and that was a deliberate decision
because orange reads as a table lamp. The orange Chris is describing is not
the pack. **It is the exit sign**, which in a real building is the one amber
thing in a cold-white room, and the store did not have any.

## Why an exit sign is not just a glowing box

**It is lit in both power states, and that is the whole point of it.** An exit
sign runs off its own battery, so it is the one fixture in the building that
does not care what the breaker is doing. In this project that means the sign's
light carries `LLK_Emergency` and the sign body must NOT carry
`LLK_MainsFixture` - the tag `ULLKPowerSubsystem::ApplyToFixtures` uses to
decide what goes dark. Get that wrong and the signs go out the moment the
keeper throws the breaker, which is the exact opposite of what they are for.

**It casts a pool.** A sign that glows and lights nothing is a sticker, and
there are already 44 overhead actors in this map on placeholder materials to
prove how that reads. Each sign gets a small amber point light at the
emergency layer's own intensity band - 1.3 to 2.8 cd - so the doorway beneath
it has a colour the rest of the floor does not. On a floor measured at 0.0005
peak brightness, three candela of amber is a landmark.

**It is wayfinding, and that is why it answers more than the complaint.**
Chris's other sentence was *"there's nothing to do"*, and part of nothing-to-do
is not knowing where anything is. An amber glow at a door, visible across a
dark selling floor, is the cheapest legible goal a horror floor can have.

## What could not be signed

Chris also said:

> *"there's supposed to be like a couple exits or entrances from the outside to
> Boscov's... Plus the whole exit for the loading dock... we don't even have
> like an exterior or anything."*

**The level has no exterior and no exterior doors at all.** This tool signs
the exits that exist - the mall entrances and the dock doors - and reports the
ones Chris named that are not in the map, rather than inventing a door to hang
a sign over. A sign above a wall is worse than no sign.

Idempotent. Creates one material instance, so it needs `-AllowUnsafeMapSave`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
CUBE = "/Engine/BasicShapes/Cube.Cube"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
FIXTURE_PARENT = MATERIAL_DIR + "/M_LLK_LightFixture"
SIGN_MATERIAL = MATERIAL_DIR + "/MI_LLK_Sign_Exit"

EMERGENCY_TAG = "LLK_Emergency"
MAINS_FIXTURE_TAG = "LLK_MainsFixture"

# The amber. Not the pack's colour: a running-man sign's own red-orange, which
# is the one warm thing in a cold-white emergency room.
SIGN_EMISSIVE_COLOUR = (1.00, 0.30, 0.07)
# 0.15, and the three renders that settled it are the point of this number.
#
# At **4.0** the face was a WHITE rectangle with an amber halo. At **1.8** it
# was still cream - dropping it by more than half changed almost nothing. At
# **0.35** it went amber-yellow, and at **0.12** it was unmistakably orange.
#
# So the scale is not off by a little. This floor peaks at 0.0005 measured
# brightness, auto-exposure opens all the way up for it, and anything emissive
# above roughly 0.2 clips to white before the viewer ever sees its colour.
# That is also why `MI_LLK_Fixture_Emergency` at **2.5** has read as a row of
# white tubes in every render for three days, and why the honest answer to
# "the fittings are the brightest thing in frame" was never to dim them a
# little. An emissive bright enough to blow out has no colour at all.
SIGN_EMISSIVE = 0.15
SIGN_BASE_COLOUR = (0.04, 0.04, 0.05)

# A sign box, in cm. Shallow, wide, and small enough to read as a sign rather
# than as a cabinet.
SIGN_W, SIGN_T, SIGN_H = 56.0, 8.0, 20.0
SIGN_GAP_CM = 3.0            # below the header it hangs from
FACE_OFFSET_CM = 14.0        # how far onto the store side of the opening

# The light under it. Inside the emergency layer's own 1.3-2.8 cd band, at the
# top of it, because a doorway is where a keeper needs to see.
SIGN_LIGHT_CD = 2.8
SIGN_LIGHT_RADIUS_CM = 420.0
SIGN_LIGHT_DROP_CM = 10.0

PREFIX = "BSW_EXIT_SIGN_"

# header or door actor -> (short name, which way is the store side in y)
# The sign hangs under each one, read from its own bounds rather than from a
# coordinate typed in here, because the route pass has moved these twice.
TARGETS = {
    "BSW_ROUTE_REPAIR_MALL_W_4800_HEADER": ("MALL_W_4800", +1.0),
    "BSW_ROUTE_REPAIR_MALL_E_4800_HEADER": ("MALL_E_4800", +1.0),
    "BSW_ROUTE_REPAIR_MALL_W_6350_HEADER": ("MALL_W_6350", +1.0),
    "BSW_ROUTE_REPAIR_MALL_E_6350_HEADER": ("MALL_E_6350", +1.0),
    "LLK_DOCK_CHAPTER_STAFF_DOOR_HEADER":  ("DOCK_STAFF", +1.0),
}

# Exits Chris named that this tool looked for and did not find. Reported, not
# invented. An exterior door cannot be signed before there is an exterior.
NAMED_BUT_ABSENT = (
    "two exterior entrances from the outside into Boswell's",
    "the loading dock's own exterior exit",
    "any exterior shell at all - the store has no outside",
)

# Signs that already exist and are wearing the wrong material.
REPAINT = ("BSW_KIDS_SIGN_EXIT_FACE_NORTH", "BSW_KIDS_SIGN_EXIT_FACE_SOUTH")


def say(message):
    unreal.log("EXIT " + message)
    print("EXIT " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("EXIT " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
actors_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actors_sub.get_all_level_actors()

cube = unreal.EditorAssetLibrary.load_asset(CUBE)
require(cube is not None, "no cube mesh")

by_label = {}
for actor in actors:
    by_label.setdefault(actor.get_actor_label(), []).append(actor)


# =====================================================================
# 1. The amber material
# =====================================================================

sign_mat = unreal.EditorAssetLibrary.load_asset(SIGN_MATERIAL)
if sign_mat is None:
    parent = unreal.EditorAssetLibrary.load_asset(FIXTURE_PARENT)
    require(parent is not None, "no %s to instance" % FIXTURE_PARENT)
    sign_mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "MI_LLK_Sign_Exit", MATERIAL_DIR,
        unreal.MaterialInstanceConstant,
        unreal.MaterialInstanceConstantFactoryNew())
    require(sign_mat is not None, "could not create MI_LLK_Sign_Exit")
    sign_mat.set_editor_property("parent", parent)
    say("created MI_LLK_Sign_Exit off M_LLK_LightFixture")
else:
    say("MI_LLK_Sign_Exit already exists")

unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
    sign_mat, "EmissiveColour", unreal.LinearColor(*SIGN_EMISSIVE_COLOUR, 1.0))
unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
    sign_mat, "BaseColour", unreal.LinearColor(*SIGN_BASE_COLOUR, 1.0))
unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
    sign_mat, "Emissive", SIGN_EMISSIVE)
# save_loaded_asset, not save_asset: on the run that CREATES the instance it
# is not in the asset registry yet, and save_asset looks it up by path.
unreal.EditorAssetLibrary.save_loaded_asset(sign_mat)
say("amber authored: emissive %.1f at rgb(%.2f %.2f %.2f)"
    % ((SIGN_EMISSIVE,) + SIGN_EMISSIVE_COLOUR))


# =====================================================================
# 2. A sign under every exit header that exists
# =====================================================================

def reuse_or_spawn(label, cls, location):
    existing = [a for a in by_label.get(label, []) if isinstance(a, cls)]
    for extra in existing[1:]:
        actors_sub.destroy_actor(extra)
    if existing:
        existing[0].set_actor_location(location, False, True)
        return existing[0], False
    actor = actors_sub.spawn_actor_from_class(cls, location)
    require(actor is not None, "spawn failed for " + label)
    actor.set_actor_label(label)
    return actor, True


signed = []
absent = []
for target, (short, store_side) in sorted(TARGETS.items()):
    found = by_label.get(target)
    if not found:
        absent.append(target)
        continue
    origin, extent = found[0].get_actor_bounds(False)

    # Hang it under the header, on the store side of the opening.
    sign_z = origin.z - extent.z - SIGN_GAP_CM - SIGN_H * 0.5
    sign_y = origin.y + store_side * FACE_OFFSET_CM
    label = PREFIX + short

    sign, is_new = reuse_or_spawn(
        label, unreal.StaticMeshActor, unreal.Vector(origin.x, sign_y, sign_z))
    comp = sign.static_mesh_component
    if comp.static_mesh != cube:
        comp.set_static_mesh(cube)
    comp.set_material(0, sign_mat)
    sign.set_actor_scale3d(unreal.Vector(SIGN_W / 100.0, SIGN_T / 100.0,
                                          SIGN_H / 100.0))
    comp.set_mobility(unreal.ComponentMobility.STATIC)
    comp.set_editor_property("can_ever_affect_navigation", False)

    # An exit sign is battery-backed, so it must not be on the mains fixture
    # tag. If it were, throwing the breaker would switch it, and the one thing
    # in the building that is supposed to be reliable would blink.
    sign.set_editor_property("tags", [])

    light_label = label + "_LIGHT"
    light, _ = reuse_or_spawn(
        light_label, unreal.PointLight,
        unreal.Vector(origin.x, sign_y, sign_z - SIGN_LIGHT_DROP_CM))
    lc = light.point_light_component
    lc.set_editor_property("intensity", SIGN_LIGHT_CD)
    lc.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    lc.set_editor_property("attenuation_radius", SIGN_LIGHT_RADIUS_CM)
    lc.set_editor_property("light_color",
                           unreal.Color(255, 92, 28, 255))
    lc.set_editor_property("cast_shadows", False)
    lc.set_editor_property("visible", True)
    light.set_editor_property("tags", [unreal.Name(EMERGENCY_TAG)])

    signed.append((label, origin.x, sign_y, sign_z))
    say("%-28s under %-38s (%7.0f %7.0f %6.0f)%s"
        % (short, target, origin.x, sign_y, sign_z,
           "" if not is_new else "  NEW"))

for target in absent:
    say("  no such header in the level, so nothing signed: " + target)


# =====================================================================
# 3. The signs that were already there, in the wrong material
# =====================================================================

repainted = 0
for label in REPAINT:
    for actor in by_label.get(label, []):
        for comp in actor.get_components_by_class(unreal.MeshComponent):
            for slot, current in enumerate(comp.get_materials()):
                name = current.get_name() if current else "<none>"
                if name == sign_mat.get_name():
                    continue
                say("  repainting %s slot %d: %s -> MI_LLK_Sign_Exit"
                    % (label, slot, name))
                comp.set_material(slot, sign_mat)
                repainted += 1
say("existing sign faces repainted: %d" % repainted)


# =====================================================================
# 4. The end state, asserted
# =====================================================================

require(signed, "no exit was signed, so this tool did nothing")

after = actors_sub.get_all_level_actors()
for actor in after:
    label = actor.get_actor_label()
    if not label.startswith(PREFIX):
        continue
    tags = [str(t) for t in actor.tags]
    if label.endswith("_LIGHT"):
        require(EMERGENCY_TAG in tags,
                "%s is not on the emergency circuit, so it would be an "
                "untagged light burning in both states - the exact defect "
                "docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md is about" % label)
        comp = actor.point_light_component
        require(comp.get_editor_property("intensity") > 0.0
                and comp.get_editor_property("visible"),
                "%s is dark, so the sign above it glows over nothing" % label)
    else:
        require(MAINS_FIXTURE_TAG not in tags,
                "%s carries %s, so throwing the breaker would switch an exit "
                "sign" % (label, MAINS_FIXTURE_TAG))

say("signed %d exit(s), each lit on the emergency circuit and on no other"
    % len(signed))
say("exits Chris named that are NOT in this level:")
for missing in NAMED_BUT_ABSENT:
    say("    " + missing)

saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))
