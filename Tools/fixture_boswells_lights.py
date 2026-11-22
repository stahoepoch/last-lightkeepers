"""Give every light in Boswell's something it is visibly coming out of.

Chris, 2026-11-22:

    "It kind of looks like lights are coming from nowhere. Is there an
    emergency light that actually gives off light, or is it just a light that
    comes randomly out of the ceiling? It has to be believable, like a fixture
    or something, to show that it is coming from where it's coming from.
    Otherwise, we're just making stuff up and it's not going to be real."

**He is right, and `Tools/audit_boswells_floor_and_fixtures.py` counted it:
104 of 174 lights in the map have no static mesh within 260 cm of them.** 46
of those are the emergency and mains fills added earlier the same day, which
are bare `APointLight` actors hanging in mid-air. The rest are egress lights,
dock lights and route pools inherited from earlier passes. Only the original
`ANCHOR_EMERGENCY` spots were done properly - 19 of 20 have housings, which is
why the ceiling in the first screenshots reads correctly and nothing else
does.

A light with no source is the same class of defect this project keeps finding
in its code: **a correct system over an absent thing.** The illumination is
right, the thing it is supposed to come from does not exist, and in a still
it looks like a rendering error rather than a building.

## What it places

One troffer per naked light, at the light's own position, so the geometry and
the illumination cannot disagree about where the source is. Two materials:

- **Emergency** fixtures glow faintly and always. They are the lights that
  stayed on when the building died, and seeing which ones they are is how a
  player learns the difference between the building's own light and a way out.
- **Mains** fixtures are dark housings that light up when the breaker is
  thrown. They are tagged `LLK_MainsFixture` and `ULLKPowerSubsystem` drives
  their emissive, because a fixture that glows while its light is off is worse
  than no fixture at all - it tells the player the power is on and the room
  disagrees.

Idempotent: everything it creates is `BSW_FIXTURE_*` and is removed first.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_FIXTURE_"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"
MASTER_NAME = "M_LLK_LightFixture"

MAINS_TAG = "LLK_Mains"
MAINS_FIXTURE_TAG = "LLK_MainsFixture"

NEAR_CM = 260.0

# Half-span above which a mesh is architecture rather than a light fitting.
# 3 m: a ceiling field, a floor slab, a wall. A troffer is 0.6 m.
BIG_SURFACE_CM = 300.0

# A ceiling troffer: 120 x 30 x 8 cm, from the engine's 100 cm cube.
TROFFER_SCALE = unreal.Vector(1.2, 0.3, 0.08)
# Sat just above the light, so the illumination reads as leaving its underside.
FIXTURE_RISE_CM = 9.0

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("FIXTURE map unavailable")

subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

# ---- the master, with the parameters the runtime drives -----------------
master_path = "%s/%s" % (MATERIAL_DIR, MASTER_NAME)
master = unreal.EditorAssetLibrary.load_asset(master_path)
if master is None:
    master = tools.create_asset(MASTER_NAME, MATERIAL_DIR,
                                unreal.Material, unreal.MaterialFactoryNew())

    base = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionVectorParameter, -500, 0)
    base.set_editor_property("parameter_name", "BaseColour")
    base.set_editor_property("default_value", unreal.LinearColor(0.07, 0.07, 0.08, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(
        base, "", unreal.MaterialProperty.MP_BASE_COLOR)

    colour = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionVectorParameter, -700, 260)
    colour.set_editor_property("parameter_name", "EmissiveColour")
    colour.set_editor_property("default_value", unreal.LinearColor(1.0, 0.96, 0.88, 1.0))

    amount = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionScalarParameter, -700, 420)
    amount.set_editor_property("parameter_name", "Emissive")
    amount.set_editor_property("default_value", 0.0)

    # Colour x amount, so the runtime moves one scalar to switch a fixture on.
    mult = unreal.MaterialEditingLibrary.create_material_expression(
        master, unreal.MaterialExpressionMultiply, -420, 300)
    unreal.MaterialEditingLibrary.connect_material_expressions(colour, "", mult, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(amount, "", mult, "B")
    unreal.MaterialEditingLibrary.connect_material_property(
        mult, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    unreal.MaterialEditingLibrary.recompile_material(master)
    unreal.EditorAssetLibrary.save_asset(master_path)
    print("FIXTURE created_master=%s" % master_path)
else:
    print("FIXTURE master_exists=%s" % master_path)


def instance(name, emissive, colour):
    path = "%s/%s" % (MATERIAL_DIR, name)
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing is not None:
        return existing
    inst = tools.create_asset(name, MATERIAL_DIR,
                              unreal.MaterialInstanceConstant,
                              unreal.MaterialInstanceConstantFactoryNew())
    unreal.MaterialEditingLibrary.set_material_instance_parent(inst, master)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        inst, "Emissive", emissive)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        inst, "EmissiveColour", colour)
    unreal.EditorAssetLibrary.save_asset(path)
    print("FIXTURE created_instance=%s emissive=%.1f" % (name, emissive))
    return inst


# Emergency glows faintly and always; mains start dark and the breaker lights
# them. The default on the mains instance is 0 for exactly that reason.
emergency_mat = instance("MI_LLK_Fixture_Emergency", 2.5,
                         unreal.LinearColor(0.72, 0.84, 1.0, 1.0))
mains_mat = instance("MI_LLK_Fixture_Mains", 0.0,
                     unreal.LinearColor(1.0, 0.97, 0.90, 1.0))

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
if cube is None:
    raise RuntimeError("FIXTURE /Engine/BasicShapes/Cube missing")

actors = subsystem.get_all_level_actors()

removed = 0
for actor in actors:
    if actor.get_actor_label().startswith(PREFIX):
        subsystem.destroy_actor(actor)
        removed += 1
print("FIXTURE removed=%d" % removed)

actors = subsystem.get_all_level_actors()
meshes = []
for actor in actors:
    if actor.get_actor_label().startswith(PREFIX):
        continue
    # **A ceiling is not a fixture.** Once the lights hang 26 cm under the
    # ceiling they belong to, every one of them has a very large mesh very
    # close by, and a nearness test that counts it says every light in the
    # map is fitted while none of them has a fitting. A troffer is a small
    # object; anything spanning more than this is the room, not the lamp.
    centre, extent = actor.get_actor_bounds(False)
    if extent.x > BIG_SURFACE_CM or extent.y > BIG_SURFACE_CM:
        continue
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        if comp.static_mesh is not None:
            meshes.append(actor.get_actor_location())
            break

placed_emergency = placed_mains = already = 0
for actor in actors:
    if not isinstance(actor, (unreal.SpotLight, unreal.PointLight, unreal.RectLight)):
        continue

    here = actor.get_actor_location()
    if any((here - where).length() <= NEAR_CM for where in meshes):
        already += 1
        continue

    is_mains = actor.actor_has_tag(MAINS_TAG)
    fixture = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(here.x, here.y, here.z + FIXTURE_RISE_CM))
    if fixture is None:
        raise RuntimeError("FIXTURE could not spawn a housing")

    fixture.set_actor_label("%s%s" % (PREFIX, actor.get_actor_label()))
    comp = fixture.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(cube)
    fixture.set_actor_scale3d(TROFFER_SCALE)
    comp.set_material(0, mains_mat if is_mains else emergency_mat)
    # A housing is seen, never walked into: the ceiling above it already
    # carries collision, and a collider here would be a thing the navmesh
    # and any future thrown object would have to care about for no reason.
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)

    if is_mains:
        fixture.set_editor_property("tags", [MAINS_FIXTURE_TAG])
        placed_mains += 1
    else:
        placed_emergency += 1

print("FIXTURE placed_emergency=%d placed_mains=%d already_had_one=%d" % (
    placed_emergency, placed_mains, already))

unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
print("FIXTURE saved=True")
