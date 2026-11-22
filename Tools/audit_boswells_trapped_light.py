"""Read only: which lights are stuck above a ceiling, and what reads as black.

Chris, 2026-11-22: *"The ceiling beams."*

`Tools/audit_boswells_ceiling.py` found that the sales floor has three layers
overhead - a suspended tile field at 5.16 m in the west and east wings, the
mezzanine deck ring at 6.38 m, and `ANCHOR_FULL_ROOF` at 14.88 m still wearing
`BasicShapeMaterial`. It also found the median light in the map sits at 7.56 m,
**above the wings' ceiling**, which would mean a light illuminating the top of
a tile field and nothing a player can stand under.

This prints, per light, which ceiling layer is between it and the floor. A
light with a ceiling under it is the same defect as a light with no fixture,
one storey up: the illumination exists and the room it is supposed to be in
never receives it.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("TRAPPED map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

# The big flat lids, as boxes, so "is this light above one" is a lookup.
lids = []
for actor in actors:
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not any(c.static_mesh is not None for c in comps):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if extent.z > 40.0 or extent.x < 300.0 or extent.y < 300.0:
        continue
    if centre.z < 300.0:
        continue
    lids.append((actor.get_actor_label(), centre, extent))
print("TRAPPED lids=%d" % len(lids))
for label, centre, extent in sorted(lids, key=lambda r: -r[2].x * r[2].y)[:12]:
    print("TRAPPED lid %s underside=%.0f span_x=%.0f..%.0f span_y=%.0f..%.0f" % (
        label, centre.z - extent.z, centre.x - extent.x, centre.x + extent.x,
        centre.y - extent.y, centre.y + extent.y))

trapped = {}
clear = 0
for actor in actors:
    if not isinstance(actor, (unreal.SpotLight, unreal.PointLight, unreal.RectLight)):
        continue
    here = actor.get_actor_location()
    over = None
    for label, centre, extent in lids:
        if (abs(here.x - centre.x) <= extent.x and
                abs(here.y - centre.y) <= extent.y and
                here.z > centre.z + extent.z):
            # The LOWEST lid above it is the one that blocks the floor.
            if over is None or centre.z < over[1]:
                over = (label, centre.z)
    if over is None:
        clear += 1
        continue
    key = "%s under %s" % (actor.get_actor_label().rstrip("0123456789").rstrip("_"),
                           over[0])
    trapped[key] = trapped.get(key, 0) + 1

print("TRAPPED lights_clear_of_any_lid=%d" % clear)
print("TRAPPED lights_above_a_lid:")
for key in sorted(trapped, key=lambda k: -trapped[k]):
    print("TRAPPED   %s n=%d" % (key, trapped[key]))
print("TRAPPED total_above_a_lid=%d" % sum(trapped.values()))

# And the placeholder materials still overhead, by name, so the ceiling pass
# knows what it is replacing rather than recolouring everything it can see.
print("TRAPPED --- placeholder materials overhead ---")
bad = {}
for actor in actors:
    centre, extent = actor.get_actor_bounds(False)
    if centre.z - extent.z < 220.0:
        continue
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        for mat in comp.get_materials():
            name = mat.get_name() if mat else "<none>"
            if name in ("BasicShapeMaterial", "WorldGridMaterial", "<none>",
                        "DefaultMaterial"):
                bad.setdefault(name, []).append(actor.get_actor_label())
for name, labels in bad.items():
    uniq = sorted(set(labels))
    print("TRAPPED placeholder %s n=%d e.g. %s" % (name, len(uniq), uniq[:8]))

# ---- and the department signs, with their heights --------------------------
# `Tools/floor_boswells_departments.py` picks a carpet by the NEAREST sign in
# XY and ignores Z, which conflates the sales floor with the mezzanine above
# it. The evidence it is wrong: nine signs were found, HOME among them, and no
# carpet cell chose HOME, while SHOES - which has three signs - took 17 of 45.
# A store with 38% of its floor in shoes and no home department is not a store.
print("TRAPPED --- department signs ---")
for actor in actors:
    if not isinstance(actor, unreal.TextRenderActor):
        continue
    comp = actor.text_render
    text = str(comp.text).strip() if comp else ""
    if not text or len(text) > 24:
        continue
    here = actor.get_actor_location()
    print("TRAPPED sign %-14s at (%.0f,%.0f,%.0f) label=%s" % (
        text, here.x, here.y, here.z, actor.get_actor_label()))

# ---- what a new ceiling must NOT roof over -------------------------------
# The gap in the ceiling is the centre strip and the north band, and a plane
# laid blindly across either would cut through the escalators and cap the
# atrium. A department store's atrium is open to the roof on purpose - it is
# the one place the building shows you it has an upstairs - so the holes are
# part of the design, not an omission. These are their bounds.
print("TRAPPED --- openings a ceiling must respect ---")
for actor in actors:
    label = actor.get_actor_label()
    if not any(k in label.upper() for k in
               ("ESCALATOR", "ESC_", "ATRIUM", "VOID", "OPENING_WELL", "STAIR")):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if extent.x < 60.0 and extent.y < 60.0:
        continue
    print("TRAPPED opening %s x=%.0f..%.0f y=%.0f..%.0f z=%.0f..%.0f" % (
        label, centre.x - extent.x, centre.x + extent.x,
        centre.y - extent.y, centre.y + extent.y,
        centre.z - extent.z, centre.z + extent.z))
