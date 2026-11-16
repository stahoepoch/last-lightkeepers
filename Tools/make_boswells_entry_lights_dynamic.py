"""One-time migration for two entry spots placed before the mobility fix."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("wrong map")
actors = {a.get_actor_label(): a for a in
          unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
for side in ("WEST", "EAST"):
    label = "BSW_OPENING_ENTRY_POOL_" + side
    actor = actors.get(label)
    if not isinstance(actor, unreal.SpotLight):
        raise RuntimeError("missing entry light " + label)
    actor.spot_light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP):
    raise RuntimeError("map save failed")
print("BOSWELLS_ENTRY_LIGHT_MOBILITY SAVED: 2 dynamic entry lights")
