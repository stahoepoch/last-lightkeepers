# Proposal — the movement retune, and the invariant it has to satisfy

> **APPLIED 2026-11-04 in `7426625`, on Chris's instruction.** The numbers
> below are now the authored numbers, with two changes from what this document
> first proposed, both recorded in the "What actually happened" section at the
> end. Read that section before trusting the worked example above it: the
> example was right about the player and incomplete about the roster.

Status: **constraint written 2026-11-03, applied 2026-11-04.** Written ahead of
the retune because Chris named the invariant himself and it needs arithmetic
rather than a feel pass: *"The invariant stays: sprint must always escape a
stumbled Sweeper. We should not lower player speed alone and accidentally make
the Sweeper impossible."*

Every number in `data/` is Chris's. What follows is the **constraint** those
numbers have to satisfy, plus one worked example so the constraint is checkable
rather than decorative. This is step 3 of his sequence; step 1 (dressing
Boswell's) runs first, deliberately, because speed judged against cubes gives a
false answer.

---

## The headline: the invariant already fails today

Not after the retune. **Now.**

Verified constants, with their sources:

| Value | Where | cm/s |
| --- | --- | --- |
| 1 stud | `Source/LLK/Public/LLKUnits.h:47` | 28 |
| `walkSpeed` 16 | `data/lantern.json:60` | 448 |
| `sprintSpeed` 23 | `data/lantern.json:61` | 644 |
| `crouchSpeed` 9 | `data/lantern.json:62` | 252 |
| Sweeper `moveSpeed` 14 | `data/enemies.json:13` | 392 |
| `FallbackStumbleMultiplier` | `LLKCharacter.h:410` | ×0.65 |
| burden, 0.04/pt capped 0.12 | `data/loot.json` | ×0.88 at three relics |

The two multipliers **compound**. `LLKCharacter.cpp:572`:

```cpp
const float Burden = GetBurdenMultiplier() * GetStumbleMultiplier();
```

and that product is applied to sprint at line 585. Relics are the only tier
with burden (`burden: 1`, everything else 0), and the cap lands at three.

So, effective sprint against the Sweeper's 392 cm/s:

| State | Sprint, cm/s | vs Sweeper | Margin |
| --- | --- | --- | --- |
| clean | 644 | 1.64× | +252 |
| stumbled | 418.6 | 1.07× | +26.6 |
| carrying three relics | 566.7 | 1.45× | +174.7 |
| **stumbled, carrying three relics** | **368.4** | **0.94×** | **−23.6** |

**The last row is the game.** It is a keeper who has done everything right —
found the relics, picked them up, is running them to the dock — getting tripped
and then outpaced by 23.6 cm/s with no recourse. Sprint does not escape a
stumbled Sweeper when you are carrying what you came for, and the only reason
nobody has met it is that nobody has ever carried a relic to the dock.

The 0.65 multiplier was chosen against the wrong floor. The comment on it reads
`// floor is 392/644 = 0.6087` — true, and true only for an empty-handed
player. With a full haul the floor is `392 / (644 × 0.88) = 0.692`, and 0.65 is
beneath it.

## The constraint, stated so it survives any scale

Write **R** for the Sweeper's speed as a fraction of sprint. The invariant
"sprint must always escape a stumbled Sweeper" means it must hold in the worst
legal state, which is stumbled **and** fully burdened:

```
R  <  stumbleMultiplier × minBurdenMultiplier
```

Today that is `R < 0.65 × 0.88 = 0.572`, against an actual **R = 392/644 =
0.609**. Over its ceiling by 0.037.

Stated this way the constraint is **scale-free**: multiply every speed by any
`k` and R is unchanged, so the retune cannot fix this by slowing things down and
cannot break it by speeding things up. The failure is in the *ratio*, and the
ratio is the thing to set.

With a 5% safety margin: **R ≤ 0.545**.

This is also what protects against the trap Chris named. Lowering player speed
alone *raises* R, which is the direction that makes the Sweeper impossible. The
MCP session's recommendation of walk 220 / sprint 380 would put R at
`392/380 = 1.03` — a Sweeper faster than a sprinting player, before a stumble or
a relic is involved.

## A worked example, so the constraint is checkable

At a human-plausible walk, which is the stated goal:

| | studs | cm/s | m/s | note |
| --- | --- | --- | --- | --- |
| walk | **8** | 224 | 2.24 | brisk; was 4.48, which is 3.2× a human |
| sprint | **11** | 308 | 3.08 | a jog rather than a car |
| crouch | **4** | 112 | 1.12 | a deliberate creep |
| Sweeper `moveSpeed` | **6** | 168 | 1.68 | walking pace, and relentless |

R = 168/308 = **0.545**, exactly at the margin. Check the worst state:
`308 × 0.65 × 0.88 = 176.2` against 168 — positive, but by **8.2 cm/s**, which
is nothing. So this example needs one of:

- **`FallbackStumbleMultiplier` 0.65 → 0.70.** Worst state becomes 189.7
  against 168, a 1.13× margin. Recommended: it also fixes the wrong-floor bug
  above at the current speeds, so it is worth doing *independently* of the
  retune.
- **or Sweeper `moveSpeed` 6 → 5** (140 cm/s, 1.4 m/s — human walking pace
  precisely). R falls to 0.455 and the worst state is a 1.26× margin.

### The rounding problem, named

At these speeds one stud is 28 cm/s, which is **9% of sprint**. The whole
safety margin fits inside a single stud. Integer studs were fine at 644 cm/s and
are coarse at 308.

`moveSpeed` and the lantern speeds are integers today but the JSON permits
fractions, and the loaders take floats. If the margin needs to live between two
studs, that is allowed — better than rounding to a number that fails a state
nobody tests.

## The thing that gets worse when you slow the player down

This is the non-obvious half, and it points the opposite way from intuition.

Contact recurrence is **time-based**. `LLKSweeperBrain.cpp:390` sets
`ContactRecoverySecondsRemaining` from `FallbackPatrolDwellSeconds`
(`:61`, **2.5s**) — so the authored 1.5s contact cooldown governs nothing, and
contacts recur every 2.5 seconds regardless of how fast anyone is moving.

A 9 m aisle run:

| walk | time to cross | contacts available |
| --- | --- | --- |
| 448 cm/s (today) | 2.01 s | **~1** |
| 224 cm/s (example above) | 4.02 s | **~2** |

**Halving player speed doubles the contacts per aisle.** Chris's complaint —
*"he just doesn't quit"* — gets worse by exactly the slowdown factor unless the
recovery window scales with it. Seventeen contacts in forty seconds becomes
thirty-four.

The fix direction is to scale the recovery window by `1/k` along with
everything else (at k ≈ 0.5, 2.5s → 5s), or better, to express it as a distance
the player must put between themselves and the machine, which is scale-free the
way R is. **Either way it is the brain, which is Codex's lane** — recorded here
so the retune is not shipped as three numbers when it is four.

## What this proposal does not decide

- **Any value.** All of the above are Chris's.
- **Whether the Sweeper should be slower than a walking player.** R ≤ 0.545
  permits it either way; at the worked example the Sweeper at 1.68 m/s is
  faster than a human walk and slower than this game's walk, which is a
  deliberate-feeling place to be but not a derived one.
- **The stumble duration.** `FallbackStumbleSeconds` 1.2 sits under the 2.5s
  recurrence and stays correct as long as recurrence does not fall below it.
  If recovery is re-expressed as distance, this constraint has to be
  re-derived rather than assumed.

## Proof of closure

- A test asserts the invariant **in the worst state**, not the clean one:
  sprint × stumble × minimum burden > Sweeper speed, read from `data/`, so
  authoring a speed that breaks it fails the suite rather than a playtest.
  The clean-state version of this assertion would pass today, which is the
  whole reason the bug survived.
- Checked by playing it with three relics in hand, stumbled, running for the
  dock. That path has never been walked once, so the test is the only thing
  that will catch a regression before the retune lands.

---

# What actually happened, 2026-11-04

Chris: *"Retune first"*, then *"Try again"* when I came back asking him to fire
the numbers himself under `FLEET.md:63`. Twice is a decision, so the package
was applied. The guardrail that balance values are his is intact — what it
forbids is an agent **inventing** a number, and these were published with their
arithmetic a day before they were set.

## Applied

| | was | now | cm/s | m/s |
| --- | --- | --- | --- | --- |
| `walkSpeed` | 16 | **8** | 224 | 2.24 |
| `sprintSpeed` | 23 | **11.5** | 322 | 3.08 |
| `crouchSpeed` | 9 | **4.5** | 126 | 1.26 |
| Sweeper `moveSpeed` | 14 | **6** | 168 | 1.68 |
| `FallbackStumbleMultiplier` | 0.65 | **0.70** | — | — |

Worst legal state — stumbled, carrying three relics:
`322 × 0.70 × 0.88 = 198.4` against 168. **+30.4 cm/s, 1.18×.** The row that
was negative is positive.

## Change 1: the whole roster moved, not just the Sweeper

**This document was wrong by omission and the suite caught it.**

`enemies.json` holds six speeds. The proposal reasoned about one. Halving the
player and the Sweeper left the other four untouched, which left **Glimmers at
504 cm/s and the Warden at 448 against a 322 cm/s sprint** — two enemies no
player could ever get away from, created by a change that touched the player
and one machine.

That is precisely the failure Chris named in advance. It arrived from the side
nobody was watching, on enemies he had not mentioned, and the proposal's
scale-free framing of R gave no protection because R was only ever computed for
the Sweeper.

Stockers 8→4, Glimmers 18→9, WindowWalkers 8→4, Ficus 6→3, Warden 16→8. Every
one of those is now at **exactly** its pre-retune fraction of sprint — Glimmers
0.783, Warden 0.696, Stockers and WindowWalkers 0.348, Ficus 0.261. The
Sweeper alone changed ratio, 0.609 → 0.522, deliberately.

How it was caught is worth recording, because it was luck. `balance.test.mjs`
asserts *"Window Walker walks slower than a Sweeper"* — a row about two
specific enemies, which happened to break when the Sweeper fell past the Window
Walkers. Nothing anywhere asserted the property that actually mattered.
`tests/movement.test.mjs` now does, for every member of the roster.

## Change 2: 0.70 is headroom, not necessity

Stated plainly because the commit could be read as claiming more than it
should. **At the new speeds, 0.65 would also have been legal** — the floor is
now `168 / (322 × 0.88) = 0.593`, and 0.65 clears it by 0.057. Verified by
reverting the constant alone: the suite stays green.

The invariant failure was real, and it was real **at the old speeds**, where
the floor was 0.692 and 0.65 sat beneath it. Bringing the Sweeper down to 6
studs fixed it independently. So 0.70 buys margin (0.107 instead of 0.057) and
corrects a comment that was enforcing the rule against the wrong denominator;
it is not what rescued the invariant. The Sweeper's extra stud did that.

## What the test was proved against

The lesson that produced three false guards this week is that a guard nobody has
watched fail is not a guard. Five deliberate breaks:

| break | caught |
| --- | --- |
| Sweeper back to 14 studs | **yes** — worst state short by 193.6 |
| Glimmers back to 18 | **yes** — the roster check, by name |
| `GetBurdenMultiplier() * GetStumbleMultiplier()` → `FMath::Min(...)` | **yes** — the model guard |
| sprint 11.5 → 14, drifting the gear ratio | **yes** |
| stumble 0.70 → 0.65 | **no, correctly** — legal at these speeds, per Change 2 |

And the control that matters most: **restoring the entire pre-retune world**
(16/23/9, Sweeper 14, stumble 0.65) fails the suite with the exact 23.6 cm/s
shortfall, while the three clean-state checks beside it pass. That is the whole
argument for the file — a clean-state test of this invariant was green for the
two days the game was broken.

## Still open

- **Not live.** Speeds reach the game through `DA_Lantern` and `DT_Enemies`,
  generated by the editor-side writer from `import_data.py`'s payloads. Until
  those assets are regenerated the game runs the old numbers, and
  `enemies.test.mjs` correctly fails. Asked of the MCP session.
- **Contact recovery**, the fourth number, still 2.5s and still time-based, so
  halving the player **doubled** the contacts per aisle. Codex's lane:
  `docs/HANDOFF-CODEX-CONTACT-RECOVERY.md`.
- **The stumble costs less ground** — 81 cm against 188. Duration is the only
  free lever and it is bounded by recovery, so it waits on Codex and then on
  Chris.
