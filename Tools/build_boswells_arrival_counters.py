"""Dress the lower store arrival without occupying its front promenade.

The four original counter blocks obstruct the reserved y=-2350..-2050 route.
Replace them at the four entrance sockets in BoswellsFullStore_Sockets.csv.
Only these blocks are removed. Run with Unreal Editor closed.
"""

import csv
from pathlib import Path

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_ARRIVAL_"
CSV = Path(unreal.Paths.project_dir()) / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"
ORIGINAL = {
    "DEPT_CHECKOUT_WEST": (-3500, -3100, -2290, -2210, 0, 100),
    "DEPT_CHECKOUT_EAST": (3100, 3500, -2290, -2210, 0, 100),
    "DEPT_COSMETICS_COUNTER": (-1500, -900, -2100, -2000, 0, 95),
    "DEPT_JEWELRY_CASE": (900, 1500, -2090, -2010, 0, 95),
}
SOCKETS = {
    "CHECKOUT_W": (-2100, -2600, -1500, -2400),
    "CHECKOUT_E": (1500, -2600, 2100, -2400),
    "PERFUME_CASE": (-3800, -2600, -2800, -2400),
    "JEWELRY_CASE": (2800, -2600, 3800, -2400),
}


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_ARRIVAL " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before), "pass already exists")
require(all(label in before for label in ORIGINAL), "old counters changed")
require(all(label in before for label in (
    "MALL_ENTRANCE_APPROACH_FLOOR", "ROUTE_MALL_GRILLE_22",
    "ROUTE_ESCALATOR_ASCEND", "ANCHOR_LOADING_DOCK_EXIT")),
    "route or entrance changed")
for label, expected in ORIGINAL.items():
    centre, extent = before[label].get_actor_bounds(False)
    actual = (centre.x - extent.x, centre.x + extent.x,
              centre.y - extent.y, centre.y + extent.y,
              centre.z - extent.z, centre.z + extent.z)
    require(all(abs(a - b) < 1 for a, b in zip(actual, expected)),
            "old bounds changed for " + label)

with CSV.open(newline="", encoding="utf-8") as file:
    rows = {r["name"]: r for r in csv.DictReader(file) if r["floor"] == "lower"}
for name, expected in SOCKETS.items():
    require(name in rows, "socket missing: " + name)
    row = rows[name]
    actual = tuple(float(row[k]) for k in ("xmin_cm", "ymin_cm", "xmax_cm", "ymax_cm"))
    require(actual == expected, "socket moved: " + name)

counter = unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/SM_BSW_ServiceCounter")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
glass = unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_MallEntranceGlass")
wood = unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/dark_wood")
chrome = unreal.EditorAssetLibrary.load_asset("/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome")
stock_colors = [unreal.EditorAssetLibrary.load_asset(
    "/Game/Props/Boswells/" + name) for name in ("rose", "butter", "mint", "brass")]
require(isinstance(counter, unreal.StaticMesh) and isinstance(cube, unreal.StaticMesh),
        "fixture mesh missing")
require(all(isinstance(m, unreal.MaterialInterface) for m in (glass, wood, chrome)),
        "case material missing")
require(all(isinstance(m, unreal.MaterialInterface) for m in stock_colors),
        "stock material missing")
extent = counter.get_bounds().box_extent
origin = counter.get_bounds().origin
require(abs(extent.x - 93) < 1 and abs(extent.y - 35.5) < 1 and abs(extent.z - 51) < 1,
        "service counter changed scale")
counter_ground_z = extent.z - origin.z

for label in ORIGINAL:
    require(subsystem.destroy_actor(before[label]), "could not remove " + label)


def make(label, mesh, x, y, z, folder, size=None, material=None, collision=True):
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
    require(actor is not None, "spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Arrival/" + folder)
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(mesh), "mesh set failed: " + label)
    if size:
        actor.set_actor_scale3d(unreal.Vector(*(dimension / 100 for dimension in size)))
    if material:
        comp.set_material(0, material)
    actor.set_actor_enable_collision(collision)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                               if collision else unreal.CollisionEnabled.NO_COLLISION)
    if not collision and hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    return actor


# Three 186 cm counters span each 600 cm checkout socket. Their real mesh
# collision is narrower than the socket, leaving a 114 cm band to the tiled
# promenade and the central entry entirely open.
for side, xs in (("WEST", (-2000, -1800, -1600)),
                 ("EAST", (1600, 1800, 2000))):
    for index, x in enumerate(xs):
        make("CHECKOUT_{}_{}".format(side, index), counter, x, -2500,
             counter_ground_z,
             "Checkouts")

# Low wood plinths, glass hoods, dark chrome edge bars, and small merchandise
# silhouettes. Glass hoods collide so the player and Sweeper cannot walk
# through a case. Contents and trim are visual only, never nav carvers.
for kind, xs in (("PERFUME", (-3690, -3500, -3310, -3120, -2930)),
                 ("JEWELRY", (2930, 3120, 3310, 3500, 3690))):
    for index, x in enumerate(xs):
        stem = "{}_{}".format(kind, index)
        make(stem + "_PLINTH", cube, x, -2500, 30, kind,
             (186, 90, 60), wood)
        make(stem + "_GLASS", cube, x, -2500, 79, kind,
             (186, 90, 38), glass)
        for edge_y, suffix in ((-2546, "S"), (-2454, "N")):
            make(stem + "_EDGE_" + suffix, cube, x, edge_y, 100, kind,
                 (186, 3, 3), chrome, collision=False)
        for slot, offset in enumerate((-52, 0, 52)):
            # One or two short vertical bottles/pedestals per case. A player
            # sees merchandise behind the glazed band rather than an empty box.
            height = 14 + ((index + slot) % 3) * 5
            make(stem + "_GOODS_{}".format(slot), cube, x + offset, -2500,
                 60 + height / 2, kind, (12, 12, height),
                 stock_colors[(index + slot) % len(stock_colors)],
                 collision=False)

after = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require(not any(label in after for label in ORIGINAL), "old block survived")
require(sum(label.startswith(PREFIX) for label in after) == 76,
        "arrival actor count differs")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
message = "BOSWELLS_ARRIVAL SAVED: 6 checkout modules, 10 stocked glass cases; promenade clear"
unreal.log(message)
print(message)
