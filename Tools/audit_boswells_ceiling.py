"""Read only: what is overhead in the opening map, and what it is made of.

Chris, 2026-11-22, picking the next thing to fix: *"The ceiling beams."*

In every sales-floor screenshot the overhead reads as a row of black blades
rather than as a store ceiling. This prints every piece of geometry above
head height - its span, its height, its material, and how much of the sales
floor it covers - so the fix is aimed at named actors rather than at an
impression of a screenshot.

`Tools/build_boswells_lower_ceiling.py` already built a tiled suspended
ceiling for `LLK_Boswells_ApparelPass` with `M_BSW_LowerCeilingTile`. The
question this answers is whether the opening map has one at all.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
HEAD_CM = 220.0          # above a 145 cm keeper's reach: this is "overhead"

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("CEILING map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

rows = []
for actor in actors:
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not comps:
        continue
    if not any(c.static_mesh is not None for c in comps):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if centre.z - extent.z < HEAD_CM:
        continue
    mats = sorted({m.get_name() if m else "<none>"
                   for c in comps for m in c.get_materials()})
    rows.append((extent.x * extent.y * 4.0 / 10000.0, actor.get_actor_label(),
                 centre, extent, mats))

rows.sort(reverse=True)
print("CEILING overhead_pieces=%d" % len(rows))
print("CEILING --- biggest 24, by footprint ---")
for area, label, centre, extent, mats in rows[:24]:
    print("CEILING %s %.0fm2 c=(%.0f,%.0f,%.0f) e=(%.0f,%.0f,%.0f) mats=%s" % (
        label, area, centre.x, centre.y, centre.z,
        extent.x, extent.y, extent.z, ",".join(mats)))

# Grouped, because 100 beams named BSW_BEAM_* are one decision not a hundred.
groups = {}
for area, label, centre, extent, mats in rows:
    key = label.rstrip("0123456789").rstrip("_")
    g = groups.setdefault(key, [0, 0.0, set(), 1e9, -1e9])
    g[0] += 1
    g[1] += area
    g[2].update(mats)
    g[3] = min(g[3], centre.z - extent.z)
    g[4] = max(g[4], centre.z + extent.z)
print("CEILING --- groups ---")
for key in sorted(groups, key=lambda k: -groups[k][1]):
    n, area, mats, low, high = groups[key]
    print("CEILING group %s n=%d area=%.0fm2 underside=%.0f top=%.0f mats=%s" % (
        key, n, area, low, high, ",".join(sorted(mats))))

# Is there a tiled ceiling asset ready to reuse, as the apparel pass has?
for name in ("M_BSW_LowerCeilingTile", "T_BSW_LowerCeilingTile",
             "M_BSW_StoneTile"):
    for folder in ("/Game/LLK/Boswells/Architecture/",
                   "/Game/LLK/Boswells/Textures/"):
        asset = unreal.EditorAssetLibrary.load_asset(folder + name)
        if asset:
            print("CEILING asset %s class=%s path=%s" % (
                name, asset.get_class().get_name(), folder + name))
            break
    else:
        print("CEILING asset %s MISSING" % name)

# And how high the room actually is, because the ceiling has to clear the
# lights that are already hanging and the mezzanine that is already there.
lights = [a.get_actor_location().z for a in actors
          if isinstance(a, (unreal.SpotLight, unreal.PointLight, unreal.RectLight))]
if lights:
    print("CEILING light_heights min=%.0f median=%.0f max=%.0f n=%d" % (
        min(lights), sorted(lights)[len(lights) // 2], max(lights), len(lights)))
