"""Check saved canopy placement and the stair/route collision boundary."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong saved map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
canopy = [actor for label, actor in by_label.items()
          if label.startswith("BSW_CANOPY_")]
if len(canopy) != 52:
    raise RuntimeError("expected 52 canopy parts, got {}".format(len(canopy)))
for actor in canopy:
    if actor.get_actor_enable_collision():
        raise RuntimeError("canopy actor blocks movement: " + actor.get_actor_label())
    center, extent = actor.get_actor_bounds(False)
    if center.z - extent.z < 648:
        raise RuntimeError("canopy part enters lower route: " + actor.get_actor_label())
for label in ("ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
              "ROUTE_ATRIUM_GUARD_W", "ROUTE_ATRIUM_GUARD_E",
              "ANCHOR_FULL_ROOF"):
    if label not in by_label:
        raise RuntimeError("route or shell missing: " + label)
for name in ("M_BSW_SmokedMirror", "M_BSW_DarkChrome",
             "M_BSW_NeonBlue", "M_BSW_NeonAmber"):
    if not unreal.EditorAssetLibrary.does_asset_exist(
            "/Game/LLK/Boswells/Architecture/" + name):
        raise RuntimeError("material missing: " + name)
unreal.log("BOSWELLS_CANOPY_VERIFY PASSED: 52 nonblocking parts, four materials, route intact")
