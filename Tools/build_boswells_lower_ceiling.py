"""Give the lower sales wings a suspended store ceiling without roofing the atrium.

Run only with Unreal Editor closed. This pass adds visual geometry; existing
route lights, lantern rules, collision, and navigation stay untouched.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_LOWER_CEILING_"
WINGS = {"WEST": (-4350.0, -900.0), "EAST": (900.0, 4350.0)}
Y_MIN, Y_MAX = -2700.0, 2100.0
FIELD_Z = 516.0  # underside 514; 5.14 m clear, spots at z485


def require(ok, reason):
    if not ok:
        raise RuntimeError("BOSWELLS_LOWER_CEILING " + reason)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before),
        "already built; refusing duplicate")
require(all(label in before for label in (
    "ANCHOR_FULL_FLOOR_LOWER", "ANCHOR_UPPER_WEST", "ANCHOR_UPPER_EAST",
    "ROUTE_ESCALATOR_ASCEND", "MALL_ENTRANCE_APPROACH_FLOOR",
    "BSW_KIDS_CEILING_TILE_FIELD", "ANCHOR_LOADING_DOCK_EXIT")),
    "sales floor, atrium, arrival, or dock differs")

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
ceiling_material = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_LowerCeilingTile")
require(isinstance(cube, unreal.StaticMesh), "cube source missing")
require(isinstance(ceiling_material, unreal.MaterialInterface),
        "lower ceiling material missing; generate and import it first")

# Validate the fixtures before placing the field. A light above the new plane
# would illuminate its top and leave the sales floor darker.
wing_spots = []
old_spots_to_lower = []
for label, actor in before.items():
    if not isinstance(actor, unreal.SpotLight):
        continue
    loc = actor.get_actor_location()
    if (Y_MIN < loc.y < Y_MAX and
            any(xmin < loc.x < xmax for xmin, xmax in WINGS.values()) and
            0 < loc.z < 650):
        if loc.z >= 500:
            require(label.startswith("ANCHOR_EMERGENCY_LOWER_") and
                    abs(loc.z - 600) < 1,
                    "unexpected high light would be swallowed: " + label)
            old_spots_to_lower.append(actor)
        wing_spots.append(label)
require(len(wing_spots) >= 14, "lower wing light inventory changed")
require(len(old_spots_to_lower) == 12, "original lower-light inventory changed")

# These twelve original spots were mounted at z=600, above the new underside.
# Put their emitters alongside the existing wing-route spots at z=485. Keep
# their intensity and visual-only behavior; judge the changed pools in PIE.
for actor in old_spots_to_lower:
    loc = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(loc.x, loc.y, 485), False, False)
    housing_label = actor.get_actor_label() + "_Housing"
    housing = before.get(housing_label)
    require(housing is not None, "original light housing missing: " + housing_label)
    centre, _ = housing.get_actor_bounds(False)
    require(abs(centre.z - 615) < 1, "light housing moved: " + housing_label)
    loc = housing.get_actor_location()
    housing.set_actor_location(unreal.Vector(loc.x, loc.y, 508), False, False)


def piece(label, x, y, z, width, length, depth, material):
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=0, yaw=0, roll=0))
    require(actor is not None, "spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/LowerSales/Ceiling")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "mesh failed: " + label)
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100, depth / 100))
    comp.set_material(0, material)
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    return actor


for side, (xmin, xmax) in WINGS.items():
    width = xmax - xmin
    length = Y_MAX - Y_MIN
    xmid, ymid = (xmin + xmax) / 2, (Y_MIN + Y_MAX) / 2
    # One material repeats a 115 x 120 cm tile thirty by forty times. A
    # physical strip per seam overfilled Lumen's surface cache in PIE.
    piece(side + "_FIELD", xmid, ymid, FIELD_Z, width, length, 4,
          ceiling_material)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
after = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
require(sum(label.startswith(PREFIX) for label in after) == 2,
        "saved ceiling inventory differs")
message = ("BOSWELLS_LOWER_CEILING SAVED 2 tiled fields; "
           "lowered {} original spots and housings, retained {} wing spots".format(
               len(old_spots_to_lower), len(wing_spots)))
unreal.log(message)
print(message)
