"""Make every light fitting show the state of its own light.

2026-11-23, found by rendering the upper storey the moment it had a ceiling.

`Tools/ceiling_boswells_upper_storey.py` brought the mains and emergency grid
down from the roof at 1462 to 26 cm under a new ceiling at 1050, and the first
shot of the result shows a department store **with the lights on**: rows of
white tubes receding to the far wall, every garment colour readable at thirty
metres. The breaker is off. Not one of those lights is lit.

They are the *housings*. A fitting's glow is a material parameter, not a
light, and it had come loose from the circuit in two separate ways:

**1. The housings still tell yesterday's story.** `MI_LLK_Fixture_Emergency`,
`_Mains` and `_Dead` exist and are correct; what picks between them is which
circuit the light was on *when the fitting was placed*. Yesterday's
`repair_boswells_emergency_state.py` moved 110 lights between circuits and
touched no housing, so 13 `LLK_DOCK_CHAPTER_WING_POOL` fittings and six
`BSW_DEPT_ROUTE_POOL_*` fittings glow with the emergency material over lights
that are now mains and dark.

**2. A mains housing is authored lit.** `ULLKPowerSubsystem::ApplyToFixtures`
is right - it makes a dynamic instance per housing and sets `Emissive` to 0
when the power is off - but that runs in PIE, and **every picture either of us
sees of this game is a render taken without PIE**. The saved map has to agree
with the boot state, which is the same argument that authored the mains lights
invisible yesterday. So `MI_LLK_Fixture_Mains` is authored at 0 here.

That is an edit to a shared asset, and the subsystem's own comment warns
against touching one at runtime because it would change every fixture in every
level at once. Authoring it is the opposite case and the change is the
intended one: the header's first line is *"Emergency is the default and always
will be"*, so a mains fitting that has never been switched on should be dark
everywhere, and the dynamic instance still lights it the moment the breaker
goes on.

## The rule

**A fitting wears the circuit of its own light, and is tagged to match.**

- light on `LLK_Mains`      -> `MI_LLK_Fixture_Mains`, tagged `LLK_MainsFixture`
- light on `LLK_Emergency`, with output -> `MI_LLK_Fixture_Emergency`
- anything else, or no output at all    -> `MI_LLK_Fixture_Dead`

A housing with no light and a light with no housing are both reported rather
than guessed at, because a fitting that is not attached to anything is how
this floor ended up with fifty lights nobody chose.

Idempotent. Saves the map and the material, so it needs `-AllowUnsafeMapSave`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"

MAINS_TAG = "LLK_Mains"
EMERGENCY_TAG = "LLK_Emergency"
MAINS_FIXTURE_TAG = "LLK_MainsFixture"

MATS = {
    "mains": MATERIAL_DIR + "/MI_LLK_Fixture_Mains",
    "emergency": MATERIAL_DIR + "/MI_LLK_Fixture_Emergency",
    "dead": MATERIAL_DIR + "/MI_LLK_Fixture_Dead",
}

# Authored dark, for the reason in the docstring. The runtime still raises it
# to ULLKPowerSubsystem::MainsFixtureEmissive (6) when the breaker goes on.
MAINS_AUTHORED_EMISSIVE = 0.0


def say(message):
    unreal.log("FITTING " + message)
    print("FITTING " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("FITTING " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

materials = {}
for key, path in MATS.items():
    asset = unreal.EditorAssetLibrary.load_asset(path)
    require(asset is not None, "fixture material missing: " + path)
    was = unreal.MaterialEditingLibrary \
        .get_material_instance_scalar_parameter_value(asset, "Emissive")
    say("%-10s %s  Emissive=%.2f" % (key, path.split("/")[-1], was))
    materials[key] = asset


# =====================================================================
# 1. Author the mains fitting dark
# =====================================================================

was = unreal.MaterialEditingLibrary \
    .get_material_instance_scalar_parameter_value(materials["mains"],
                                                  "Emissive")
if abs(was - MAINS_AUTHORED_EMISSIVE) > 0.001:
    unreal.MaterialEditingLibrary \
        .set_material_instance_scalar_parameter_value(
            materials["mains"], "Emissive", MAINS_AUTHORED_EMISSIVE)
    unreal.EditorAssetLibrary.save_asset(MATS["mains"])
    say("MI_LLK_Fixture_Mains authored dark: Emissive %.2f -> %.2f"
        % (was, MAINS_AUTHORED_EMISSIVE))
else:
    say("MI_LLK_Fixture_Mains already authored dark")


# =====================================================================
# 2. Pair each light with its housing
# =====================================================================

by_label = {}
for actor in actors:
    by_label.setdefault(actor.get_actor_label(), []).append(actor)

housings = {}
for actor in actors:
    label = actor.get_actor_label().upper()
    if "FIXTURE" in label or label.endswith("_HOUSING"):
        housings[actor.get_actor_label()] = actor

paired = {}
lights_without = []
for actor in actors:
    if not isinstance(actor, unreal.Light):
        continue
    label = actor.get_actor_label()
    found = []
    for candidate in ("BSW_FIXTURE_" + label, label + "_Housing"):
        found.extend(by_label.get(candidate, []))
    if not found:
        lights_without.append(label)
        continue
    for mesh in found:
        paired[mesh.get_actor_label()] = actor

say("lights: %d   fitting-shaped actors: %d   paired: %d"
    % (sum(1 for a in actors if isinstance(a, unreal.Light)),
       len(housings), len(paired)))
say("lights with no housing: %d  %s"
    % (len(lights_without), sorted(lights_without)[:6]))
orphans = sorted(set(housings) - set(paired))
say("housings with no light: %d  %s" % (len(orphans), orphans[:6]))


# =====================================================================
# 3. Dress each housing in its own light's circuit
# =====================================================================

def wanted_for(light):
    comp = light.light_component
    tags = [str(t) for t in light.tags]
    lit = (comp is not None
           and comp.get_editor_property("intensity") > 0.0
           and comp.get_editor_property("visible"))
    if MAINS_TAG in tags:
        return "mains"
    if EMERGENCY_TAG in tags and lit:
        return "emergency"
    return "dead"


counts = {"mains": 0, "emergency": 0, "dead": 0}
repainted = 0
retagged = 0
for mesh_label, light in sorted(paired.items()):
    mesh = housings.get(mesh_label)
    if mesh is None:
        continue
    key = wanted_for(light)
    counts[key] += 1
    material = materials[key]
    for comp in mesh.get_components_by_class(unreal.MeshComponent):
        for slot, current in enumerate(comp.get_materials()):
            if current is not None and current.get_name() == material.get_name():
                continue
            comp.set_material(slot, material)
            repainted += 1

    # The tag follows the material, or ApplyToFixtures reaches a housing the
    # map says is an emergency head and lights it with the mains.
    tags = [str(t) for t in mesh.tags]
    keep = [t for t in tags if t != MAINS_FIXTURE_TAG]
    if key == "mains":
        keep.append(MAINS_FIXTURE_TAG)
    if keep != tags:
        mesh.set_editor_property("tags", [unreal.Name(t) for t in keep])
        retagged += 1

say("housings dressed: %d mains, %d emergency, %d dead"
    % (counts["mains"], counts["emergency"], counts["dead"]))
say("material slots repainted: %d   LLK_MainsFixture tags corrected: %d"
    % (repainted, retagged))

# The placeholder check the ceiling pass already applies overhead: a light
# fitting wearing `BasicShapeMaterial` is not a fitting, it is an untextured
# box, and twenty `ANCHOR_EMERGENCY_*_Housing` wore one.
placeholder = 0
for label, mesh in sorted(housings.items()):
    for comp in mesh.get_components_by_class(unreal.MeshComponent):
        for slot, current in enumerate(comp.get_materials()):
            name = current.get_name() if current else "<none>"
            if name in ("BasicShapeMaterial", "WorldGridMaterial",
                        "DefaultMaterial"):
                comp.set_material(slot, materials["dead"])
                placeholder += 1
say("fittings still on an engine placeholder material, now dead: %d"
    % placeholder)


# =====================================================================
# 4. The end state, asserted
# =====================================================================

wrong = []
for mesh_label, light in sorted(paired.items()):
    mesh = housings.get(mesh_label)
    if mesh is None:
        continue
    key = wanted_for(light)
    tags = [str(t) for t in mesh.tags]
    if (key == "mains") != (MAINS_FIXTURE_TAG in tags):
        wrong.append((mesh_label, key, tags))
for label, key, tags in wrong[:10]:
    say("  WRONG %-46s wants %s, tags %s" % (label, key, tags))
require(not wrong, "%d housing(s) disagree with their own light's circuit"
        % len(wrong))

saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))
