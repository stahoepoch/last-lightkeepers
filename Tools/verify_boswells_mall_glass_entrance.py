"""Trace the south mall glass, preserved shell, and unchanged lighting budget.

Run headlessly with the editor closed. Writes a result file in Saved/Temp so
another commandlet cannot overwrite the proof in the shared Unreal log.
"""

import json
from pathlib import Path

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("BOSWELLS_MALL_GLASS_VERIFY wrong map")
actors = {a.get_actor_label(): a for a in
          unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
results = {}


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_MALL_GLASS_VERIFY " + message)


def bounds(actor):
    center, extent = actor.get_actor_bounds(False)
    return (center.x - extent.x, center.x + extent.x,
            center.y - extent.y, center.y + extent.y,
            center.z - extent.z, center.z + extent.z)


def trace(name, start, end, expected):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*start), unreal.Vector(*end),
        unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
        unreal.DrawDebugTrace.NONE, True)
    actor = hit.to_dict()["hit_actor"] if hit else None
    label = actor.get_actor_label() if actor else None
    results[name] = label
    require(label == expected,
            "{} hit {}; expected {}".format(name, label, expected))


require("ROUTE_SOUTH_HEADER" not in actors, "old header still seals glass")
for name in ("ROUTE_SOUTH_LINTEL", "MALL_GLASS_ENTRANCE",
             "MALL_GLASS_RETURN_W", "MALL_GLASS_RETURN_E",
             "ROUTE_MALL_GRILLE_22", "ANCHOR_UPPER_SOUTH",
             "ANCHOR_FULL_ROOF", "ROUTE_SOUTH_WALL_W",
             "ROUTE_SOUTH_WALL_E"):
    require(name in actors, "shell part absent: " + name)
require(all(abs(a - b) < 1 for a, b in zip(
    bounds(actors["ROUTE_SOUTH_LINTEL"]),
    (-900, 900, -2720, -2680, 1468, 1500))), "lintel bounds drifted")
require(all(abs(a - b) < 1 for a, b in zip(
    bounds(actors["MALL_GLASS_ENTRANCE"]),
    (-900, 900, -2880, -2876, 280, 1468))), "front glass bounds drifted")
for name, expected in (
        ("ROUTE_SOUTH_WALL_W", (-4370, -900, -2720, -2680, 0, 1500)),
        ("ROUTE_SOUTH_WALL_E", (900, 4370, -2720, -2680, 0, 1500)),
        ("ANCHOR_UPPER_SOUTH", (-4350, 4350, -2700, -900,
                                637.5, 662.5))):
    require(all(abs(a - b) < 1 for a, b in zip(bounds(actors[name]), expected)),
            "shell bounds drifted: " + name)
for name in ("MALL_GLASS_ENTRANCE", "MALL_GLASS_RETURN_W",
             "MALL_GLASS_RETURN_E"):
    require(actors[name].get_actor_enable_collision(),
            "glazing no longer blocks movement: " + name)
require(sum(n.startswith("MALL_GLASS_") for n in actors) == 15,
        "entrance actor count differs")
require(sum(n.startswith("ROUTE_MALL_GRILLE_") for n in actors) == 45,
        "night grille bar inventory changed")
require(not any(n.startswith("LOT_GLASS_") for n in actors),
        "lot frontage was changed in this pass")
glass = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_MallEntranceGlass")
teal = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_MallMullionTeal")
require(isinstance(glass, unreal.Material) and
        glass.get_editor_property("blend_mode") == unreal.BlendMode.BLEND_TRANSLUCENT and
        isinstance(teal, unreal.Material), "entrance materials changed")
for name, actor in actors.items():
    if name.startswith("MALL_GLASS_") or name == "ROUTE_SOUTH_LINTEL":
        require(not actor.get_components_by_class(unreal.LightComponent),
                "entrance acquired a light component: " + name)
        expected = glass if name in ("MALL_GLASS_ENTRANCE",
                                     "MALL_GLASS_RETURN_W",
                                     "MALL_GLASS_RETURN_E") else teal
        require(actor.static_mesh_component.get_material(0) == expected,
                "entrance material changed: " + name)

# Three direct rays from the inside of the glazed face to the rod stack.
for height in (1120, 1280, 1400):
    trace("inside_to_rods_{}".format(height), (0, -2670, height),
          (0, 0, height), None)
trace("outside_through_glass", (0, -3000, 1280),
      (0, 0, 1280), "MALL_GLASS_ENTRANCE")
trace("upper_walk_to_glass", (0, -1500, 814.5),
      (0, -3000, 1280), "MALL_GLASS_ENTRANCE")
trace("upper_walk_to_rods", (0, -1500, 814.5),
      (0, 0, 1280), None)
trace("lower_walk_to_grille", (0, -2600, 100),
      (0, -2800, 100), "ROUTE_MALL_GRILLE_22")

# These three negatives catch the wrong shortcut: cutting the upper floor,
# roof, or grille to manufacture a line of sight.
trace("lower_glass_to_rods_blocked", (0, -2680, 280),
      (0, 0, 1100), "ANCHOR_UPPER_SOUTH")
trace("sky_still_blocked", (0, 0, 2000),
      (0, 0, 1200), "ANCHOR_FULL_ROOF")
trace("grille_still_closed", (0, -2600, 100),
      (0, -2800, 100), "ROUTE_MALL_GRILLE_22")

roof = bounds(actors["ANCHOR_FULL_ROOF"])
require(abs(roof[4] - 1487.5) < 1, "roof underside changed")
canopy = {n: a for n, a in actors.items() if n.startswith("BSW_CANOPY_GLASS_")}
require(len(canopy) == 24, "mirror canopy count changed")
mirror = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_SmokedMirror")
require(isinstance(mirror, unreal.MaterialInterface), "mirror material missing")
for name, actor in canopy.items():
    require(abs(actor.get_actor_location().z - 1472) < 1,
            "canopy pane moved: " + name)
    require(actor.static_mesh_component.get_material(0) == mirror,
            "canopy pane became exterior glass: " + name)

volumes = [a for a in actors.values() if isinstance(a, unreal.PostProcessVolume)]
require(len(volumes) == 1, "exposure volume inventory changed")
volume = volumes[0]
settings = volume.get_editor_property("settings")
require(volume.get_editor_property("unbound") and
        abs(settings.get_editor_property("auto_exposure_min_brightness") - .5) < .001 and
        abs(settings.get_editor_property("auto_exposure_max_brightness") - .5) < .001,
        "fixed exposure changed")

for prefix, expected_count in (("BSW_EGRESS_", 31),
                               ("BSW_KIDS_CEILING_FILL_", 6)):
    lights = {n: a for n, a in actors.items() if n.startswith(prefix)}
    require(len(lights) == expected_count,
            "light inventory changed: " + prefix)
    for name, actor in lights.items():
        comp = actor.get_component_by_class(unreal.LightComponent)
        require(comp is not None and
                abs(comp.get_editor_property("intensity") - 8) < .001 and
                abs(comp.get_editor_property("attenuation_radius") - 1050) < 1,
                "emergency fixture changed: " + name)

results["canopy_panes"] = len(canopy)
results["egress_lights"] = 31
results["kids_fill_lights"] = 6
results["exposure"] = [.5, .5]
out = (Path(__file__).resolve().parent.parent / "Saved" / "Temp" /
       "verify_boswells_mall_glass_entrance.json")
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(results, indent=2), encoding="utf-8")
message = "BOSWELLS_MALL_GLASS_VERIFY PASSED: 10 traces, sealed roof and grille, unchanged light budget"
unreal.log(message)
print(message)
