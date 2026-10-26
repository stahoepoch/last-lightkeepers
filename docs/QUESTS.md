# The ladder — quests that make it easier and harder at once

Status: **the opening five are Chris's, 2026-10-26.** Everything after them is
proposed. Chris:

> *"The whole Galleria can be the eventual world, but the game begins with one
> sealed department store and earns its expansion. Quests should be practical
> recovery jobs, not 'collect ten glowing widgets.' Each one changes something
> visible, unlocks a route or tool, and introduces a new threat… The player is
> not simply leveling up — they are rebuilding the tools needed to reach
> farther into a place that notices it is being reopened."*

That last clause is the thesis of the whole game and should be quoted at anyone
who joins the project.

## The shape every quest has

> **Restore one system. Gain what it does for you. The building gains the same
> thing.**

The premise already pays for this and nobody had noticed: **the machines run on
light, and they charge off power.** So every wire you reconnect is a wire they
are on too. Restoring the store is not a safety gain the design then has to
claw back for balance — it is *literally* feeding them, by the rule locked on
day one (`DECISIONS.md` 5).

So there is no difficulty tax to invent, no raid level to tune, and nothing
arbitrary to explain to a player. Light a wing and that wing stands up. A player
understands it the first time it happens to them and never needs telling again.

**Four rules keep it honest:**

1. **Practical recovery jobs only.** Chris's words, and it is a hard filter: a
   quest is a thing a person would actually do to a broken building. Fuses,
   tools, lift parts, a radio, a gate. Never *collect ten of a glowing thing* —
   the moment a quest item exists only to be counted, the building stops being
   a building.
2. **The trade is stated before you accept, in plain words.** The ledger says
   what opens and what wakes. A trap the player chose is a decision; a trap
   sprung on them is a bug with a story attached.
3. **No restoration is reversible.** You are changing the building for good.
   The order you pick is your fingerprint on it, and two crews' buildings
   genuinely differ by hour twenty.
4. **Nothing on this ladder ever makes light free.** A quest may open a floor, a
   route, an exit, a tool, information, or a room at home. A quest may never
   lower Storm Charge, reduce the lantern's cost, or make an enemy weaker.

## Where quests come from

**The ledger** — the order's work book, and the same object as the Good Cheer
Club roll on the hall wall. The roll already records who paid for which part of
the building (`docs/PREMISE.md`); the ledger is the open half of it: the work
not done yet, in the order's own hand, with a line left blank for the name of
whoever does it.

No quest-giver NPC and no exclamation marks. The hall already owns the right
object, and a keeper reading their order's unfinished work is the premise
rather than a UI.

## The opening five — Chris's, 2026-10-26

| # | The job | Salvage | Opens | Wakes |
| --- | --- | --- | --- | --- |
| 1 | **Restore the Departure Generator** | Fuses and wiring, ground floor | Reliable departures, and basic map markers | Powered lights make **Window Walkers** active |
| 2 | **Recover the Workshop Tools** | The old maintenance cage | A simple **defensive tool or noisemaker** | Opens the staff corridors, where **Sweepers** patrol |
| 3 | **Bring Back the Lift Parts** | Heavy, dangerous salvage from the stockrooms | The upper department-store floor | **Mannequins among the displays** |
| 4 | **Restore the Lanternhold Radio** | Electronic components, deeper in | Warnings, quest leads, better extraction information | Attracts **Glimmers**, and broadcasts the crew's activity |
| 5 | **Open the Loading Dock Gate** | A multi-run restoration job | **The next mall section** | Gives the **Warden** a more direct route into the store |

**What that ordering gets right**, beyond the trades themselves:

- It introduces five of the six enemies in five rungs, each on the run that
  made it relevant — so the roster teaches itself and nothing is wasted. The
  Ficus is the only one left over, and it is the one that needs no introduction
  because it is a houseplant until it isn't.
- Rung 2 answers the combat question by *earning* it rather than shipping a
  weapon in the player's hands on run one. See `docs/COMBAT.md`.
- Rung 4 is the only rung whose reward is **information**, and it is the one
  that makes the crew legible to the building in return. Knowing more and being
  known more is the cleanest version of this ladder's whole trade.
- Rung 5 is multi-run on purpose, which is what makes it feel like a gate and
  not a key, and it hands the Warden a route — the one enemy that cannot be
  fought, arriving by the door you built.

**Two reconciliations this needs** (both value calls, so both Chris's):

- `data/loot.json` currently has the **LoadingDock** as the *reliable* exit at
  1.0x and the **FreightLift** as the risky 1.6x one gated behind
  `SubBasementBreakerRestored`. On the ladder above, rung 1's generator is what
  makes departures reliable and rung 5's gate is the way into the mall. Those
  are compatible but not yet aligned: the breaker flag probably *is* the
  generator, and the dock gate is probably a third thing that is not an exit at
  all. Worth one line from Chris before anyone builds either.
- **Map markers** (rung 1) and **warnings and extraction information** (rung 4)
  are both HUD capability that currently exists unconditionally. Gating them
  means the base HUD gets *less* until rung 1, which is the right shape and
  needs saying out loud before it looks like a regression.

## Later rungs — proposed, mine, not Chris's

Kept because the building has floors the opening five do not reach, and
deliberately placed *after* them. None of these is approved.

| The job | Opens | Wakes |
| --- | --- | --- |
| **The pump** — drain the bargain basement | A whole floor, and the best Components in the game | What the water preserved. It kept working down there |
| **The well** — clear the escalator fall | Fast vertical movement between every floor | Theirs too. A route is a route in both directions |
| **The bell** — restore the public-address panel | A decoy you *aim*: noise, somewhere else, on purpose | The PA is how the store called its staff. They still answer it, and it tells them where the panel is |
| **The auditorium** — recover the hall's chairs and the programmes | The meeting room at home, and the full roll wall | The deepest room in the store, four floors up, one way in |

## Why this answers "what's the point of having things"

Chris, earlier the same day: *"there's just so many things to loot, so what's
the point of having them… there needs to be reasons to get things and have
things and do things and go out."*

A ladder turns loot from a score into a **shopping list with a reason and a
deadline**. Before: you carry out forty objects and a number goes up. After:
the pump needs a motor you have only seen on the fourth floor, and the fourth
floor has no roof.

Three open problems close as a side effect:

- **Route choice gets a motive.** `docs/VERTICAL-SLICE.md` element 1 wants one
  route with two readings. A ladder gives every floor a night when it is the
  floor you need.
- **The hall gets something to restore.** Element 5, the largest unbuilt thing
  in the design, stops being an abstract "rooms open" and becomes named rooms
  with named costs — the generator, the workshop, the radio, each with a
  visible home. `docs/THE-STORE.md` carries the department-to-room mapping.
- **Difficulty stops needing a difficulty system.** The building gets harder
  because the player made it work. No slider, no raid level, nothing to balance
  that the player did not ask for.

## What a rung is made of

For whoever builds the first one, a rung is five things and no more:

1. A line in the ledger: the job, the trade, the blank for a name.
2. A **place** in the store that is visibly not working.
3. A **cost** — Supplies, Components, crates. Never relics (`REWARDS.md`).
4. A **thing that opens**, which the player can see from where they stand.
5. A **thing that wakes**, announced in advance and permanent.

Nothing above needs a quest system in the software sense: no branching, no
state machine, no objective markers. Five flags and five costs.

## Open, and Chris's

- **Every cost figure.** None are written here on purpose.
- **The two reconciliations above** — the exits, and gating the HUD.
- **Whether rungs 2 through 4 are ordered or picked.** They read as a sequence;
  leaving them independent would let two crews' buildings diverge, which is the
  more interesting version and the harder one to tune.
- **How many rungs before the concourse.** Five is Chris's shape. If the store
  is fun at three, the mall comes sooner.
