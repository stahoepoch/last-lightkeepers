"""Read only: where Boswell's departments are, and on which storey.

Chris, 2026-11-22: *"Fix the carpet."*

`Tools/floor_boswells_departments.py` colours each carpet cell by the nearest
department sign **in XY, ignoring Z**. The building has two storeys, so that
conflates the sales floor with the mezzanine above it, and the result proved
it: nine signs were found including HOME, **no carpet cell chose HOME**, and
SHOES - which has three signs - took 17 of 45 cells. A store that is 38%
shoes and has no home department is not a store.

This prints every candidate source of "what department is here", with its
height, so the fix picks the best one rather than patching the one that was
to hand:

1. `TextRenderActor` signs - what the tool uses now.
2. Actors whose own labels name a department AND a storey, e.g.
   `BSW_DEPT_ROUTE_POOL_00_LOWER_SHOES`, `..._07_UPPER_HOME`. If these are
   spread over the floor they are a better source than the signs, because
   they already answer the storey question that the signs cannot.
3. The lower floor's real extent, so the carpet sweep stops using a typed
   4350 x 2700 that nothing checks.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"

# The mezzanine deck's underside, from `audit_boswells_ceiling.py`. Anything
# at or above this is upstairs and must not name a sales-floor department.
MEZZANINE_UNDERSIDE_CM = 638.0

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("DEPTAUDIT map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

print("DEPTAUDIT --- 1. text signs ---")
for actor in actors:
    if not isinstance(actor, unreal.TextRenderActor):
        continue
    comp = actor.text_render
    text = str(comp.text).strip() if comp else ""
    if not text or len(text) > 24:
        continue
    here = actor.get_actor_location()
    print("DEPTAUDIT sign %-16s (%7.0f,%7.0f,%6.0f) %-6s label=%s" % (
        text, here.x, here.y, here.z,
        "UPPER" if here.z >= MEZZANINE_UNDERSIDE_CM else "lower",
        actor.get_actor_label()))

print("DEPTAUDIT --- 2. actors whose labels name a storey and a department ---")
rows = []
for actor in actors:
    label = actor.get_actor_label().upper()
    if "_LOWER_" not in label and "_UPPER_" not in label:
        continue
    storey, _, rest = label.partition("_LOWER_" if "_LOWER_" in label
                                      else "_UPPER_")
    name = rest.split("_")[0]
    if not name.isalpha() or len(name) < 3:
        continue
    here = actor.get_actor_location()
    rows.append(("LOWER" if "_LOWER_" in label else "UPPER", name,
                 here.x, here.y, here.z, actor.get_actor_label()))

by_key = {}
for storey, name, x, y, z, label in rows:
    key = (storey, name)
    g = by_key.setdefault(key, [])
    g.append((x, y, z, label))
for key in sorted(by_key):
    pts = by_key[key]
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    zs = [p[2] for p in pts]
    print("DEPTAUDIT labelled %-6s %-10s n=%2d x=%6.0f..%6.0f y=%6.0f..%6.0f "
          "z=%5.0f..%5.0f e.g. %s" % (
              key[0], key[1], len(pts), min(xs), max(xs), min(ys), max(ys),
              min(zs), max(zs), pts[0][3]))

print("DEPTAUDIT --- 3. the lower sales floor's real extent ---")
best = None
for actor in actors:
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not any(c.static_mesh is not None for c in comps):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if extent.z > 60.0 or centre.z > 200.0 or centre.z < -200.0:
        continue
    area = extent.x * extent.y
    if best is None or area > best[0]:
        best = (area, actor.get_actor_label(), centre, extent)
if best is not None:
    _, label, centre, extent = best
    print("DEPTAUDIT floor %s x=%.0f..%.0f y=%.0f..%.0f z=%.0f" % (
        label, centre.x - extent.x, centre.x + extent.x,
        centre.y - extent.y, centre.y + extent.y, centre.z))

print("DEPTAUDIT --- 4. the carpet as it stands ---")
counts = {}
for actor in actors:
    label = actor.get_actor_label()
    if not label.startswith("BSW_DEPT_CARPET_"):
        continue
    name = label[len("BSW_DEPT_CARPET_"):].rsplit("_", 2)[0]
    counts[name] = counts.get(name, 0) + 1
total = sum(counts.values())
for name in sorted(counts, key=lambda k: -counts[k]):
    print("DEPTAUDIT carpet %-10s %2d cells (%4.1f%%)" % (
        name, counts[name], 100.0 * counts[name] / max(1, total)))
print("DEPTAUDIT carpet_total=%d" % total)

print("DEPTAUDIT --- 5. merchandise, by department word in the actor label ---")
WORDS = ("LADIES", "MENS", "SHOES", "JUNIORS", "ELECTRONICS", "HOME",
         "KIDS", "TOYS", "SEASONAL", "FURNITURE", "APPAREL", "HOUSEWARES",
         "BEDDING", "CURTAINS", "FABRICS")
found = {}
for actor in actors:
    label = actor.get_actor_label().upper()
    for word in WORDS:
        if word not in label:
            continue
        here = actor.get_actor_location()
        found.setdefault(word, []).append((here.x, here.y, here.z))
for word in sorted(found, key=lambda w: -len(found[w])):
    pts = found[word]
    lower = [p for p in pts if p[2] < MEZZANINE_UNDERSIDE_CM]
    upper = [p for p in pts if p[2] >= MEZZANINE_UNDERSIDE_CM]
    line = "DEPTAUDIT stock %-12s n=%3d lower=%3d upper=%3d" % (
        word, len(pts), len(lower), len(upper))
    if lower:
        xs = [p[0] for p in lower]
        ys = [p[1] for p in lower]
        line += "  lower x=%6.0f..%6.0f y=%6.0f..%6.0f mid=(%6.0f,%6.0f)" % (
            min(xs), max(xs), min(ys), max(ys),
            sum(xs) / len(xs), sum(ys) / len(ys))
    print(line)
