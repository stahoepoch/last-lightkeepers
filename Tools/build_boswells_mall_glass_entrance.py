"""Open Boswell's south header to a projecting, sealed mall glass entrance.

Run with Tools/run_editor_python.ps1 while Unreal Editor is closed. This is
the mall face only: the night grille, upper slab, roof, canopy, dock, exposure,
and authored emergency fixtures are left in place. The glass is structural
and colliding; the pass adds no LightComponent or gameplay-light source.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
ROOT = "/Game/LLK/Boswells/Architecture"
PREFIX = "MALL_GLASS_"
CUBE = "/Engine/BasicShapes/Cube.Cube"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_MALL_GLASS " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require(not any(n.startswith(PREFIX) or n == "ROUTE_SOUTH_LINTEL"
                for n in before), "entrance pass already exists")
for name in ("ROUTE_SOUTH_HEADER", "ROUTE_SOUTH_WALL_W",
             "ROUTE_SOUTH_WALL_E", "ROUTE_MALL_GRILLE_22",
             "ANCHOR_UPPER_SOUTH", "ANCHOR_FULL_ROOF",
             "BSW_CANOPY_GLASS_02_02"):
    require(name in before, "required shell actor absent: " + name)


def bounds(actor):
    center, extent = actor.get_actor_bounds(False)
    return (center.x - extent.x, center.x + extent.x,
            center.y - extent.y, center.y + extent.y,
            center.z - extent.z, center.z + extent.z)


header_bounds = bounds(before["ROUTE_SOUTH_HEADER"])
require(all(abs(a - b) < 1 for a, b in zip(
    header_bounds, (-900, 900, -2720, -2680, 280, 1500))),
    "south header bounds changed; review before cutting")
require(sum(n.startswith("BSW_CANOPY_GLASS_") for n in before) == 24,
        "interior mirror canopy changed")
require(sum(n.startswith("BSW_EGRESS_") for n in before) == 31 and
        sum(n.startswith("BSW_KIDS_CEILING_FILL_") for n in before) == 6,
        "emergency fixture inventory changed")

mesh = unreal.EditorAssetLibrary.load_asset(CUBE)
require(isinstance(mesh, unreal.StaticMesh), "engine cube missing")


def material(name, base, metallic, roughness, opacity=None, emissive=None):
    path = ROOT + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = unreal.EditorAssetLibrary.load_asset(path)
        require(isinstance(existing, unreal.Material), "material path occupied: " + path)
        return existing
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, ROOT, unreal.Material, unreal.MaterialFactoryNew())
    require(isinstance(asset, unreal.Material), "material creation failed: " + name)
    if opacity is not None:
        asset.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        asset.set_editor_property("two_sided", True)
    edit = unreal.MaterialEditingLibrary
    for prop, value, row in (
            (unreal.MaterialProperty.MP_BASE_COLOR, base, -200),
            (unreal.MaterialProperty.MP_EMISSIVE_COLOR, emissive, 200)):
        if value is None:
            continue
        color = edit.create_material_expression(
            asset, unreal.MaterialExpressionConstant3Vector, -430, row)
        color.set_editor_property("constant", unreal.LinearColor(*value, 1))
        edit.connect_material_property(color, "", prop)
    for prop, value, row in (
            (unreal.MaterialProperty.MP_METALLIC, metallic, -40),
            (unreal.MaterialProperty.MP_ROUGHNESS, roughness, 40),
            (unreal.MaterialProperty.MP_OPACITY, opacity, 300)):
        if value is None:
            continue
        scalar = edit.create_material_expression(
            asset, unreal.MaterialExpressionConstant, -430, row)
        scalar.set_editor_property("r", value)
        edit.connect_material_property(scalar, "", prop)
    edit.recompile_material(asset)
    return asset


glass = material("M_BSW_MallEntranceGlass", (.34, .56, .60), 0, .06,
                 opacity=.22, emissive=(.012, .027, .030))
teal = material("M_BSW_MallMullionTeal", (.045, .15, .17), .64, .27)


def block(name, xmin, xmax, ymin, ymax, bottom, top, mat):
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector((xmin + xmax) / 2, (ymin + ymax) / 2,
                      (bottom + top) / 2), unreal.Rotator(0, 0, 0))
    require(actor is not None, "could not spawn: " + name)
    actor.set_actor_label(name)
    actor.set_folder_path("Boswells/Architecture/MallGlassEntrance")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(mesh), "cube assignment failed: " + name)
    actor.set_actor_scale3d(unreal.Vector((xmax - xmin) / 100,
                                          (ymax - ymin) / 100,
                                          (top - bottom) / 100))
    comp.set_material(0, mat)
    actor.set_actor_enable_collision(True)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    return actor


# Keep the original perimeter's roof joint. Its old solid header is the only
# piece removed; below it the existing grille remains the night door.
require(subsystem.destroy_actor(before["ROUTE_SOUTH_HEADER"]),
        "could not remove sealed south header")
block("ROUTE_SOUTH_LINTEL", -900, 900, -2720, -2680, 1468, 1500, teal)

# The front pane is a thin wall at the projecting face, not a two-metre-thick
# glass solid. Side returns span the 160 cm projection; together they form
# the stated entrance bay bounds without turning the canopy into a skylight.
block("MALL_GLASS_ENTRANCE", -900, 900, -2880, -2876,
      280, 1468, glass)
block("MALL_GLASS_RETURN_W", -900, -896, -2880, -2680,
      280, 1468, glass)
block("MALL_GLASS_RETURN_E", 896, 900, -2880, -2680,
      280, 1468, glass)

# Retail curtain-wall rhythm, teal as in the supplied exterior atrium photo.
# No central vertical mullion or horizontal at 1120/1280/1400: the measured
# sightline traces must strike glass first, not metal.
for x in (-900, -600, -300, 300, 600, 900):
    block("MALL_GLASS_MULLION_V_{}".format(x), x - 5, x + 5,
          -2884, -2872, 280, 1468, teal)
for z in (280, 660, 1000, 1468):
    block("MALL_GLASS_MULLION_H_{}".format(z), -900, 900,
          -2884, -2872, z - 5, z + 5, teal)
for side, xmin, xmax in (("W", -905, -895), ("E", 895, 905)):
    block("MALL_GLASS_RETURN_RAIL_" + side, xmin, xmax,
          -2880, -2680, 1458, 1468, teal)

after = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require("ROUTE_SOUTH_HEADER" not in after and
        "ROUTE_SOUTH_LINTEL" in after and
        "ROUTE_MALL_GRILLE_22" in after, "entrance shell incomplete")
require(sum(n.startswith(PREFIX) for n in after) == 15,
        "entrance actor inventory differs")
for asset in (glass, teal):
    require(unreal.EditorAssetLibrary.save_loaded_asset(asset),
            "material save failed: " + asset.get_name())
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
message = "BOSWELLS_MALL_GLASS SAVED: projecting glazed entrance, grille and roof retained"
unreal.log(message)
print(message)
