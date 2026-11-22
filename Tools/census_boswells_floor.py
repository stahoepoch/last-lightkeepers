"""Read only: what is actually under the player's feet, by area.

The first census found tile and carpet materials in the map but only 22 and
12 pieces of them, against 21,000 m2 of flat floor - and 113 components
wearing WorldGridMaterial, which is the engine's default checkerboard.

This answers the question that matters for Chris's note: walk the store and
list every large flat surface low enough to stand on, biggest first, with the
material it wears. If the top of that list is one slab in one material, the
store has a warehouse floor no matter how many fixtures stand on it.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("FLOOR map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

rows = []
for actor in actors:
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not comps:
        continue
    center, extent = actor.get_actor_bounds(False)
    # Flat, wide, and low enough that a keeper stands on it rather than
    # walks under it. 300 cm of head clearance is the cutoff.
    if extent.z > 60.0 or extent.x < 150.0 or extent.y < 150.0:
        continue
    if center.z > 300.0:
        continue
    mats = []
    for comp in comps:
        for mat in comp.get_materials():
            mats.append(mat.get_name() if mat else "<none>")
    area = (extent.x * 2.0) * (extent.y * 2.0) / 10000.0
    rows.append((area, actor.get_actor_label(), center.z, sorted(set(mats))))

rows.sort(reverse=True)
print("FLOOR pieces=%d" % len(rows))
for area, label, z, mats in rows[:40]:
    print("FLOOR %.0fm2 z=%.0f %s mats=%s" % (area, z, label, ",".join(mats)))

total = sum(r[0] for r in rows)
print("FLOOR total_m2=%.0f" % total)

# Area by material, which is the figure the note is really about.
by_mat = {}
for area, label, z, mats in rows:
    share = area / max(len(mats), 1)
    for mat in mats:
        by_mat[mat] = by_mat.get(mat, 0.0) + share
for mat in sorted(by_mat, key=lambda k: -by_mat[k]):
    print("FLOOR_BY_MAT %s=%.0fm2 (%.1f%%)" % (
        mat, by_mat[mat], by_mat[mat] / max(total, 1.0) * 100.0))

# And the placeholder problem, named by actor so it can be fixed.
grid = []
for actor in actors:
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        for mat in comp.get_materials():
            name = mat.get_name() if mat else "<none>"
            if name in ("WorldGridMaterial", "BasicShapeMaterial", "<none>"):
                grid.append((actor.get_actor_label(), name))
                break
print("PLACEHOLDER components=%d" % len(grid))
seen = {}
for label, name in grid:
    key = label.split("_")[0] + "/" + name
    seen[key] = seen.get(key, 0) + 1
for key in sorted(seen, key=lambda k: -seen[k])[:25]:
    print("PLACEHOLDER %s=%d" % (key, seen[key]))
