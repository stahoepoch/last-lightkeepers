"""Make the witnessed family legible from the child's starting cover.

The first runtime pass executed but disappeared into a black grille. This
adds a narrow emergency pool on the people, not on the release witness mark,
and tags the lantern so it appears when Dad drops it.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
LABEL = "BSW_CAPTURE_WITNESS_POOL"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
assert world is not None and world.get_name() == "LLK_Boswells_OpeningPass"
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
pickup = actors["BSW_OPENING_LANTERN"]
pickup.set_editor_property("tags", [unreal.Name("LLK.Opening.FallenLantern")])

positions = {
    "DAD": (780, -2460, 4),
    "BROTHER": (650, -2530, 4),
    "OTHER_A": (560, -2360, 4),
    "OTHER_B": (470, -2495, 4),
}
for name, xyz in positions.items():
    assert actors["BSW_CAPTURE_" + name].set_actor_location(
        unreal.Vector(*xyz), False, False)
machine = actors["BSW_CAPTURE_HAUL_MACHINE"]
mesh = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Fixtures/SM_BSW_OpeningHauler")
assert isinstance(mesh, unreal.StaticMesh)
if machine.static_mesh_component.get_editor_property("static_mesh") != mesh:
    assert machine.static_mesh_component.set_static_mesh(mesh)
machine.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
assert machine.set_actor_location(unreal.Vector(810, -2600, 0), False, False)
machine.set_actor_scale3d(unreal.Vector(1, 1, 1))

cream = unreal.EditorAssetLibrary.load_asset(
    "/Game/Props/Boswells/Materials/cream")
assert isinstance(cream, unreal.MaterialInterface)
# A single light coat catches the emergency pool; Dad's red coat stays red.
other = actors["BSW_CAPTURE_OTHER_A"]
body = other.get_editor_property("skeletal_mesh_component")
for slot in range(body.get_num_materials()):
    body.set_material(slot, cream)

pool = actors.get(LABEL) or sub.spawn_actor_from_class(
    unreal.PointLight, unreal.Vector(650, -2470, 285), unreal.Rotator())
assert pool is not None
pool.set_actor_label(LABEL)
pool.set_folder_path("Boswells/Opening/Capture")
lamp = pool.get_editor_property("point_light_component")
lamp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
lamp.set_editor_property("intensity", 6.0)
lamp.set_editor_property("attenuation_radius", 540.0)
lamp.set_editor_property("source_radius", 40.0)
lamp.set_editor_property("indirect_lighting_intensity", 0.25)
lamp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

assert unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
unreal.log("BOSWELLS_CAPTURE_READ SAVED: one 6 cd witness pool and delayed lantern")
print("BOSWELLS_CAPTURE_READ SAVED")
