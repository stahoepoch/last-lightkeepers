# Proposal — what a Sweeper's contact should do instead of a knockdown

Status: **proposed**, 2026-11-02. Nothing here is built. Every number marked
**[CHRIS]** is a balance value and his.

From his own description: *"it slows you down, it's annoying, it can trip you
but we should be able to escape it, or interact with it... it should smack
your feet, trip you up, maybe alert other sweepers... but it should be able to
leave you alone somehow."*

---

## The problem this fixes

`data/lantern.json` now has `knockdownOnContact: false`, noted as *"this
maintenance machine is not a containment enemy."* **The instinct is right and
the execution leaves a hole:** the Sweeper still lands hits, those hits still
accumulate against `hitsToKnockDown: 3`, and the third one now resolves into
**nothing at all**. The enemy has no consequence.

That is the wrong repair for the complaint. The NPU result in
`docs/FEAR-AND-LIGHT.md` says uncued threat produces unresolving anxiety and
the fix is a **cue**, not the removal of consequence. Strip the consequence
and the predictable condition collapses into the no-threat baseline the other
conditions are measured against — there is nothing left to make legible.

**So: replace it, do not delete it.** With the ladder below,
`knockdownOnContact: false` becomes correct rather than hollow.

## What is already authored and currently dead

This is mostly a wiring job. Two things exist and are consumed by nothing:

| Authored | Value | State |
| --- | --- | --- |
| `Sweepers.swarmCallRadius` | **45** studs | In `enemies.json`, in `FLLKEnemyDef`, asserted for *presence* by two tests, **read by nothing in the Unreal build** |
| `sweeperContact` ladder | `contactRange 5`, `contactCooldownSeconds 1.5`, `hitsToKnockDown 3`, `hitDecaySeconds 6` | Fully implemented in `LLKSweeperBrain`, and now resolves to nothing |

**The Roblox prototype did implement the swarm call.**
`src/server/services/EnemyService.luau` carries `patrolsAlerted`, and when the
floor is alerted a Sweeper's reach becomes
`math.max(reach, def.swarmCallRadius)`. So "alert other sweepers" is not a new
idea — it is shipped prototype behaviour that did not survive the port. Third
authored-and-dead key this week, after `attractedToDroppedLoot` and the four
cue delegates.

## The ladder

Repoint the existing accumulator. Same three hits, same decay, same cooldown.

| Hit | What happens | Reads |
| --- | --- | --- |
| **1 and 2** | **It smacks your feet.** A short speed penalty plus a small impulse along the Sweeper's direction of travel, so you are shoved off your line | `contactCooldownSeconds 1.5` already prevents chain-contact |
| **3** | **The floor goes alert.** It does not hurt you — it *reports* you. Every Sweeper's effective notice radius becomes `swarmCallRadius` (45) instead of 12 dark / 38 lit | `swarmCallRadius 45`, Chris's |

**Escape is already authored.** `hitDecaySeconds: 6` means hits expire: take
two, break away for six seconds, and the ladder resets. That is the
"we should be able to escape it" half, and it needs no new rule.

**"Leave you alone" is already in the state machine.**
`losesTargetAfterSeconds` → `Search` → `Patrol`. It gives up on its own.
Nothing is missing there either.

## The one rule I would hold hard

> **A stumble costs distance and tempo. It must never cost input.**

No camera lock, no input suppression, no animation you have to wait out. You
stay in control for every frame of it — you are slower, and you are pushed off
your line. A control-removal the player cannot act through is the same failure
as a knockdown, only shorter, and it is the specific thing that makes people
put a game down.

This is also what makes it *annoying* in the way Chris asked for rather than
punishing.

## On the physics problem

*"I know I can't kick and pick things up, this isn't VR."* Correct, and this
needs none of that.

A floor machine clipping your ankles is **a movement impulse plus a brief
speed multiplier, with the right sound and the right framing**. No simulation,
no IK, no foot placement. `LLKBurden::SpeedMultiplier` is already the
precedent for "something external modifies the keeper's speed" — burden runs
`speedPenaltyPerPoint 0.04` capped at `maxSpeedPenalty 0.12`, a small
*persistent* penalty. A stumble is the other axis: **sharper and brief**.

## What is new code, and what is not

| Piece | Status |
| --- | --- |
| Hit accumulation, decay, cooldown, contact range | exists, untouched |
| Give-up and resume | exists, untouched |
| `swarmCallRadius` as a value | authored, needs a consumer |
| Stumble output from the brain | **new** — one flag on `FLLKSweeperStep` |
| Floor-alert state | **new** — run-scoped, not per-Sweeper |
| Speed/impulse application on the keeper | **new** — small, on the character |
| A cue for the alert | **new** — see below |

## Guardrails

1. **The alert must announce itself.** An escalation the player cannot
   perceive is uncued by definition, which is the exact failure this proposal
   exists to fix. Hit three needs a cue, on the `OnTelegraph` pattern —
   and note that nothing in the project binds any of those delegates yet
   (`HANDOFFS.md` C2), so this inherits that problem rather than escaping it.
2. **It reports, it does not retaliate.** Row 47. The machine's answer to
   being bumped three times is to widen what the floor notices, not to punish
   the keeper. That distinction is the whole character of the enemy.
3. **Visual-only stays visual-only.** Nothing here may read or write the
   gameplay light path — `GetLitLightSources()` and `GetGameplayRadiusCm()`
   are untouched.
4. **Row 64 compatibility, and this is the open question below.** Row 64 says
   escalation permanently increases what the building is authorized to do, and
   is *caused by choices the crew can identify, never by failing.* Three
   contacts is identifiable. Whether the alert is permanent for the run is
   still Chris's call.

## Numbers and calls for Chris

| | Question |
| --- | --- |
| **[CHRIS]** | Stumble duration — how long the penalty lasts |
| **[CHRIS]** | Stumble severity — how much slower, against burden's 0.04/0.12 as the scale reference |
| **[CHRIS]** | Impulse: is there a shove at all, or only a slow? A shove reads more physical and is more likely to annoy in the bad way near ledges |
| **[CHRIS]** | **Does the floor-alert ever end?** Permanent for the run fits row 64 and makes the third hit a threshold worth defending. Decaying gives recovery and softens it. This is the biggest design question in the proposal |
| **[CHRIS]** | Does the alert apply to Sweepers only, or to every enemy family? `swarmCallRadius` is authored on Sweepers alone |

## Proof of closure

- Two hits produce a stumble and **no** alert; the third produces the alert
  **once**.
- Letting `hitDecaySeconds` expire between hits resets the ladder, so two hits
  and a six-second break never escalate.
- The alert widens notice to 45 for a Sweeper that did not land the hits —
  it is a floor state, not a grudge.
- **The test must read the rule from data, not assert it in a fixture.**
  `LLKSweeperBrainTest` currently hardcodes `Rules.bKnockdownOnContact = true`,
  so the knockdown path stays green no matter what `lantern.json` says. That is
  the third fixture-blindness instance this week after `bHasStockBay` and
  `AggroRadiusDark`, and this proposal must not add a fourth.
- Proven red before being called done, per `ADVERSARY.md`.
