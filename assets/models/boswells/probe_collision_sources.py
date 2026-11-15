"""Where is the counter's blocking actually coming from?

Run inside an editor process:
    UnrealEditor-Cmd.exe <project> "-ExecCmds=py \"<this file>\", QUIT_EDITOR"
        -unattended -nosplash -nopause

THE QUESTION, AND WHY IT IS THE RIGHT ONE
-----------------------------------------
After a day of it, everything about the two failing fixtures has been
eliminated by direct measurement: position, height, trace geometry, trace
length, start distance, process type as a sole cause, the collision profile
call, box count, box extents, collision-enabled, the profile itself,
re-saving from a full editor, the Nanite flag, Nanite at build time,
delete-and-rebuild, the collision-building call, and the structure of the
source geometry. Six hypotheses have been raised and withdrawn.

Every one of those assumed the counter blocks **because of its box**, and
asked why the same box fails on the other two. Nobody checked the
assumption. If the counter still blocks with its box removed, then its
collision was never coming from the box, the box mechanism is broken for all
three, and **the asymmetry everyone has been explaining is the wrong one.**

That reframing is not mine - it came from the session that did the measuring,
offered after it had stopped running things, which is the best moment to have
an idea and the hardest one to act on.

WHAT IT DOES
------------
For each of the three fixtures, in one process, one body at a time, spawned
and destroyed between: trace it as-is, then with its simple collision
removed, then with complex-as-simple. Three questions whose combination says
where the collision lives:

    as-is   no-box   complex      reading
    BLOCK   BLOCK    -            not the box. Something else blocks, and
                                  the whole day's asymmetry is misframed
    BLOCK   clear    -            the box works here and nowhere else
    clear   -        BLOCK        simple collision is broken, complex is not
    clear   clear    clear        nothing in the asset blocks at all

It changes assets in memory and **does not save**. Nothing is written, no map
is opened. Re-run the import to get the assets back exactly as committed.
"""

import unreal

MESH_ROOT = "/Game/Props/Boswells"
FIXTURES = ["SM_BSW_ServiceCounter",      # the one that blocks
            "SM_BSW_FoldedGoodsTable",    # the two that do not
            "SM_BSW_WallMerchBay"]
HEIGHTS = [40.0, 88.0, 152.0]
REPEATS = 5

TRACE_CHANNEL = getattr(unreal.TraceTypeQuery, "ECC_VISIBILITY",
                        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1)
WHERE = unreal.Vector(70000.0, 0.0, 0.0)


def actors():
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if subsystem is None:
        raise RuntimeError("EditorActorSubsystem is None; run this in a real "
                           "editor process. Nothing changed")
    return subsystem


def blocked(world, start, end):
    """HitResult or None - measured, and the only signal the binding offers:
    every field on a Python HitResult in 5.8 is protected."""
    return unreal.SystemLibrary.line_trace_single(
        world, start, end, TRACE_CHANNEL, False, [],
        unreal.DrawDebugTrace.NONE, True) is not None


def answer(world, start, end):
    """Five traces, and disagreement is reported rather than summarised. The
    flakiness so far has been across processes rather than inside one, so
    this should never split - if it does, that is the most interesting line
    in the output and must not be rounded away."""
    hits = sum(blocked(world, start, end) for _ in range(REPEATS))
    if hits == REPEATS:
        return "BLOCK"
    if hits == 0:
        return "clear"
    return f"UNSTABLE {hits}/{REPEATS}"


def trace_it(world, mesh, half_depth):
    out = []
    actor = actors().spawn_actor_from_class(
        unreal.StaticMeshActor, WHERE, unreal.Rotator(0.0, 0.0, 0.0))
    try:
        if actor is None or not actor.static_mesh_component.set_static_mesh(
                mesh):
            return ["could not spawn"]
        actor.static_mesh_component.set_collision_profile_name("BlockAll")
        for height in HEIGHTS:
            start = unreal.Vector(WHERE.x, WHERE.y - half_depth - 90.0,
                                  WHERE.z + height)
            end = unreal.Vector(WHERE.x, WHERE.y + half_depth + 320.0,
                                WHERE.z + height)
            out.append(f"{height:.0f}cm {answer(world, start, end)}")
    finally:
        if actor is not None:
            actors().destroy_actor(actor)
    return out


def main():
    world = unreal.get_editor_subsystem(
        unreal.UnrealEditorSubsystem).get_editor_world()
    if world is None:
        raise RuntimeError("no editor world, so no trace can be trusted")

    for stem in FIXTURES:
        mesh = unreal.load_asset(f"{MESH_ROOT}/{stem}")
        if mesh is None:
            unreal.log_error(f"{stem} is not imported")
            continue

        body = mesh.get_editor_property("body_setup")
        aggregate = body.get_editor_property("agg_geom")
        boxes = list(aggregate.get_editor_property("box_elems"))
        half = mesh.get_bounds().box_extent.y
        unreal.log(f"\n=== {stem} ===  box elems {len(boxes)}  "
                   f"half-depth {half:.1f}")

        unreal.log(f"  as imported      {trace_it(world, mesh, half)}")

        # 1. No simple collision at all. If it still blocks, the box was
        #    never what was blocking.
        aggregate.set_editor_property("box_elems", [])
        body.set_editor_property("agg_geom", aggregate)
        unreal.log(f"  box removed      {trace_it(world, mesh, half)}")

        # 2. Complex as simple, still with no box - does the render mesh
        #    itself carry anything a trace can hit?
        body.set_editor_property(
            "collision_trace_flag",
            unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        unreal.log(f"  complex-as-simple{trace_it(world, mesh, half)}")

        # Put the asset back the way it was found, in memory. Nothing is
        # saved by this script either way, but leaving a loaded asset
        # mutated is how a later run in the same process reads a lie.
        body.set_editor_property(
            "collision_trace_flag",
            unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AND_COMPLEX)
        aggregate.set_editor_property("box_elems", boxes)
        body.set_editor_property("agg_geom", aggregate)

    unreal.log("\nNothing saved. Re-run import_fixtures.py to be certain the "
               "assets on disk are the committed ones.")


if __name__ == "__main__":
    main()
