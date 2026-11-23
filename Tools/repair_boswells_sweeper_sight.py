"""Make the racks the player hides behind exist for the machine that hunts them.

Chris, 2026-11-23: re-check the Sweeper against the new aisles and the new
light. `Tools/audit_boswells_sweeper_sight.py` did, and found two things.

---

## 1. The floor the player sees and the floor the Sweeper sees are different

The audit read collision off every static mesh on the ground floor and asked
which ones block `ECC_Visibility`, the channel `FLLKWorldVisibility` traces.
The answer:

| | blocks sight | sees through |
| --- | --- | --- |
| `BSW_APPAREL_LOWER_*` socket bodies | **36** | 0 |
| `BSW_JUNIORS_*` dressing | 0 | **465** |
| `BSW_LEFTDAY_APPAREL_*` | 0 | **60** |
| `BSW_CIRCULATION_LOWER_STOCK_*` | 0 | **72** |
| `BSW_ARRIVAL_PERFUME_*`, `BSW_ARRIVAL_JEWELRY_*` | 0 | **30** |

**Nothing here is a bug in the tool that placed it.** `dress_boswells_juniors.py`
says so in its header - *"The existing socket bodies carry collision"* - and
its `visual_only()` helper sets `NO_COLLISION` and
`can_ever_affect_navigation = False` on purpose. Dressing should not each
carry a collider; that is how you get 600 navmesh obstacles and a floor nobody
can path across. Every tool did the right thing on its own.

The gap is what they add up to. **The player walks into a store packed with
clothing and the Sweeper walks through an open hall with thirty-six objects in
it.** A child crouching behind a rail of coats is, to the machine, standing in
an empty aisle. That is the worst shape a stealth defect can take, because the
player's own eyes tell them they are hidden and nothing on screen says
otherwise.

`docs/FINDINGS-AT-NINE-YOU-CANNOT-SEE-OVER-THE-RACKS.md` argued this floor's
stealth from the height of its fixtures. The height is real. The fixtures are
not there.

### What this changes, and what it deliberately does not

Each visual-only fixture that stands in the band a child's body occupies gets
its collision turned on as **`QUERY_ONLY`, ignoring every channel except
`ECC_Visibility`, which it blocks.** Nothing else moves:

- **`can_ever_affect_navigation` stays false**, so the navmesh is untouched and
  no rebuild is needed. `LLKSweeper.cpp` carries this project's scar from a
  decorative mesh carving nav - a Sweeper that could not path out of a hole it
  was carrying - and this does not go near it.
- **The Pawn channel stays unblocked**, so the player still walks through these
  racks exactly as they do today. **That is not a fix and it is not pretended
  to be one.** Walking through a rail of coats is wrong, and it is wrong right
  now, before this change. What this does is stop the floor being wrong in
  *two* ways at once: today the dressing neither stops you nor hides you, and
  afterwards it hides you. Making it stop you as well needs solid bodies, a
  navmesh rebuild and someone to walk the result, and that is a separate job
  with a playtest attached.

Fixtures are skipped if they are hidden in game - `dress_boswells_juniors.py`
hides the overlay rails it replaces, and a rack nobody can see must not
conceal anybody - or if they do not stand in the band at all.

## 2. One of three patrol points is inside the back wall

`ANCHOR_PATROL_2` sits at **(-1400, 2100)**. The sales plate ends at y 2090 and
the back-of-house wall runs 2090 to 2110, so the point is **ten centimetres
inside the wall**. The audit measured what it can see from there: of 179 floor
samples within its lit aggro radius, **zero**. One third of this floor's patrol
is a machine standing in masonry looking at the inside of it.

It moves to the centre line of the back-of-house walk, **y 1965** - the same
line `docs/plans/BOSWELLS-WALKWAY-AND-CEILING-RULES.md` measures the escape
route along, 125 cm off the wall in a 250 cm walk. Its x is unchanged, because
nothing is wrong with where it is along the building, only with how far back.

## What is reported and NOT changed

**Three patrol points cover 23% of the floor.** 894 of 3,942 samples are within
1,064 cm - the lit aggro radius - of any patrol point, so **more than
three-quarters of this floor is somewhere no Sweeper can notice anything,
ever.** That is a design decision about what the machine is for, not a defect
to patch, and it is left for Chris with the number attached.

Idempotent: it sets properties rather than adding actors, and re-running it
re-derives the same set. Saves the map, so it needs `-AllowUnsafeMapSave`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"

SALES_CEILING = 514.0
PLATE_BACK = 2090.0
BOH_WALK_HALF = 125.0           # a 250 cm walk, so its centre line is 125 off

PATROL_IN_WALL = "ANCHOR_PATROL_2"

# The band a child's body occupies. The keeper's capsule centre at nine is 67
# and the Sweeper's eye is 88, so a sightline between them lives between those
# two heights; anything whose top is below 60 cannot interrupt it and anything
# whose base is above 120 is over it. Generous at both ends on purpose - this
# decides what is scenery and what is cover, and the cost of being slightly
# wide is a rack that conceals from a little further away than it should.
BAND_LOW = 60.0
BAND_HIGH = 120.0

# Families placed visual-only by the dressing tools. Named rather than inferred
# because "has no collision" is also true of light housings, neon, signage and
# ceiling parts, and none of those should start concealing anybody.
DRESSING = ("BSW_JUNIORS_", "BSW_LEFTDAY_APPAREL_", "BSW_LEFTDAY_DOCK_",
            "BSW_CIRCULATION_LOWER_STOCK_", "BSW_ARRIVAL_PERFUME_",
            "BSW_ARRIVAL_JEWELRY_", "BSW_ARRIVAL_CHECKOUT_")

IGNORE = unreal.CollisionResponseType.ECR_IGNORE
BLOCK = unreal.CollisionResponseType.ECR_BLOCK


def say(message):
    unreal.log("SWEEPFIX " + message)
    print("SWEEPFIX " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("SWEEPFIX " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()


# =====================================================================
# 1. The dressing becomes cover
# =====================================================================

made = 0
skipped_hidden = 0
skipped_band = 0
already = 0
for actor in actors:
    label = actor.get_actor_label()
    if not label.startswith(DRESSING):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if centre.z - extent.z > SALES_CEILING:
        continue                                   # the storey above
    if actor.get_editor_property("hidden"):
        skipped_hidden += 1
        continue
    if actor.is_hidden_ed():
        skipped_hidden += 1
        continue
    base, top = centre.z - extent.z, centre.z + extent.z
    if top < BAND_LOW or base > BAND_HIGH:
        skipped_band += 1
        continue

    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    touched = False
    for comp in comps:
        if comp.get_editor_property("static_mesh") is None:
            continue
        if (comp.get_collision_enabled() != unreal.CollisionEnabled.NO_COLLISION
                and comp.get_collision_response_to_channel(
                    unreal.CollisionChannel.ECC_VISIBILITY) == BLOCK):
            continue                               # already cover, leave it
        comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)
        comp.set_collision_response_to_all_channels(IGNORE)
        comp.set_collision_response_to_channel(
            unreal.CollisionChannel.ECC_VISIBILITY, BLOCK)
        # The line this project learned the hard way. A decorative mesh with
        # collision carves the navmesh unless this is false, and in Boswell's
        # the navmesh is Dynamic, so it would carve it at runtime.
        comp.set_editor_property("can_ever_affect_navigation", False)
        touched = True
    if touched:
        # Actor-level collision overrides the component, and every dressing
        # tool turned it off. Without this the component settings above are
        # written and inert, which would look exactly like a working fix.
        actor.set_actor_enable_collision(True)
        made += 1
    else:
        already += 1

say("dressing: {} fixtures now block sight, {} already did, "
    "{} skipped as hidden, {} skipped as outside the {:.0f}-{:.0f} cm band"
    .format(made, already, skipped_hidden, skipped_band, BAND_LOW, BAND_HIGH))
require(made > 0, "nothing was changed - the dressing prefixes no longer match "
                  "anything in this map and this tool needs rewriting")


# =====================================================================
# 2. The patrol point in the wall
# =====================================================================

moved = 0
for actor in actors:
    if actor.get_actor_label() != PATROL_IN_WALL:
        continue
    loc = actor.get_actor_location()
    require(loc.y > PLATE_BACK - BOH_WALK_HALF,
            "%s is at y=%.0f, already on the floor - this repair's premise is "
            "gone and it should be deleted rather than run" % (PATROL_IN_WALL, loc.y))
    target = PLATE_BACK - BOH_WALK_HALF
    actor.set_actor_location(unreal.Vector(loc.x, target, loc.z), False, True)
    say("patrol: {} moved from y={:.0f} (inside the back wall) to y={:.0f}, "
        "the centre line of the back-of-house walk".format(
            PATROL_IN_WALL, loc.y, target))
    moved += 1
require(moved == 1, "expected exactly one %s, found %d" % (PATROL_IN_WALL, moved))


saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))
