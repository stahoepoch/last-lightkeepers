"""Which step of import_fixtures.py turns a working mesh into one a sight
trace passes through?

Run inside an editor process:
    UnrealEditor-Cmd.exe <project> "-ExecCmds=py \"<this file>\", QUIT_EDITOR"
        -unattended -nosplash -nopause

THE EXPERIMENT THIS CONTINUES
-----------------------------
2026-11-15, after seventeen candidates eliminated and seven hypotheses
withdrawn, a controlled run split the problem cleanly: the SAME .obj files,
in the SAME process, with the SAME `add_simple_collisions(BOX)` call, block
when a throwaway importer brings them in and do not when
`import_fixtures.py` does. Three fixtures, both ways, five traces each.

So it is not the geometry, not Nanite, not complexity, not the engine. **It
is this kit's own import code**, which is mine, and the remaining question is
which part of it.

WHY A BISECT RATHER THAN A GUESS
--------------------------------
`import_fixtures.py` does seven things the throwaway importer did not:
deletes the existing asset first and re-imports over the same package path;
calls `remove_collisions` before adding; sets the collision trace flag; sets
the profile on the body setup; turns Nanite off; saves the asset; and renames
the palette materials out of the package root.

Today's record on choosing between candidates by reasoning is nought for
seven. So this does not choose. It applies the steps cumulatively, to a fresh
package path per variant so nothing can inherit anything, and traces after
each one. **The first variant that stops blocking names the step.**

The last variant is the one that matters most and is easy to leave out: save
the asset, drop it from memory, load it back, and trace THAT. A step that
only breaks the thing on disk would pass every in-memory check above it -
and "the saved package reloads clear" is already an observed fact today.
"""

import unreal

SOURCE_DIR = unreal.Paths.project_dir() + "assets/models/boswells/"
PACKAGE_ROOT = "/Game/ImportBisect"
STEMS = ["SM_BSW_ServiceCounter",      # blocks under both importers
         "SM_BSW_FoldedGoodsTable",    # blocks only under the throwaway one
         "SM_BSW_WallMerchBay"]
HEIGHTS = [40.0]
REPEATS = 5

TRACE_CHANNEL = getattr(unreal.TraceTypeQuery, "ECC_VISIBILITY",
                        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1)
WHERE = unreal.Vector(80000.0, 0.0, 0.0)

# Cumulative. Each variant does everything above it, plus its own line.
STEPS = [
    "import only",
    "+ remove_collisions then add_simple_collisions",
    "+ collision_trace_flag = SIMPLE_AND_COMPLEX",
    "+ profile BlockAll on the body setup",
    "+ Nanite off",
    "+ saved",
    "+ saved, unloaded, reloaded from disk",
]


def actors():
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if subsystem is None:
        raise RuntimeError("EditorActorSubsystem is None; needs a real editor")
    return subsystem


def blocked(world, start, end):
    return unreal.SystemLibrary.line_trace_single(
        world, start, end, TRACE_CHANNEL, False, [],
        unreal.DrawDebugTrace.NONE, True) is not None


def answer(world, mesh):
    """Five traces across the short axis. A split is reported, never
    summarised - the flakiness so far has been across processes and never
    inside one, so a split here would be the most interesting line printed."""
    half = mesh.get_bounds().box_extent.y
    actor = actors().spawn_actor_from_class(
        unreal.StaticMeshActor, WHERE, unreal.Rotator(0.0, 0.0, 0.0))
    try:
        if actor is None or not actor.static_mesh_component.set_static_mesh(
                mesh):
            return "could not spawn"
        actor.static_mesh_component.set_collision_profile_name("BlockAll")
        start = unreal.Vector(WHERE.x, WHERE.y - half - 90.0,
                              WHERE.z + HEIGHTS[0])
        end = unreal.Vector(WHERE.x, WHERE.y + half + 320.0,
                            WHERE.z + HEIGHTS[0])
        hits = sum(blocked(world, start, end) for _ in range(REPEATS))
    finally:
        if actor is not None:
            actors().destroy_actor(actor)
    if hits == REPEATS:
        return "BLOCK"
    if hits == 0:
        return "clear"
    return f"UNSTABLE {hits}/{REPEATS}"


def build(stem, level):
    """Import to a package path of this variant's own and apply steps 0..level.

    A fresh path every time is the point. The leading explanation for the
    whole fault is that re-importing over a path an asset already occupies
    inherits something the delete left behind, so a bisect that reused paths
    would be measuring that instead of the steps.
    """
    path = f"{PACKAGE_ROOT}/V{level}"
    task = unreal.AssetImportTask()
    task.filename = SOURCE_DIR + stem + ".obj"
    task.destination_path = path
    task.destination_name = stem
    task.automated = True
    task.replace_existing = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(f"{path}/{stem}")
    if mesh is None:
        return None, f"{stem} did not import at V{level}"

    if level >= 1:
        subsystem = unreal.get_editor_subsystem(
            unreal.StaticMeshEditorSubsystem)
        subsystem.remove_collisions(mesh)
        added = subsystem.add_simple_collisions(
            mesh, unreal.ScriptingCollisionShapeType.BOX)
        if isinstance(added, int) and not isinstance(added, bool) and added < 0:
            return mesh, f"add_simple_collisions returned {added}"

    body = mesh.get_editor_property("body_setup")
    if level >= 2:
        body.set_editor_property(
            "collision_trace_flag",
            unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AND_COMPLEX)
    if level >= 3:
        body.get_editor_property("default_instance").set_editor_property(
            "collision_profile_name", "BlockAll")
    if level >= 4:
        settings = mesh.get_editor_property("nanite_settings")
        settings.set_editor_property("enabled", False)
        setter = getattr(unreal.get_editor_subsystem(
            unreal.StaticMeshEditorSubsystem), "set_nanite_settings", None)
        if setter is not None:
            setter(mesh, settings, True)
        else:
            mesh.set_editor_property("nanite_settings", settings)
    if level >= 5:
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    if level >= 6:
        asset_path = f"{path}/{stem}"
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        del mesh
        # Drop it from memory so the trace below is against what is on disk,
        # not against a live object that never left it.
        unreal.SystemLibrary.collect_garbage()
        mesh = unreal.load_asset(asset_path)
        if mesh is None:
            return None, f"{stem} would not reload at V{level}"
    return mesh, None


def main():
    world = unreal.get_editor_subsystem(
        unreal.UnrealEditorSubsystem).get_editor_world()
    if world is None:
        raise RuntimeError("no editor world, so no trace can be trusted")

    width = max(len(s) for s in STEPS)
    for stem in STEMS:
        unreal.log(f"\n=== {stem} ===")
        for level, label in enumerate(STEPS):
            mesh, problem = build(stem, level)
            if problem:
                unreal.log_warning(f"  V{level} {label:<{width}}  {problem}")
            if mesh is None:
                continue
            boxes = len(mesh.get_editor_property("body_setup")
                        .get_editor_property("agg_geom")
                        .get_editor_property("box_elems"))
            unreal.log(f"  V{level} {label:<{width}}  "
                       f"{answer(world, mesh)}  boxes={boxes}")

    unreal.log(f"\nThe first level that stops blocking names the step. "
               f"Everything was written under {PACKAGE_ROOT}, which is "
               f"scratch - delete the whole folder afterwards. No map was "
               f"opened and nothing in /Game/Props was touched.")


if __name__ == "__main__":
    main()
