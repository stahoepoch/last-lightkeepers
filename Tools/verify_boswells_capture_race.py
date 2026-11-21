"""Check the fastest keeper cannot reach the capture's open grille.

Read the saved map, authored sprint and C++ timing in an independent editor
process. The path is an optimistic straight line through the counter and to
the very edge of the opening; real collision can only make arrival later.
"""

import json
import math
import re
from pathlib import Path

import unreal


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_CAPTURE_RACE " + why)


root = Path(unreal.Paths.project_dir())
data = json.loads((root / "data/lantern.json").read_text(encoding="utf-8"))
source = (root / "Source/LLK/Private/World/LLKOpeningGrille.cpp").read_text(
    encoding="utf-8")
header = (root / "Source/LLK/Public/World/LLKOpeningGrille.h").read_text(
    encoding="utf-8")
feel = (root / "Source/LLK/Private/Lantern/LLKLanternFeel.cpp").read_text(
    encoding="utf-8")
units = (root / "Source/LLK/Public/LLKUnits.h").read_text(encoding="utf-8")


def number(text, pattern):
    match = re.search(pattern, text)
    require(match is not None, "missing authored number: " + pattern)
    return float(match.group(1))


sprint = float(data["movement"]["sprintSpeed"]) * number(
    units, r"StudsToCm\s*=\s*([0-9.]+)f")
height = number(feel, r"KeeperHeightCm\s*=\s*([0-9.]+)f")
raise_cm = number(header, r"RaiseCm\s*=\s*([0-9.]+)f")
fall_start = number(source, r"CaptureFallStart\s*=\s*([0-9.]+)f")
fall_end = number(source, r"CaptureFallEnd\s*=\s*([0-9.]+)f")
haul_end = number(source, r"CaptureHaulEnd\s*=\s*([0-9.]+)f")
finish = number(source, r"CaptureFinish\s*=\s*([0-9.]+)f")
require(0 < height < raise_cm and 0 < fall_start < fall_end <= finish,
        "capture timing or height invalid")
require(haul_end <= fall_start, "people still in the bars when they descend")

world = unreal.EditorLoadingAndSavingUtils.load_map(
    "/Game/LLK_Boswells_OpeningPass")
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong saved map")
actors = {a.get_actor_label(): a for a in unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()}
start = actors["ANCHOR_PLAYERSTART_ENTRANCE"].get_actor_location()
grille = actors["ROUTE_MALL_GRILLE_22"].get_actor_location()
wall = actors["ROUTE_SOUTH_WALL_E"]
wall_center, wall_extent = wall.get_actor_bounds(False, False)
opening_edge_x = wall_center.x - wall_extent.x
require(850 < opening_edge_x < 950 and -2720 < grille.y < -2680,
        "opening geometry changed; recalculate race")

# Assume the keeper can sprint through the counter and cross at the wall's
# exact edge, even though their capsule actually needs more clearance.
minimum_distance = math.hypot(start.x - opening_edge_x, start.y - grille.y)
earliest_arrival = minimum_distance / sprint
last_passable = fall_start + (1.0 - height / raise_cm) * (fall_end - fall_start)
margin = earliest_arrival - last_passable
require(margin >= 0.15,
        "clean sprint reaches open bars: arrival %.3fs, passable until %.3fs" %
        (earliest_arrival, last_passable))
message = ("BOSWELLS_CAPTURE_RACE PASSED: optimistic sprint arrival %.3fs; "
           "bars too low by %.3fs; margin %.3fs" %
           (earliest_arrival, last_passable, margin))
unreal.log(message)
print(message)
