# The pivot deletes the loop and keeps the point, and the point is already written

Status: **proposed**, 2026-11-09, from Chris's own message, which is the source
for every intention below:

> *"We have this as an extraction. But we don't have anywhere to go. I want to
> tell a story, or even have the player tell the story... the player is
> actually a kid, young adulting. Like 9-13... he's with Lamplighters, and
> it's a special mission, and they all get taken... you know are kid who must
> navigate, hide, manipulate, discover. Keep horror psychological. Keep
> storms, but they don't determine how long someone stays bc no extraction...
> Still might have to have a safe area we make our own, that's to build
> towards, but that safe area isn't the real purpose, it's really finishing
> the mission. Finding the other Lamplighters (family, brotherhood)?... Lamp
> is still like a type of flashlight multi tool, still has same premise I
> guess, just no extraction and some forward progression through......???"*

Nothing here is a `DECISIONS.md` row. Four rows are **proposed** at the end;
writing them is Chris's.

## The headline

**The game in that message is the one `docs/HOW-THE-PLAYER-LEARNS.md` has
described since 2026-10-21.** That document's whole thesis is that the story
arrives by arrangement and by one line attached to something the player
earned, that *"the dark is the storytelling"*, and that **"the risky choice is
the only way to learn anything."** `data/relics.json` has **twenty relics
already written** to a stated rule — two sentences, never three, about a
person and never about the world — and its own header calls itself *"the whole
lore-delivery system."*

So "I want to tell a story, or even have the player tell the story" is not a
new direction for this project. It is the oldest finished design in it, and it
has been a side system underneath a loop.

What the pivot actually proposes to delete is **three** things: extraction, the
mall, and the adult. Everything else in the message is already here.

That matters for one practical reason. A pivot that invents a new game costs
a year. A pivot that promotes a written side system to the spine and deletes a
loop costs weeks, and the risk is not "can we build it" but "will Chris still
want it in February."

## What extraction was actually doing

Deleting it is cheap in code and expensive in design, because one actor was
doing five jobs. Each needs a new home or an explicit deletion.

| Job | Where it goes |
| --- | --- |
| **1. The commit point** — the moment you stop gathering and start defending | **Must be replaced.** A horror game with no commit point is a walking sim with robots. This is the `??` at the end of his message |
| **2. The risk ratchet** — "one more store, or call it?" | **Already replaced, and better.** The lantern-only relic tier is the ratchet: light to find anything, and light is what the valley notices |
| **3. The session boundary** — what makes a *run* a unit | **Dissolves.** A story's unit is a place, not a run. `run.targetSeconds` 660 and the 10-12 minute band stop being design and become level-sized |
| **4. The payout gate** — loot only counted if you got out | **Dissolves, with one trap.** See "the hub stops being optional" below |
| **5. The reason the storm was a clock** | **Already solved, and he does not know it.** See below |

### Job 5 is already done

`Source/LLK/Public/Storm/LLKNightStage.h` is explicit, against row 64: Zone
Pressure *"permanently increases what the building is AUTHORIZED to do"*, the
stage *"is that authority"*, and it is **monotonic — `Index` can only rise
because `Charge` can only rise.**

That is not a countdown to death. It is a rising record of what the world is
allowed to do to you because of what you have done. It is exactly the system a
story game wants, and it needs no change at all.

The only part of the storm that is a clock is one number:
`data/zone-pressure.json` → `baselineRatePerSecond`, currently **1**, with the
file's own note that *"one pressure unit is one second at the authored baseline
rate."* Set it to 0 and pressure stops being time and becomes consequence.
**One value in a JSON file, in Chris's own lane, no code.**

The file is already drifting this way on its own: its header records row 72
retiring the lantern multiplier on weather, because *"weather arrives on its
own, while lantern light changes local machine awareness."*

## The spine, which is job 1

Three candidates. They are not exclusive and the recommendation is to take two.

**A. The lead chain.** Each place tells you where the next Lamplighter is.
Story-native, very cheap, and it is what "finding the other Lamplighters"
already implies. Weakness: a lead chain on its own is a fetch quest in a
story's clothes, and it supplies no commit point.

**B. The one-way descent.** The Valley base goes down, and down is easy while
up is expensive. The decision stops being "leave or push on" and becomes **"go
deeper now, or go back and prepare."** That is extraction's decision with its
sign flipped, and it needs no new system. It also gives psychological horror
its best free instrument: the player always knows how far from the surface
they are, and that number only gets worse.

**C. The storm as a tide, not a clock.** The storm does not count down — it
**opens and closes the world.** Storm up: the surface is impassable, but the
valley is loud and blind, so light and noise do not carry. Storm down: you can
move, and so can everything that is looking for you. Progression is then a
world that breathes, and *when* you go somewhere is a real decision.

**Recommended: B and C together**, because they are one mechanic seen twice.
The commit point becomes a **state rather than a timer**:

> You go in. The tide turns. The way you came is shut until it turns again.

Nobody is counting. You simply are not getting out that way for a while, and
the only directions are forward and wait. That is a stronger version of
extraction's tension than extraction had, because the pressure is the
building's, not the HUD's.

### And it gives the safe area exactly the job he asked for

He wants *"a safe area we make our own... but that safe area isn't the real
purpose."* Under a tide, the safe area is **where you wait out the storm.**
Load-bearing every single session, never the objective. That is the thing he
asked for, and it falls out of C rather than needing to be designed.

## Co-op, which his fiction already answers

Row 36 is 1-4 private co-op. *"They all get taken"* plus 1-4 co-op has one
obvious reading, and it is good:

**The other Lamplighters are the other players.**

- **In co-op**, a captured Lamplighter you free is a player rejoining. The
  revive economy becomes a **rescue economy**, and "find the others" stops
  being narrative flavour and becomes the multiplayer system.
- **In solo**, each one freed is a capability you did not have — one hears
  through walls, one carries what a child cannot, one grew up in the base.
  Which is also how a solo player stops being three channels short of the
  story, the gap `HOW-THE-PLAYER-LEARNS.md` already flags for Steam.

Row 9's Anchor Revive survives intact as its best version: you are the last
one up, and you get yourself up once, loudly.

## What the pivot costs in code

Counted, not estimated. 43 public headers.

| Area | Headers | Under the pivot |
| --- | --- | --- |
| `Storm/` light field, night stage, zone pressure | 4 | **Survives untouched.** This is the escalation system a story game wants |
| `Enemies/` Sweeper, brain, perception, roster, defs | 5 | **Survives.** A military robot is a re-skin and a data row, not a new system |
| `Loot/` relics, burden, interactor, illumination | 5 | **Survives, and gets promoted.** "Pick up objects, distract robots" is this code |
| `Lantern/` component, data, feel, light component | 3-4 | **Survives.** A flashlight multitool is the same gameplay/cosmetic split |
| `Characters/`, `UI/`, `Data/`, `Harness/` | ~12 | **Survives**, with the HUD's objective line re-pointed |
| `Run/` extraction, zone, snapshot, readout | **5** | **Mothballed, not deleted** |

So roughly **five headers of forty-three** are extraction-specific — about
12% — and the mall is content rather than code.

**Mothball rather than delete**, for a specific reason: the extraction state
machine is *hold a place under pressure while something arrives, pausing when
the route empties and never rewinding.* A base door that cycles, a lift that
has to be called, a generator that needs ninety seconds — every one of those is
that machine with new words. It is tested, it is green, and it took three
sessions to make it speak. Deleting it would be throwing away the one mechanic
in the project that has been debugged in play.

## Two traps, and the second one is the expensive one

### A 9-13 year old breaks every measured sightline in the project

This is the cost nobody would notice until the floor felt wrong.

`KeeperEyeHeightCm` is **152**. Clothing runs are blocked at **137** precisely
so an adult eye clears a run by 15 cm while the lantern at rest (133) does not
— that is the whole read of the sales floor, and
`PROPOSAL-BOSWELLS-RETAIL-FLOOR.md` names 152 as the one height to avoid.

**A 9-13 year old's eye is roughly 120-140 cm.** At 137, a child sees nothing
over anything. Every sightline number in the project is derived from an adult,
and they all invert.

That is not an argument against the child. It is an argument that the child is
**a measurement decision, not a vibe** — and it is arguably a *better* game:
a protagonist who cannot see over the shelves, in a world built for adults, is
psychological horror doing its own level design. The crouch/stand read becomes
stand/climb. But every number re-derives, and the Anchor's fixture heights are
already an open question (audit B8), so this is the cheapest moment in the
project's life to decide it.

It also locks something Chris has been circling for weeks: **a child cannot
fight**, so no-combat stops being a design preference and becomes a premise.
`docs/COMBAT.md` can close.

### The hub stops being optional, which is the opposite of what he just decided

Four days ago: *"just getting the raid to work is good, home can wait."* That
was correct **because extraction was the spine** — you banked loot and the
numbers went up, and the Keeping Room could arrive later.

Promote relics to the spine and the delivery mechanism is *"bring one home, it
goes on a plinth in the Keeping Room, and it gets one or two sentences."* **No
hub means no story.** The pivot silently makes the thing he deprioritised
load-bearing on day one.

There is a clean way out, and it is a real choice with a real cost:

| | Plaque on hand-in (as written) | Plaque in the field |
| --- | --- | --- |
| Needs a hub | **Yes, immediately** | No |
| Keeps "earn it home" | Yes | **No** — the ratchet weakens |
| The safe area accrues | Yes, visibly, over weeks | Needs another reason to exist |
| Fits a child alone in a valley | Less — who is he bringing it to? | **More** — he reads it by lamplight, now |

**Recommended: plaque in the field, and the safe area accrues something else**
— the others you have freed. The Keeping Room becomes the room that fills with
people instead of objects, which is a better fit for *"family, brotherhood"*
than a shelf is, and it still answers "build towards" without being the
purpose. The twenty written relics need no rewrite either way: they are about
people, which works the same in a dead valley as in a dead mall.

## The one word that costs a year

*"maybe free world in a way, or stages?"*

**Stages.** The Anchor is one building at 87 x 54 m and it is currently **3.7%
furnished** — nine boxes on a plate nine times the size of the room that
already felt empty. A free-roam valley is that problem multiplied by a
landscape. Stages that *feel* connected — shared skyline, a road you can see
from two of them, one place you re-enter from a different direction — read as a
world for a small fraction of the cost, and `LLK_Galleria_Shell`'s whole
argument in `THE-MALL-IS-STILL-TRADING.md` §8 is already this: *"the biggest
first level is the one that teaches the mall by refusing to let you into it."*

Same trick, new valley.

## Four rows this proposes, for Chris to write or refuse

1. **The player is a child, 9-13, and every sightline number re-derives from a
   child's eye height.** Supersedes nothing; `KeeperEyeHeightCm` becomes a
   measurement task rather than a constant.
2. **No extraction. Progression is a one-way descent, and the storm is a tide
   that opens and closes the world rather than a clock.**
   `baselineRatePerSecond` → 0.
3. **The other Lamplighters are the other players.** Rescue replaces revive as
   the co-op economy; freeing one is a rejoin in co-op and a capability in
   solo.
4. **Relic text arrives in the field, not at a plinth.** The safe area accrues
   people rather than objects.

Row 28 (*one sealed department store, the rest is earned*) and row 43 (*the
department store is the tutorial*) are both LOCKED and both become wrong under
this pivot. They need retiring by name, not quietly leaving behind — which is
the whole reason `HANDOFFS.md` exists.

## Turned over

**Operand.** This pivot, against finishing the extraction raid as designed.
Not against "some other game" — against the specific thing that is 80% built
and that he has now played twice without finding the exit.

**Floor.** A system already written and tested is a floor. A relic already
written is a floor — there are twenty. A free-roam valley is a filled
invention. A child's eye height is a measurement nobody has taken. "Military
base in the Valley" is a label until a room has a shape.

**Clock.** The pivot's clock is how long the written material stays true:
`HOW-THE-PLAYER-LEARNS.md`, `relics.json` and the Storm systems are
setting-agnostic and do not rot. `THE-MALL-IS-STILL-TRADING.md`'s eight
sections of real-building research and `docs/reference/boswells/` **do** rot,
and that is the sunk cost this pivot actually pays — not code.

**Falsifier.** The claim that this is cheap falls if a played child in a
re-measured room is not scarier than an adult in the same room. The tide falls
if "the way you came is shut" reads as a locked door rather than as weather —
which is a sound-and-light problem, not a systems one, and is testable in the
existing light lab. The rescue economy falls if a freed Lamplighter in solo is
a follower nobody wants to babysit; row 74's *"never an escort NPC"* is already
the warning, and it was written about this exact shape.

**Upside down.** Extraction was never the game. It was the reason to leave,
and the thing it was pulling you away from — a dark room with one object in it
that tells you about a person who is gone — was always the point. He is not
pivoting away from what he built. He is deleting the part that kept
interrupting it.
