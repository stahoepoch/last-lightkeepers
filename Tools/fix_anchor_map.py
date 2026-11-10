"""Phase 1 of docs/PLAN-AUDIT-BOSWELLS-ANCHOR.md, applied to the Anchor map.

Run headless, with no editor open:

    Tools/fix_anchor_map.ps1

or directly:

    UnrealEditor-Cmd.exe LastLightkeepers.uproject -run=pythonscript
        -script=Tools/fix_anchor_map.py -stdout -unattended -nosplash -nopause

What it does, and nothing else. Every number here is from the audit's part A,
which was read by decoding the map's own transform records:

  D1  Re-seat every DEPT_* and UPPER_* box. The map was authored with
      z = plate + 100 * scaleZ; /Engine/BasicShapes/Cube has its origin at
      its centre, so the rule is z = plate + 50 * scaleZ. 22 actors.
      DEPT_* sit on the lower plate (z 0); UPPER_* on the upper plate
      (slab top z 662.5).
  D2  Delete ANCHOR_WEST_RAMP. It rises correctly and arrives underneath
      ANCHOR_UPPER_SOUTH, which is one scaled cube with no opening, so a
      176 cm body's head meets the slab at about z 461 with 6.4 m of ramp
      left. The atrium void is where the escalator goes (audit B2); the
      ramp would need a second hole cut, so it is a deletion, not a repair.
  D3  Delete both ramp guards with it. They carried the right pitch and
      length and sat 430 cm clear of the deck on each side.
  D5  Move ANCHOR_RELIC_RADIO off ANCHOR_RELIC_PERFUME_LEDGER. The two were
      at the identical transform, so one of the two relics did not exist as
      far as a player was concerned. The radio goes to the teens block,
      which is where section 13 puts the CRT sets.
  D6  Move ANCHOR_PATROL_2 off the dock approach. It was at (0, 2100): on
      the spawn-to-exit axis, 375 cm from the extraction zone.
  D7  Add one red EXIT sign over the dock. The building had nine department
      signs and no exit signage at all.
  D8  Delete the three actors still named _TEST.

Idempotent. Re-seating is computed from each actor's own scale, deletions
check for existence, and the sign is only spawned if its label is absent.

NOT done here, because they are not mine to decide (audit B8): fixture
heights stay as authored - the block-in's 137 is Chris's call - and no
fixture is added, so the plate is still at 3.7% coverage.
"""

import unreal

MAP = "/Game/LLK_Boswells_Anchor"

LOWER_PLATE_Z = 0.0
UPPER_PLATE_Z = 662.5

DELETE = [
    "ANCHOR_WEST_RAMP",
    "ANCHOR_WEST_RAMP_GUARD_A",
    "ANCHOR_WEST_RAMP_GUARD_B",
    "ANCHOR_EMERGENCY_LIGHT_TEST",
    "ANCHOR_NAV_SPOT_TEST",
    "ANCHOR_SIGN_TEST",
]

# label -> (x, y, z), with the reason in the audit section it closes.
MOVE = {
    "ANCHOR_PATROL_2": (-1400.0, 2100.0, 110.0),
    "ANCHOR_RELIC_RADIO": (-1800.0, 1750.0, 140.0),
}

EXIT_SIGN_LABEL = "ANCHOR_SIGN_EXIT_DOCK"
EXIT_SIGN_LOCATION = (0.0, 2600.0, 430.0)
# The nine department signs are all yaw 90. This one is yaw -90 so it faces
# the approach from the south, which is the only direction anyone reaches it
# from. Whether the other nine face their own approaches is not something the
# transforms can answer and is flagged in the audit rather than changed here.
EXIT_SIGN_YAW = -90.0
STYLE_DONOR = "ANCHOR_SIGN_SERVICE"

# unreal.log lands in Saved/Logs/LastLightkeepers.log and NOT on stdout, so
# a run piped to a file printed nothing about what it had done. Say it twice.
def log(msg):
    unreal.log(msg)
    print(msg)


def warn(msg):
    unreal.log_warning(msg)
    print("WARNING: " + msg)


def err(msg):
    unreal.log_error(msg)
    print("ERROR: " + msg)


def all_level_actors():
    """get_all_level_actors, whichever of the two spellings this build has."""
    try:
        return unreal.get_editor_subsystem(
            unreal.EditorActorSubsystem).get_all_level_actors()
    except Exception as exc:
        log("EditorActorSubsystem unavailable ({0}); using "
            "EditorLevelLibrary".format(exc))
        return unreal.EditorLevelLibrary.get_all_level_actors()


def destroy(actor):
    try:
        unreal.get_editor_subsystem(
            unreal.EditorActorSubsystem).destroy_actor(actor)
    except Exception:
        unreal.EditorLevelLibrary.destroy_actor(actor)


def spawn(cls, location, rotation):
    try:
        return unreal.get_editor_subsystem(
            unreal.EditorActorSubsystem).spawn_actor_from_class(
                cls, location, rotation)
    except Exception:
        return unreal.EditorLevelLibrary.spawn_actor_from_class(
            cls, location, rotation)


def plate_for(label):
    if label.startswith("DEPT_"):
        return LOWER_PLATE_Z
    if label.startswith("UPPER_"):
        return UPPER_PLATE_Z
    return None


def main():
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
    if world is None:
        err("could not load {0} - nothing was changed".format(MAP))
        return 1

    actors = all_level_actors()
    by_label = {}
    for actor in actors:
        by_label.setdefault(actor.get_actor_label(), []).append(actor)
    log("loaded {0} with {1} actors".format(MAP, len(actors)))

    # A map that does not look like the one the audit read is a map this
    # script must not touch. The ten checked labels are the shell, which the
    # audit found correct and which this script never moves.
    expected = [
        "ANCHOR_FULL_FLOOR_LOWER", "ANCHOR_UPPER_SOUTH", "ANCHOR_UPPER_NORTH",
        "ANCHOR_UPPER_EAST", "ANCHOR_UPPER_WEST", "ANCHOR_FULL_ROOF",
        "ANCHOR_LOADING_DOCK_EXIT", "ANCHOR_PLAYERSTART_ENTRANCE",
        "DEPT_LOADING_APPROACH", "ANCHOR_ATRIUM_POST_0",
    ]
    missing = [name for name in expected if name not in by_label]
    if missing:
        err("this is not the map the audit read - missing {0}. "
            "Nothing was changed.".format(missing))
        return 1

    reseated = []
    unchanged = []
    for label, found in sorted(by_label.items()):
        plate = plate_for(label)
        if plate is None:
            continue
        for actor in found:
            scale = actor.get_actor_scale3d()
            loc = actor.get_actor_location()
            want = plate + 50.0 * scale.z
            if abs(loc.z - want) < 0.01:
                unchanged.append(label)
                continue
            actor.set_actor_location(
                unreal.Vector(loc.x, loc.y, want), False, True)
            reseated.append((label, loc.z, want, scale.z))

    for label, was, now, scale_z in reseated:
        log("D1 re-seat {0:30s} z {1:8.2f} -> {2:8.2f} "
            "(height {3:.0f}, was floating {4:.1f})".format(
                label, was, now, scale_z * 100.0, was - now))
    if unchanged:
        log("D1 already seated: {0}".format(sorted(set(unchanged))))

    moved = []
    for label, target in MOVE.items():
        for actor in by_label.get(label, []):
            loc = actor.get_actor_location()
            if (abs(loc.x - target[0]) < 0.01 and abs(loc.y - target[1]) < 0.01
                    and abs(loc.z - target[2]) < 0.01):
                log("already moved: {0}".format(label))
                continue
            actor.set_actor_location(unreal.Vector(*target), False, True)
            moved.append((label, (loc.x, loc.y, loc.z), target))
    for label, was, now in moved:
        log("move {0:28s} ({1[0]:.0f}, {1[1]:.0f}, {1[2]:.0f}) -> "
            "({2[0]:.0f}, {2[1]:.0f}, {2[2]:.0f})".format(label, was, now))

    deleted = []
    for label in DELETE:
        for actor in by_label.get(label, []):
            destroy(actor)
            deleted.append(label)
    for label in deleted:
        log("delete {0}".format(label))
    absent = [name for name in DELETE if name not in by_label]
    if absent:
        log("already deleted: {0}".format(absent))

    # Find or create, then always restyle, so a re-run finishes anything a
    # previous run could not set rather than reporting the sign as present
    # and leaving it half-dressed.
    sign = None
    spawned = False
    existing = by_label.get(EXIT_SIGN_LABEL, [])
    if existing:
        sign = existing[0]
        log("restyling existing {0}".format(EXIT_SIGN_LABEL))
    else:
        sign = spawn(
            unreal.TextRenderActor,
            unreal.Vector(*EXIT_SIGN_LOCATION),
            unreal.Rotator(0.0, 0.0, EXIT_SIGN_YAW))
        if sign is None:
            warn("could not spawn {0}; every other edit still "
                 "applied".format(EXIT_SIGN_LABEL))
        else:
            sign.set_actor_label(EXIT_SIGN_LABEL)
            spawned = True
            log("spawn {0} at ({1[0]:.0f}, {1[1]:.0f}, {1[2]:.0f}) "
                "yaw {2:.0f}".format(
                    EXIT_SIGN_LABEL, EXIT_SIGN_LOCATION, EXIT_SIGN_YAW))

    restyled = False
    if sign is not None:
        comp = sign.text_render
        comp.set_text(unreal.Text("EXIT"))
        # Red is the one legitimately red thing in an emergency-lit
        # building: there is no national colour mandate, and red is the US
        # norm after the 1949 Fire Marshals Association vote.
        comp.set_text_render_color(unreal.Color(r=255, g=24, b=16, a=255))
        donor = by_label.get(STYLE_DONOR, [])
        if donor:
            try:
                comp.set_world_size(donor[0].text_render.world_size)
                log("sign sized from {0}".format(STYLE_DONOR))
            except Exception as exc:
                warn("could not copy size from {0}: {1}".format(
                    STYLE_DONOR, exc))
        # UE's own enum carries a typo (EHorizTextAligment), and unlike most
        # enums exposed to Python it keeps its member prefix: the answer is
        # `EHTA_CENTER`, after ETC_CENTER and CENTER were both guessed wrong.
        # Asked rather than hard-coded because the next build may differ, and
        # because centring a sign must never abort a run that has 22
        # re-seated actors in memory and nothing saved yet.
        try:
            enum = unreal.HorizTextAligment
            names = [n for n in dir(enum) if not n.startswith("_")]
            log("HorizTextAligment members: {0}".format(names))
            centre = next(n for n in names if "CENTER" in n.upper()
                          or "CENTRE" in n.upper())
            comp.set_horizontal_alignment(getattr(enum, centre))
            log("sign centred via HorizTextAligment.{0}".format(centre))
        except Exception as exc:
            warn("could not centre the sign text ({0}); left at its default "
                 "alignment, which on a sign at x 0 means it reads off to "
                 "one side".format(exc))
        restyled = True

    # The sign restyle is unconditional, so a re-run does save - it is the
    # one edit here that cannot be detected as already done, and re-writing
    # the same values costs nothing but a changed file hash.
    touched = (len(reseated) + len(moved) + len(deleted)
               + (1 if (spawned or restyled) else 0))
    if touched == 0:
        log("nothing to do - the map already reads as fixed. Not saving.")
        return 0

    if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP):
        err("edits applied in memory but the save FAILED - "
            "the map on disk is unchanged")
        return 1

    log("SAVED {0}: {1} re-seated, {2} moved, {3} deleted, {4} added, "
        "{5} restyled".format(
            MAP, len(reseated), len(moved), len(deleted),
            1 if spawned else 0, 1 if restyled else 0))
    return 0


main()
