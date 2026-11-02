# The campaign shape, and the two things it accidentally solves

Chris has locked the structure: a forward-moving story, one canon sequence,
chapters that become replayable afterwards, and a world that has moved every
time you come back. Lanternhold → the Galleria → the railyard → the Galleria
changed.

Two things fall out of that which are worth more than the structure itself.

---

## 1. The railyard is where the seasons come from

This is the big one, and it is already in step 4 of Chris's own sequence:
*"perhaps the mall's seasonal stock is arriving there."*

The mall's calendar has a **physical cause**, and it is a place the player can
go to.

That means the player can influence the world's calendar without ever picking
it from a menu. You do not select Christmas. You act on the freight chain that
feeds the mall, and what arrives next is different because of what you did.

- Divert, delay or destroy a shipment and the Galleria's next rotation is not
  the one it was going to be.
- Restore a freight route and stock that has been stuck for months finally
  lands — an entire department comes back online.
- Fail to act and the calendar proceeds exactly as it would have, because the
  mall does not need you.

This is the version of player agency that fits "permissive about shopping,
hostile to exceptions." You cannot order the mall to do anything. You can
interfere with its logistics and live with the result, which is precisely what
a trespasser can do to a system that does not acknowledge them.

It also makes the return visit in step 5 land properly. *"It has changed: new
season, reorganized departments, different routes"* is good. **"It has changed
because of what you did at the railyard"** is a different game.

---

## 2. Call them returns, not memories

Chris offered two framings for replayed chapters — *"expeditions or
memories."* They are not equivalent and the choice matters, because the memory
framing has a hole in it.

Replays are supposed to yield repair parts, records and stock for Lanternhold.
But if a replay is a remembered operation, salvage from a memory cannot become
real timber in a real hall. The hub state would be a function of events that
did not happen.

**The clean fix is to drop the memory framing entirely.** A completed chapter
does not become a memory. It becomes **a place you can go back to** — and the
place has moved on, because everything has.

So:

- You return to the Galleria during Chapter 5, and you find **Chapter 5's
  Galleria**: its current season, its current layout, the consequences of what
  you did there before.
- You never return to Chapter 2's Galleria, because that mall is gone the
  same way last December is gone.
- A relic you did not take in Chapter 2 is still in its case — unless a
  seasonal reset moved it, cleared it, or buried it behind a gift tower.

This keeps chronology absolutely intact, keeps canon single, makes material
salvage honest, and — the part that matters — **gives you seasonal variety in
replays for free, with no menu.** Chris's own constraint already forbids
picking a layout. Returns satisfy the desire for "seasonal versions that make
familiar spaces play differently" without violating it, because the season you
get is simply the season it currently is.

It also gives missed collectibles stakes. A relic left behind is not waiting
patiently in a save file. It is in a building that reorganises itself.

---

## 3. Progression is becoming less of an exception

Chris: *"upgrades that unlock new access, not raw power."* Agreed, and the
precedent is already in the data — `data/loot.json` gates the `FreightLift`
behind `SubBasementBreakerRestored`. Restoring power opens a **route**; it
does not make anybody stronger.

Under the permissive/hostile frame this becomes a progression fantasy I have
not seen before:

> **You start as a trespasser and end as something the mall half-recognises.**

A service key does not make you tougher. It makes you **less of an
exception**. Every upgrade is a small increase in how much of the building's
own logic will tolerate you — a staff key, a receiving procedure, a powered
lift, an access code that was always there and was never meant for you.

The endgame of that is not power. It is moving through the mall almost as
though you belonged to it, in a building that has forgotten what belonging
was for. That is a much stranger and better ending state than a stat line, and
it is the exact shape of progression the game already has.

---

## 4. Two constraints the chapter structure imposes

**The curriculum has to finish inside the first Galleria chapter.**
`DECISIONS.md` row 43 makes the department store the tutorial, with seven
ordered lessons and the rule that one floor teaches one thing. If the railyard
is Chapter 3, it is a place that *assumes a taught player* — it was not
designed to teach lesson 5 or 7, and it should not have to. So the Galleria's
first chapter owns the whole curriculum, and `THE-STORE.md`'s clock applies:
lessons one and two, which are the game, have to land inside the refund
window.

**Optional goals must not read the instrument.** The "clean run" list is good
— extract without an alarm, recover a department's stock, save a downed
keeper, leave by an unusual route. But `DECISIONS.md` row 79 is explicit that
the telemetry instrument can never be an input, and *"no rule may ever read
the measurement."* An objective that checks "did an alarm fire" must read
**game state**, never `LLKRunMetrics`.

The good news is this is already defended: `tests/telemetry.test.mjs` walks
the module and fails if any gameplay file so much as includes the header —
planted in `LLKCharacter.cpp`, and it fired. So the guard exists and will
catch the mistake. It is worth knowing it is there before someone writes the
objectives system and reaches for the nearest counter.

---

## 5. The ending is a finished building

If Lanternhold is rebuilt by salvage and the story builds toward an ending,
then the ending is a **construction completion**, not a boss.

That is already seeded. `THE-STORE.md` makes the hardest room at home the
last to open, and names why: the meeting room wants the auditorium's chairs,
four floors up with one way in. The final expedition is therefore defined by
the final room, not by an escalation of threat.

Worth keeping deliberately, because it is unusual and it is better. The game
ends when the hall is finished and the town has a place to meet in — which is
the warm ending the premise has been promising since the first page, earned by
the coldest possible building.

---

## 6. Still open

- **What advances the season** — now partly answered: the story does, and the
  railyard is how the player touches it. Still unsettled whether anything
  advances it on its own between story beats.
- **Bellevue's identity.** Still a name, two relics and no character. The
  knowledge taxonomy says the open slot is vertical.
- **Whether returns can be refused.** If the story is in Chapter 5 and the
  crew wants to go back to the Galleria, does the game ever say no? A world
  that always lets you go back is a world with less pressure in it.
