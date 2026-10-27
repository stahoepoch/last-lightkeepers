# The vertical slice — the 10–15 minutes that has to be fun

Status: **proposed**, 2026-10-26. Chris's five elements, checked against the
build. Nothing is marked done here on the strength of code existing: an element
is done when it is *fun in a session with a person in it*.

Chris: "Once that 10–15 minute loop feels alive, adding floors, stores,
enemies, and Steam-grade polish becomes building outward from a real game
rather than hoping volume creates one."

## The two blockers — both settled 2026-10-26

**1. The loss rule is locked.** Only extraction secures loot. Death or the
timer running out loses the carried haul. No banking, ever. Full reasoning in
`DECISIONS.md` O3; the risk it carries and how the design answers it are at the
top of `BANKING.md`.

**What the relay is now, stated once so nobody re-invents the bank:** a route,
intel and lantern-support point. It shows the active exit on its board, and it
may recharge a lantern if that optional tuning is enabled. It never touches a
haul. The code says so too - `RunStateService.touchRelay`, renamed from `bank`
on the day the rule was locked.

**The argument the run is built around**, in Chris's words: *"We have enough -
call the loud extraction now, defend it, or risk one more store?"* Calling the
exit is the loudest act in the game: a flare and a siren, every patrol
converging, and a flat Storm Charge spike regardless of what the meter was
sitting at. That spike is the new part - the siren and the convergence were
already built, which meant calling early was strictly free and there was no
argument to have.

**2. The Storm Charge baseline is tuned**, to Chris's curve, and asserted in
`tests/balance.test.mjs` so it cannot drift again:

| | first pressure | serious pursuit | severe |
| --- | --- | --- | --- |
| Fully dark | 4:30 | 8:00 | 11:40 |
| Lantern used regularly | 2:34 | 4:34 | 6:40 |

Thresholds moved from 480 / 720 / 960 to **270 / 480 / 700**. The one-lantern
2.5x anchor is untouched; only the thresholds moved. The old numbers put the
Warden at 15:59 for a dark crew - past the end of any run anyone would play -
so darkness carried no consequence whatsoever, which is the opposite of what
the premise claims.

**One thing this surfaced, and Chris settled it on 2026-10-27.** The curve hit
all four of his anchors for a solo keeper and for a pair, but ran roughly two
minutes early for three or four keepers, because every lit lantern stacked and
the clamp then made the fourth one free. That sat against "friends make
rescues, split routes, and extraction defense more interesting rather than
mandatory" if a four-player run is always the shortest one.

His call: **"Switch `storm-charge.json` to the diminishing curve so no keeper's
light is free."** One line of data moved; the shape was already written and
tested. What it bought, from `node tests/pacing.mjs`:

| crew, 25% duty | Warden, `linearCapped` | Warden, `diminishing` |
| --- | --- | --- |
| solo | 8:15 | 8:15 |
| two | 6:39 | **6:57** |
| three | 5:31 | **6:41** |
| four | 5:31 | **6:13** |

The clamp is visible in that table: under `linearCapped` a crew of three and a
crew of four reached the Warden at the *same second*, because the fourth
lantern cost nothing. Both of those rows read WARDEN BEFORE A SHORT RUN ENDS.
Under `diminishing` all four crew sizes land mid-run, and the marginal cost of
the Nth lantern is +1.50 / +0.90 / +0.54 / +0.32 - smaller each time, never
zero. A bigger crew still escalates faster, which is the co-op pressure Chris
wanted; it just no longer stops escalating. The 2.5x one-lantern anchor is
untouched, `maxMultiplier` is now inert, and `tests/balance.test.mjs` asserts
that no keeper's light is free under whichever curve is actually active, so a
quiet revert fails the suite.

## 1. One polished mall route

**Narrowed 2026-10-26.** The slice's route is now inside **one sealed
department store**, not the whole Galleria — Chris: *"furnishing one building
from top to bottom."* The Galleria is not cut; it is what the last rung of the
ladder opens. See `docs/THE-STORE.md` and `docs/QUESTS.md`.

**Exists:** the Galleria is built and audited — perimeter sealed, 81% of the
contract unlit, four extractions all reachable. Exits include `LoadingDock` and
`FreightLift`. `RaidDirector` applies one wrinkle per raid.

**Missing:** a route is not a map. Nothing currently *shapes* a path — there is
no intended short way and long way, no reason to take one, no sightline that
teaches you the next room before you enter it. The ladder supplies the missing
motive: a rung makes one floor tonight's floor. The dressing supplies the rest,
and signage is most of it.

**Done when:** a new player walking in at the spawn finds the dock run without
being told, and a player on their tenth run deliberately ignores it because the
long way pays better and closes earlier. One route, two readings.

## 2. One meaningful risky pickup

**Exists:** relics are lantern-only (`RelicVisibilityController`), taking is a
hold rather than a tap, and carrying them costs movement through the burden
rule — which the workshop's first line reduces and never removes.

**Missing:** one *specific, authored* pickup. The risky pickup is currently a
category, not a moment. It wants a place in the building where the geometry
makes the hold genuinely frightening: visible from somewhere you cannot watch,
no second exit, or a hold long enough that you must decide to commit before you
start.

**Done when:** players talk about *that* cache by name, and some of them leave
it behind.

## 3. One frightening enemy encounter

**Exists:** six enemies specified in `data/enemies.json`, each with its former
job, its counterplay and real numbers — Sweepers (swarm call), Stockers (sight
cone, loses you), Glimmers (scan and tag), Window Walkers and the Ficus (wake
on an active lantern), the Warden (unkillable, spawns on a charge stage). Rig
files exist for three. Audio cues are stubbed pending Chris's uploads.

**Missing:** choreography. Six enemies that each work is not an encounter. The
slice needs one scene that is *staged* — a Window Walker you walk past twice in
the dark before anything happens, so that the moment your lantern catches it
the room retroactively becomes a different room. The ladder stages this for
free: rung 1 powers the lights and wakes the Walkers, so the floor the player
already knows is the floor that changes.

**And they can now be fought, within one rule** — you win the fights you start.
Settled 2026-10-26; see `docs/COMBAT.md`. The health values five of the six
have been carrying since the handoff finally mean something.

**Done when:** the first time is frightening and the tenth time is a route
decision. If the tenth time is still frightening it is noise; if the first time
is a route decision it is wallpaper.

## 4. One extraction holdout

**Exists, and this is the most finished element.** One crew holdout at a time,
belonging to the whole crew; enemies converge on the circle; nothing spawns
inside it; downed keepers can be dragged in; HUD alert and an ambience bed
switch when it opens. Exits close over the run on a timer
(`EXIT_CLOSE_SECONDS`), so the way out moves.

**Missing:** a reason the holdout is a fight you can lose and still laugh
about. It also inherits blocker 1 directly — a holdout you fail currently costs
everything, which makes the correct play never to hold at all.

**Done when:** a crew chooses to hold with loot on them, and the ones who
failed want to go again immediately.

## 5. One visible Lanternhold improvement

**Half built, 2026-10-26.** Chris chose this over the first store rung, and the
systems side is done: `src/server/services/LedgerService.luau` holds the hall's
rooms and the ledger board, `data/ledger.json` holds four rooms at 1 / 3 / 6 /
10 crates - Chris's figures, kept on review on 2026-10-27 - the costs are
committed out of a keeper's credited
crates without ever subtracting from the credit, and only the host's rooms show
or open. **What is missing is geometry** - four sealed doorways tagged
`HallRoom` and one board tagged `LedgerBoard`, which is the map lane's. The tag
contract is in `COORDINATION.md`. Until it is tagged, the service is a no-op,
like every other hall fixture without its part.

**What it was before today.** `HubService` managed the lamp wall, the relic
plinths, the roll board and three landmark plates. **There were no rooms that
open.** The hall is already dark in the right way, but
nothing in it changes because of a run except a plate's colour.

This is the first session's entire payoff, the pitch's biggest hook — "whole
rooms you can see into but can't go in yet… everything you bring back turns
another light on" — and the reason anyone plays run two.

**What it needs:** rooms the builder already makes, sealed and visibly
unfinished, each with a crate cost and a name on the roll when it opens; the
side room lit at the start, because the order really did meet in a side room
for years while the hall stood unpaid for.

**The ladder names them**, which it did not before: the generator, the
maintenance cage's tools, the radio. Each is a repair with a visible home at
both ends — `docs/QUESTS.md`, and `docs/THE-STORE.md` for the
department-to-room mapping that gives every floor of the store a room at home
it supplies.

**Done when:** a player finishes their first run, hands in, watches one door
open, and goes back out without being asked to.

## What the slice does not include

No second zone. No new enemy. No new currency. No Steam build. No publishing.

`ZONES.md` already gates the solar farm behind the playtest question — *do
testers kill their lanterns, bank their haul, and argue for one more room?* One
third of that question cannot currently be observed, which is blocker 1 again.

## The honest order

1. The two blockers. They are decisions, not work, and nothing can be judged
   without them.
2. Element 5, the rooms that open. Without it there is no reason for run two,
   so there is no loop to test.
3. Elements 2 and 3, the authored pickup and the staged encounter — they are
   placement and choreography inside a building that already exists.
4. Element 1, the route's two readings.
5. Element 4 last. It is nearly built, and it depends on blocker 1 to mean
   anything.
