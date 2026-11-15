"""Give the children's department its own photographed ceiling height and fill."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_KIDS_CEILING_"


def material(name, color, roughness):
    path = "/Game/LLK/Boswells/Fixtures/" + name
    existing = (unreal.EditorAssetLibrary.load_asset(path)
                if unreal.EditorAssetLibrary.does_asset_exist(path) else None)
    if existing:
        if not isinstance(existing, unreal.Material):
            raise RuntimeError("ceiling material path has wrong type: " + path)
        return existing
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, "/Game/LLK/Boswells/Fixtures", unreal.Material,
        unreal.MaterialFactoryNew())
    if not isinstance(asset, unreal.Material):
        raise RuntimeError("ceiling material creation failed: " + path)
    edit = unreal.MaterialEditingLibrary
    base = edit.create_material_expression(
        asset, unreal.MaterialExpressionConstant3Vector, -320, -80)
    base.set_editor_property("constant", unreal.LinearColor(*color, 1.0))
    edit.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = edit.create_material_expression(
        asset, unreal.MaterialExpressionConstant, -320, 90)
    rough.set_editor_property("r", roughness)
    edit.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(asset)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset):
        raise RuntimeError("ceiling material save failed: " + path)
    return asset


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
if any(name.startswith(PREFIX) for name in before):
    raise RuntimeError("ceiling already exists; refusing duplicate")
if "BSW_KIDS_SALES_DESK" not in before:
    raise RuntimeError("children's department absent")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
cream = material("ceiling_cream", (.58, .57, .53), .65)
grid = material("ceiling_grid", (.15, .16, .16), .76)
if not isinstance(cube, unreal.StaticMesh) or not cream or not grid:
    raise RuntimeError("ceiling material or cube source absent")

def panel(label, x, y, z, width, length, height, material):
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=0, yaw=0, roll=0))
    if not actor or not actor.static_mesh_component.set_static_mesh(cube):
        raise RuntimeError("ceiling panel failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Children/Ceiling")
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100,
                                          height / 100))
    actor.set_actor_enable_collision(False)
    comp = actor.static_mesh_component
    comp.set_material(0, material)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    return actor


# One clean underside and physical seams avoid the OBJ tile tessellation's
# discontinuous normals, which produced triangular bright and dark facets.
panel("TILE_FIELD", -2500, -1150, 1100, 2800, 1800, 3, cream)
for ix in range(1, 28):
    panel("GRID_LONG_{:02d}".format(ix), -3900 + ix * 100, -1150,
          1098, 2, 1800, 2, grid)
for iy in range(1, 18):
    panel("GRID_CROSS_{:02d}".format(iy), -2500, -2050 + iy * 100,
          1098, 2800, 2, 2, grid)

# The sales desk is built against a column, as in the supplied photo. This
# post ends in the new suspended ceiling and does not intrude on the route.
column = actors.spawn_actor_from_class(
    unreal.StaticMeshActor, unreal.Vector(-2550, -1970, 881.25),
    unreal.Rotator(pitch=0, yaw=0, roll=0))
if not column or not column.static_mesh_component.set_static_mesh(cube):
    raise RuntimeError("sales column failed")
column.set_actor_label(PREFIX + "SALES_COLUMN")
column.set_folder_path("Boswells/Children/Ceiling")
column.set_actor_scale3d(unreal.Vector(.44, .44, 4.375))
column.static_mesh_component.set_material(0, cream)

# The prior upper spots would illuminate the *top* of this lowered ceiling.
# Bring only the spots within its footprint below the underside.
moved = []
for actor in before.values():
    if not isinstance(actor, unreal.SpotLight):
        continue
    loc = actor.get_actor_location()
    if (-3900 < loc.x < -1100 and -2050 < loc.y < -250 and loc.z > 1100):
        actor.set_actor_location(unreal.Vector(loc.x, loc.y, 1050), False, False)
        moved.append(actor.get_actor_label())
if not 1 <= len(moved) <= 4:
    raise RuntimeError("upper light inventory differs: " + str(moved))

for index, x in enumerate((-3550, -2650, -1750)):
    for y in (-1750, -650):
        label = PREFIX + "FILL_{:02d}_{}".format(index, y)
        actor = actors.spawn_actor_from_class(
            unreal.PointLight, unreal.Vector(x, y, 930),
            unreal.Rotator(pitch=0, yaw=0, roll=0))
        if not actor:
            raise RuntimeError("light spawn failed: " + label)
        actor.set_actor_label(label)
        actor.set_folder_path("Boswells/Children/Ceiling")
        light = actor.point_light_component
        light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
        light.set_editor_property("intensity", 8.0)
        light.set_editor_property("attenuation_radius", 1050.0)
        light.set_editor_property("source_radius", 45.0)
        light.set_editor_property("indirect_lighting_intensity", .25)
        light.set_editor_property("cast_shadows", False)

if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP):
    raise RuntimeError("map save failed")
after = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
if sum(name.startswith(PREFIX) for name in after) != 52:
    raise RuntimeError("saved ceiling inventory differs")
message = "BOSWELLS_KIDS_CEILING SAVED: tiled field, column, six fill lights; moved " + str(moved)
unreal.log(message)
print(message)
