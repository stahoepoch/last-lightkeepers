"""Check saved lower ceiling geometry, light placement, and route clearance."""

import unreal


def require(ok, reason):
    if not ok:
        raise RuntimeError("BOSWELLS_LOWER_CEILING_VERIFY " + reason)


world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/LLK_Boswells_ApparelPass")
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
actors = {actor.get_actor_label(): actor for actor in
          unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
ceiling = {label: actor for label, actor in actors.items()
           if label.startswith("BSW_LOWER_CEILING_")}
require(len(ceiling) == 2, "ceiling field count differs")
material_asset = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_LowerCeilingTile")
require(isinstance(material_asset, unreal.Material), "ceiling material absent")
expressions = unreal.MaterialEditingLibrary.get_material_expressions(material_asset)
glow_values = [expr.get_editor_property("r") for expr in expressions
               if isinstance(expr, unreal.MaterialExpressionConstant)]
require(any(abs(value - 0.035) < 0.0001 for value in glow_values),
        "ceiling underside ambient value changed")
uvs = [expr for expr in expressions
       if isinstance(expr, unreal.MaterialExpressionTextureCoordinate)]
require(len(uvs) == 1 and abs(uvs[0].get_editor_property("u_tiling") - 30) < 0.01
        and abs(uvs[0].get_editor_property("v_tiling") - 40) < 0.01,
        "tile spacing changed")

for side, expected_x in (("WEST", -2625), ("EAST", 2625)):
    field = ceiling.get("BSW_LOWER_CEILING_{}_FIELD".format(side))
    require(field is not None, side + " field absent")
    centre, extent = field.get_actor_bounds(False)
    require(all(abs(a - b) < 1 for a, b in (
        (centre.x, expected_x), (centre.y, -300), (centre.z, 516),
        (extent.x, 1725), (extent.y, 2400), (extent.z, 2))),
        side + " field bounds differ")
    material = field.static_mesh_component.get_material(0)
    require(material and material.get_path_name() ==
            "/Game/LLK/Boswells/Architecture/M_BSW_LowerCeilingTile.M_BSW_LowerCeilingTile",
            side + " field material differs")

for label, actor in ceiling.items():
    centre, extent = actor.get_actor_bounds(False)
    require(centre.x + extent.x <= -900 or centre.x - extent.x >= 900,
            label + " intrudes into central atrium/entrance volume")
    require(centre.z - extent.z >= 514, label + " hangs into the sales route")
    require(not actor.get_actor_enable_collision(), label + " has collision")

for label, actor in actors.items():
    if not label.startswith("ANCHOR_EMERGENCY_LOWER_"):
        continue
    if label.endswith("_Housing"):
        centre, _ = actor.get_actor_bounds(False)
        require(abs(centre.z - 508) < 1, label + " detached from new ceiling")
    elif isinstance(actor, unreal.SpotLight):
        require(abs(actor.get_actor_location().z - 485) < 1,
                label + " sits above the ceiling")

for x in (-2450, 2450):
    for y in (-2250, 1250):
        hit = unreal.SystemLibrary.line_trace_single(
            world, unreal.Vector(x, y, 300), unreal.Vector(x, y, 600),
            unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [],
            unreal.DrawDebugTrace.NONE, True)
        require(not hit or not hit.to_dict().get("hit_actor"),
                "sales-floor headroom blocked at {},{}".format(x, y))

require(all(label in actors for label in (
    "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
    "MALL_ENTRANCE_APPROACH_FLOOR", "ANCHOR_LOADING_DOCK_EXIT")),
    "chapter route actor absent")
message = "BOSWELLS_LOWER_CEILING_VERIFY PASSED: 2 fields, 12 spots and housings, central void, 4 headroom traces"
unreal.log(message)
print(message)
