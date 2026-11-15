"""Build a first dressed Boswell's map from the preserved route plate.

Runs headlessly with Tools/run_editor_python.ps1. Creates a new map and refuses
to overwrite it. Replaces only six old department placeholder solids with the
authored apparel fixtures named in BoswellsFullStore_Sockets.csv. Gameplay
actors, slopes, grille, service doors, and dock route carry through unchanged.
"""

import csv
from pathlib import Path

import unreal


SOURCE = "/Game/LLK_Boswells_RoutePass"
TARGET = "/Game/LLK_Boswells_ApparelPass"
KIT = "/Game/LLK/Boswells/Fixtures/"
CSV = Path(unreal.Paths.project_dir()) / "docs/reference/boswells/BoswellsFullStore_Sockets.csv"
MESHES = {
    "clothing_run": ("SM_BSW_ClothingRun", 1.0),
    "four_way": ("SM_BSW_FourWay", 0.78),
    "round_rack": ("SM_BSW_RoundRack", 1.0),
}
OLD_BLOCKS = (
    "DEPT_LADIES_WEST", "DEPT_LADIES_WEST_N", "DEPT_JUNIORS",
    "DEPT_TEENS", "DEPT_ELECTRONICS", "DEPT_HOME_PREVIEW",
    "DEPT_SHOES_EAST",
)


def log(message):
    unreal.log("BOSWELLS_APPAREL " + message)


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_APPAREL " + message)


def load_sockets(meshes):
    with CSV.open(newline="", encoding="utf-8") as file:
        rows = [row for row in csv.DictReader(file)
                if row["floor"] == "lower"
                and row["department"] in ("juniors", "ladies", "mens")
                and row["kind"] in MESHES]
    require(len(rows) == 36, "expected 36 apparel sockets, got " + str(len(rows)))
    require(len({row["name"] for row in rows}) == len(rows), "duplicate socket names")
    for row in rows:
        mesh, scale = meshes[row["kind"]]
        bounds = mesh.get_bounds().box_extent
        width = float(row["xmax_cm"]) - float(row["xmin_cm"])
        length = float(row["ymax_cm"]) - float(row["ymin_cm"])
        require(2 * bounds.x * scale <= width + 0.1 and
                2 * bounds.y * scale <= length + 0.1,
                "fixture overhangs " + row["name"])
    return rows


def main():
    require(unreal.EditorAssetLibrary.does_asset_exist(SOURCE), "source map missing")
    require(not unreal.EditorAssetLibrary.does_asset_exist(TARGET),
            "target map already exists; refusing overwrite")
    meshes = {}
    for kind, (name, scale) in MESHES.items():
        asset = unreal.EditorAssetLibrary.load_asset(KIT + name)
        require(isinstance(asset, unreal.StaticMesh), "missing fixture " + name)
        meshes[kind] = (asset, scale)
    rows = load_sockets(meshes)

    require(unreal.EditorAssetLibrary.duplicate_asset(SOURCE, TARGET) is not None,
            "map duplication failed")
    world = unreal.EditorLoadingAndSavingUtils.load_map(TARGET)
    require(world is not None and world.get_name() == "LLK_Boswells_ApparelPass",
            "duplicate did not load")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    old = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
    require(all(label in old for label in OLD_BLOCKS),
            "source placeholders differ; abort before editing")
    require("ROUTE_ESCALATOR_ASCEND" in old and
            "ANCHOR_LOADING_DOCK_EXIT" in old,
            "route or dock missing")

    for label in OLD_BLOCKS:
        require(actors.destroy_actor(old[label]), "could not remove " + label)
    log("removed {} old department blocks in duplicate only".format(len(OLD_BLOCKS)))

    for row in rows:
        mesh, scale = meshes[row["kind"]]
        x = (float(row["xmin_cm"]) + float(row["xmax_cm"])) / 2.0
        y = (float(row["ymin_cm"]) + float(row["ymax_cm"])) / 2.0
        z = float(row["floor_z_cm"])
        name = "BSW_APPAREL_" + row["name"]
        actor = actors.spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
        require(actor is not None, "spawn failed " + name)
        actor.set_actor_label(name)
        require(actor.static_mesh_component.set_static_mesh(mesh),
                "mesh assignment failed " + name)
        actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
        actor.set_folder_path("Boswells/Apparel/" + row["department"])

    require(unreal.EditorLoadingAndSavingUtils.save_map(world, TARGET),
            "map save failed")
    labels = {actor.get_actor_label() for actor in actors.get_all_level_actors()}
    require(sum(label.startswith("BSW_APPAREL_") for label in labels) == 36,
            "saved world missing apparel fixtures")
    require("ROUTE_ESCALATOR_ASCEND" in labels and
            "ANCHOR_LOADING_DOCK_EXIT" in labels,
            "saved world lost route or dock")
    log("SAVED {} with 36 apparel fixtures and {} actors".format(
        TARGET, len(labels)))


main()
