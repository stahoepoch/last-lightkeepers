"""Fresh-process proof that the witnessed opening is staged in the saved map.

This checks authored actors and their collision, not player perception. The
timed haul and voice still require a real -game launch and a visual pass.
"""

import unreal


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_CAPTURE_VERIFY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(
    "/Game/LLK_Boswells_OpeningPass")
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
labels = {a.get_actor_label(): a for a in actors}
require(len(labels) == len(actors), "duplicate actor label")
capture = [a for a in actors if a.get_actor_label().startswith("BSW_CAPTURE_")]
require(len(capture) == 7, "capture count changed")


def tagged(tag):
    return [a for a in actors if tag in [str(t) for t in a.get_editor_property("tags")]]


people = tagged("LLK.Opening.Captive")
machine = tagged("LLK.Opening.Captor")
storm = tagged("LLK.Opening.StormFlash")
require(len(people) == 4 and len(machine) == 1 and len(storm) == 1,
        "scene tags changed")
require(len(tagged("LLK.Opening.FallenLantern")) == 1 and
        tagged("LLK.Opening.FallenLantern")[0].get_actor_label() ==
        "BSW_OPENING_LANTERN", "Dad's timed lantern is not tagged")
require(all(isinstance(p, unreal.SkeletalMeshActor) for p in people),
        "a captive is not skeletal")
mesh = unreal.EditorAssetLibrary.load_asset(
    "/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin")
walk = unreal.EditorAssetLibrary.load_asset(
    "/Game/Characters/UEFN_Mannequin/Animations/LookAtPOI/LookAtPOI_test_walk_loop")
require(isinstance(mesh, unreal.SkeletalMesh) and isinstance(walk, unreal.AnimSequence),
        "walk asset or human mesh missing")
require(mesh.get_editor_property("skeleton").get_path_name() ==
        walk.get_editor_property("skeleton").get_path_name(),
        "walk and mesh skeletons diverged")
for person in people:
    body = person.get_editor_property("skeletal_mesh_component")
    require(body.get_editor_property("skeletal_mesh_asset") == mesh,
            "human mesh detached: " + person.get_actor_label())
    require(body.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION,
            "person blocks the child: " + person.get_actor_label())
require(isinstance(machine[0], unreal.StaticMeshActor) and
        machine[0].static_mesh_component.get_collision_enabled() ==
        unreal.CollisionEnabled.NO_COLLISION,
        "hauler blocks the route")
require(machine[0].static_mesh_component.get_editor_property("mobility") ==
        unreal.ComponentMobility.MOVABLE,
        "hauler cannot follow the captured people")
rover_mesh = machine[0].static_mesh_component.get_editor_property("static_mesh")
require(rover_mesh is not None and rover_mesh.get_path_name() ==
        "/Game/LLK/Boswells/Fixtures/SM_BSW_OpeningHauler.SM_BSW_OpeningHauler",
        "hauler reverted to a primitive")
rover_extent = rover_mesh.get_bounds().box_extent
require(machine[0].get_actor_location().x + rover_extent.x < 900.0,
        "rover intersects the east wall beside the grille")
require(-2780.0 - rover_extent.y > -2880.0,
        "rover's final nose intersects the exterior glass")
require(isinstance(storm[0], unreal.PointLight), "storm source wrong class")
lamp = storm[0].get_editor_property("point_light_component")
require(abs(lamp.get_editor_property("intensity")) < 0.01,
        "storm flash lights inspection detail at rest")
pool = labels.get("BSW_CAPTURE_WITNESS_POOL")
require(isinstance(pool, unreal.PointLight), "witness pool absent")
pool_lamp = pool.get_editor_property("point_light_component")
require(abs(pool_lamp.get_editor_property("intensity") - 6.0) < 0.01 and
        abs(pool_lamp.get_editor_property("attenuation_radius") - 540.0) < 0.1,
        "witness pool changed")
sound = unreal.EditorAssetLibrary.load_asset(
    "/Game/Audio/Opening/opening-sayers-machine-v1")
require(isinstance(sound, unreal.SoundWave), "machine call missing")

message = "BOSWELLS_CAPTURE_VERIFY PASSED: four skinned people, compatible walk, no route collision, six-cd witness pool, silent storm at rest, timed lantern, machine call"
unreal.log(message)
print(message)
