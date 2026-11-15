"""Fork the verified apparel map for the witnessed-capture opening.

Run with Tools/run_editor_python.ps1 while Unreal Editor is closed. This
creates one new map and refuses to overwrite it. The static south grille and
the source ApparelPass are deliberately left alone; the release actor and
capture presentation need their runtime contract before they are placed.
"""

import unreal


SOURCE = "/Game/LLK_Boswells_ApparelPass"
TARGET = "/Game/LLK_Boswells_OpeningPass"
START = (1600.0, -2370.0, 120.0)
LANTERN = (1450.0, -2260.0, 9.0)


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING " + message)


require(unreal.EditorAssetLibrary.does_asset_exist(SOURCE), "source map missing")
require(not unreal.EditorAssetLibrary.does_asset_exist(TARGET),
        "opening map already exists; refusing overwrite")
require(unreal.EditorAssetLibrary.duplicate_asset(SOURCE, TARGET) is not None,
        "map duplication failed")
world = unreal.EditorLoadingAndSavingUtils.load_map(TARGET)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map loaded after duplication")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {actor.get_actor_label(): actor
          for actor in subsystem.get_all_level_actors()}
for label in ("ANCHOR_PLAYERSTART_ENTRANCE", "BSW_ARRIVAL_CHECKOUT_EAST_0",
              "ROUTE_MALL_GRILLE_22", "ROUTE_ESCALATOR_ASCEND",
              "MALL_GLASS_ENTRANCE"):
    require(label in actors, "source is missing " + label)
require("BSW_OPENING_LANTERN" not in actors, "pickup already exists")

# The checkout counter at x=1600 is existing, saved cover. The keeper begins
# on its north side. Looking straight south through it is blocked at crouch
# height; looking diagonally through its aisle-end gap reaches the grille.
start = actors["ANCHOR_PLAYERSTART_ENTRANCE"]
require(start.set_actor_location(unreal.Vector(*START), False, False),
        "could not move PlayerStart")
require(start.set_actor_rotation(unreal.Rotator(0, -90, 0), False),
        "could not face PlayerStart toward the grille")

# A world pickup actor, not a second usable lantern. The cylinder is an unlit
# placeholder silhouette that will be replaced with the issued lantern mesh.
# Its component has NoCollision in C++; the player must move out of cover to
# get close enough for Claude's short-reach, line-of-sight interaction.
pickup = subsystem.spawn_actor_from_class(
    unreal.LLKLanternPickup, unreal.Vector(*LANTERN),
    unreal.Rotator(0, 20, 80))
require(pickup is not None, "could not spawn pickup actor")
pickup.set_actor_label("BSW_OPENING_LANTERN")
pickup.set_folder_path("Boswells/Opening")
body = pickup.get_component_by_class(unreal.StaticMeshComponent)
cylinder = unreal.EditorAssetLibrary.load_asset(
    "/Engine/BasicShapes/Cylinder.Cylinder")
chrome = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome")
require(body is not None and isinstance(cylinder, unreal.StaticMesh),
        "pickup body or placeholder mesh missing")
require(body.set_static_mesh(cylinder), "pickup silhouette assignment failed")
if isinstance(chrome, unreal.MaterialInterface):
    body.set_material(0, chrome)
pickup.set_actor_scale3d(unreal.Vector(0.18, 0.18, 0.32))

require(unreal.EditorLoadingAndSavingUtils.save_map(world, TARGET),
        "opening map save failed")
message = ("BOSWELLS_OPENING SAVED: source preserved; start behind east "
           "entrance counter; one unlit lantern pickup on the open tile")
unreal.log(message)
print(message)
