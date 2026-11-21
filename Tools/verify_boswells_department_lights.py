"""Reload OpeningPass and check the added emergency route lighting survived.

This verifies authored lights and floor placement. Final visibility and
readability still need a rendered camera; Unreal properties cannot prove it.
"""

import csv
from pathlib import Path

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_DEPT_ROUTE_POOL_"
CSV = Path(unreal.Paths.project_dir()) / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_DEPT_LIGHT_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
require(len(by_label) == len(actors), "duplicate actor labels")
new = [(name, actor) for name, actor in by_label.items() if name.startswith(PREFIX)]
old = [(name, actor) for name, actor in by_label.items() if name.startswith("BSW_EGRESS_")]
require(len(new) == 16 and len(old) == 31, "route light inventory changed")
require(sum(name.startswith("BSW_DEPARTMENT_") and
            name.endswith("_AUTHORED_VISUAL") for name in by_label) == 48,
        "department fixtures missing")

with CSV.open(newline="", encoding="utf-8") as file:
    sockets = [row for row in csv.DictReader(file)
               if row["kind"] in {"shoe_display", "shoe_bench", "bed_vignette",
                                  "home_display", "feature_table", "toy_gondola",
                                  "sofa_vignette", "appliance_pod"}]
require(len(sockets) == 48, "fixture plan changed")

for name, actor in new:
    require(isinstance(actor, unreal.SpotLight), "new route is not a spot: " + name)
    loc = actor.get_actor_location()
    light = actor.spot_light_component
    require(light.get_editor_property("mobility") == unreal.ComponentMobility.MOVABLE and
            light.get_editor_property("intensity_units") == unreal.LightUnits.CANDELAS and
            light.get_editor_property("cast_shadows") and
            abs(light.get_editor_property("indirect_lighting_intensity") - 0.25) < 0.01,
            "route light mode changed: " + name)
    require(10 <= light.get_editor_property("intensity") <= 24 and
            abs(light.get_editor_property("attenuation_radius") - 950) < 0.01 and
            abs(light.get_editor_property("outer_cone_angle") - 50) < 0.01,
            "route light shape changed: " + name)
    require(abs(loc.z - (1320 if "UPPER" in name else 485)) < 1,
            "route spot escaped ceiling: " + name)
    # Each downlight must be in a walkable gap, not inside the merchandise
    # footprint. This catches a bad coordinate or future socket re-layout.
    floor = "upper" if "UPPER" in name else "lower"
    for socket in sockets:
        if socket["floor"] != floor:
            continue
        require(not (float(socket["xmin_cm"]) <= loc.x <= float(socket["xmax_cm"]) and
                     float(socket["ymin_cm"]) <= loc.y <= float(socket["ymax_cm"])),
                "spot placed inside display: " + name + " / " + socket["name"])

for name, actor in old:
    require(isinstance(actor, unreal.SpotLight) and
            actor.spot_light_component.get_editor_property("mobility") ==
            unreal.ComponentMobility.MOVABLE,
            "old route light reverted to unbuilt mobility: " + name)

require("BSW_OPENING_LANTERN" in by_label and
        "ROUTE_ESCALATOR_ASCEND" in by_label and
        "ANCHOR_LOADING_DOCK_EXIT" in by_label,
        "opening route changed")
print("BOSWELLS_DEPT_LIGHT_VERIFY PASSED: 16 department pools in walk gaps, "
      "31 inherited route spots movable, 48 fixtures and opening route intact")
