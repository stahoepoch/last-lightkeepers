"""Verify the saved decorative chandelier stays out of gameplay and routes."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_ATRIUM_CHANDELIER_"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_CHANDELIER_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
actors = {a.get_actor_label(): a for a in
          unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
chandelier = {name: a for name, a in actors.items() if name.startswith(PREFIX)}
require(len(chandelier) == 50, "chandelier part inventory differs")
require(PREFIX + "MOUNT" in chandelier, "mount missing")
require(len([name for name in chandelier if name.startswith(PREFIX + "ROD_")]) == 49,
        "glass rod inventory differs")
glass = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_ChandelierGlass")
require(isinstance(glass, unreal.Material), "glass material missing")
require(glass.get_editor_property("blend_mode") == unreal.BlendMode.BLEND_TRANSLUCENT,
        "glass no longer translucent")

for name, actor in chandelier.items():
    require(not actor.get_actor_enable_collision(), "actor collides: " + name)
    comp = actor.static_mesh_component
    require(comp.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION,
            "mesh collides: " + name)
    require(not comp.get_editor_property("cast_shadow"),
            "decorative rod shadows the route: " + name)
    center, extent = actor.get_actor_bounds(False)
    require(abs(center.x) + extent.x <= 125 and
            abs(center.y) + extent.y <= 125,
            "part leaves escalator well: " + name)
    require(center.z - extent.z >= 1100 and center.z + extent.z <= 1472,
            "part enters walking plane or roof: " + name)
    if name != PREFIX + "MOUNT":
        require(comp.get_material(0) == glass,
                "rod lost glass material: " + name)

for name in ("ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
             "ANCHOR_FULL_ROOF", "BSW_CANOPY_GLASS_02_02"):
    require(name in actors, "well landmark missing: " + name)

message = ("BOSWELLS_CHANDELIER_VERIFY PASSED: 49 translucent rods, "
           "no light, collision, or route intrusion")
unreal.log(message)
print(message)
