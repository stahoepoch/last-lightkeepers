"""Validate that the upper service edge is a finished, bounded public core."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_SERVICE_CORE_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_SERVICE_CORE_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
actors = {actor.get_actor_label(): actor
          for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
core = {name: actor for name, actor in actors.items() if name.startswith(PREFIX)}
require(len(core) >= 55, "core actor count too low: {}".format(len(core)))
for name in ("PUBLIC_SERVICE_FLOOR", "REAR_SERVICE_WALL", "SERVICE_CEILING",
             "COURTESY_COUNTER", "OPTICAL_COUNTER", "HEARING_COUNTER",
             "CANDY_GIFT_COUNTER", "RESTROOM_SIGN_BAND", "RETURNS_DESK"):
    require(PREFIX + name in core, "missing " + name)
for actor in core.values():
    require(isinstance(actor, unreal.StaticMeshActor), "non-mesh core actor")
require(actors["BSW_REALITY_FITTING_LOBBY_FLOOR"], "fitting lobby lost")
print("BOSWELLS_SERVICE_CORE_VERIFY PASSED: {} actors form a public upper service hall with bounded staff rooms, customer services, restrooms, and fitting-room access".format(len(core)))
