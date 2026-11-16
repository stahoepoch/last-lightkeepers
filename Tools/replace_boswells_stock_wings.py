"""Replace cuboid apparel stock with four authored hanging-stock meshes.

The 48 existing hidden bodies keep exactly their collision and navigation
footprints. Only their 432 non-colliding decorative companions are replaced.
Run once with the editor closed; the map must be clean in Git first.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_CROWD_"
MESH_ROOT = "/Game/LLK/Boswells/Fixtures/"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_STOCK_WING_REPLACE " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
all_actors = subsystem.get_all_level_actors()
labels = {a.get_actor_label(): a for a in all_actors}
old = {name: actor for name, actor in labels.items() if name.startswith(PREFIX)}
require(len(old) == 480 and len(old) == sum(
    a.get_actor_label().startswith(PREFIX) for a in all_actors),
    "expected 480 unique original crowd actors")
require(not any(name.endswith("_STOCK_MESH") for name in old),
        "stock wing replacement already present")
for name in ("ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
             "BSW_OPENING_GRILLE_CONTROLLER", "ROUTE_ESCALATOR_ASCEND",
             "ROUTE_ESCALATOR_DESCEND", "BSW_KIDS_SWEEPER"):
    require(name in labels, "route actor missing before edit: " + name)

meshes = {}
for length in ("Short", "Long"):
    for variant in ("A", "B"):
        stem = f"SM_BSW_StockWing_{length}_{variant}"
        mesh = unreal.EditorAssetLibrary.load_asset(MESH_ROOT + stem)
        require(isinstance(mesh, unreal.StaticMesh), "mesh missing: " + stem)
        meshes[(length, variant)] = mesh

bodies = {name[:-5]: actor for name, actor in old.items()
          if name.endswith("_BODY")}
require(len(bodies) == 48, "expected 48 original collision bodies")
groups = {}
for prefix, body in sorted(bodies.items()):
    members = {name: actor for name, actor in old.items()
               if name.startswith(prefix + "_")}
    comp = body.static_mesh_component
    require(comp.get_collision_enabled() ==
            unreal.CollisionEnabled.QUERY_AND_PHYSICS,
            "original body lost collision: " + prefix)
    require(comp.get_editor_property("can_ever_affect_navigation"),
            "original body stopped carving navigation: " + prefix)
    require(not comp.get_editor_property("visible"),
            "original body became visible: " + prefix)
    center, extent = body.get_actor_bounds(False)
    length = "Long" if abs(extent.y * 2 - 600) < 1 else "Short"
    expected = 13 if length == "Long" else 7
    require(abs(extent.x * 2 - 45) < 1 and
            abs(extent.y * 2 - (600 if length == "Long" else 150)) < 1 and
            abs(center.z - 85) < 1 and len(members) == expected,
            "original footprint or group inventory changed: " + prefix)
    require(all(a.static_mesh_component.get_collision_enabled() ==
                unreal.CollisionEnabled.NO_COLLISION
                for name, a in members.items() if a != body),
            "a decorative companion acquired collision: " + prefix)
    groups[prefix] = (body, members, center, extent, length)
require(sum(len(group[1]) for group in groups.values()) == 480,
        "group labels overlap or miss a piece")

# The new visual mesh is bounded by the unchanged hidden body. Preflight the
# entire map before destroying even one decorative actor.
for index, (prefix, (_, _, _, extent, length)) in enumerate(sorted(groups.items())):
    mesh = meshes[(length, "A" if index % 2 == 0 else "B")]
    asset_extent = mesh.get_bounds().box_extent
    require(asset_extent.x <= extent.x + 0.1 and
            asset_extent.y <= extent.y + 0.1 and
            asset_extent.z <= extent.z + 0.1,
            "new visual exceeds existing collision body: " + prefix)

removed = 0
created = 0
for index, (prefix, (body, members, center, _, length)) in enumerate(
        sorted(groups.items())):
    for name, actor in members.items():
        if actor == body:
            continue
        require(subsystem.destroy_actor(actor), "could not remove " + name)
        removed += 1
    mesh = meshes[(length, "A" if index % 2 == 0 else "B")]
    visual = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(center.x, center.y, 0), unreal.Rotator())
    require(visual is not None, "visual spawn failed: " + prefix)
    visual.set_actor_label(prefix + "_STOCK_MESH")
    visual.set_folder_path("Boswells/Departments/lower/StockedApparel")
    comp = visual.static_mesh_component
    require(comp.set_static_mesh(mesh), "visual mesh assignment failed: " + prefix)
    visual.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)
    created += 1

require(removed == 432 and created == 48,
        f"replacement count wrong: removed={removed} created={created}")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
message = ("BOSWELLS_STOCK_WING_REPLACE SAVED: 48 new modeled stock wings, "
           "432 cuboid decorations removed; 48 collision bodies unchanged")
unreal.log(message)
print(message)
