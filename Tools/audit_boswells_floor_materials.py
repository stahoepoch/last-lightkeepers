"""Read-only saved floor material inventory before changing the store skin."""

import unreal


world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/LLK_Boswells_ApparelPass")
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    label = actor.get_actor_label()
    if ("FLOOR" in label or label.startswith("ANCHOR_UPPER_")
            or label == "MALL_ENTRANCE_APPROACH_FLOOR"):
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not component:
            continue
        material = component.get_material(0)
        centre, extent = actor.get_actor_bounds(False)
        print("FLOOR_MATERIAL {} {} {} {}".format(
            label, material.get_path_name() if material else "NONE",
            round(centre.z - extent.z, 1), round(centre.z + extent.z, 1)))
