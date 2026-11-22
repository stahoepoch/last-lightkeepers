"""Read only: a census of the opening map, asked the way a shopper would.

Not "does it boot" - every gate already answers that. This asks whether the
thing a player walks into reads as a department store: what is on the floor,
what is above their head, how much of the floor area has anything in it at
all, and whether the surfaces under their feet change when they move from a
walkway into a department.

Chris, 2026-11-22: "the department store doesn't look like a department
store. It's empty... there's walkways that are linoleum or ceramic... and
then there's carpeted areas."

Prints LogPython lines prefixed CENSUS so they can be grepped out of
Saved/Logs/LastLightkeepers.log - unreal.log and print() do not reach stdout
from the pythonscript commandlet.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("CENSUS map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()

print("CENSUS actors_total=%d" % len(actors))


def prefix(label):
    """Group by the leading token, which is how this project names actors."""
    for sep in ("_", " "):
        if sep in label:
            return label.split(sep)[0].upper()
    return label.upper()


# ---- 1. what is in here, by family -------------------------------------
groups = {}
for actor in actors:
    groups.setdefault(prefix(actor.get_actor_label()), []).append(actor)

for name in sorted(groups, key=lambda k: -len(groups[k])):
    print("CENSUS group %s=%d" % (name, len(groups[name])))

# ---- 2. classes, because labels lie ------------------------------------
classes = {}
for actor in actors:
    classes.setdefault(actor.get_class().get_name(), 0)
    classes[actor.get_class().get_name()] += 1
for name in sorted(classes, key=lambda k: -classes[k]):
    print("CENSUS class %s=%d" % (name, classes[name]))

# ---- 3. the floor, and what it is made of ------------------------------
#
# The question Chris asked twice. A single material over the whole slab is a
# warehouse; a racetrack of hard walkway with carpeted departments either
# side is a store.
materials = {}
floor_area_cm2 = 0.0
lowest = None

for actor in actors:
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        mats = comp.get_materials()
        center, extent = actor.get_actor_bounds(False)
        flat = extent.z <= 60.0 and extent.x > 150.0 and extent.y > 150.0
        for mat in mats:
            if mat is None:
                materials.setdefault("<none>", 0)
                materials["<none>"] += 1
                continue
            key = mat.get_name()
            materials.setdefault(key, 0)
            materials[key] += 1
        if flat and center.z < 200.0:
            floor_area_cm2 += (extent.x * 2.0) * (extent.y * 2.0)
            if lowest is None or center.z < lowest:
                lowest = center.z

for name in sorted(materials, key=lambda k: -materials[k]):
    print("CENSUS material %s=%d" % (name, materials[name]))
print("CENSUS flat_floor_area_m2=%.0f lowest_z=%s" % (
    floor_area_cm2 / 10000.0, "%.0f" % lowest if lowest is not None else "none"))

# ---- 4. is there anything over their head ------------------------------
ceiling = [a for a in actors
           if "CEIL" in a.get_actor_label().upper()
           or "ROOF" in a.get_actor_label().upper()
           or "SOFFIT" in a.get_actor_label().upper()]
print("CENSUS ceiling_pieces=%d %s" % (
    len(ceiling), [a.get_actor_label() for a in ceiling][:12]))

# ---- 5. light ----------------------------------------------------------
lights = [a for a in actors if isinstance(a, unreal.Light)]
by_kind = {}
for light in lights:
    by_kind.setdefault(light.get_class().get_name(), 0)
    by_kind[light.get_class().get_name()] += 1
print("CENSUS lights_total=%d %s" % (len(lights), by_kind))

# ---- 6. the store's own overall footprint ------------------------------
#
# Density is the number Chris is reacting to: fixtures per square metre of
# sales floor. Reported rather than judged, with the extent so the figure
# can be argued with.
min_x = min_y = 1e9
max_x = max_y = -1e9
for actor in actors:
    center, extent = actor.get_actor_bounds(False)
    if extent.x > 1e6 or extent.y > 1e6:
        continue
    min_x = min(min_x, center.x - extent.x)
    max_x = max(max_x, center.x + extent.x)
    min_y = min(min_y, center.y - extent.y)
    max_y = max(max_y, center.y + extent.y)

span_m2 = ((max_x - min_x) / 100.0) * ((max_y - min_y) / 100.0)
print("CENSUS footprint_m=%.0fx%.0f span_m2=%.0f" % (
    (max_x - min_x) / 100.0, (max_y - min_y) / 100.0, span_m2))
print("CENSUS actors_per_100m2=%.1f" % (len(actors) / max(span_m2, 1.0) * 100.0))
