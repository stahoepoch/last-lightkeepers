"""Dress the opening's first physical question in the separate opening map.

Run with Tools/run_editor_python.ps1 while the editor is closed. The script
refuses to layer duplicates and touches no source map or runtime asset.
The release is visual hardware; the grille does not move until its C++ actor
exists. All added geometry is non-colliding and navigation irrelevant.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_OPENING_SCENE_"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING_SCENE " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor
          for actor in subsystem.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before),
        "opening scene dressing already exists")
for label in ("ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
              "BSW_ARRIVAL_CHECKOUT_EAST_0", "ROUTE_MALL_GRILLE_22",
              "MALL_GLASS_ENTRANCE", "MALL_ENTRANCE_APPROACH_FLOOR"):
    require(label in before, "missing opening landmark: " + label)

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
materials = {
    "metal": "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome",
    "steel": "/Game/Props/Boswells/Materials/steel",
    "scuff": "/Game/Props/Boswells/Materials/charcoal",
    "cloth": "/Game/Props/Boswells/Materials/burgundy",
    "wear": "/Game/Props/Boswells/Materials/cream",
    "glass": "/Game/LLK/Boswells/Architecture/M_BSW_MallEntranceGlass",
}
materials = {key: unreal.EditorAssetLibrary.load_asset(path)
             for key, path in materials.items()}
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
require(all(isinstance(value, unreal.MaterialInterface)
            for value in materials.values()), "one scene material is missing")


def piece(name, center, size, material, yaw=0.0, roll=0.0):
    rotation = unreal.Rotator()
    rotation.yaw = yaw
    rotation.roll = roll
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(*center), rotation)
    require(actor is not None, "could not spawn " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Opening/SceneDressing")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "could not set mesh on " + name)
    actor.set_actor_scale3d(unreal.Vector(*(dimension / 100 for dimension in size)))
    comp.set_material(0, materials[material])
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)
    return actor


# This inherited module was a bare checkout. A few small samples and a cloth
# jewelry pad make it the perfume-and-jewelry counter Grok's opening names,
# without changing the already verified cover body or its escape gap.
piece("COUNTER_SAMPLE_TRAY", (1580, -2500, 104), (105, 42, 4), "metal")
for index, x in enumerate((1545, 1580, 1615)):
    piece("COUNTER_BOTTLE_%02d" % index, (x, -2500, 115),
          (9, 9, 18), "glass")
    piece("COUNTER_BOTTLE_CAP_%02d" % index, (x, -2500, 126),
          (7, 7, 4), "steel")
piece("COUNTER_JEWELRY_PAD", (1670, -2500, 104),
      (34, 25, 4), "cloth")


# On the store side of the grille, near its eastern post. The boy can reach
# the low bar; the overhead roller shows what that bar pulls. The heavy hood
# shades the witness mark, so orientation light can show the mechanism's
# silhouette while the close detail waits for Dad's lantern.
piece("RELEASE_BACK", (780, -2643, 144), (96, 12, 156), "metal")
piece("RELEASE_RAIL_W", (728, -2633, 144), (7, 15, 164), "steel")
piece("RELEASE_RAIL_E", (832, -2633, 144), (7, 15, 164), "steel")
piece("RELEASE_SHADE", (780, -2618, 207), (118, 52, 9), "metal")
piece("RELEASE_WITNESS", (780, -2635, 168), (26, 2, 5), "wear")
piece("RELEASE_BAR", (780, -2608, 111), (78, 12, 11), "steel")
piece("RELEASE_STEM", (780, -2620, 137), (8, 34, 8), "steel")
piece("RELEASE_LINK", (780, -2663, 248), (5, 5, 96), "metal")
piece("RELEASE_HEADER_ROLLER", (780, -2663, 298), (76, 18, 18), "steel")

# The caught coat is a blockout silhouette, not a body or a collectible.
# It sits on the exterior side of the closed bars and cannot block the seam.
piece("COAT_SLEEVE", (650, -2712, 151), (34, 7, 62), "cloth", roll=13)
piece("COAT_CUFF", (659, -2716, 115), (23, 8, 10), "wear", roll=13)

# Two scraped tracks run toward the mouth, and a weaker branch toward the
# escalator dies out. They are thin surface marks, not collision or a path
# spline that could alter navigation. No mark crosses the central exit lane.
for index, (x, y, length, width) in enumerate((
        (1150, -2518, 104, 8), (1052, -2552, 94, 7),
        (962, -2585, 81, 8), (879, -2614, 75, 6),
        (817, -2641, 52, 5))):
    piece("DRAG_MAIN_%02d" % index, (x, y, 0.75),
          (length, width, 1.5), "scuff", yaw=-161)
for index, (x, y, length, width) in enumerate((
        (1010, -2491, 64, 5), (958, -2458, 47, 4),
        (917, -2430, 28, 3))):
    piece("DRAG_LOST_%02d" % index, (x, y, 0.6),
          (length, width, 1.2), "scuff", yaw=145)

after = [actor.get_actor_label() for actor in subsystem.get_all_level_actors()]
require(sum(label.startswith(PREFIX) for label in after) == 27,
        "scene dressing inventory differs")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "opening map save failed")
message = ("BOSWELLS_OPENING_SCENE SAVED: reachable visual release, sleeve, "
           "two drag trails; static grille preserved")
unreal.log(message)
print(message)
