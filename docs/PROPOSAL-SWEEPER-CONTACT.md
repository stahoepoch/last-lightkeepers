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
| ~~**[CHRIS]**~~ | ~~Does the floor-alert ever end?~~ **DECIDED 2026-11-02: it decays, quickly.** Chris's call. So the alert is a *state the player can get out of*, which makes the third hit a setback rather than a sentence, and keeps the run recoverable |
| **[CHRIS]** | Does the alert apply to Sweepers only, or to every enemy family? `swarmCallRadius` is authored on Sweepers alone |

## Ending the alert — decided, plus one idea that cannot ship

**Chris, 2026-11-02:** *"let it decay, quickly if we can, like maybe using the
lantern in a way to overwhelm it, like a superflash stun. or it runs away and
loses interest?"*

### Decay: decided, and it needs no new number

The alert times out. `Def.LosesTargetAfterSeconds` already means *how long
before it gives up on something it cannot resolve*, which is the same shape,
and reusing it keeps this at zero invented values. If the floor should forget
faster than a single Sweeper forgets a keeper, that is a **[CHRIS]** number.

### "It runs away and loses interest" — yes to half of it

**Loses interest: yes**, and it is already built. `Search` → `Patrol` is the
Sweeper disengaging on its own.

**Runs away: no**, and the distinction is row 47's whole character. A machine
does not flee, because fleeing is fear and these things have none. It
**disengages** — the job is finished, or the fault is no longer reported, so
it returns to its route. Same observable behaviour, opposite meaning, and the
meaning is the thing the game is about. A Sweeper that visibly *flees* a
keeper has been promoted to a creature.

### The superflash stun cannot ship, and it is not close

> **`DECISIONS.md` row 5, LOCKED:** *"Light reveals value and attracts danger.
> There is **no** 'light repels enemies' exception."* Its own reasoning line:
> *"Softening this deletes the game's only decision."*

A flash that stuns a machine is that exception exactly, whatever fiction is
wrapped around it — a dazzled sensor is still light protecting the keeper. The
cost model is what breaks, not the lore: **light currently buys capability and
charges visibility.** Give light a defensive use and the charge disappears,
because the thing that got you into trouble now gets you out of it. There is
then no reason to ever turn it off, and the game's single recurring decision
stops being a decision.

This is the third variant of the same idea to come up — after "different
light does different things to the robots" and the RMB shutter as free
concealment — which is a sign it is a genuinely attractive shape rather than a
careless one. It is still the one thing the design cannot have.

### What to build instead, and it is already specified

**The lantern already has a legal way to act on this, and it is the `Present`
verb.** `WORLD-SYSTEMS-BOARD.md` defines it: *offer a valid maintenance
pattern at a local service port — a badge, routing tag, calibration mark, or
work order.*

So the player-agency version of clearing an alert is **filing a correction**:
present at an authored service port and the floor's alert clears, because the
building's own record now says the fault is resolved. That is:

- **Row 5 intact.** The light acts on *infrastructure*, never on a creature —
  the same line Codex and I landed on for the focused beam. "A beam that opens
  a shutter" is legal; "a beam that stops a robot" is not.
- **Row 47 intact.** Nothing is stunned, frightened, or overwhelmed. A record
  was corrected and the machines believe it, which is more unsettling than a
  stun and costs nothing in rules.
- **Agency, which is what the superflash was reaching for.** Waiting out a
  timer is not a decision; walking to a port while the floor is at 45-stud
  notice very much is.
- **No new verb.** It is `Present`, which the board already specifies and
  which `HANDOFFS.md` already wants authored.

**[CHRIS]** Whether clearing is instant on presenting, or shortens the decay.

### One note on the physics research

The stumble in this proposal is deliberately specified to need **no physics
and no animation work**: a movement impulse plus a short speed multiplier.
If the parallel physics and animation research turns up something richer, it
is welcome — but nothing here is blocked on it, and this should not wait.

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
