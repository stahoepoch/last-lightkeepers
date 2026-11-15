"""One-shot repair for six fall edges in the first saved concourse slice."""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "GALLERIA_NEAR_"


def require(ok, why):
    if not ok:
        raise RuntimeError("GALLERIA_EDGE_GUARD " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require("GALLERIA_NEAR_PUBLIC_FLOOR" in actors and
        "MALL_ENTRANCE_APPROACH_FLOOR" in actors,
        "floor seam missing")
guard_names = [PREFIX + side + "_" + part + "_EDGE"
               for side in ("W", "E")
               for part in ("ENTRANCE", "SHOP_START", "SHOP_END")]
require(all(name not in actors for name in guard_names),
        "edge guards already installed")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
material = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_MallMullionTeal")
require(isinstance(cube, unreal.StaticMesh) and
        isinstance(material, unreal.MaterialInterface),
        "guard asset missing")

for side, x in (("W", -900), ("E", 900)):
    for part, y0, y1 in (("ENTRANCE", -3100, -2880),
                         ("SHOP_START", -3300, -3100),
                         ("SHOP_END", -8100, -7900)):
        actor = subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(x, (y0 + y1) / 2, 67.5), unreal.Rotator())
        require(actor is not None, "guard spawn failed")
        actor.set_actor_label(PREFIX + side + "_" + part + "_EDGE")
        actor.set_folder_path("Galleria/NearConcourse/Safety")
        comp = actor.static_mesh_component
        require(comp.set_static_mesh(cube), "guard mesh failed")
        actor.set_actor_scale3d(unreal.Vector(.18, (y1 - y0) / 100, 1.35))
        comp.set_material(0, material)
        actor.set_actor_enable_collision(True)
        comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
        comp.set_editor_property("can_ever_affect_navigation", True)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "save failed")
message = "GALLERIA_EDGE_GUARD SAVED: six solid guards close the apron and shop-end fall edges"
unreal.log(message)
print(message)
