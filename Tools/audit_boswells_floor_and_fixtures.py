"""Read only: where the floor zones are, and which lights come from nowhere.

Two questions Chris asked on 2026-11-22:

    "the floors, the walkways, the carpeted areas"

    "It kind of looks like lights are coming from nowhere. Is there an
    emergency light that actually gives off light, or is it just a light that
    comes randomly out of the ceiling? It has to be believable, like a fixture
    or something, to show that it is coming from where it's coming from.
    Otherwise, we're just making stuff up and it's not going to be real."

The second is correct about the fills added earlier today: they are
`APointLight` actors at ceiling height with no geometry at all. This prints,
for every light in the map, whether any static mesh stands near enough to be
read as its fixture - so "which lights are lying" is a list rather than an
impression.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("FLOORFIX map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

# ---- 1. the floor, as zones rather than as a total ----------------------
print("FLOORFIX --- floor pieces, biggest first ---")
rows = []
for actor in actors:
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not comps:
        continue
    center, extent = actor.get_actor_bounds(False)
    if extent.z > 60.0 or extent.x < 150.0 or extent.y < 150.0 or center.z > 300.0:
        continue
    mats = sorted({m.get_name() if m else "<none>"
                   for c in comps for m in c.get_materials()})
    rows.append(((extent.x * 2.0) * (extent.y * 2.0) / 10000.0,
                 actor.get_actor_label(), center, extent, mats))
rows.sort(reverse=True)
for area, label, center, extent, mats in rows[:16]:
    print("FLOORFIX floor %s %.0fm2 c=(%.0f,%.0f,%.0f) e=(%.0f,%.0f) mats=%s" % (
        label, area, center.x, center.y, center.z, extent.x, extent.y,
        ",".join(mats)))

# ---- 2. what carpet and hard-floor materials already exist --------------
print("FLOORFIX --- floor-ish materials in the project ---")
registry = unreal.AssetRegistryHelpers.get_asset_registry()
for path in ("/Game/LLK", "/Game"):
    assets = registry.get_assets_by_path(path, recursive=True)
    names = sorted({a.asset_name for a in assets
                    if "Material" in str(a.asset_class_path.asset_name)
                    and any(k in str(a.asset_name).lower()
                            for k in ("carpet", "tile", "floor", "vinyl",
                                      "terrazzo", "lino", "wood"))})
    if names:
        print("FLOORFIX materials_under_%s=%s" % (path, [str(n) for n in names]))
        break

# Is the existing carpet a material or an instance with parameters we can
# recolour per department? That decides whether new carpets are cheap.
for name in ("M_BSW_KidsSageCarpet", "M_BSW_StoneTile"):
    for folder in ("/Game/LLK/Boswells/Architecture/", "/Game/LLK/Boswells/Materials/"):
        asset = unreal.EditorAssetLibrary.load_asset(folder + name)
        if asset:
            print("FLOORFIX asset %s class=%s path=%s" % (
                name, asset.get_class().get_name(), folder + name))
            if isinstance(asset, unreal.MaterialInstanceConstant):
                params = unreal.MaterialEditingLibrary.get_vector_parameter_names(asset)
                print("FLOORFIX   vector_params=%s" % [str(p) for p in params])
            elif isinstance(asset, unreal.Material):
                params = unreal.MaterialEditingLibrary.get_vector_parameter_names(asset)
                print("FLOORFIX   vector_params=%s" % [str(p) for p in params])
            break

# ---- 3. which lights have a fixture, and which come from nowhere --------
print("FLOORFIX --- lights without geometry ---")
meshes = []
for actor in actors:
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        if comp.static_mesh is not None:
            meshes.append((actor.get_actor_label(), actor.get_actor_location()))
            break

NEAR_CM = 260.0
naked = {}
clothed = {}
for actor in actors:
    if not isinstance(actor, (unreal.SpotLight, unreal.PointLight, unreal.RectLight)):
        continue
    here = actor.get_actor_location()
    found = None
    for label, where in meshes:
        if (here - where).length() <= NEAR_CM:
            found = label
            break
    group = actor.get_actor_label().rsplit("_", 2)[0]
    if found is None:
        naked[group] = naked.get(group, 0) + 1
    else:
        clothed[group] = clothed.get(group, 0) + 1

print("FLOORFIX lights_with_a_fixture_within_%dcm:" % NEAR_CM)
for key in sorted(clothed, key=lambda k: -clothed[k])[:14]:
    print("FLOORFIX   %s=%d" % (key, clothed[key]))
print("FLOORFIX lights_coming_from_NOWHERE:")
for key in sorted(naked, key=lambda k: -naked[k])[:14]:
    print("FLOORFIX   %s=%d" % (key, naked[key]))
print("FLOORFIX totals with_fixture=%d from_nowhere=%d" % (
    sum(clothed.values()), sum(naked.values())))
