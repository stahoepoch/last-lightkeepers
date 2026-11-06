# The Sweeper cannot reach the misrouted crate, and it has been failing in a loop

**Status: measured live in PIE, 2026-11-05. Not fixed — the fix moves authored
dressing and that is Chris's call. This is the first time anything in this
project has been read out of a running game rather than out of the source.**

Found while testing the extraction problem with the editor bridge. It is not
the extraction problem. It is worse, and it is the beat the whole Sweeper
exists to perform.

## What the running game says

`LogLLK`, from a PIE session on `LLK_Boswells_FirstRun`, repeating roughly
every three seconds for as long as the session runs:

```
LogLLK: LLKSweeper_0 began filing misplaced stock.
LogLLK: Warning: LLKSweeper_0 could not reach the misplaced stock it noticed
        and gave up. Check the navmesh between it and the relic.
LogLLK: LLKSweeper_0 began filing misplaced stock.
LogLLK: Warning: LLKSweeper_0 could not reach the misplaced stock it noticed
        and gave up.
```

Notice → walk → give up → notice again. **The loop never completes and never
stops**, so the Sweeper's entire reason for existing on this floor never
happens. `C2`'s four cues are *started*, *interrupted*, *filed* and the
telegraph; two of those three stock cues can never fire, because the job is
abandoned before it can be interrupted or completed.

The warning was deliberately written as a map problem rather than a cue —
`LLKSweeper.cpp:629`: *"An unreachable relic should be findable as a warning
rather than as a Sweeper that quietly stopped filing."* It worked. Nobody had
read the log.

## Why it cannot reach it, in centimetres

The abandon is `LLKSweeperBrain.cpp:239` — `DistanceToMisplacedStuds >
Contact.ContactRange` sustained for the approach window. `contactRange` is
**5 studs, 140 cm** (`data/lantern.json`).

Measured from the live editor, the crate and its neighbours:

| Actor | Position | Distance to crate |
| --- | --- | --- |
| `BSW_WRONG_CRATE` | `(-520, -40, 130)` | — |
| `WL_RecessLip` | `(-535, -40, 178)` | 50 cm |
| `WL_LabelCounter` | `(-650, -40, 95)` | 135 cm |
| nearest aisle dressing (`DRESS_Rack_Deck_WS_2_*`) | `(-339.5, -158.3, …)` | 219 cm |

**The crate sits in a recess, behind a lip, beside a counter.** The open floor
a pawn can stand on begins out at the rack line, `x = -339.5`. The crate is at
`x = -520`. That is **180.5 cm of horizontal separation** before accounting for
the 34 cm capsule radius that keeps the Sweeper's centre off the geometry, and
Recast insets the navmesh by the agent radius again on top of that.

So the closest the Sweeper's origin can plausibly get is on the order of
180–215 cm from the crate, against a `contactRange` of **140 cm**. It is short
by roughly 40–75 cm, every time, forever. That is why it is a clean repeating
loop rather than an intermittent failure: nothing about it is marginal in the
sense of sometimes working.

## What this is not

**Not the extraction bug.** That was the reason the editor was opened, and the
same session cleared the thing I thought was most likely — see
`FINDINGS-EXTRACTION-NEVER-COMPLETES.md`, where `M13` is now disproven.

**Not a navmesh hole, necessarily.** The warning text says *"check the navmesh
between it and the relic"*, and the arithmetic says the floor may be perfectly
navigable right up to the recess — the crate is simply **further into the
recess than the Sweeper's arm is long**. Those have different fixes and the
warning only guesses at the first one. Worth correcting in the message.

**Not `ArriveToleranceStuds`.** That is 3 studs, 84 cm, and it governs path
arrival rather than this check. The gate that fails is `contactRange`.

## The three fixes, cheapest first — Chris's call

1. **Move the crate out of the recess**, onto the counter face or the aisle
   side of the lip, so standable floor comes within 140 cm. Smallest change,
   and it keeps every number. The crate is a *misrouted* crate: it is supposed
   to be somewhere wrong, not somewhere unreachable, and a shopper could not
   reach into that recess either.
2. **Raise `contactRange`**, which is a balance value and so his — but it is
   the same number that decides when a Sweeper clips the keeper's feet
   (`LLKSweeperBrain.cpp:338`), so widening it to fix a reach problem also
   widens the stumble, and that is a real cost for an unrelated gain.
3. **Open the recess**, which is dressing geometry and the most work.

Recommendation is (1), and I did not do it: relic placement against authored
dressing is a content decision, and the scope note is explicit that the map
authors spawn spacing rather than the pawn inventing it.

## Two smaller things the same session established

**The `WL_` pattern again.** Both actors boxing the crate in — `WL_RecessLip`
and `WL_LabelCounter` — carry the `WL_` prefix that `C12` already flagged after
four orphans turned up, one of which (`WL_DarkPocket_Partition`) is the other
open map defect. That is now five, and the common factor is that `WL_` actors
are somebody's temporary rig that authored content has since been placed
against.

**Everything else in the log was correct.** The same PIE run confirmed, in
engine, every number the docs were corrected to this week: walk 224, sprint
350, crouch 126 cm/s; `LLKSweeper_0` at 168 cm/s noticing a dark keeper at 12
studs and a lit one at 38; lantern radius 30 studs / 840 cm, cone 110°,
intensity 25, detectability 1.00; and *"jump 370 cm/s at gravity x1.25 — 56cm
apex over 0.60s"*, which is the arithmetic in `THE-HARD-RUN.md` to the
centimetre and the hundredth of a second. The speed-scale corrections hold
against the running game, not just against `data/`.
