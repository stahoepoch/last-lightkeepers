"""Place three Boswell's cover fixtures on the children's deck of
LLK_Boswells_ApparelPass, then prove each one blocks a sight trace.

The table and wall bay temporarily use hidden engine-cube collision proxies.
Their imported simple collision does not reliably cook; the counter's does.
Keep the proxies until the import defect is fixed and re-tested after reload.

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
    re-run deletes its own tag first, including the two collision proxies,
    so it replaces rather than stacks.
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
PROXY_MESH = "/Engine/BasicShapes/Cube.Cube"
# These two imported meshes render correctly but their simple collision has
# failed repeated Visibility traces. The proxy is a measured, temporary cover
# body; the import defect remains open for a separate investigation.
PROXY_COVER = {"SM_BSW_FoldedGoodsTable", "SM_BSW_WallMerchBay"}

# Measured, not approximated: four slabs carry the deck and all four tops are
# at z = 662.500. The carpets sit 1.05 cm proud with collision off.
FLOOR_Z = 662.5

# The heights the game asks its question at.
# The keeper's body, and these three numbers are the whole reason this kit
# exists at the sizes it does. They are NOT independent of the game: sight is
# traced between capsule centres and the player looks out from the camera, so
# all three are functions of the capsule authored in
# Source/LLK/Private/Lantern/LLKLanternFeel.cpp.
#
# He is a boy, 145 cm, since 2026-11-15. He was a 176 cm adult before that,
# which Chris found by standing next to the racks. Written the same way the
# C++ writes it - one authored height and one scale - so the two can be
# compared mechanically instead of by eye. tests/asset-deps.test.mjs fails if
# they drift.
KEEPER_HEIGHT_CM = 145.0
_K = KEEPER_HEIGHT_CM / 176.0

TRACE_STANDING_CM = 88.0 * _K
TRACE_CROUCHED_CM = 40.0 * _K
EYE_HEIGHT_CM = (88.0 + 64.0) * _K

# 5.8 exposes the channel by name, which settles what TRACE_TYPE_QUERY1 only
# implied - the older name is the first built-in trace channel, and reading
# Config/DefaultEngine.ini for an absent [/Script/Engine.CollisionProfile]
# section was the long way round to the same answer. getattr rather than a
# straight switch because the named form is reported, not verified here, and a
# missing enum member would take the script down at import; the numbered form
# is measured working. Whichever resolves, prove_the_instrument checks it
# against known geometry before anything is spawned, which is the only
# guarantee that matters.
TRACE_CHANNEL = getattr(unreal.TraceTypeQuery, "ECC_VISIBILITY",
                        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1)

# All three meshes are BASE-PIVOT: local bounds origin z equals the half-height,
# so the base sits at local z 0 and the actor goes at FLOOR_Z exactly. Codex's
# cube-built blocks are centre-pivot and go at FLOOR_Z + height/2. Getting that
# backwards sinks a fixture 40-95 cm into the deck.
PLACEMENTS = [
    dict(mesh="SM_BSW_FoldedGoodsTable",
         label="BSW_KIDS_COVER_WEST_TABLE",
         xy=(-3400.0, -1150.0), yaw=0.0,
         size=(160.0, 90.0), height=69.0,
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
         # The thinnest control of the three: 30 cm past the bay's own end and
         # 27 cm off RACK_-2150_-900. That holds for the zero-radius line
         # trace this fires, and was checked independently, but it leaves no
         # room at all. **If this fixture moves, re-solve the offset rather
         # than keeping -150** - the aisle is only 354 cm wide and the control
         # has to clear both the bay and the rack column.
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

    # spawn_actor_from_class, not spawn_actor_from_object. The latter returns
    # None in a commandlet and the next line dies on NoneType - the same
    # family as StaticMeshEditorSubsystem being None headless. This is the
    # route all eleven Tools/ build scripts use, and set_static_mesh returns a
    # bool that is checked rather than assumed, because an actor that spawned
    # with no mesh on it is an invisible nothing that still passes a label
    # check.
    actor = editor_actors().spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(spec["xy"][0], spec["xy"][1], FLOOR_Z),
        unreal.Rotator(0.0, 0.0, spec["yaw"]))
    if actor is None:
        raise RuntimeError(
            f"{spec['label']} did not spawn. Nothing saved")
    if not actor.static_mesh_component.set_static_mesh(mesh):
        raise RuntimeError(
            f"{spec['label']} spawned but {spec['mesh']} would not attach to "
            f"it, so it is an empty actor standing where cover should be. "
            f"Nothing saved")
    actor.set_actor_label(spec["label"])
    # The repo's working idiom, from build_boswells_kids_stealth.py:107, with
    # an explicit unreal.Name rather than a str left to coerce. This matters
    # more than it looks: clear_previous() finds its own work by this tag, so
    # a tag that silently failed to stick means the SECOND run stacks three
    # more fixtures on the first three instead of replacing them. The re-run
    # guard is what protects the map from this script, so it is read back
    # below rather than assumed.
    actor.set_editor_property("tags", [unreal.Name(TAG)])
    if not actor.actor_has_tag(TAG):
        raise RuntimeError(
            f"{spec['label']} did not keep its {TAG} tag, so a re-run could "
            f"not find it to replace. Refusing to continue: the next run "
            f"would stack fixtures rather than replace them. Nothing saved")
    # Only one body owns collision. The table and bay have unreliable cooked
    # bodies; their separately tagged proxy owns both Pawn and Visibility.
    actor.static_mesh_component.set_collision_profile_name(
        "NoCollision" if spec["mesh"] in PROXY_COVER else "BlockAll")
    return actor


def place_proxy(spec, where=None):
    """A hidden-in-game engine cube with the fixture's measured solid bounds.

    The built-in cube already has working simple collision. Its 100 cm unit
    bounds are scaled to the fixture footprint and height, with its centre
    half a fixture height above the floor. This is a collision workaround,
    never a substitute for proving the sightline in the saved level.
    """
    cube = unreal.load_asset(PROXY_MESH)
    if cube is None:
        raise RuntimeError(f"Engine collision cube missing: {PROXY_MESH}")
    x, y = where if where is not None else spec["xy"]
    actor = editor_actors().spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(x, y, FLOOR_Z + spec["height"] / 2.0),
        unreal.Rotator(0.0, 0.0, spec["yaw"]))
    if actor is None or not actor.static_mesh_component.set_static_mesh(cube):
        raise RuntimeError(f"Collision proxy for {spec['label']} did not spawn")
    actor.set_actor_scale3d(unreal.Vector(
        spec["size"][0] / 100.0,
        spec["size"][1] / 100.0,
        spec["height"] / 100.0))
    actor.static_mesh_component.set_collision_profile_name("BlockAll")
    actor.set_actor_enable_collision(True)
    actor.set_actor_hidden_in_game(True)
    actor.set_actor_label(spec["label"] + "_COLLISION")
    actor.set_editor_property("tags", [unreal.Name(TAG)])
    if not actor.actor_has_tag(TAG):
        raise RuntimeError(
            f"{spec['label']} proxy did not keep its replacement tag")
    return actor


# How many times every trace is fired before its answer is believed, and the
# reason the number is not 1.
#
# Two wrong claims landed today on single green observations - an asset-level
# assert that counted boxes, and a Nanite fix verified once - against a system
# that has since been shown to give different answers to the identical
# question in different processes AND in the same process minutes apart. A
# flaky result read once is indistinguishable from a true one.
#
# So a trace is not a measurement here; the agreement of several is. Traces
# are cheap and there are a few dozen of them, so this costs nothing worth
# counting.
TRACE_REPEATS = 5


def blocked_stable(world, start, end, what):
    """Fire the same trace TRACE_REPEATS times and refuse to summarise a
    disagreement.

    **Disagreement is its own outcome, not a tie to be broken.** A majority
    vote here would be the most dangerous thing in this file: it would convert
    "this system is not reproducible" into a clean boolean and let the
    placement proceed on it. That is precisely the failure that put two wrong
    claims in the log today. So this returns the answer only when every run
    agrees, and raises when they do not.
    """
    answers = [blocked(world, start, end) for _ in range(TRACE_REPEATS)]
    if all(answers):
        return True
    if not any(answers):
        return False
    raise RuntimeError(
        f"UNSTABLE: {what} gave {sum(answers)} blocked and "
        f"{len(answers) - sum(answers)} clear out of {len(answers)} identical "
        f"traces in one process. Nothing has been saved. This is not a "
        f"failed check - it is the check reporting that the thing it "
        f"measures does not hold still, and no number of further runs fixes "
        f"that. Do not place cover on a body that flickers")


def blocked(world, start, end):
    """One Visibility trace. True means something is in the way.

    MEASURED, 2026-11-15, after a round trip that cost a cycle. The binding
    returns an `FHitResult` for a blocking hit and `None` otherwise: a line
    across the 44 cm sales column returns a HitResult, a line down the open
    cross aisle returns None. So `result is not None` is the whole test.

    **There is no more careful version of this available.** Every field on a
    Python `HitResult` in UE 5.8 is protected and unreadable - `bBlockingHit`,
    `Distance`, `Location` and `Time` all refuse - and
    `GameplayStatics.break_hit_result` is not in the bindings. The return
    itself is the only signal there is.

    This was briefly rewritten to unpack a `(bool, HitResult)` tuple, on a
    warning that the binding collapsed the out param into the return. The
    warning was a guess and it was wrong, and the rewrite's HitResult branch
    raised on the one shape that actually occurs. **The lesson is not "the
    guess was wrong"** - it is that the defensive branch added to survive an
    unverified claim read a property nobody had verified was readable, so the
    hedge was more fragile than the thing it hedged. `prove_the_instrument`
    below is the honest version of that instinct: it does not speculate about
    the shape, it checks the answer against geometry whose answer is known.
    """
    result = unreal.SystemLibrary.line_trace_single(
        world, start, end,
        TRACE_CHANNEL,
        False,                      # simple collision, as the game traces
        [],
        unreal.DrawDebugTrace.NONE,
        True)
    return result is not None


def prove_the_instrument(world):
    """Fire two traces whose answers are already known, and refuse to go on if
    either is wrong.

    This is the check that makes the probes worth running. Every failure mode
    of `blocked` above - the tuple read as a truthy object, a None world, a
    trace channel that is not the one the game uses - produces a uniform
    answer: everything blocked, or everything clear. Two traces with opposite
    known answers catch all of them, and no amount of reading the binding
    documentation does.

    The geometry is chosen so that only one thing can be responsible:

      - `BSW_KIDS_CEILING_SALES_COLUMN` at (-2550,-1970) is 44 x 44 cm and runs
        floor to ceiling. A line crossing it meets the column and nothing else
        at any of the three heights - the sales desk stops 13 cm west of it.
      - The cross aisle at x = -3530, from y -1240 to -830, is clear of all 30
        collidable actors on the deck at all three heights, and stays clear
        after these three fixtures are placed.
    """
    checks = [
        ("the 44 cm sales column", True,
         unreal.Vector(-2550.0, -2100.0, 0.0),
         unreal.Vector(-2550.0, -1850.0, 0.0)),
        ("the open cross aisle at x=-3530", False,
         unreal.Vector(-3530.0, -1240.0, 0.0),
         unreal.Vector(-3530.0, -830.0, 0.0)),
    ]
    for label, expected, start, end in checks:
        for height in (TRACE_CROUCHED_CM, TRACE_STANDING_CM, EYE_HEIGHT_CM):
            s = unreal.Vector(start.x, start.y, FLOOR_Z + height)
            e = unreal.Vector(end.x, end.y, FLOOR_Z + height)
            got = blocked_stable(world, s, e,
                                 f"instrument {label} @{height:.0f}")
            if got != expected:
                raise RuntimeError(
                    f"INSTRUMENT FAILED: {label} at {height:.0f} cm read "
                    f"{'blocked' if got else 'clear'} and is known to be "
                    f"{'blocked' if expected else 'clear'}. Every probe below "
                    f"would be meaningless, so nothing was spawned and "
                    f"nothing saved. Check the trace channel, the world, and "
                    f"how line_trace_single's result is being read")
    unreal.log("instrument proven: the column blocks and the open aisle does "
               "not, at 40, 88 and 152 cm")


def preflight_the_fixtures(world):
    """Prove each fixture's chosen collision body before map placement.

    The instrument check above proves the trace works, using the map's own
    geometry. It does not prove the three assets do, and that gap is exactly
    where two of them hid: they passed every asset-level check, they had a
    working instrument fired at them, and they blocked nothing.

    The counter uses its imported body. The table and bay use measured engine
    cube proxies while their importer defect is investigated. Each source is
    spawned well clear of the department, traced, then destroyed again.

    Suggested by the session that found the asset defect, with the right
    reasoning: this would have caught it on the first run instead of the
    ninth.
    """
    problems = []
    for spec in PLACEMENTS:
        mesh = unreal.load_asset(f"{MESH_ROOT}/{spec['mesh']}")
        if mesh is None:
            problems.append(f"{spec['mesh']} is not imported")
            continue

        is_proxy = spec["mesh"] in PROXY_COVER
        where = unreal.Vector(60000.0, 0.0, FLOOR_Z if is_proxy else 0.0)
        half = spec["size"][1] / 2.0
        twin = None
        try:
            if is_proxy:
                twin = place_proxy(spec, (where.x, where.y))
            else:
                twin = editor_actors().spawn_actor_from_class(
                    unreal.StaticMeshActor, where, unreal.Rotator(0.0, 0.0, 0.0))
                if twin is None or not twin.static_mesh_component.set_static_mesh(
                        mesh):
                    problems.append(f"{spec['mesh']} would not spawn for preflight")
                    continue
                twin.static_mesh_component.set_collision_profile_name("BlockAll")
            for height in spec["blocks"]:
                start = unreal.Vector(where.x, where.y - half - 90.0,
                                      where.z + height)
                end = unreal.Vector(where.x, where.y + half + 320.0,
                                    where.z + height)
                if not blocked_stable(world, start, end,
                                      f"preflight {spec['mesh']} @{height:.0f}"):
                    problems.append(
                        f"{spec['mesh']} {'proxy' if is_proxy else 'asset'} "
                        f"does not block at {height:.0f} cm in EMPTY SPACE. "
                        f"Nothing saved")
        finally:
            if twin is not None:
                editor_actors().destroy_actor(twin)

    if problems:
        for problem in problems:
            unreal.log_error(problem)
        raise RuntimeError(
            f"{len(problems)} fixture collision source(s) failed before placement was "
            f"even attempted. The map has not been touched")
    unreal.log("preflight: counter body and two proxy bodies block in empty space, "
               f"{TRACE_REPEATS} consecutive traces each")


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
        if not blocked_stable(world, *pair(height),
                              what=f"{spec['label']} blocks @{height:.0f}"):
            problems.append(
                f"{spec['label']}: a keeper at {height:.0f} cm is VISIBLE "
                f"through it. Blocking that height is the only thing that "
                f"makes this cover")
        # Spatial control: past the end of the fixture, must be clear.
        if blocked_stable(world, *pair(height, ctl),
                          what=f"{spec['label']} control @{height:.0f}"):
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
        if blocked_stable(world, *pair(height),
                          what=f"{spec['label']} clear @{height:.0f}"):
            problems.append(
                f"{spec['label']} is required to be CLEAR at {height:.0f} cm "
                f"and is blocked. Either the fixture is taller than authored "
                f"or something else stands in this line - and the height "
                f"control for its blocking heights is now worthless")

    return problems


def box_extents(mesh):
    """The measurement nobody on either side of this has taken.

    Every argument so far has counted `box_elems == 1` and read a profile
    string. **A box element of the wrong size, or of no size, satisfies both
    and collides with nothing.** One property read settles whether the build
    path produced a usable box, and it should come before any further theory
    about Nanite or world partition.
    """
    try:
        body = mesh.get_editor_property("body_setup")
        elems = body.get_editor_property("agg_geom").get_editor_property(
            "box_elems")
        return [(e.get_editor_property("x"), e.get_editor_property("y"),
                 e.get_editor_property("z")) for e in elems]
    except Exception as error:  # noqa: BLE001 - diagnostic, never fatal
        return f"unreadable ({error})"


def diagnose(world, spec, actor):
    """Run only when a probe has already failed. Reports the facts that tell
    the live hypotheses apart, rather than adding another one.

    As of 2026-11-15 the open question is why these fixtures block at
    (6000,0,0) and not at their authored coordinates on the deck, while the
    counter blocks in both places. The candidates are the map's world
    partition, the z height specifically, the body not being where the mesh is
    drawn, and a degenerate collision box. Each line below rules one in or
    out, and all of them are cheap reads or one extra spawn.
    """
    mesh = unreal.load_asset(f"{MESH_ROOT}/{spec['mesh']}")
    x, y = spec["xy"]
    report = [f"--- {spec['label']} ---"]

    # Is the actor where we asked, and do its COLLIDING components have any
    # extent at all? A body registered at the origin rather than at the spawn
    # transform, and a mesh whose collision bounds are empty, both show here.
    report.append(f"  actor location   {actor.get_actor_location()}")
    try:
        origin, extent = actor.get_actor_bounds(True)
        report.append(f"  colliding bounds origin {origin} extent {extent}")
    except Exception as error:  # noqa: BLE001
        report.append(f"  colliding bounds unreadable ({error})")

    report.append(f"  box elems (x,y,z) {box_extents(mesh)}")
    component = actor.static_mesh_component
    report.append(f"  collision enabled {component.get_collision_enabled()} "
                  f"profile {component.get_collision_profile_name()}")

    # Straight down onto the fixture from well above it. If this blocks while
    # the horizontal probe does not, the body is present and the probe line is
    # the problem; if it also misses, the body is not there.
    top = unreal.Vector(x, y, FLOOR_Z + 400.0)
    floor = unreal.Vector(x, y, FLOOR_Z + 1.0)
    # Counted, not sampled. The whole open question is whether this body
    # answers the same way twice, so a diagnostic that fires once and prints
    # BLOCK or clear is throwing away the only interesting variable.
    down = sum(blocked(world, top, floor) for _ in range(TRACE_REPEATS))
    report.append(f"  trace down onto it: {down}/{TRACE_REPEATS} blocked")

    # Two duplicates of the same mesh in the same process and the same frame,
    # each varying exactly one thing. Far away at the same z isolates x/y;
    # same x/y at z 0 isolates the height. Both are destroyed again.
    for label, where in (("far away, same z", unreal.Vector(x + 50000.0, y, FLOOR_Z)),
                         ("same x/y, z=0", unreal.Vector(x, y, 0.0))):
        twin = None
        try:
            twin = editor_actors().spawn_actor_from_class(
                unreal.StaticMeshActor, where, unreal.Rotator(0.0, 0.0, 0.0))
            twin.static_mesh_component.set_static_mesh(mesh)
            twin.static_mesh_component.set_collision_profile_name("BlockAll")
            half = spec["size"][1] / 2.0
            start = unreal.Vector(where.x, where.y - half - 90.0,
                                  where.z + TRACE_CROUCHED_CM)
            end = unreal.Vector(where.x, where.y + half + 320.0,
                                where.z + TRACE_CROUCHED_CM)
            hits = sum(blocked(world, start, end)
                       for _ in range(TRACE_REPEATS))
            report.append(f"  twin {label}: {hits}/{TRACE_REPEATS} blocked "
                          f"at 40 cm")
        except Exception as error:  # noqa: BLE001
            report.append(f"  twin {label}: could not test ({error})")
        finally:
            if twin is not None:
                editor_actors().destroy_actor(twin)

    return report


def main():
    unreal.EditorLoadingAndSavingUtils.load_map(MAP)
    world = editor_world()

    # Before anything is spawned, and before any map change: does a trace in
    # this process actually answer the question?
    prove_the_instrument(world)
    preflight_the_fixtures(world)

    for label in clear_previous():
        unreal.log(f"replaced previously placed {label}")

    placed = {}
    for spec in PLACEMENTS:
        placed[spec["label"]] = place(spec)
        if spec["mesh"] in PROXY_COVER:
            place_proxy(spec)
        unreal.log(f"placed {spec['label']} at {spec['xy']} z {FLOOR_Z} - "
                   f"{spec['why']}")

    problems = []
    failed = []
    for spec in PLACEMENTS:
        found = probe(world, spec)
        problems += found
        if found:
            failed.append(spec)

    if problems:
        for problem in problems:
            unreal.log_error(problem)
        # A failing run should hand back a diagnosis, not just a verdict. The
        # process is already loaded, the actors are already standing there,
        # and the reads below cost nothing next to another round trip.
        unreal.log_warning("--- diagnostics for the failing fixtures ---")
        for spec in failed:
            for line in diagnose(world, spec, placed[spec["label"]]):
                unreal.log_warning(line)
        raise RuntimeError(
            f"{len(problems)} problem(s). NOTHING SAVED - the map on disk is "
            f"untouched. Fix the positions and run again")

    # save_map's return is CHECKED, which save_current_level's was not. All
    # eleven build scripts in Tools/ use this idiom, and the reason is the one
    # this kit has already been bitten by: add_simple_collisions returns -1 on
    # failure without raising, so the import script carried on and asserted
    # against collision it had never added. A save that logs success whether
    # or not it saved is the same fault in the same kit, one file along.
    if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAP):
        raise RuntimeError(
            "the map did NOT save, and every probe above passed. The fixtures "
            "exist only in this process and vanish when it exits. Check the "
            "package is not read-only or held by an open editor")

    unreal.log(f"{len(PLACEMENTS)} fixtures placed (two with collision proxies), probed against both "
               f"controls, and saved. Racks and the 7c737df encounter "
               f"untouched.")


if __name__ == "__main__":
    main()
