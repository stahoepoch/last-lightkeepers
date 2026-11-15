"""Import the two Boswell's cover fixtures into Unreal and make them actually
block sight.

Run inside the editor:
    py "<repo>/assets/models/boswells/import_fixtures.py"

It imports SM_BSW_FoldedGoodsTable and SM_BSW_ServiceCounter into
/Game/Props/Boswells/, gives each a simple box collider, sets the body setup to
a profile that blocks the Visibility channel, and builds four material
instances. **It does not open, edit or save any map** - placement is Codex's,
and `LLK_Boswells_ApparelPass` is not touched.

WHY COLLISION IS THE WHOLE JOB
------------------------------
`docs/FINDINGS-KIDS-FLOOR-STEALTH.md`: the sight trace runs on **ECC_Visibility,
not ECC_WorldStatic**, and *"a fixture that does not block that channel is not
cover no matter how solid it looks."* A beautifully modelled counter with
collision set to QueryOnly-Pawn is scenery. That is the single way these two
assets can look finished and do nothing, so it is checked here and asserted at
the end rather than assumed.

NOT RUN YET
-----------
This has not been executed: there is no editor on the bridge that wrote it.
The geometry it imports is verified (`make_fixture_kit.py` refuses to write a
fixture that breaks the cover rule) but the import itself is unproven. Treat a
clean run as the first evidence, not this comment.
"""

import unreal

PACKAGE_PATH = "/Game/Props/Boswells"
SOURCE_DIR = unreal.Paths.project_dir() + "../assets/models/boswells/"

FIXTURES = [
    ("SM_BSW_FoldedGoodsTable", 80.0),
    ("SM_BSW_ServiceCounter", 102.0),
]

# Palette slots the OBJs reference, mapped to what each must read as. Flat
# blockout colour is what these replace, so each gets roughness and a tint
# rather than a single base colour.
MATERIALS = {
    "MI_BSW_LaminateTop":   {"BaseColor": (0.26, 0.19, 0.14), "Roughness": 0.28, "Metallic": 0.0},
    "MI_BSW_PaintedMetal":  {"BaseColor": (0.55, 0.57, 0.54), "Roughness": 0.46, "Metallic": 0.35},
    "MI_BSW_CarcassPanel":  {"BaseColor": (0.60, 0.55, 0.45), "Roughness": 0.62, "Metallic": 0.0},
    "MI_BSW_FoldedStock":   {"BaseColor": (0.24, 0.31, 0.43), "Roughness": 0.88, "Metallic": 0.0},
}


def import_mesh(stem):
    task = unreal.AssetImportTask()
    task.filename = SOURCE_DIR + stem + ".obj"
    task.destination_path = PACKAGE_PATH
    task.destination_name = stem
    task.automated = True
    task.replace_existing = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    mesh = unreal.load_asset(f"{PACKAGE_PATH}/{stem}")
    if mesh is None:
        raise RuntimeError(f"{stem} did not import; nothing else below is true")
    return mesh


def give_it_collision(mesh, stem):
    """Simple box collision, then the profile that makes it cover.

    Box rather than complex-as-simple on purpose: these are convex fixtures,
    a box is the cheapest thing a sight trace can hit, and complex collision
    is not used by line traces against a mesh set to simple.
    """
    subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    subsystem.remove_collisions(mesh)
    subsystem.add_simple_collisions(mesh, unreal.ScriptingCollisionShapeType.BOX)

    body = mesh.get_editor_property("body_setup")
    body.set_editor_property("collision_trace_flag",
                             unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AND_COMPLEX)
    # The profile placed components inherit. BlockAll blocks Visibility, which
    # is the channel the sight trace actually uses.
    default_instance = body.get_editor_property("default_instance")
    default_instance.set_collision_profile_name("BlockAll")
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)


def build_materials():
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    parent = unreal.load_asset("/Engine/BasicShapes/BasicShapeMaterial")
    made = []
    for name, params in MATERIALS.items():
        path = f"{PACKAGE_PATH}/Materials/{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            made.append(path)
            continue
        factory = unreal.MaterialInstanceConstantFactoryNew()
        instance = tools.create_asset(name, f"{PACKAGE_PATH}/Materials",
                                      unreal.MaterialInstanceConstant, factory)
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, parent)
        colour = params["BaseColor"]
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            instance, "Color", unreal.LinearColor(colour[0], colour[1], colour[2], 1.0))
        for scalar in ("Roughness", "Metallic"):
            unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
                instance, scalar, params[scalar])
        unreal.EditorAssetLibrary.save_loaded_asset(instance)
        made.append(path)
    return made


def assert_it_is_cover(mesh, stem, expected_top_cm):
    """The two things that make these assets worth importing, checked rather
    than hoped for: the silhouette is the height it was authored at, and the
    collider blocks the channel sight is traced on."""
    problems = []

    bounds = mesh.get_bounds()
    top = bounds.box_extent.z * 2.0
    if abs(top - expected_top_cm) > 1.0:
        problems.append(f"{stem} imported {top:.1f} cm tall, authored "
                        f"{expected_top_cm:.1f}. Check the import scale - a "
                        f"fixture at the wrong height is not the cover it was "
                        f"measured to be")

    body = mesh.get_editor_property("body_setup")
    aggregate = body.get_editor_property("agg_geom")
    if len(aggregate.get_editor_property("box_elems")) == 0:
        problems.append(f"{stem} has no simple box collision, so a sight trace "
                        f"passes straight through it")

    profile = body.get_editor_property("default_instance").get_collision_profile_name()
    if str(profile) not in ("BlockAll", "BlockAllDynamic"):
        problems.append(f"{stem} defaults to collision profile '{profile}'. The "
                        f"sight trace runs on ECC_Visibility; a profile that "
                        f"does not block it makes this scenery")

    return problems


def main():
    problems = []
    for stem, expected_top in FIXTURES:
        mesh = import_mesh(stem)
        give_it_collision(mesh, stem)
        problems += assert_it_is_cover(mesh, stem, expected_top)

    for path in build_materials():
        unreal.log(f"material ready: {path}")

    if problems:
        for problem in problems:
            unreal.log_error(problem)
        raise RuntimeError(f"{len(problems)} fixture(s) imported but are not cover")

    unreal.log("Both fixtures imported, boxed and blocking Visibility. "
               "No map was opened or saved.")


if __name__ == "__main__":
    main()
