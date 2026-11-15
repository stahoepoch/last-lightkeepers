"""Place the three Boswell's cover fixtures on the children's deck of
LLK_Boswells_ApparelPass, then prove each one blocks a sight trace.

Run with the commandlet runner, not the MCP bridge:
    Tools/run_editor_python.ps1 "<repo>/assets/models/boswells/place_fixtures.py"

The commandlet exits and cannot race a human's save; the MCP bridge needs a
live editor holding the project. For a map write, pick the one that exits.

WHY THIS FILE EDITS A MAP, WHICH THE IMPORT SCRIPT DELIBERATELY DID NOT
-----------------------------------------------------------------------
`import_fixtures.py` opens no map, on Chris's instruction: *"Don't edit or save
the map."* He withdrew that on 2026-11-15 - *"place them in the map, commit
your work"* - so this is the companion that does, and it is the only file in
the asset lane allowed to.

It is still narrow on purpose:

  - It touches ONLY actors it spawned itself, tagged `LLK_CoverFixture`. A
    re-run deletes its own tag first, so it replaces rather than stacks.
  - It never moves, edits or deletes anything it did not place. The 21 rolling
    racks stay open and unreliable, which is the point of them, and the
    encounter `7c737df` built is not touched.
  - It refuses to save if a fixture does not block, and says which.

WHAT THE DECK ALREADY HAS, AND WHY THESE THREE STILL EARN A SPOT
----------------------------------------------------------------
`7c737df` *"Build children's-floor signs and hide encounter"* put a live
`LLKSweeper` at (-850,-1300), three `LLK.Patrol` points, and two solid
displays on this deck before these fixtures existed. So "the floor needs
somewhere ducking works" was already answered, and three more cover objects
risk being a second cover kit beside a working one - the same mistake as a
second fixture generator. What each fixture blocks, at the heights the game
actually asks at:

    fixture                 top cm   crouch 40   stand 88    eye 152
    rolling rack x21         145      BLOCK       BLOCK      clear
    COVER_NORTH/SOUTH         78      BLOCK       clear      clear
    UPPER_BEDDING             95      BLOCK       BLOCK      clear
    UPPER_SEASONAL           105      BLOCK       BLOCK      clear
    SALES_DESK               209      BLOCK       BLOCK      BLOCK
    CEILING_SALES_COLUMN     437      BLOCK       BLOCK      BLOCK
    SM_BSW_FoldedGoodsTable   80      BLOCK       clear      clear
    SM_BSW_ServiceCounter    102      BLOCK       BLOCK      clear
    SM_BSW_WallMerchBay      190      BLOCK       BLOCK      BLOCK

Read down the columns and the three justifications are different:

  - **The counter is the only authored stand-hide-watch cover on the deck.**
    The two fixtures that share its row - `UPPER_BEDDING` and
    `UPPER_SEASONAL` - do it by accident, predate the department, and both
    interpenetrate racks (see the handoff row). Nothing was placed to do this
    job on purpose.
  - **The bay is the only hidden-and-blind cover away from one corner.** It
    shares its row with the sales desk and the column, and those stand 100 cm
    apart at (-2650,-1900) and (-2550,-1970). Everywhere else on an
    1800 x 2800 cm floor, a standing keeper can be seen.
  - **The table duplicates `COVER_NORTH/SOUTH`'s job exactly**, 80 cm against
    78. Its justification is location, not function: both displays sit on the
    x = -2150 line, so the entire west half of the department has no crouch
    cover at all. It goes west. It is also a real fixture where those two are
    blockout cubes, which is an argument for retiring them in its favour -
    **that is Chris's call and this script does not touch them.**

WHAT "BLOCKS" MEANS HERE, AND WHY IT IS THE WHOLE TEST
------------------------------------------------------
The sight trace runs on **ECC_Visibility**, between **capsule centres** - 88 cm
standing, 40 cm crouched - while the player's eye is at 152 cm. Cover is judged
at the chest, not the head, and a fixture is cover at the height the trace runs
at or it is furniture. (Those heights were read out of `LLKSweeper.cpp:327`
and `:366`. `FINDINGS-KIDS-FLOOR-STEALTH.md` is where they are written down,
but that document carries a staleness banner from `ff8914f` and its *play*
conclusions must not be cited for this deck - they were measured before the
displays, the patrol and the machine existed.)

The traces below run `TraceTypeQuery1`. The project defines no custom collision
channels - there is no `[/Script/Engine.CollisionProfile]` section in
`Config/DefaultEngine.ini` - so `TraceTypeQuery1` is the engine's built-in
Visibility channel, the one `FLLKWorldVisibility::VisibilityBetween` uses. That
is a config read, not an assumption, and it stops being true the day somebody
adds a channel to that file.

TWO CONTROLS PER FIXTURE, BECAUSE ONE PROVES LESS THAN IT LOOKS
---------------------------------------------------------------
A probe without a control is a guess that produces numbers. Each fixture gets
both kinds:

  1. **A height control.** The same line, from the same two points, at a height
     the fixture is *required to be clear at*. Nothing but the fixture changes
     between the two traces, so a block at 40 and a clear at 88 can only be the
     table. The bay blocks at every height the game uses and so has no height
     control - that is why it also gets the second kind.
  2. **A spatial control.** The same trace slid along the fixture's long axis
     until it is past the end, which must be clear. The offsets are not round
     numbers: they were each solved against the full collidable actor list so
     the control line stays inside an aisle. The rack grid is 500 cm in x and
     the racks are 146 cm wide, so a lazy +/-260 slide puts the control
     straight through a rack column and confounds it.

Every main line was also segment-tested against all 30 collidable actors on the
deck before being written here: each one meets only its own fixture, and every
required-clear height is unobstructed.
"""

import unreal

MAP = "/Game/LLK_Boswells_ApparelPass"
MESH_ROOT = "/Game/Props/Boswells"
TAG = "LLK_CoverFixture"

# Measured, not approximated: four slabs carry the deck and all four tops are
# at z = 662.500. The carpets sit 1.05 cm proud with collision off.
FLOOR_Z = 662.5

# The heights the game asks its question at.
TRACE_CROUCHED_CM = 40.0
TRACE_STANDING_CM = 88.0
EYE_HEIGHT_CM = 152.0

# All three meshes are BASE-PIVOT: local bounds origin z equals the half-height,
# so the base sits at local z 0 and the actor goes at FLOOR_Z exactly. Codex's
# cube-built blocks are centre-pivot and go at FLOOR_Z + height/2. Getting that
# backwards sinks a fixture 40-95 cm into the deck.
PLACEMENTS = [
    dict(mesh="SM_BSW_FoldedGoodsTable",
         label="BSW_KIDS_COVER_WEST_TABLE",
         xy=(-3400.0, -1150.0), yaw=0.0,
         size=(160.0, 90.0), height=80.0,
         blocks=[TRACE_CROUCHED_CM],
         clear=[TRACE_STANDING_CM, EYE_HEIGHT_CM],
         control=-130.0,
         why="the west half of the department has no crouch cover at all - "
             "both solid displays sit on the x=-2150 line. Mid-aisle between "
             "the -3650 and -3150 rack columns, in the main east-west band, "
             "so a keeper running west has somewhere to drop"),

    dict(mesh="SM_BSW_ServiceCounter",
         label="BSW_KIDS_COVER_COUNTER",
         xy=(-1400.0, -1150.0), yaw=0.0,
         size=(186.0, 71.0), height=102.0,
         blocks=[TRACE_CROUCHED_CM, TRACE_STANDING_CM],
         clear=[EYE_HEIGHT_CM],
         control=260.0,
         why="the first authored stand-hide-watch cover on the deck, placed "
             "114 cm north of the Sweeper's own patrol line at y=-1300 and on "
             "the way to the east bypass corridor, so the one spot you can "
             "watch the machine from is the spot you must cross"),

    dict(mesh="SM_BSW_WallMerchBay",
         label="BSW_KIDS_COVER_BAY",
         xy=(-1900.0, -1050.0), yaw=0.0,
         size=(240.0, 74.0), height=190.0,
         blocks=[TRACE_CROUCHED_CM, TRACE_STANDING_CM, EYE_HEIGHT_CM],
         clear=[],
         control=-150.0,
         why="the only hidden-and-blind cover away from the desk-and-column "
             "corner. 240 cm across the aisle the patrol walks, so it breaks "
             "the sightline and makes you pick an end blind - a decision, "
             "where a narrow blocker is a pillar you strafe past"),
]


def editor_actors():
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if subsystem is None:
        raise RuntimeError(
            "EditorActorSubsystem is None, so this process cannot spawn or "
            "enumerate actors. Nothing has been changed. Run this through "
            "Tools/run_editor_python.ps1 rather than a bare commandlet")
    return subsystem


def editor_world():
    """A world to trace in. Asked for explicitly rather than assumed, because
    a None world makes every trace return no hit - which reads as 'nothing is
    in the way' and would pass this script's checks by failing them."""
    subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = subsystem.get_editor_world() if subsystem else None
    if world is None:
        raise RuntimeError(
            "No editor world, so no trace can be trusted. A None world "
            "returns no hit for every trace, which looks exactly like clear "
            "air. Nothing has been saved")
    return world


def clear_previous():
    """Delete only our own tagged actors. Anything untagged belongs to someone
    else and is not touched."""
    removed = []
    for actor in editor_actors().get_all_level_actors():
        if actor.actor_has_tag(TAG):
            removed.append(actor.get_actor_label())
            editor_actors().destroy_actor(actor)
    return removed


def place(spec):
    mesh = unreal.load_asset(f"{MESH_ROOT}/{spec['mesh']}")
    if mesh is None:
        raise RuntimeError(f"{spec['mesh']} is not imported. Run "
                           f"import_fixtures.py first; nothing saved")

    actor = editor_actors().spawn_actor_from_object(
        mesh,
        unreal.Vector(spec["xy"][0], spec["xy"][1], FLOOR_Z),
        unreal.Rotator(0.0, 0.0, spec["yaw"]))
    actor.set_actor_label(spec["label"])
    actor.tags = [TAG]
    # The asset already defaults to BlockAll from import_fixtures.py. Set it on
    # the component too rather than trusting inheritance: a component that
    # overrides the asset's profile is the exact silent failure that whole
    # script was written to catch.
    actor.static_mesh_component.set_collision_profile_name("BlockAll")
    return actor


def blocked(world, start, end):
    """One Visibility trace. True means something is in the way."""
    hit = unreal.SystemLibrary.line_trace_single(
        world, start, end,
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,
        False,                      # simple collision, as the game traces
        [],
        unreal.DrawDebugTrace.NONE,
        True)
    return hit is not None


def probe(world, spec):
    """Is this fixture cover at the heights the game asks at, is it clear at
    the heights it is supposed to be clear at, and do both controls behave?"""
    problems = []
    x, y = spec["xy"]
    yaw = spec["yaw"] * 3.14159265358979 / 180.0
    # Short axis is the fixture's local y; the trace must cross the fixture,
    # not run down its length. A sightline along a rack's mid-span is clear at
    # every height, which is how that whole mistake was found.
    nx, ny = -unreal.MathLibrary.sin(yaw), unreal.MathLibrary.cos(yaw)
    lx, ly = unreal.MathLibrary.cos(yaw), unreal.MathLibrary.sin(yaw)
    near, far = 90.0, 320.0
    ctl = spec["control"]

    def pair(height, slide=0.0):
        keeper = unreal.Vector(x - nx * near + lx * slide,
                               y - ny * near + ly * slide, FLOOR_Z + height)
        machine = unreal.Vector(x + nx * far + lx * slide,
                                y + ny * far + ly * slide, FLOOR_Z + height)
        return keeper, machine

    for height in spec["blocks"]:
        if not blocked(world, *pair(height)):
            problems.append(
                f"{spec['label']}: a keeper at {height:.0f} cm is VISIBLE "
                f"through it. Blocking that height is the only thing that "
                f"makes this cover")
        # Spatial control: past the end of the fixture, must be clear.
        if blocked(world, *pair(height, ctl)):
            problems.append(
                f"{spec['label']}: the open-floor control at {height:.0f} cm "
                f"(slid {ctl:+.0f} cm along its length) ALSO blocked, so the "
                f"pass above proves nothing about the fixture. Something else "
                f"is in the way at this spot")

    for height in spec["clear"]:
        # Height control, and a design requirement in its own right: being
        # hidden at the chest while your eye looks over the top is the one
        # good accident on this floor, and it should fail loudly if a position
        # takes it away.
        if blocked(world, *pair(height)):
            problems.append(
                f"{spec['label']} is required to be CLEAR at {height:.0f} cm "
                f"and is blocked. Either the fixture is taller than authored "
                f"or something else stands in this line - and the height "
                f"control for its blocking heights is now worthless")

    return problems


def main():
    unreal.EditorLoadingAndSavingUtils.load_map(MAP)
    world = editor_world()

    for label in clear_previous():
        unreal.log(f"replaced previously placed {label}")

    for spec in PLACEMENTS:
        place(spec)
        unreal.log(f"placed {spec['label']} at {spec['xy']} z {FLOOR_Z} - "
                   f"{spec['why']}")

    problems = []
    for spec in PLACEMENTS:
        problems += probe(world, spec)

    if problems:
        for problem in problems:
            unreal.log_error(problem)
        raise RuntimeError(
            f"{len(problems)} problem(s). NOTHING SAVED - the map on disk is "
            f"untouched. Fix the positions and run again")

    unreal.EditorLoadingAndSavingUtils.save_current_level()
    unreal.log(f"{len(PLACEMENTS)} fixtures placed, probed against both "
               f"controls, and saved. Racks and the 7c737df encounter "
               f"untouched.")


if __name__ == "__main__":
    main()
