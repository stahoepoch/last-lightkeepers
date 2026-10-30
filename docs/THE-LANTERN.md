# The lantern — what it is, how it works, and what it costs

Status: Chris's three-mode model is **his, 2026-10-29**, recorded as
`DECISIONS.md` row 67, and so are the sealed cell, the scarcity rules and the
handling-not-power upgrade axis, recorded the same day as rows 68 and 69.
Everything still marked *proposed* below is Claude's derivation and awaits his
word. Everything marked **locked** is cited to the
row that locked it and is not up for grabs here.

Written answering four questions he asked on 2026-10-29: *"What happens if a
lantern loses all its charge? We can't add more fuel? How does a lantern work?
What is it made from, how does it work? Do they upgrade?"*

The constraint running through all four answers: **the lantern is the one
variable the whole game measures.** Row 3's gate asks whether testers kill
their lanterns. Every answer below is checked against whether it keeps that
question askable.

---

## What it is, and why the fiction already decided most of this

`PREMISE.md` is load-bearing here and it is not vague: **the machines run on
light.** They were service units in a city that had gone solar, and when the
storm took the grid they fell back on ambient light. In daylight they are fed,
all awake, everywhere — which is why nobody goes. At night they run on reserve
and some stand dormant exactly where they stopped.

> *"So a lantern is not only a thing that reveals you. It is a small sun, and
> you are feeding them."* — `PREMISE.md`

That single sentence fixes most of what a lantern can be:

- **It emits the thing they eat.** Not a signal they detect — a supply they
  consume. This is why light is not a stealth mechanic with a vision cone
  bolted on, and why there is no "light repels small enemies" exception
  anywhere in the design.
- **It cannot be electric.** The grid is dead and has been since the first
  flood. A lantern that ran on the grid would be dead with it, and a lantern
  that ran on a battery would make the storm its charger — which would hand
  the player a reason to want the storm.
- **It is amber, against the storm's electric blue** (`PREMISE.md`). That is
  the colour-blind readability rule carrying the premise rather than decorating
  it: their power is blue and yours is warm, and you can tell at a glance whose
  light you are standing in.
- **It belongs to the order, not to the scavenger.** The Lightkeepers'
  work, since the first flood, has been *"keeping the records and keeping a
  light burning"* (`PREMISE.md`). `LANTERNHOLD.md` makes it literal: *"while
  you are out there, your lamp burns here, where everyone can see it."*

### So what is it made from — Chris, 2026-10-29

**A standardized Lightkeeper inspection lantern with a sealed power cell,
charged back at Lanternhold between raids, designed to outlast one night
window.** His words, and the three negatives in them are the design:

> *"No gasoline cans. No field battery scavenging. No 'you used the named tool
> and now the game leaves you blind.'"*

Each of those closes off a whole genre of busywork. A fuel can is an errand; a
scavenged battery makes light a loot problem; and a lantern that dies from
normal use punishes the player for using the one tool the game is about.

**A sealed cell is a stronger answer than the mantle I proposed**, for a reason
worth recording because it generalises. A mantle explains the light but invites
the question *"so where do I get another mantle"* — it is a consumable with
good manners. A cell that is standardized, sealed and serviced at the hall is
**institutional**: it is the order's equipment, issued and inspected, and the
reason you cannot top it up in the field is that you are not supposed to. That
matches who the Lightkeepers are (*"an order, not a job. Inherited."*) and it
makes the hall the lantern's home rather than its shop.

What it keeps from the earlier reasoning:

- **Still amber, still warm, still continuous** — because the premise needs a
  machine to read it as food rather than as a signal, and because
  `PREMISE.md`'s amber-against-electric-blue is the colour-blind readability
  rule carrying the premise.
- **Still steady.** A lantern that stutters is a scare instrument and
  `SOUND.md` already has one in the storm strike. The lantern is steady; the
  building is not. That separation is what lets a flicker mean something when
  we want one — which is exactly what Chris reserved it for below.

**The one logic question it opens, and it is small:** if the cell is charged at
Lanternhold, what charges Lanternhold? The grid has been dead since the first
flood. Three answers that all work and one that does not: the hall has its own
restored means (which makes it a restoration target, and crates already feed
`LanternholdLandmarks`); it is hand-cranked, which is labour and suits the
order; or **the storm charges it**, which is the most interesting and the
cruellest — the thing that ends your night is the thing that buys you the next
one. The answer that does not work is solar, because the premise spends
daylight on the machines.

---

## The three modes — row 67, Chris's, 2026-10-29

| Mode | Hands | What it does | What it costs |
| --- | --- | --- | --- |
| **Stowed, off** | Free | Full movement; darkness | You lose light and orientation |
| **Stowed, lit** | Free | A low, body-mounted pool for route-finding | Full exposure. The building sees light either way |
| **Raised, lit** | One occupied | You aim the cone, read relics, inspect shelves, light a route deliberately | Exposure, plus a hand |

His framing of the middle row is the part that makes this a design rather than
a convenience: **stowed-lit is convenience, not the best vision mode.** It
lights your feet and nearby route clues; it does not cleanly reveal a shelf
across the room and it does not substitute for raising the lantern to identify
a relic. And *it costs the same exposure*, because the building sees light
either way.

That last clause is what stops the middle mode from being strictly better than
the other two. Without it, stowed-lit would be free light and the other modes
would be for people who had not noticed.

**Bulky carry is the fourth state and it is a consequence, not a mode.** A
crate takes both hands, so the lantern is stowed by physics: no mantle, no
revive, no tool use until you set it down. `data/loot.json` already authors
this — `RestorationCrates` carry `carrySlots: 0`,
`blocksCombatWhileCarried: true`, and `restorationCratesArePocketable: false`.
The data was waiting for the verb.

### Peek — *proposed, and deliberately not built yet*

Hold the lantern past a doorframe or a shelf edge. The cone reaches the room
before you do; you learn what is in there, and the room learns about you.

Chris's reason for preferring this to a generic camera lean is the right one
and it is the project's own rule: **you gain information by risking light.**
A lean gives free knowledge, which is a corner-camera exploit with better
manners. This gives knowledge at the exact price everything else in the game
charges.

Not built until real shelves and doorframes exist, per his call. The mode model
is worth locking now because it decides what `ULLKLanternComponent` is a state
machine *of*; the verb that uses it can wait for geometry.

---

## What happens when the charge runs out

**Today: nothing, because it cannot happen.** `data/lantern.json` authors
`charge.secondsOfLightPerFullCharge: 0`, which means light does not drain.
The charge has exactly one spender in the whole game — the Anchor Revive, at
`chargesPerRun: 1` — and `autoOffAtEmpty: true` is therefore inert. Chris's own
note in the data says why:

> *"Not a fuel cost: with light unlimited, spending the charge would strand a
> player lightless for the rest of the run."*

So the honest answer to *"we can't add more fuel?"* is that there is no fuel to
add, by decision, and the field is a resource with one use rather than a tank.

### Kept, as a mercy rule — Chris, 2026-10-29

> *"That was an intentional mercy rule, and I'd keep it for the first slice."*

Decided, and the reasons for it stand: a fuel timer would mean a dark run
proves nothing about whether players *choose* darkness, which is the only
question row 3's gate exists to answer, and the telemetry would look identical
either way.

### If we ever want lantern scarcity, it is not depletion — his list

> *"Never 'the player used light normally, so now they are soft-locked in
> darkness.'"*

Three shapes he will accept instead, and the common property is that every one
of them is **caused by an event, not by use**:

| Shape | What causes it |
| --- | --- |
| A damaged lantern may flicker | A particular threat did it to you |
| A rare expedition requires a spare cell | The expedition is unusual, and says so before you leave |
| A restored workshop repairs a broken lens or mount | You already broke it, and the hall is where it gets fixed |

That is the line to hold: **scarcity may be inflicted and it may be
situational, but it is never the bill for ordinary use.** A flicker after a
threat is the building having done something to you, which the player can name.
An empty tank after a careful hour is the game taxing the verb it is about.

And the flicker has a home now. The lantern is steady precisely so that a
damaged one reading as unsteady is legible — one channel, no tutorial, and it
satisfies row 16 without a line of audio.

---

## Do they upgrade?

**Not in power. In handling — Chris, 2026-10-29, row 69.** Which narrows an
existing lock rather than breaking it: `REWARDS.md` keeps *"anything touching
the lantern"* off the shelf, and his rule says what that lock was actually
protecting.

> *"No upgrade should make it brighter, longer-range, or remove machine
> attention; that would sell back the game's tension."*

| On the shelf | What it changes |
| --- | --- |
| A chest harness | Stowed-lit is steadier |
| A shutter or hood | A low downward walking light |
| A better grip | Less visual sway while sprinting |
| A service mount | The lantern stays up while both hands carry light items |

**Why the distinction holds up under the numbers.** The lantern is the
instrument the game is calibrated against: every radius in `enemies.json` is
tuned to its 30-stud reach, and `aggroRadiusLit` 38 is *eight studs past the
edge of your own light*, which is row 5's trade made physical. Brightness,
range and attention are the three terms in that relationship. Handling is not
in it at all — so a grip that reduces sway cannot change what any enemy number
means, and that is exactly why it is safe to sell.

**And three of the four are already authored.** `FLLKLanternFeelRules` carries
`WalkSwayCm`, `SprintSwayCm`, `MaxTrailDegrees` and `MaxLandingKickCm`, so the
harness and the grip are numbers on an existing struct rather than new systems.
The shutter is the one that needs building, and it is the most interesting of
the four because a downward walking light is a fourth mode in all but name -
worth deciding whether it joins row 67's model rather than sitting beside it.

**What else upgrades**, unchanged: the hall, everything you carry that is not
the lantern, and verbs at full price rather than discounts on what light costs.
`GAME-THESIS.md` calls that capability with cost.

---

## Settled, 2026-10-29: the lantern does not feed the storm

Chris, ending the ambiguity rather than smoothing it:

> *"The storm gets worse on its own. A lantern does not feed the storm. It only
> lets nearby machines notice you."*

And on the words, which is where the confusion lived:

> *"We should avoid the word 'charge' for both systems. Storm Charge is really
> the Night / Storm Arrival meter. The lantern has a cell, if it ever needs
> one."*

So the world's logic, in his order: dusk, machines on reserve, some slow or
dormant. Night storm, power and flooding arriving, the building waking
progressively until extraction is the only sensible choice. And the lantern -
local, warm, human-made - revealing routes and relics, waking a mannequin,
giving you away to a Sweeper, making a room feel occupied, and **never making
the weather worse.**

This supersedes the reading `THE-NIGHT.md` proposed on 2026-10-28, which kept
the lantern as a second input to one meter. It also narrows `DECISIONS.md` row
45, which is LOCKED and says the meter *"is now how much of tonight's storm has
arrived"* - that sentence is now exactly right, and it is right because the
lantern is no longer one of its inputs. Row 68 records both.

### Three things this costs, which are not in his message

Recorded here because they are consequences rather than objections, and because
two of them are **locked decisions that priced themselves in the meter he has
just made untouchable.**

**1. The run stops having a variable length.** `data/storm-charge.json` authors
`baselineRatePerSecond: 1` and then multiplies it by the lantern curve -
`perLanternBonus: 1.5` on a diminishing curve at `diminishingDecay: 0.6`,
giving 2.5x for one lantern and about 4.75x for four. The run timings are tuned
*against that*: `run.targetSeconds` 660, `earlyExitSeconds` 360,
`hardCeilingSeconds` 780, with three named duty profiles (`normal` 0.25,
`bright` 0.5, `panic`). `tests/balance.test.mjs` simulates
`wardenAt(crew, duty)` and asserts that a fully dark crew meets the Warden near
the end of the night rather than before it.

Remove the lantern input and all of that collapses to one number. **The Warden
arrives at the same moment every run, whatever the crew did.** Going dark no
longer buys time; it only avoids attention. That may well be the better game -
it is certainly the more legible one - but it is a different one, and the
`_dutyProfiles` table, the curve, the decay and `maxMultiplier` all become dead
data on the day it lands.

**2. The Anchor Revive loses its entire price.** Row 9 and
`data/lantern.json`'s own note: *"once per run is the price. The Anchor Revive
costs chargesPerRun (1) and a permanent Storm Charge spike of 120, and nothing
else."* If nothing a player does can move the meter, a solo keeper's
self-revive becomes free, and row 9 needs a new currency.

**3. The extraction call loses its.** `extractionCall.stormChargeSpike: 180`,
described in the data as *"three minutes of darkness applied at once"*. It does
keep `convergeAllPatrols: true` and an 8-second siren, so there is an obvious
replacement price in attention rather than in the meter - but it is a decision,
not a rename.

Row 64 names *"the extraction call"* and *"the anchor revive"* among the
legitimate sources of escalation, alongside light. His new rule removes light
from that list for a good reason; whether it removes the other two is the
question the code is waiting on.

### The two coherent shapes — *the one open question*

**A. One meter, time only.** The storm is weather and nothing touches it.
Light's cost is entirely local: perception, wake states, noise, position. The
revive and the extraction call get re-priced in attention. Simplest, most
legible, and the version his message literally describes.

**B. Two meters.** *Storm Arrival* is time only and drives the night's stages
and the Warden - nothing the player does moves it. A second, choice-driven
value tracks how awake the building is, raised by light, theft, depth, alarms,
the revive and the extraction call. The lantern still never touches the
weather; it wakes machines, which is his own third bullet. Every locked price
keeps working and light keeps a crew-level cost.

Claude's recommendation is **B**, on the grounds that his third bullet already
describes a second axis - *"the building wakes up progressively"* - and that B
costs nothing that is currently working while A costs the three things above.
But A is coherent, it is simpler, and it is the one he appears to be asking
for, so this is his call and not a drafting detail.

**What is NOT waiting on it:** the rename. "Storm Charge" is a bad name under
either shape, and `ULLKStormChargeSubsystem`, `ULLKStormChargeData`,
`stormChargeSpike` and the rest can become *Storm Arrival* the moment he picks
between that and *Night*. He offered both.
