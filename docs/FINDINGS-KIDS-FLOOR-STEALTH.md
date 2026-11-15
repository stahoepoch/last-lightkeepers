# The children's floor: the racks are not cover, and ducking behind one makes it worse

**Status: measured in the live editor, 2026-11-14, against `83e5b60`. No map
edits — this was read-only, as asked.** Written for row 92's terms: a player
must be able to *"move through a believable store, hide, flee"*. This is about
the hide and the flee.

> ## ⚠ SUPERSEDED FOR THE CURRENT MAP — do not cite these conclusions
>
> **Commit `7c737df` ("Build children's-floor signs and hide encounter")
> rebuilt this floor on 2026-11-14, after these measurements were taken.**
> Verified against the live map on 2026-11-15 by commandlet read. Four things
> below are no longer the map:
>
> 1. **"There is no Sweeper up there."** There is now: `BSW_KIDS_SWEEPER`, an
>    `LLKSweeper`, at `(-850, -1300, 750.5)`.
> 2. **"Nothing patrols the children's department."** Three live points do:
>    `BSW_KIDS_PATROL_1/2/3` at `(-1750,-1300)`, `(-850,-1300)`, `(-850,-250)`.
> 3. **The three `ANCHOR_PATROL` markers are still at `z = 110`** — that part
>    holds — but they were retagged `LLK.Patrol` → `LLK.Patrol.Lower` and are
>    inert, and `ANCHOR_SWEEPER_CONCOURSE` was deleted. The lower floor now has
>    no Sweeper and no live patrol.
> 4. **"Ducking behind a rack is the wrong move"** is still true of the 21
>    remaining racks, but two racks at `(-2150, -1650)` and `(-2150, -500)` were
>    replaced with solid displays collidable to **78 cm above the floor**,
>    built specifically so that crouching works. The rack count is 21, not 23.
>
> **Two further corrections to what is written below.** The column is
> **44 × 44 cm** — narrower than a standing capsule, so the claim that it is
> real cover needs re-measuring, not quoting. And the floor is at **z = 662.500
> exactly**, not "≈663"; all three cover fixtures are base-pivot and belong at
> that z, not at floor + height/2.
>
> The sight and escape conclusions below were taken against a deck with no
> displays, no patrol and no machine on it. **They do not describe the deck as
> it now stands and have not been re-run.** The geometry tables are still
> correct for the geometry they name; the play conclusions are not.
>
> Full current-state read, with every collidable actor's bounds and a free-floor
> grid: the layout read left for the MCP session in this group's shared notes,
> `KIDS-DECK-LAYOUT-READ.md`. It also records four pre-existing collidable
> intersections on this deck (`UPPER_BEDDING` and `UPPER_SEASONAL` against
> three racks and the sales desk) that confound any cover reading taken in
> those two aisles.

Codex asked for Sweeper **sight, hearing and escape** around the new racks and
column. Two of those three have answers. The third does not exist.

## 0. There is no Sweeper up there, and no hearing anywhere

**No `LLKSweeper` is placed in `LLK_Boswells_ApparelPass` at all**, and all
three `ANCHOR_PATROL` markers sit at `z = 110`, on the lower floor. So nothing
patrols the children's department today and none of this is observable in play
yet. Everything below is the geometry's answer to what *would* happen, which is
the useful thing to know before a patrol is authored rather than after.

**The Sweeper cannot hear.** There is no noise, sound or volume input anywhere
in the enemy code — every `Volume` in `LLKSweeper.cpp` is cue playback, output
rather than input. Notice is light and line of sight only. That is a deliberate
old position (*"wiring volume into aggro is a second notice rule"*), but row 92
asks for stealth as a pillar, and a stealth machine that cannot hear you is a
design decision that should now be re-taken on purpose rather than inherited.
**Nothing was built for it here.**

## 1. How sight actually works, which decides everything else

`LLKPerception.cpp:256` → `FLLKWorldVisibility::VisibilityBetween`, and two
details matter more than the radii:

- The trace is **`ECC_Visibility`**, not `ECC_WorldStatic`. A fixture that does
  not block that channel is not cover no matter how solid it looks.
- The endpoints are **`GetActorLocation()` on both sides** —
  `LLKSweeper.cpp:327` and `:366`. That is the capsule **centre**, not the eye.

So the game decides whether you are seen at **88 cm** standing and **40 cm**
crouched above the floor, while you look out from 152 cm. Cover is judged at
your chest, not your head.

## 2. The measurement

No actor-bounds tool exists on this bridge, so rack heights could not be read.
`ScreenCoordsToWorld` is a trace, so the camera was placed at a would-be
observer's chest, aimed at a would-be keeper's chest, and the hit distance read
— the same question the bounds would have answered, asked of an instrument that
can answer it. **Every sweep included open-floor controls**, which came back
clear as they should; a probe without a control is a guess that produces
numbers.

One rack, at `(-2150, -900)` on the `z ≈ 663` deck, probed 220 cm either side:

| height above floor | across (0°) | 30° | 45° | 60° | along (90°) |
| --- | --- | --- | --- | --- | --- |
| 10–70 cm | **blocks** | clear | clear | clear | clear |
| 88–130 cm | **blocks** | **blocks** | **blocks** | **blocks** | clear |
| 150–170 cm | clear | clear | clear | clear | clear |

Footprint, crossed at 20 cm intervals: the rack is solid only between
`y = -920` and `y = -880` and its near face is **67 cm** from centre, so it is
about **134 cm across by 60 cm deep**, roughly **130–150 cm tall**.

## 3. What that means, and it is not what the fixture looks like

**The racks are not a screen. They are two end posts with air between them.**
A sight line along the rack's mid-span is clear at *every height tested*; one
down an upright blocks. That is the shape of a real garment rack, and it is
honest modelling — but it means a keeper standing behind the middle of a rack
is simply visible.

**Crouching behind a rack makes you more visible, not less.** The only band
that blocks at an angle is **88–130 cm**. Crouch and your traced point drops to
40 cm — *below* the cover. At 30°, 45° and 60° the rack blocks a standing
keeper and does nothing for a crouched one. Only dead-on across the rack does
crouching still work.

That inverts the thing row 92 asks for in Chris's own words — *"duck behind
counters"*. On this floor, ducking behind a rack is the wrong move and the game
will not tell you. **This is the one finding here I would act on.**

**The column is real cover.** `BSW_KIDS_CEILING_SALES_COLUMN` at
`(-2550, -1970)` blocked at both heights, floor to ceiling. It is the only
dependable hiding place in the department, it is one object, and it is beside
the sales desk — so the desk-and-pillar corner is currently the whole stealth
vocabulary of the floor.

**One accident worth keeping.** The rack tops out at 130–150 cm and the trace
reads your chest at 88, while your eye is at 152. So you can stand behind a
rack, see the machine over it, and remain unseen. That is a genuinely good
stealth affordance and nobody designed it; it falls out of tracing centres
instead of eyes. Worth making deliberate before someone "fixes" the trace to
use eye height and deletes it.

## 4. Escape is fine, and that is worth saying plainly

The rack grid is **400–500 cm** apart in x and the rack lines **400 cm** apart
in y; with a 60 cm deep rack the walking gap between lines is about **340 cm**.
Against a 68 cm capsule and Recast's inset that is not close to a pinch
anywhere in the department. No equivalent of the lower floor's 60 cm aisle
closure exists up here.

## 5. What I would change, in order — none of it done

1. **Decide what a rack is for.** If it is meant to hide a crouching player, it
   needs a solid band below 40 cm (a skirt, a stock shelf, folded goods). If it
   is meant to be see-through dressing, then the department needs real cover
   objects placed on purpose and the racks should stop looking like cover.
2. **Give the floor more than one column.** One dependable blocker in a
   department this size means one hiding place.
3. **Re-take the hearing decision**, explicitly, as a row. Not inherited.
4. **Place a patrol** before any of the above is judged in play — none of this
   has been seen, only measured.

Fixture heights and the block-in CSV were not touched, per the handoff.
