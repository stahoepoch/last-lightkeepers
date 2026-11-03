# Proposal — two telegraph stages, so the sounds mean something

Status: **proposed**, 2026-11-03. **Codex's lane to build** — this is design,
written because the Sweeper's code is his while he is working it. Numbers
marked **[CHRIS]** are balance values and his.

Chris, after playing: *"he shouldn't actually respond immediately, give you
some time to know that he has seen you, and then give u time to know oh, he's
getting ready to come at me, i know what all those sounds mean, or
telegraphs, or whatever..."*

---

## What is there now, and why it reads as nothing

One stage. `Patrol` → noticed → `Telegraph` for `audioTelegraphSeconds`
(**1.0s** for Sweepers) with one cue → `Pursue`.

So "it saw you" and "it is coming" are **the same event, one second apart**.
There is nothing to distinguish, which is why no amount of listening teaches
you anything.

**The escape already exists and nobody can use it.** The brain's `Telegraph`
case checks the notice again when the timer expires: still noticed → `Pursue`,
lost → `Search`. So going dark during the telegraph *already* stops it coming.
It is one second long, so in practice the window is invisible.

This is the stumble problem again in a different place: the mechanism is
correct, tested, and shipping, and the player cannot perceive it.

## The shape: two stages, two sounds, two windows

| Stage | What it is | Cue | Duration |
| --- | --- | --- | --- |
| **Notice** | It has seen you. It stops and turns toward you. **It has not decided.** | new, distinct | **[CHRIS]** `noticeSeconds` |
| **Commit** | It has decided and is about to move. | the existing telegraph cue | `audioTelegraphSeconds`, already 1.0 |
| **Pursue** | Coming. | — | — |

**The window that matters is Notice.** Break line of sight or go dark inside
it and it never commits: it goes to `Search` and then back to its route. That
is already the brain's behaviour on a lost notice — what changes is that
there is finally enough time to cause it.

That also answers *"he should run away at least."* A machine does not flee —
row 47, and the distinction is the enemy's whole character. But **giving up
and going back to work** is what Chris is describing, and it is already
built. He has not seen it because it re-engages 2.5 seconds later.

## Why two stages rather than one longer one

Because the NPU result in `docs/FEAR-AND-LIGHT.md` is about **cues**, not
duration. A single two-second hold is still one event: you learn "something is
about to happen" and nothing else. Two cues give you two different pieces of
information, in order, and that is what makes a sound *mean* something —
which is exactly the word Chris used.

It is also the WildStar shape he knows from the inside: a cast you can see
start, then a telegraph you can see land, with a decision between them.

## What this costs

Small. The pieces exist:

- `ELLKSweeperCue` already carries four distinct values, so a fifth is free
  and the multicast path already works.
- `FLLKSweeperMind` already holds per-state countdowns; this is one more.
- The notice→Pursue/Search branch is written and correct.

What is new: one state, one cue value, one authored number.

## Numbers for Chris

| | |
| --- | --- |
| ~~**[CHRIS]**~~ | **DECIDED 2026-11-03: `noticeSeconds` = 3.0.** Chris's call. It has precedent in the data — the Warden already authors 3.0s of telegraph — so this is the house's own upper end rather than a new scale. **Total is then 4 seconds** from "it has seen you" to "it moves": 3 of Notice, 1 of Commit |
| **[CHRIS]** | The recovery window. Chris: *"he just doesn't quit."* Contact currently recurs every **2.508 s**, because `ContactRecoverySecondsRemaining` uses `FallbackPatrolDwellSeconds` (2.5) — so the authored 1.5 s contact cooldown governs nothing. Seventeen contacts in forty seconds is what that feels like |

### The risk in 3 seconds, named so it is checked rather than discovered

Four seconds is a long time to watch a machine not move, and guardrail 2 says
Notice must not translate it. **A motionless enemy for four seconds can read
as broken rather than as tense.**

What stops that is the thing already in the table: during Notice it **turns to
face you**. Rotation is not translation, so the window stays honest, but the
machine is visibly doing something — orienting, deciding — rather than
standing inert. If the stage is built without the turn, the risk is real and
the first playtest will say so.

The second thing 3 seconds buys, which is worth wanting: going dark becomes
**reliably** survivable rather than luckily so. That is not a deflation of the
threat, because darkness still costs what it always cost — you cannot read a
relic, you pick up slower, and you cannot see what is coming. Row 5 is intact
and the trade simply became legible, which is the whole point of the change.

If it plays as too generous, the number comes down. Coming down from too much
is one playtest; creeping up from too little is several, which is the lesson
the camera kick taught this afternoon.

## Guardrails

1. **Two cues must be distinguishable with the volume low**, not merely
   different. If they are two pitches of the same ping, this buys nothing.
   The attenuation is already correct — falloff set to the authored notice
   distances, so what you hear is what can see you.
2. **Notice must not move it.** If it drifts toward you during Notice, the
   stage is just a slower Pursue and the window is a lie.
3. **Row 47 holds.** It is not startled and it does not flee. It noticed, it
   evaluated, and it either commits or returns to work.
4. **No new state for the player to track.** Two sounds, one decision: stay
   or go dark. If it needs a HUD element it has failed.

## Proof of closure

- With the volume up, a player can tell "it has seen me" from "it is coming"
  without looking.
- Going dark inside the Notice window demonstrably sends it to `Search`
  rather than `Pursue`, and it returns to its route.
- The brain test asserts both branches out of Notice — committed, and
  abandoned — because a stage with only one tested outcome is half a stage.
- Not from a fixture that hardcodes the outcome. Three instances of that this
  week.
