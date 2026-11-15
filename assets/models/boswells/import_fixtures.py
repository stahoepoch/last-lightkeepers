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
     collision call raised on NoneType in a commandlet. **The fix for this
     one was itself the bug** - see below. Patching it with a hand-built
     `KBoxElem` turned a loud crash into two silently uncollidable assets,
     and a crash would have been the better outcome by a wide margin.
  3. `BodyInstance` has no `set_collision_profile_name` in Python; the
     profile is an editor property.
  4. `EditorStaticMeshLibrary.add_simple_collisions` returns **-1 on failure
     and does not raise**, so the script carried on and asserted against
     collision it had never added. That one was the dangerous one.

What is proven is asset-level: the mesh carries a box and defaults to a
profile that blocks `ECC_Visibility`. **Nothing has been placed and nothing
has been sighted through.** A trace fired at a placed fixture is still the
only thing that proves cover in play.

AND THAT CAVEAT TURNED OUT TO BE THE WHOLE STORY, 2026-11-15
------------------------------------------------------------
It was written as a disclaimer and it was describing a live defect. A trace
at placed instances found `SM_BSW_FoldedGoodsTable` and `SM_BSW_WallMerchBay`
**missed at every height, at every position, on simple and complex traces
alike**, while both passed every check this script makes. Only
`SM_BSW_ServiceCounter` blocks.

The cause is fault 2's patch: a `KBoxElem` written straight onto the body
setup is a struct the engine never cooks physics for. The asset reads
box_elems 1, CTF_USE_SIMPLE_AND_COMPLEX, BlockAll, QUERY_AND_PHYSICS and
correct colliding bounds, and a sight line goes through it.

**This is the `add_simple_collisions` returns -1 fault moved up one level:
from an unchecked return to an unverified claim.** The first version asserted
against collision it had never added; this version asserted against collision
that existed as data and not as physics. Both times the assert was the thing
that was too weak, and both times it looked like rigour.

Two changes came out of it. The headless fallback is deleted rather than
repaired, because there is no headless way to cook collision and a fallback
that cannot work only converts "this cannot run here" into "this ran and
produced cover". And this script no longer claims the fixtures block anything
- it claims they are configured, which is what it can actually see.
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

    # THE HEADLESS PATH IS GONE, AND DELETING IT IS THIS FILE'S MAIN REPAIR.
    #
    # It used to build a `KBoxElem` by hand and write it onto the body setup's
    # aggregate geometry when the subsystem was absent. That produces a struct
    # which reads perfectly - box_elems 1, correct extents, BlockAll,
    # QUERY_AND_PHYSICS - and which **the engine never cooks physics data
    # for**, so a Visibility trace passes straight through the finished asset.
    #
    # It shipped. `1ee88b8` is titled "Three cover fixtures imported, boxed and
    # blocking Visibility"; measured by trace on 2026-11-15, two of the three
    # are missed at every height, at every position, on simple and complex
    # traces alike. Only the counter blocks, and the plausible reason is that
    # it was the one processed while a live editor was attached.
    #
    # There is no headless repair, and that was measured rather than assumed:
    # `BodySetup` exposes no `InvalidatePhysicsData`, no `CreatePhysicsMeshes`,
    # nothing with "phys", "cook" or "invalidate" on it at all; rewriting the
    # element through the property setter leaves it clear; and
    # CTF_USE_COMPLEX_AS_SIMPLE does not help because the complex trace is
    # clear too - there is no cooked collision of either kind to fall back on.
    #
    # So a fallback that cannot work is worse than no fallback: it turns "this
    # cannot run here" into "this ran and produced cover", which is the exact
    # trade the rest of this file exists to refuse.
    if subsystem is None:
        raise RuntimeError(
            "StaticMeshEditorSubsystem is None, so this is not a real editor "
            "and simple collision cannot be built. It is the engine's only "
            "build path and it is editor-only. Nothing has been changed. Do "
            "NOT hand-write a KBoxElem instead: it produces an asset that "
            "passes every asset-level check and that a sight trace goes "
            "straight through")

    subsystem.remove_collisions(mesh)
    added = subsystem.add_simple_collisions(
        mesh, unreal.ScriptingCollisionShapeType.BOX)

    # Checked by VALUE, not truthiness. This call returns the index of the
    # element it added and -1 on failure, so `if not added` is False for -1
    # and would wave the failure through - the identical mistake to the one
    # this file's docstring already records, written while fixing it. An
    # index of 0 is a success and is falsy; -1 is a failure and is truthy.
    if isinstance(added, bool):
        ok = added
    elif isinstance(added, int):
        ok = added >= 0
    else:
        ok = added is not None
    if not ok:
        raise RuntimeError(
            f"{stem}: add_simple_collisions returned {added!r} and built no "
            f"box. Nothing below is true and nothing has been saved")

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
    """What the asset's own properties can be made to admit - and that is less
    than it sounds, which is the whole lesson of this file.

    These three checks all passed on all three fixtures on 2026-11-15 while
    two of them were invisible to a Visibility trace. **An asset-level check
    cannot tell configured collision from working collision.** box_elems
    counts a struct; collision_profile_name reads a string; neither consults
    the physics scene. Keep them - they catch a failed import and a wrong
    profile, which are real - but do not read a pass here as cover.

    Cover is proven by a trace at a placed instance, and the only thing in
    this kit that does that is place_fixtures.py, which refuses to save the
    map when a fixture does not block."""
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

    unreal.log(
        f"{len(FIXTURES)} fixtures imported at their authored heights, each "
        f"carrying one box element and defaulting to BlockAll. No map was "
        f"opened or saved.\n"
        f"NOT PROVEN HERE: that any of them blocks a sight trace. That needs "
        f"a trace at a placed instance, and place_fixtures.py is what runs "
        f"it. On 2026-11-15 every check above passed on all three fixtures "
        f"while two of them were missed by a trace at every height.")


if __name__ == "__main__":
    main()
