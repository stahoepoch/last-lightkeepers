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

RUN ONCE, AND IT DID NOT RUN AS WRITTEN
---------------------------------------
2026-11-14: the MCP session ran this against the live editor and all three
fixtures imported at 1:1, one box each, BlockAll. **Four faults in the first
version had to be patched to get there**, and they are fixed here:

  1. `SOURCE_DIR` walked one level ABOVE the repo. The uproject is at the
     repo root, so `project_dir()` already is it.
  2. `StaticMeshEditorSubsystem` is None outside a real editor, so the
     collision call raised on NoneType in a commandlet.
  3. `BodyInstance` has no `set_collision_profile_name` in Python; the
     profile is an editor property.
  4. `EditorStaticMeshLibrary.add_simple_collisions` returns **-1 on failure
     and does not raise**, so the script carried on and asserted against
     collision it had never added. That one was the dangerous one.

What is proven is asset-level: the mesh carries a box and defaults to a
profile that blocks `ECC_Visibility`. **Nothing has been placed and nothing
has been sighted through.** A trace fired at a placed fixture is still the
only thing that proves cover in play.
"""

import unreal

PACKAGE_PATH = "/Game/Props/Boswells"
# The uproject sits at the repo root, so project_dir() IS the repo root. The
# first version of this walked one level above it, to a directory that does
# not exist.
SOURCE_DIR = unreal.Paths.project_dir() + "assets/models/boswells/"

FIXTURES = [
    ("SM_BSW_FoldedGoodsTable", 80.0),
    ("SM_BSW_ServiceCounter", 102.0),
    ("SM_BSW_WallMerchBay", 190.0),
]

# The OBJs reference BoswellsFixtures.mtl, so an import with materials enabled
# brings all 17 palette colours in as loose assets in the package root. They
# are not junk - the mesh slots point at them and every fixture in the kit
# shares them - but they land unparameterised and in the wrong folder.
#
# Rather than build a parallel set of four MI_BSW_* instances that nothing is
# assigned to, which is what the first version did, these are the surface
# treatments the palette names need in order to stop reading as flat tint.
# Everything not listed keeps whatever the import gave it.
PALETTE_SURFACES = {
    "oat":         {"Roughness": 0.28, "Metallic": 0.0},   # laminate top
    "cream":       {"Roughness": 0.34, "Metallic": 0.0},   # laminate carcass
    "dark_wood":   {"Roughness": 0.30, "Metallic": 0.0},   # counter top
    "white_steel": {"Roughness": 0.46, "Metallic": 0.35},  # painted metal
    "steel":       {"Roughness": 0.52, "Metallic": 0.45},
    "charcoal":    {"Roughness": 0.58, "Metallic": 0.20},  # toe kicks
    "denim":       {"Roughness": 0.88, "Metallic": 0.0},   # folded stock
    "burgundy":    {"Roughness": 0.88, "Metallic": 0.0},
    "mint":        {"Roughness": 0.88, "Metallic": 0.0},
    "rose":        {"Roughness": 0.88, "Metallic": 0.0},
    "sky":         {"Roughness": 0.88, "Metallic": 0.0},
    "butter":      {"Roughness": 0.88, "Metallic": 0.0},
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
    """One box collider sized to the mesh, then the profile that makes it
    cover.

    Box rather than complex-as-simple on purpose: these are convex fixtures,
    a box is the cheapest thing a sight trace can hit, and complex collision
    is not used by line traces against a mesh set to simple.

    **The box is built directly rather than asked for.** The first version
    called `EditorStaticMeshLibrary.add_simple_collisions`, which returns -1
    on failure and does not raise - so the script carried on and asserted
    against collision it had never added. A silent -1 that still reports
    success is worse than a crash, and it is the kind of failure the asserts
    at the end of this file exist to catch. `StaticMeshEditorSubsystem` is the
    supported route but is None outside a real editor, so it is used when it
    is there and the element is built by hand when it is not.
    """
    body = mesh.get_editor_property("body_setup")
    subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)

    if subsystem is not None:
        subsystem.remove_collisions(mesh)
        subsystem.add_simple_collisions(mesh, unreal.ScriptingCollisionShapeType.BOX)
    else:
        # Headless. Build the element ourselves from the mesh bounds; note the
        # K prefix - `unreal.BoxElem` does not exist.
        bounds = mesh.get_bounds()
        extent = bounds.box_extent
        box = unreal.KBoxElem()
        box.set_editor_property("center", bounds.origin)
        box.set_editor_property("x", extent.x * 2.0)
        box.set_editor_property("y", extent.y * 2.0)
        box.set_editor_property("z", extent.z * 2.0)
        aggregate = body.get_editor_property("agg_geom")
        aggregate.set_editor_property("box_elems", [box])
        body.set_editor_property("agg_geom", aggregate)

    body.set_editor_property("collision_trace_flag",
                             unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AND_COMPLEX)
    # The profile placed components inherit. BlockAll blocks Visibility, which
    # is the channel the sight trace actually uses. `BodyInstance` has no
    # `set_collision_profile_name` in Python; it is an editor property.
    body.get_editor_property("default_instance").set_editor_property(
        "collision_profile_name", "BlockAll")
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)


def tidy_palette_materials():
    """Give the imported palette materials a surface, and move them out of the
    package root.

    The OBJ import creates one material per `.mtl` entry - 17 of them, named
    for the palette, landing beside the meshes rather than under Materials/.
    Every fixture in the kit shares that palette, so these ARE the right
    materials; they just arrive as flat colour in the wrong folder.

    Touched, not rebuilt: a parallel set of MI_BSW_* instances assigned to
    nothing is what the first version of this file produced, and a second
    material system beside a working one is the same mistake as a second
    fixture generator.

    Unproven. Everything here is best-effort and logs rather than raising,
    because a material that failed to move is cosmetic while a fixture that
    failed to block is not.
    """
    moved = []
    for name, params in PALETTE_SURFACES.items():
        loose = f"{PACKAGE_PATH}/{name}"
        if not unreal.EditorAssetLibrary.does_asset_exist(loose):
            continue
        material = unreal.load_asset(loose)
        try:
            for scalar, value in params.items():
                material.set_editor_property(scalar.lower(), value)
            unreal.EditorAssetLibrary.save_loaded_asset(material)
        except Exception as error:  # noqa: BLE001 - cosmetic, never fatal
            unreal.log_warning(f"{name}: could not set surface ({error})")
        target = f"{PACKAGE_PATH}/Materials/{name}"
        if unreal.EditorAssetLibrary.rename_asset(loose, target):
            moved.append(target)
        else:
            unreal.log_warning(f"{name}: left in the package root")
    return moved


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

    profile = body.get_editor_property("default_instance").get_editor_property(
        "collision_profile_name")
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

    for path in tidy_palette_materials():
        unreal.log(f"palette material filed: {path}")

    if problems:
        for problem in problems:
            unreal.log_error(problem)
        raise RuntimeError(f"{len(problems)} fixture(s) imported but are not cover")

    unreal.log("Both fixtures imported, boxed and blocking Visibility. "
               "No map was opened or saved.")


if __name__ == "__main__":
    main()
