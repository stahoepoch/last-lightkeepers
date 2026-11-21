"""Read-only proof that the saved Boswell's circulation pass is present."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_CIRCULATION_"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_CIRCULATION_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {actor.get_actor_label(): actor for actor in sub.get_all_level_actors()}

calls = [actors.get(PREFIX + name) for name in ("ELEVATOR_LOWER_CALL", "ELEVATOR_UPPER_CALL")]
require(all(calls), "both elevator calls missing")
require(all("LLKStoreElevator" in str(call.get_class()) for call in calls), "call actor class is wrong")

shaft = actors.get("BSW_REALITY_ELEVATOR_SHAFT")
# `set_actor_hidden_in_game` is a runtime-only flag; collision is the durable
# map-level proof that the legacy solid shaft no longer seals this route.
require(shaft and not shaft.get_actor_enable_collision(), "legacy solid elevator shaft still blocks")

stock = [name for name in actors if name.startswith(PREFIX) and "_STOCK_" in name]
require(len(stock) >= 140, "dense stock missing: {}".format(len(stock)))
require(all(not actors[name].get_actor_enable_collision() for name in stock), "stock is blocking routes")

unreal.log("BOSWELLS_CIRCULATION VERIFIED: 2 paired elevator calls, opened old shaft, {} nonblocking dense stock runs".format(len(stock)))
