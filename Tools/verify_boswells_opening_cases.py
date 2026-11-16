"""Verify saved opening cases after merchandise replacement and reload."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING_CASES_VERIFY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
all_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
actors = {a.get_actor_label(): a for a in all_actors}
require(len(actors) == len(all_actors), "duplicate labels")
require(all(x in actors for x in (
    "ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
    "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
    "ROUTE_MALL_GRILLE_22")), "opening route changed")
case_count = 0
for kind, xs in (("PERFUME", (-3690, -3500, -3310, -3120, -2930)),
                 ("JEWELRY", (2930, 3120, 3310, 3500, 3690))):
    for i, x in enumerate(xs):
        prefix = f"BSW_ARRIVAL_{kind}_{i}"
        shell = []
        for suffix in ("PLINTH", "GLASS", "EDGE_S", "EDGE_N"):
            shell.append(actors.get(prefix + "_" + suffix))
        require(all(shell), "case shell missing: " + prefix)
        require(all(isinstance(a, unreal.StaticMeshActor) for a in shell),
                "case shell type changed: " + prefix)
        require(all(a.static_mesh_component.get_collision_enabled() ==
                    unreal.CollisionEnabled.QUERY_AND_PHYSICS for a in shell[:2]),
                "case blocker changed: " + prefix)
        require(all(a.static_mesh_component.get_collision_enabled() ==
                    unreal.CollisionEnabled.NO_COLLISION for a in shell[2:]),
                "case trim acquired collision: " + prefix)
        display = actors.get(prefix + "_STOCK_MESH")
        require(isinstance(display, unreal.StaticMeshActor),
                "modeled merchandise missing: " + prefix)
        comp = display.static_mesh_component
        mesh = comp.get_editor_property("static_mesh")
        require(mesh and mesh.get_path_name().startswith(
                    f"/Game/LLK/Boswells/Fixtures/SM_BSW_CaseGoods_{kind.title()}_"),
                "wrong merchandise: " + prefix)
        require(comp.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION
                and not comp.get_editor_property("can_ever_affect_navigation"),
                "merchandise blocks route: " + prefix)
        require(all(s.get_editor_property("material_interface") is not None
                    for s in mesh.get_editor_property("static_materials")),
                "material slot empty: " + prefix)
        center, extent = display.get_actor_bounds(False)
        require(center.x - extent.x >= x - 93 and
                center.x + extent.x <= x + 93 and
                center.y - extent.y >= -2545 and
                center.y + extent.y <= -2455 and
                center.z - extent.z >= 59 and center.z + extent.z <= 99,
                f"merchandise escapes case: {prefix} center={center} extent={extent}")
        require(not any(f"{prefix}_GOODS_{slot}" in actors for slot in range(3)),
                "old cuboid survived: " + prefix)
        case_count += 1

require(case_count == 10, "case inventory differs")
print("BOSWELLS_OPENING_CASES_VERIFY PASSED: 10 modeled displays within "
      "unchanged glass cases; no route or collision change")
