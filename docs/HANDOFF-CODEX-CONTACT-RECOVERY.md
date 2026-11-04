# Handoff — Codex — contact recovery, the last piece of the retune

Written 2026-11-04. **Your lane:** `ContactRecoverySecondsRemaining` lives in
`LLKSweeperBrain.cpp`, which is yours and currently has uncommitted changes in
the shared checkout, so this is handed over rather than taken.

The movement retune landed in `7426625`. Three of its four numbers are set.
This is the fourth, and it moves in the opposite direction from intuition.

---

## The defect: the authored cooldown governs nothing

`LLKSweeperBrain.cpp:390` sets the recovery window from
`FallbackPatrolDwellSeconds` (`:61`, **2.5s**) — the patrol dwell, reused.

The authored contact cooldown is **1.5s**. It is read by nothing on this path.
So the live window is 2.5s and every comment that reasons from 1.5s is reasoning
from a number the code does not use, including the one on
`FallbackStumbleSeconds` in `LLKCharacter.h`, which I have now dated and
corrected in place rather than deleted.

That much is a plain bug and independent of the retune.

## What the retune did to it

Contact recurrence is **time**-based, so it does not scale with speed. Player
speeds halved. Therefore:

| | 9 m aisle run | contacts available |
| --- | --- | --- |
| before, walk 448 cm/s | 2.01 s | **~1** |
| now, walk 224 cm/s | 4.02 s | **~2** |

**Halving the player doubled the contacts per aisle.** Chris's complaint was
*"he just doesn't quit"* — seventeen contacts in forty seconds. That complaint
is now worse by exactly the slowdown factor, and the retune caused it. This is
the one thing in the package that got worse rather than better, and it is the
reason the four numbers were supposed to move together.

## Two shapes for the fix, and the second is better

**Scale it.** 2.5s → **5.0s**, matching `k = 0.5`. One constant, restores
contacts-per-aisle to what it was, and is the smaller change.

**Or make it a distance.** Recovery ends when the keeper has put some authored
distance between themselves and the machine, rather than when a timer expires.
This is **scale-free**, the way the escape invariant is scale-free — the next
rescale would not touch it. It is also a truer statement of what recovery
means: you got away, rather than you waited.

I would take the distance version if the brain can see separation cheaply at
that point, and the 5.0s if it cannot. Either way, **it is one number or one
predicate, not a redesign.**

## The constraint it has to satisfy, so this does not need a second pass

`FallbackStumbleSeconds` is **1.2s** and must stay strictly under the recovery
window, or each contact re-applies the slow before the last one expires and
the keeper is permanently slowed. 1.2 < 2.5 holds today, by accident rather
than by the 1.5 the comment cites.

`tests/movement.test.mjs` asserts this, reading both constants out of the
source rather than restating them, so whichever shape you pick, the suite will
tell you if the bound breaks. If you move recovery to a distance, that
assertion no longer has two comparable numbers and **needs rewriting rather
than deleting** — say so and I will rewrite it against the new mechanism.

## The thing I could not fix from here, and would like back

The stumble now costs **81 cm** of ground over its 1.2s, against 188 cm
before. The multiplier is floor-bound by the escape invariant (see
`LLKCharacter.h`, and the floor is `Sweeper / (sprint × minBurden)`, not the
unburdened ratio the old comment used), so **duration is the only free lever**
— and duration is bounded by your recovery window.

Restoring the old bite needs about **2.8s** of stumble, which does not fit
under a 2.5s recovery and fits comfortably under 5.0s. So if you take the 5.0s
option, tell me and I will propose the duration change to Chris; if you take
the distance option, the bound has to be re-derived and I will do that instead.
**Please do not set `FallbackStumbleSeconds` yourself** — it is in my file and
it is a balance value, which makes it Chris's.

## Current state, so nothing here is taken on trust

- `7426625` pushed. `data/lantern.json`, `data/enemies.json`,
  `LLKCharacter.h`, `tests/movement.test.mjs`, `package.json`.
- **`npm test` is red and the red is correct**: the authored speeds moved and
  `Content/Data/DT_Enemies.uasset` has not been regenerated, so
  `enemies.test.mjs` reports `Glimmers.moveSpeed = 9` missing from the asset.
  The MCP session has been asked to regenerate `DA_Lantern` and `DT_Enemies`
  through the editor-side writer. **Do not silence that test to get a green
  suite** — it is doing precisely its job.
- Your uncommitted work in `LLKSweeperBrain.cpp`,
  `LLKSweeperBrainTest.cpp` and `tests/enemies.test.mjs` was not touched and
  not staged. It is still uncommitted, and `FLEET.md` is right that this means
  it does not exist yet.
