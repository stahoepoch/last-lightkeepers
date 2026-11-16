"""Replace only the 30 visual merchandise cubes in the saved opening map."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
ROOT = "/Game/LLK/Boswells/Fixtures/"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_CASE_GOODS_REPLACE " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = sub.get_all_level_actors()
by_label = {a.get_actor_label(): a for a in actors}
require(len(by_label) == len(actors), "duplicate actor labels")
meshes = {}
for kind in ("PERFUME", "JEWELRY"):
    for variant in ("A", "B"):
        stem = f"SM_BSW_CaseGoods_{kind.title()}_{variant}"
        mesh = unreal.EditorAssetLibrary.load_asset(ROOT + stem)
        require(isinstance(mesh, unreal.StaticMesh), "mesh absent: " + stem)
        meshes[(kind, variant)] = mesh

# Preflight every case before deleting a single actor. Glass, plinth and
# chrome bars belong to the existing case and retain their collisions.
for kind in ("PERFUME", "JEWELRY"):
    for i in range(5):
        prefix = f"BSW_ARRIVAL_{kind}_{i}"
        require(prefix + "_STOCK_MESH" not in by_label, "already replaced: " + prefix)
        for suffix in ("PLINTH", "GLASS", "EDGE_S", "EDGE_N"):
            require(prefix + "_" + suffix in by_label,
                    "case shell missing: " + prefix + "_" + suffix)
        for slot in range(3):
            old = by_label.get(f"{prefix}_GOODS_{slot}")
            require(isinstance(old, unreal.StaticMeshActor),
                    "old goods missing: " + prefix)
            require(old.static_mesh_component.get_collision_enabled() ==
                    unreal.CollisionEnabled.NO_COLLISION,
                    "old merchandise acquired collision: " + prefix)

removed = added = 0
for kind, xs in (("PERFUME", (-3690, -3500, -3310, -3120, -2930)),
                 ("JEWELRY", (2930, 3120, 3310, 3500, 3690))):
    for i, x in enumerate(xs):
        prefix = f"BSW_ARRIVAL_{kind}_{i}"
        for slot in range(3):
            require(sub.destroy_actor(by_label[f"{prefix}_GOODS_{slot}"]),
                    "could not remove goods: " + prefix)
            removed += 1
        actor = sub.spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(x, -2500, 0), unreal.Rotator())
        require(actor is not None, "could not spawn goods: " + prefix)
        actor.set_actor_label(prefix + "_STOCK_MESH")
        actor.set_folder_path("Boswells/Arrival/" + kind.title())
        comp = actor.static_mesh_component
        require(comp.set_static_mesh(meshes[(kind, "A" if i % 2 == 0 else "B")]),
                "mesh assignment failed: " + prefix)
        actor.set_actor_enable_collision(False)
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        comp.set_editor_property("can_ever_affect_navigation", False)
        added += 1

require(removed == 30 and added == 10, "replacement inventory differs")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("BOSWELLS_CASE_GOODS_REPLACE SAVED: 10 modeled case displays; "
      "glass, plinth and collision retained")
