"""Check the exterior against the thing it was built for, without changing it.

`Tools/build_boswells_exterior.py` asserts as it goes, which catches a tool
that failed. This asks the different question: is what Chris asked for
actually in the saved map, and did putting it there change the inside of the
store - because the complaint that began this week was that the interior was
too bright, and an exterior is the most plausible way to undo that by
accident.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_EXT_"

# Each of these is a thing Chris named, in his own order.
REQUIRED = {
    "an exterior entrance on the west, in glass": (
        "DOOR_WEST_GLASS", "DOOR_WEST_MULLION_1", "DOOR_WEST_CANOPY"),
    "an exterior entrance on the east, in glass": (
        "DOOR_EAST_GLASS", "DOOR_EAST_MULLION_1", "DOOR_EAST_CANOPY"),
    "the mall entrance at the top as well as the bottom": (
        "MALL_UPPER_GLASS", "MALL_UPPER_FRAME_L", "MALL_UPPER_FRAME_R"),
    "the loading dock's own way out": (
        "DOCK_OVERHEAD_DOOR", "DOCK_STAFF_DOOR", "DOCK_APRON"),
    "an outside to stand on": ("APRON_W", "APRON_E", "LOT_EAST"),
    "a hillside": ("HILL_W_0", "HILL_W_1", "HILL_W_2", "LOT_WEST_LOWER"),
    "a night to see it against": ("MOON", "SKYLIGHT", "FOG"),
    "the store's name on its own building": ("SIGN_WORD_W", "SIGN_WORD_E"),
}

# The interior readings from the frames taken before the exterior existed, at
# the two traced poses in docs/FINDINGS-THE-FITTINGS-WERE-NOT-THE-PROBLEM.md.
# A verifier cannot render, so these are recorded here as the numbers a future
# render must still produce, not as something this script measures.
INTERIOR_BEFORE = {
    "ground pose, frame mean": 16.71,
    "deck pose, frame mean": 41.24,
}

failures = []
notes = []


def check(condition, message):
    if not condition:
        failures.append(message)


def say(message):
    unreal.log("EXTVERIFY| " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
check(world is not None, "could not load " + MAP)
actors_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

by_label = {}
for actor in actors_sub.get_all_level_actors():
    by_label.setdefault(actor.get_actor_label(), []).append(actor)

mine = {label: group for label, group in by_label.items()
        if label.startswith(PREFIX)}
say("%d exterior actor(s) in the saved map" % sum(len(g) for g in mine.values()))

for what, labels in REQUIRED.items():
    missing = [l for l in labels if (PREFIX + l) not in by_label]
    check(not missing, "%s: missing %s" % (what, ", ".join(missing)))
    duplicated = [l for l in labels
                  if len(by_label.get(PREFIX + l, [])) > 1]
    check(not duplicated,
          "%s: duplicated %s - the tool is not idempotent"
          % (what, ", ".join(duplicated)))
    if not missing and not duplicated:
        say("ok: %s" % what)

# The placeholder elevations must be out of the way, not deleted. Deleting
# them would quietly change what docs/FINDINGS-KIDS-FLOOR-STEALTH.md counts.
for label in ("ANCHOR_FULL_WALL_WEST", "ANCHOR_FULL_WALL_EAST"):
    group = by_label.get(label, [])
    check(len(group) == 1, "%s should still exist exactly once" % label)
    for actor in group:
        comps = actor.get_components_by_class(unreal.StaticMeshComponent)
        check(comps, "%s has no mesh component" % label)
        for comp in comps:
            check(not comp.is_visible(),
                  "%s is still visible, so the player sees the engine "
                  "placeholder behind the new facade" % label)
            check(comp.get_collision_enabled()
                  == unreal.CollisionEnabled.NO_COLLISION,
                  "%s still collides, so the new door opens into an "
                  "invisible wall" % label)

# A door you cannot walk through is a picture of a door. The glass is
# deliberately non-colliding; the opening behind it must be clear.
#
# Where the door IS comes from the door, not from a constant here. The first
# version of this file carried the y values the build tool used at the time,
# and when the build started choosing them by tracing it reported two doors
# blocked that were perfectly clear - a verifier measuring a position nobody
# builds at any more.
doorways = []
for label in ("DOOR_WEST", "DOOR_EAST"):
    group = by_label.get(PREFIX + label + "_GLASS", [])
    if len(group) != 1:
        check(False, "%s has no glass to locate it by" % label)
        continue
    origin, extent = group[0].get_actor_bounds(False)
    doorways.append((label, round(origin.x / 10.0) * 10.0, origin.y, extent.y))

for label, axis_x, door_y, door_half in doorways:
    # Through a bay, not down the centreline. A storefront has a mullion in
    # the middle of it and you walk past the mullion, not through it - the
    # first version of this check reported both entrances blocked by their
    # own centre post.
    bays = [door_y - door_half * 0.75, door_y - door_half * 0.25,
            door_y + door_half * 0.25, door_y + door_half * 0.75]
    blockers = []
    for bay_y in bays:
        inner = unreal.Vector(axis_x + (300.0 if axis_x < 0 else -300.0),
                              bay_y, 120.0)
        outer = unreal.Vector(axis_x + (-300.0 if axis_x < 0 else 300.0),
                              bay_y, 120.0)
        hit = unreal.SystemLibrary.line_trace_single(
            world, inner, outer, unreal.TraceTypeQuery.ECC_VISIBILITY, False,
            [], unreal.DrawDebugTrace.NONE, True)
        if hit:
            blocker = hit.to_tuple()[9]
            blockers.append(blocker.get_actor_label() if blocker else "?")
    clear_bays = len(bays) - len(blockers)
    check(clear_bays > 0,
          "%s has no clear bay: all four are blocked by %s"
          % (label, ", ".join(sorted(set(blockers)))))
    if clear_bays:
        say("ok: %s is walkable from inside to outside through %d of its %d "
            "bays" % (label, clear_bays, len(bays)))

# The hill has to fall, and it has to fall continuously. A hillside made of
# steps is a staircase.
runs = []
for index in (0, 1, 2):
    group = by_label.get(PREFIX + "HILL_W_%d" % index, [])
    if len(group) == 1:
        origin, extent = group[0].get_actor_bounds(False)
        runs.append((origin.x, origin.z - extent.z, origin.z + extent.z))
if len(runs) == 3:
    runs.sort(reverse=True)
    for near, far in zip(runs, runs[1:]):
        check(far[2] < near[2],
              "the hill does not keep falling: a run tops out at %.0f after "
              "one that topped out at %.0f" % (far[2], near[2]))
    drop = runs[0][2] - runs[-1][1]
    check(drop > 400.0, "the west grade only covers %.0f cm" % drop)
    say("ok: the west grade falls %.0f cm across three runs" % drop)

# Exterior lights are outside the envelope and must not be tagged onto the
# store's circuits. docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md is about
# untagged lights *inside*; a car park light is not on the store's breaker,
# and tagging one would make the breaker switch the car park.
for label, group in mine.items():
    for actor in group:
        tags = [str(t) for t in actor.get_editor_property("tags")]
        if "LIGHT" in label or "WALLPACK" in label or "MOON" in label:
            check("LLK_Mains" not in tags,
                  "%s carries LLK_Mains, so the store breaker would switch "
                  "the outside" % label)
            check("LLK_Emergency" not in tags,
                  "%s carries LLK_Emergency, so it would be counted as part "
                  "of the store's emergency layer" % label)

notes.append("the interior readings this build must not have moved: "
             + "; ".join("%s %.2f" % (k, v) for k, v in INTERIOR_BEFORE.items()))

for note in notes:
    say("note: " + note)

if failures:
    for failure in failures:
        say("FAIL: " + failure)
    raise RuntimeError("EXTERIOR_VERIFY %d check(s) failed" % len(failures))
say("PASS: the exterior holds every item Chris named")
