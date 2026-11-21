"""Stage four moving silhouettes and a haul machine for the opening grille.

The map's existing ALLKOpeningGrille drives these by tag at runtime. People
and machine have no collision, cannot carve nav, and do not become enemies.
Run with Tools/run_editor_python.ps1 while the editor is closed.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_CAPTURE_"


def require(ok, reason):
    if not ok:
        raise RuntimeError("BOSWELLS_CAPTURE " + reason)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
require(not any(name.startswith(PREFIX) for name in before),
        "capture already staged")
require("BSW_OPENING_GRILLE_CONTROLLER" in before, "grille controller absent")
mesh = unreal.EditorAssetLibrary.load_asset(
    "/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
walk = unreal.EditorAssetLibrary.load_asset(
    "/Game/Characters/UEFN_Mannequin/Animations/LookAtPOI/LookAtPOI_test_walk_loop")
require(isinstance(mesh, unreal.SkeletalMesh) and isinstance(walk, unreal.AnimSequence),
        "compatible mannequin and walk sequence required")
require(mesh.get_editor_property("skeleton").get_path_name() ==
        walk.get_editor_property("skeleton").get_path_name(),
        "walk sequence does not match mannequin")
materials = {
    "dark": unreal.EditorAssetLibrary.load_asset(
        "/Game/Props/Boswells/Materials/charcoal"),
    "coat": unreal.EditorAssetLibrary.load_asset(
        "/Game/Props/Boswells/Materials/burgundy"),
    "machine": unreal.EditorAssetLibrary.load_asset(
        "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome"),
}
require(all(isinstance(m, unreal.MaterialInterface) for m in materials.values()),
        "missing silhouette materials")

positions = (
    ("DAD", 780, -2460, 0.94, "coat"),
    ("BROTHER", 650, -2530, 0.87, "dark"),
    ("OTHER_A", 560, -2360, 0.82, "dark"),
    ("OTHER_B", 470, -2495, 0.80, "coat"),
)
for name, x, y, scale, material in positions:
    rotation = unreal.Rotator()
    rotation.yaw = -90.0
    actor = sub.spawn_actor_from_class(
        unreal.SkeletalMeshActor, unreal.Vector(x, y, 4), rotation)
    require(actor is not None, "failed to spawn " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Opening/Capture")
    actor.set_editor_property("tags", [unreal.Name("LLK.Opening.Captive")])
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    body = actor.get_editor_property("skeletal_mesh_component")
    body.set_skeletal_mesh_asset(mesh)
    for slot in range(body.get_num_materials()):
        body.set_material(slot, materials[material])
    actor.set_actor_enable_collision(False)
    body.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    body.set_editor_property("can_ever_affect_navigation", False)

# A distant, low stock rover silhouette. It is deliberately a presentation
# actor; the live Sweeper remains elsewhere in the store and keeps its AI.
machine_mesh = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Fixtures/SM_BSW_OpeningHauler")
require(isinstance(machine_mesh, unreal.StaticMesh), "opening hauler mesh absent")
machine = sub.spawn_actor_from_class(
    unreal.StaticMeshActor, unreal.Vector(810, -2600, 0), unreal.Rotator())
require(machine is not None, "failed to spawn machine silhouette")
machine.set_actor_label(PREFIX + "HAUL_MACHINE")
machine.set_folder_path("Boswells/Opening/Capture")
machine.set_editor_property("tags", [unreal.Name("LLK.Opening.Captor")])
machine.static_mesh_component.set_static_mesh(machine_mesh)
machine.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
machine.static_mesh_component.set_material(0, materials["machine"])
machine.set_actor_enable_collision(False)
machine.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
machine.static_mesh_component.set_editor_property("can_ever_affect_navigation", False)

# The lightning source sits outside the south entrance glazing. At rest it
# contributes zero; the grille actor pulses it during the four-second grab.
flash = sub.spawn_actor_from_class(
    unreal.PointLight, unreal.Vector(650, -2820, 380), unreal.Rotator())
require(flash is not None, "failed to spawn storm flash")
flash.set_actor_label(PREFIX + "STORM_FLASH")
flash.set_folder_path("Boswells/Opening/Capture")
flash.set_editor_property("tags", [unreal.Name("LLK.Opening.StormFlash")])
lamp = flash.get_editor_property("point_light_component")
lamp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
lamp.set_editor_property("intensity", 0.0)
lamp.set_editor_property("attenuation_radius", 1600.0)
lamp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

after = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
require(sum(name.startswith(PREFIX) for name in after) == 6,
        "capture cast count wrong")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
unreal.log("BOSWELLS_CAPTURE SAVED: four animated silhouettes, one rover, one storm flash")
print("BOSWELLS_CAPTURE SAVED")
