"""Read-only checks for the Boswell's infrastructure repair pass."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_REALITY_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_REALITY_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None, "map unavailable")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
labels = {actor.get_actor_label(): actor for actor in actors}
new = {name: actor for name, actor in labels.items() if name.startswith(PREFIX)}
require(len(new) >= 70, "repair actor inventory too low")
for name in ("OFFICE_LEVEL", "ELEVATOR_SHAFT", "ELEVATOR_DOOR_LOWER",
             "ELEVATOR_DOOR_UPPER", "ELEVATOR_DOOR_OFFICE", "FITTING_COUNTER",
             "DOCK_FLOOR", "DOCK_ROOF", "DOCK_ROLLUP_HEADER",
             "ESC_ASCEND_LOW_LANDING_TILE", "ESC_DESCEND_HIGH_LANDING_TILE"):
    require(PREFIX + name in new, "missing " + name)
require(sum("PILLAR_" in name for name in new) == 10, "pillar count changed")
require(sum("FITTING_CUBICLE_" in name and name.endswith("_BACK") for name in new) == 4,
        "fitting rooms incomplete")
require(sum("DOCK_PALLET_" in name for name in new) == 6, "dock stocking incomplete")
for name in ("ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
             "ANCHOR_LOADING_DOCK_EXIT", "BSW_OPENING_LANTERN",
             "BSW_OPENING_GRILLE_CONTROLLER"):
    require(name in labels, "opening actor lost: " + name)
for name in ("CARPET_ZONE_00", "ESC_ASCEND_LOW_LANDING_TILE"):
    actor = new[PREFIX + name]
    require(actor.static_mesh_component.get_collision_enabled() ==
            unreal.CollisionEnabled.NO_COLLISION,
            "decorative floor blocks route " + name)
require(new[PREFIX + "DOCK_FLOOR"].static_mesh_component.get_collision_enabled() ==
        unreal.CollisionEnabled.QUERY_AND_PHYSICS, "dock has no physical floor")
print("BOSWELLS_REALITY_VERIFY PASSED: %d added actors, fitting rooms, three elevator "
      "stops, office-only third level, 10 pillars, stocked covered dock, carpets, and "
      "non-blocking escalator landing detail" % len(new))
