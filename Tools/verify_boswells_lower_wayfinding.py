"""Validate saved lower-floor signs without treating all text as lantern content."""

import unreal

MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_LOWER_WAYFINDING_"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
actors = {actor.get_actor_label(): actor for actor in
          unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
signs = {label: actor for label, actor in actors.items() if label.startswith(PREFIX)}
if len(signs) != 20:
    raise RuntimeError("expected 20 sign actors, found {}".format(len(signs)))
text_material = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_EmissiveWayfindingText")
if not isinstance(text_material, unreal.Material) or (
        text_material.get_editor_property("shading_model") !=
        unreal.MaterialShadingModel.MSM_UNLIT):
    raise RuntimeError("wayfinding text is not unlit")
expected = {"JUNIORS", "LADIES", "MENS", "SHOES", "EXIT"}
for name in expected:
    for suffix in ("BACK", "TUBE_TOP", "TUBE_BOTTOM", "WORDS"):
        label = PREFIX + name + "_" + suffix
        actor = signs.get(label)
        if not actor:
            raise RuntimeError("missing wayfinding part: " + label)
        if actor.get_actor_enable_collision():
            raise RuntimeError("sign would obstruct the route: " + label)
        if suffix == "WORDS" and not isinstance(actor, unreal.TextRenderActor):
            raise RuntimeError("sign text is not a text actor: " + label)
        if suffix == "WORDS" and actor.text_render.get_material(0) != text_material:
            raise RuntimeError("navigation label is not emissive: " + label)
        if suffix != "WORDS" and not isinstance(actor, unreal.StaticMeshActor):
            raise RuntimeError("sign backing is not a static mesh: " + label)
for actor in signs.values():
    if isinstance(actor, (unreal.PointLight, unreal.SpotLight)):
        raise RuntimeError("wayfinding registered an actual light")
    # TextRender bounds include the world origin in this engine build. Its
    # transform, not that synthetic bound, is the meaningful height check.
    if isinstance(actor, unreal.TextRenderActor):
        if not 420 <= actor.get_actor_location().z <= 450:
            raise RuntimeError("text dropped into travel: " + actor.get_actor_label())
    else:
        centre, extent = actor.get_actor_bounds(False)
        if centre.z - extent.z < 390 or centre.z + extent.z > 480:
            raise RuntimeError("sign dropped into travel: " + actor.get_actor_label())
if not all(label in actors for label in (
        "BSW_LOWER_CEILING_WEST_FIELD", "BSW_LOWER_CEILING_EAST_FIELD",
        "ROUTE_MALL_GRILLE_22", "ROUTE_ESCALATOR_ASCEND")):
    raise RuntimeError("route or ceiling actor missing")
unreal.log("BOSWELLS_LOWER_WAYFINDING_VERIFY PASSED: 5 landmarks, 20 parts, no collision or gameplay lights")
