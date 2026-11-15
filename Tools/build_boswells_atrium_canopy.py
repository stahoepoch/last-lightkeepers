"""Add the smoked-glass atrium canopy to the saved apparel map.

Run headlessly with Tools/run_editor_python.ps1 while the editor is closed.
The map's roof, posts, slabs, stairs, lighting actors and gameplay are left
in place. All new parts are overhead decoration with no collision or nav.
Material recipes in this file are the portable source for the generated assets.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
DEST = "/Game/LLK/Boswells/Architecture"
CUBE = "/Engine/BasicShapes/Cube.Cube"
PREFIX = "BSW_CANOPY_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_CANOPY " + message)


def log(message):
    unreal.log("BOSWELLS_CANOPY " + message)
    print("BOSWELLS_CANOPY " + message)


def material(name, base, metallic, roughness, emissive=None):
    path = DEST + "/" + name
    existing = (unreal.EditorAssetLibrary.load_asset(path)
                if unreal.EditorAssetLibrary.does_asset_exist(path) else None)
    if existing:
        require(isinstance(existing, unreal.Material), "unexpected asset at " + path)
        return existing
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, DEST, unreal.Material, unreal.MaterialFactoryNew())
    require(isinstance(asset, unreal.Material), "material creation failed " + name)
    edit = unreal.MaterialEditingLibrary

    color = edit.create_material_expression(
        asset, unreal.MaterialExpressionConstant3Vector, -450, -180)
    color.set_editor_property("constant", unreal.LinearColor(*base, 1.0))
    edit.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    metal = edit.create_material_expression(
        asset, unreal.MaterialExpressionConstant, -450, 0)
    metal.set_editor_property("r", metallic)
    edit.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)
    rough = edit.create_material_expression(
        asset, unreal.MaterialExpressionConstant, -450, 170)
    rough.set_editor_property("r", roughness)
    edit.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    if emissive:
        glow = edit.create_material_expression(
            asset, unreal.MaterialExpressionConstant3Vector, -450, 330)
        glow.set_editor_property("constant", unreal.LinearColor(*emissive, 1.0))
        edit.connect_material_property(
            glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edit.recompile_material(asset)
    require(unreal.EditorAssetLibrary.save_loaded_asset(asset),
            "could not save " + name)
    return asset


def cuboid(actors, mesh, label, x, y, z, width, length, height, mat):
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
    require(actor is not None, "spawn failed " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Architecture/AtriumCanopy")
    actor.set_actor_enable_collision(False)
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(mesh), "cube assignment failed " + label)
    comp.set_material(0, mat)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_cast_shadow(False)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100,
                                          height / 100))
    return actor


def main():
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
    require(world and world.get_name() == "LLK_Boswells_ApparelPass",
            "wrong map")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    before = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
    required = ["ANCHOR_FULL_ROOF", "ROUTE_ESCALATOR_ASCEND",
                "ROUTE_ESCALATOR_DESCEND"] + [
                    "ANCHOR_ATRIUM_POST_" + str(i) for i in range(4)]
    require(all(label in before for label in required),
            "atrium shell differs; aborting")
    require(not any(label.startswith(PREFIX) for label in before),
            "canopy already exists; refusing duplicate")
    roof_center, roof_extent = before["ANCHOR_FULL_ROOF"].get_actor_bounds(False)
    require(abs((roof_center.z - roof_extent.z) - 1488.0) < 1,
            "roof underside moved; canopy height must be reviewed")
    mesh = unreal.EditorAssetLibrary.load_asset(CUBE)
    require(isinstance(mesh, unreal.StaticMesh), "engine cube missing")

    # The photograph is a mirrored false ceiling, not a hole to the sky.
    # A slightly rough dark mirror reads as glass even if planar reflections
    # are unavailable; the physical grid carries its identity in that case.
    mirror = material("M_BSW_SmokedMirror", (0.16, 0.20, 0.22), .92, .16)
    frame = material("M_BSW_DarkChrome", (0.055, 0.07, 0.08), .82, .28)
    blue = material("M_BSW_NeonBlue", (.025, .08, .13), .1, .26,
                    (.018, .22, .42))
    amber = material("M_BSW_NeonAmber", (.14, .035, .012), .1, .28,
                     (.45, .07, .018))

    made = []
    for row, y in enumerate((-700, -420, -140, 140, 420, 700)):
        for col, x in enumerate((-390, -130, 130, 390)):
            made.append(cuboid(actors, mesh, "GLASS_{:02d}_{:02d}".format(
                row, col), x, y, 1472, 248, 268, 6, mirror))

    # 12 cm seams and a deeper perimeter frame give the mirror a scale that
    # can be read from the lower floor, like the photographed ceiling grid.
    for index, x in enumerate((-530, -260, 0, 260, 530)):
        made.append(cuboid(actors, mesh, "GRID_LONG_{:02d}".format(index),
                            x, 0, 1459, 10, 1690, 20, frame))
    for index, y in enumerate((-850, -560, -280, 0, 280, 560, 850)):
        made.append(cuboid(actors, mesh, "GRID_CROSS_{:02d}".format(index),
                            0, y, 1459, 1070, 10, 20, frame))

    # Department-color strips are visible landmarks. They are decorative,
    # with no PointLight and no gameplay-light registration.
    for side, x, mat in (("WEST", -525, blue), ("EAST", 525, amber)):
        made.append(cuboid(actors, mesh, "NEON_" + side,
                            x, 0, 1447, 4, 1570, 3, mat))
    for end, y, mat in (("SOUTH", -845, amber), ("NORTH", 845, blue)):
        made.append(cuboid(actors, mesh, "NEON_" + end,
                            0, y, 1447, 1030, 4, 3, mat))

    # Continue the four existing structural posts into the false ceiling.
    # The extension and sleeve are non-colliding visual wraps; they do not
    # change the upper balcony or the paths at either end of the slopes.
    for index, (x, y) in enumerate(((-650, -950), (650, -950),
                                   (-650, 950), (650, 950))):
        made.append(cuboid(actors, mesh, "POST_WRAP_" + str(index),
                            x, y, 975, 34, 34, 650, frame))
        made.append(cuboid(actors, mesh, "POST_NECK_" + str(index),
                            x, y, 1390, 34, 34, 180, mirror))
        made.append(cuboid(actors, mesh, "POST_CAP_" + str(index),
                            x, y, 1482, 48, 48, 12, frame))

    require(len(made) == 52, "part count differs: " + str(len(made)))
    require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
            "map save failed")
    after = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
    require(sum(label.startswith(PREFIX) for label in after) == len(made),
            "saved canopy count differs")
    require(all(label in after for label in required),
            "route or shell lost during save")
    log("SAVED {}: {} ceiling and post parts, four authored materials".format(
        MAP, len(made)))


main()
