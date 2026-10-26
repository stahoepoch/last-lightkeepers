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

**One thing this surfaced, and it is Chris's call.** The curve hits all four of
his anchors for a solo keeper and for a pair. For three or four keepers using
lanterns regularly it runs roughly two minutes early, because every lit lantern
stacks: `node tests/pacing.mjs` puts a crew of three at 25% duty on the Warden
at 5:31. Whether that is wrong depends on intent - it does make a big crew
coordinate its darkness, which is real co-op - but it sits against "friends
make rescues, split routes, and extraction defense more interesting rather than
mandatory" if a four-player run is always the shortest one. The fix is a choice
between the `diminishing` curve already written into `storm-charge.json` and
normalising by crew so that one crew's worth of light is 2.5x no matter who is
holding it. Nothing changed; the clamp is still `linearCapped`.

## 1. One polished mall route

**Exists:** the Galleria is built and audited — perimeter sealed, 81% of the
contract unlit, four extractions all reachable. Exits include `LoadingDock` and
`FreightLift`. `RaidDirector` applies one wrinkle per raid.

**Missing:** a route is not a map. Nothing currently *shapes* a path — there is
no intended short way and long way, no reason to take one, no sightline that
teaches you the next room before you enter it.

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
the room retroactively becomes a different room.

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

**The largest unbuilt thing in the whole design.** `HubService` manages the
lamp wall, the relic plinths, the roll board and three landmark plates. **There
are no rooms that open.** The hall is already dark in the right way, but
nothing in it changes because of a run except a plate's colour.

This is the first session's entire payoff, the pitch's biggest hook — "whole
rooms you can see into but can't go in yet… everything you bring back turns
another light on" — and the reason anyone plays run two.

**What it needs:** rooms the builder already makes, sealed and visibly
unfinished, each with a crate cost and a name on the roll when it opens; the
side room lit at the start, because the order really did meet in a side room
for years while the hall stood unpaid for.

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
