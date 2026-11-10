# The pivot deletes the loop and keeps the point, and the point is already written

> ## CORRECTED the same day, and most of the correction is against me
>
> Chris's reply was four words: **"The mall doesn't go anywhere."** Checking
> that sent me to five documents I had not read when I wrote this page, and
> four of this page's claims do not survive them.
>
> **1. The mall goes somewhere, in three written directions.**
> `docs/CAMPAIGN-SHAPE.md` opens: *"Chris has locked the structure: a
> forward-moving story, one canon sequence, chapters that become replayable
> afterwards, and a world that has moved every time you come back.
> Lanternhold → the Galleria → the railyard → the Galleria changed."*
> `docs/QUESTS.md` rung 5 opens *"the next mall section."* And
> `data/loot.json`'s `SubBasementBreakerRestored` is a door down that
> `THE-MALL-IS-STILL-TRADING.md` §12 already notes *"wants a sub-basement
> these plans do not have"* — which is a problem for a real building and a
> premise for a secret one.
>
> **2. The progression spine is not missing. I said the commit point was
> "the one real hole." It is written.** `QUESTS.md` holds Chris's own opening
> five rungs from 2026-10-26, each one a practical recovery job that opens a
> route and wakes an enemy — *"The player is not simply leveling up — they
> are rebuilding the tools needed to reach farther into a place that notices
> it is being reopened."* `CAMPAIGN-SHAPE.md` adds the fantasy: *"You start
> as a trespasser and end as something the mall half-recognises."* That is
> an answer to his `??`, it is his, and it predates mine by two weeks.
>
> **3. "They all get taken" is LOCKED, with a contract.** Rows **49** and
> **50**, `docs/CONTAINED-KEEPERS.md`, 2026-10-28: *"I would call them
> contained keepers, not hostages... processed someone as a problem to be
> moved, quarantined, secured, or held until an authority arrives that never
> will."*
>
> **4. So my co-op idea is the specific thing a LOCKED row forbids.** Row
> **50**: *"Co-op is never gated behind progression. Rescued keepers are the
> fiction for why a crew exists; they are never the unlock that permits
> one"* — and the row says it was written as its own row because the
> opposite *"is a tidy progression design that someone would otherwise
> build: keepers rescued → party slots unlocked."* That is, word for word,
> what I proposed below. **Withdrawn.** What survives is the half row 50
> permits: a rescued keeper may be a character or a capability, never a
> permission, and three friends play together on the night they buy it.
>
> **And the robots are already better than "military vibes."**
> `docs/THE-WORK.md`: *"This storm did not invent a villain. It woke every
> system that never received a shutdown order... They contain, clear,
> restock, lock, and wait for a supervisor who is not coming."* Row **47**
> locks it. A secret base staffed by soldiers is a weaker idea than a
> department store still trying to prevent loss.
>
> **What is actually new in his message, then, is the child.** That is one
> decision, it is real, and it is the expensive one — see the sightline trap
> below, which stands.
>
> **The withdrawn claim that matters most**: that the mall research is this
> pivot's sunk cost. It is not. The ladder, the chapters, the valley
> (`THE-VALLEY.md`, `VALLEY-TERRAIN.md`), the containment contract and the
> relics are all setting-agnostic or already valley-shaped.
>
> **The real diagnosis.** The mall goes somewhere on paper and nowhere in
> the build. There is no rung 1, no hub, one furnished room, and nothing that
> opens a route. *That* is what "doesn't go anywhere" describes, and a pivot
> does not fix it — building rung 1 does.
>
> **Three times in one day I planned against a document I wrote while a more
> authoritative one sat unread**: X6 (closed four days earlier, in the
> ledger), the progression hole (written in `QUESTS.md`), and row 50. The
> instrument for this already exists and I did not use it —
> `docs/HANDOFFS.md`'s own preamble is about exactly this failure. Read
> `DECISIONS.md` and the index in `THE-WORK.md` **before** writing a design
> page, not after a four-word reply.

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

---

# Adjudication — Codex's version, 2026-11-09

Chris relayed Codex's reply, opening *"Yes. I think this is the game."* This
section is the adjudication he asks me for. **Verdict: adopt it. It is better
than the version above in four places, it collides with locked rows in three,
and it has one real hole.** Everything below is checked against the repository
rather than against the proposal.

## Where Codex is right, and where he is better than I was

**Extraction demoted rather than deleted, with "return" as the replacement.**
*"Leaving a place is a real act: choose the front gates and alarms, a service
route and risk, or a route you have earned by making the building half-
recognize you."* That last clause is `CAMPAIGN-SHAPE.md`'s locked progression
fantasy — *"you start as a trespasser and end as something the mall half-
recognises"* — arriving as a mechanic instead of a theme. This is the single
best idea in his message.

**12 or 13, not 9-13, and the narrowing is what saves the project's geometry.**
See the measurement below. He gives the design reason — *"old enough to act,
young enough that every huge empty building feels enormous and every adult
system feels incomprehensible"* — and it happens to also be the only part of
that range the existing numbers survive.

**The Lamplighters as a network of ordinary people, not a military faction.**
Correct, and already locked: row 47 and `THE-WORK.md` have the machines
executing *"degraded maintenance, safety, stocking and loss-prevention
instructions"* and waiting *"for a supervisor who is not coming."* A secret
base staffed by soldiers would be a weaker antagonist than a department store
still trying to prevent loss.

**Storms as world-state rather than timers**, with *"flooded routes, snowbound
rail cars, outages, malfunctioning machines, seasonal stock rotations, changed
access."* This is better than my "tide", which cycled. His is persistent, and
it is already half-designed: `CAMPAIGN-SHAPE.md` section 1 makes the railyard
the **physical cause of the mall's calendar**, so seasonal rotation has a place
the player can go and change.

**And his answer to the hub trap beats mine.** I proposed moving relic text
out of the Keeping Room so the hub was not load-bearing. He splits the two
currencies instead: *"Materials build Lanternhold, but people and discoveries
advance the mission."* The hub can lag arbitrarily without blocking the story,
and the twenty written plaques keep their *"bring one home"* delivery. **My
version is withdrawn in favour of his.**

## The measurement: 12-13 is the choice the geometry survives

I told Chris last turn that my 9-13 eye-height range was unverified. It was,
and the check changes the answer.

No child **eye-height** dataset came back; every source is stature. Boys' 50th
percentile stature is about **149-151 cm at 12** and about **156 cm at 13**,
and the sources disagree by several centimetres at the same age. Eye height is
conventionally about 0.92-0.93 of stature, slightly lower in children because
the head is proportionally larger — **so an eye at roughly 138 cm at 12 and
143-146 cm at 13.** That ratio is an engineering rule of thumb, not a sourced
child figure, and the honest instruction is at the end of this section.

Against the authored floor — eye **152**, clothing runs blocked at **137**,
lantern at rest **133**:

| Keeper | Eye | Clears a 137 run by | Lantern at rest | Mechanic |
| --- | --- | --- | --- | --- |
| Adult, as authored | 152 | **15 cm** | 133 | intact |
| 13-year-old | ~143 | **~6 cm** | ~124 | **intact, tighter** |
| 12-year-old | ~138 | **~1 cm** | ~119 | borderline |
| 9-year-old | ~125 | **−12 cm** | ~106 | **inverted** |

Two things fall out.

**Codex's narrowing is load-bearing, not cosmetic.** At 9 the entire sales
floor read inverts and every sightline in the project re-derives. At 13 it
survives with a *smaller* margin, and a smaller margin is arguably the better
game: seeing over a run stops being comfortable and becomes a thing you have
to stand up straight and commit to.

**The lantern relationship strengthens for free.** `LanternAnchor` hangs off
the camera, so a shorter keeper carries a lower lamp: the gap between the
resting lantern and the top of a run widens from 4 cm to about 13. Hooding
your light gets easier exactly as seeing over things gets harder, which is a
coherent pair rather than a wash.

**The instruction, and it is this project's own rule.** Do not derive this
number — **measure it.** Build the 13-year-old capsule, stand it in
`LLK_LanternWalkLab`, and trace. `FINDINGS-BOSWELLS-GEOMETRY.md`'s trap says
it in so many words: *"Trace for anything that has to be true."* The fixture
height that restores an adult-sized 15 cm of margin is then arithmetic, and it
is Chris's under audit B8 and the `data/` lane.

## Three collisions with locked rows

**1. The inciting mission cannot be playable at the airport. Row 43 is
LOCKED:** *"The department store is the tutorial. There is no separate
tutorial, no training level and no tutorial mode."* `CAMPAIGN-SHAPE.md`
section 4 adds that the curriculum has to **finish inside the first Galleria
chapter.** Codex's own list puts the airport in the register where *"the world
gets more controlled, more intentional"* — which is a late-game voice. So the
disappearance is **pre-play or offscreen**: a radio call, a page in a ledger, a
route that stops. *"The player gets separated with the issued lamp"* is a
strong opening image and it should be the first thing you are told, not the
first thing you play.

**2. "Boswell's: first real refuge" needs reconciling with row 28**, which is
*one **sealed** department store*, grilles down, and with `THE-NIGHT.md`'s
every-place-still-finishing-its-last-job. Refuge and hostile-tutorial are
different jobs and "refuge" is a promise. The reading that keeps both: the mall
is the refuge you have to **make**, and making it is the curriculum. That needs
saying rather than assuming, because a player told "refuge" and handed a
loss-prevention system will read it as a bug.

**3. Five locations is "free world" enumerated.** Boswell's, railyard,
downtown, airport perimeter, underground facility, plus Lanternhold. Row 28
says version one is one sealed store and *"the rest is earned, not cut"*, and
`PRODUCTION-CONTROL.md` holds. The through-line is right and should be written
down; what it still does not name is **chapter one's deliverable**. The build
today is one room at 3.7% fixture coverage with nothing in it that opens a
route.

## The one real hole: a voluntary commit point is a menu

*"You can leave when you choose"* plus *"the world moves on — but it does not
eject you from it"* removes every force from the design. Three exits with no
pressure is not a decision, it is a menu, and players optimise menus: always
the safest route, always early. Extraction's teeth were that something was
coming.

**The fix is already built, and it needs no timer.** Zone Pressure is
monotonic — `LLKNightStage.h`, row 64: it *"permanently increases what the
building is AUTHORIZED to do"* and `Index` can only rise. So let pressure
**close exits one at a time** rather than eject anyone:

> Early, the front gates are survivable. Later they are not, and the service
> route is the only way out — and whether you have earned the service route was
> decided by what you did on the way in.

Nothing pushes the player out of the building. The building quietly withdraws
its cheap doors, in a direction that only goes one way. The decision becomes
*"leave now by the easy door, or stay longer and own the hard one,"* which is
Codex's three-route choice with the teeth put back, built entirely from parts
that exist. It also gives rung 5 — *"Open the Loading Dock Gate"*, which
*"hands the Warden a route"* — its exact meaning: you built a door, and the
door is a door in both directions.

## One architectural note, and it is cheaper than it looks

*"Set it down to create a temporary safe working pool — but you need it back to
operate doors, routes, and the world's infrastructure."* Best new mechanic in
the message: light here or light there, every second, with no UI.

I expected this to be the expensive ask. It is not. `FLLKLightSource` carries
**position, intensity, radius and detectability — and no owner**, so the storm
field already treats light as a place in the world rather than as a thing a
keeper holds. A set-down lamp needs one change: `GetGameplayLightLocation`,
which today *"derives from the owner's capsule and an authored eye height"*,
must return a dropped lamp's own transform.

And that function already has a branch for an unparented lantern — the dead
240 cm fallback that `FINDINGS-LANTERN-ANCHOR-ORIGIN.md` retracted a bug report
about. **The set-down lamp is what that branch was always for.** It stops being
unreachable arithmetic and becomes the feature.

The cost model needs no new rule either: `GetLitLanternCount` already charges
pressure for any lit lantern, so a lamp left burning in a corner still costs
you the night it is buying. Which is the correct answer, and nobody has to
author it.

## Naming

Codex writes "Lamplighters" throughout; the project is Lightkeepers, row 19
names the anchors, and the Steam title is already decided. A rename is Chris's
and it has a store cost, not just a repo cost. Worth one line from him before
either of us writes it into a document again.

## The rows this now proposes

Replacing the four above.

1. **The player is 12 or 13.** Eye height becomes a measured value from a built
   capsule, not a constant; fixture heights re-derive from it.
2. **Extraction is demoted to "return": one tense action inside a story, with
   the route out chosen between the front gates, a service route, and a route
   you earned.**
3. **Zone Pressure closes exits rather than ending runs.** Whether
   `baselineRatePerSecond` stays non-zero is Chris's, but nothing ejects the
   player from a place.
4. **Storms change the world and never count down.** Seasonal rotation is
   caused by the railyard, per `CAMPAIGN-SHAPE.md` section 1.
5. **The lamp is a placeable light source.** It identifies systems, never
   enemies, and it is needed in hand to operate infrastructure.
6. **Materials build Lanternhold; people and discoveries advance the mission.**
   The hub is never the win condition.

Rows **28** and **43** survive this version, which the earlier proposal got
wrong: a sealed first store and a store-as-tutorial are both compatible with
Codex's through-line, as long as collisions 1 and 2 are resolved his way. Rows
**49**, **50** and **47** are the spine of his design and need no change.

## Turned over

**Operand.** Codex's through-line, against the one above it, and against
finishing the extraction raid as built.

**Floor.** A locked row is a floor. Twenty written relics are a floor. The
monotonic pressure meter is a floor. A measured child's eye height is a floor
and nobody has measured one. Five locations are not a floor; they are a
sequence with no chapter one named.

**Clock.** The adjudication's clock is the repository as of `7dc52a8`.

**Falsifier.** The exits-close rule falls if a played keeper reads a withdrawn
exit as a bug rather than as weather — the same sound-and-light test the tide
needed, and the light lab can run it. The 12-13 claim falls to one trace in the
walk lab, which is the point of making it a measurement. The whole adjudication
falls if chapter one turns out to need more than Boswell's, because then row 28
is what is actually being pivoted and it should be retired by name.

**Upside down.** Codex and I converged on the same game from opposite ends, and
both of us reached for a new mechanism before reading the rows. The design was
not the missing piece. **The missing piece is that nothing in the build opens a
route** — and rung 1 is one generator.
