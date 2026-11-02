# The mall is still trading

Chris, 2026-10-31: *"they change the stock, it becomes Christmas, the mall
changes, it's always moving, it's alive and still playing itself out. Fall
wear, winter wear, summer. Robot vendors arrive, the mall changes."*

Supersedes the work-order framing in `THE-STORE-STILL-RUNS.md`, which was not
what Chris meant. The mall is not dispatching staff to tasks. **It is running
its retail calendar, forever, for customers who are never coming.**

That is a better idea and a colder one. Nothing is hunting you because nothing
has noticed the difference. The mall is doing Christmas because it is
December.

---

## 1. Three corrections to the previous page

- **The never-built anchor is filled, not empty.** The real Johnstown Galleria
  had an anchor slot that never got its store. We put our fourth store in it.
  Four anchors, all stocked, all unique. The previous page turned it into a
  hollow concrete room, which is the opposite.
- **The customer entrance is alarmed, not permissive.** Leaving a store with
  goods through the mall-facing gates sets off the alarm. This is truer and
  mechanically far better — see §4.
- **The roster being job-shaped still holds, but it is not a dispatch
  system.** Sweepers, Stockers, Window Walkers and the Ficus are all former
  retail jobs, and that still supports the mall being alive. It does not need
  the work-order layer on top.

---

## 2. The season is the game's second clock

Storm Charge is the clock *inside* a run and it only ever rises. The game has
never had a clock *between* runs.

**The season is that clock.** The mall rotates its merchandising — summer,
back-to-school, fall, Christmas, the January reset — on its own schedule,
indifferent to the crew.

The crucial rule, and it is what makes it horror rather than a shop menu:

> **You do not choose the season. You arrive in whatever the mall is
> currently doing.**

The hall needs blankets, and the store has blankets in winter rotation. So you
wait, or you go anyway and take what is on the floor. The mall is not
withholding anything from you. It simply is not thinking about you at all,
and that indifference is the whole tone.

It also solves a real design problem for free: *why is this run different from
the last one?* Not a modifier, not a seed — **the shop has changed its
displays**, which is the most ordinary and most unsettling possible answer.

---

## 3. Season changes the shape of the store, not just the loot

This is where it gets cheap to build and deep to play. A season is a fixture
set and a merchandise set, swapped — and `THE-STORE.md` already commits to
*repetition with intent*: five fixture meshes per department, one or two
merchandise meshes instanced in bulk and recoloured. A seasonal swap is
exactly that system used twice.

| Season | What is on the floor | What it does to play |
|---|---|---|
| **Christmas** | Display lighting everywhere, trees, decoration aisles, gift stock | **The brightest run and therefore the worst.** Ambient light means less need for the lantern and more being seen. See below. |
| **Winter** | Coats, heavy racks, blankets, soft goods | Best cover in the game. Dense, tall, soft — and it blinds you as much as it hides you |
| **Fall / back-to-school** | Dense small merchandise, endcaps, tight aisles | Maximum sightline breakage. The navigable season |
| **Summer** | Patio, garden, fans, big low displays | Open floor, long sightlines, almost nowhere to hide |
| **January reset** | Half the floor empty, stock in transit, bare fixtures | The floor plan you learned is gone |

**Christmas is the idea worth building first.** A Christmas rotation fills the
store with working display lights — which under the current rules means
ambient illumination across a floor that is normally black. The lantern
becomes less necessary and `aggroRadiusLit` is unchanged, so everything in
the building sees you from 38 instead of 12, all the time, without you having
chosen it.

The brightest, warmest, most nostalgic season is the most lethal one. That is
the game's central trade turned inside out for one rotation, and it needs no
new system — only fixtures that are already going to exist.

It also gives the fixture-light contract its first real exercise: Christmas
lighting is exactly the case where "orientation-level, not inspection-level"
gets hard, because strings of display lights are *everywhere* rather than in
pools. Worth measuring in the lab once the ladders are done.

---

## 4. Alarms, gates and the three-layer route

Chris: *"Leaving a store with something sets off an alarm at the gates unless
u leave from the back."*

This is the routing system the middle game has been missing, and it is drawn
straight from how malls actually work.

**Three layers, three characters:**

| Layer | Light | Speed | Risk | Carries goods? |
|---|---|---|---|---|
| **Concourse** | Lit — storm light through the skylights | Fast | Exposed, seen from distance | Yes, but you had to get out of the store first |
| **Shop floor** | Dark | Normal | Rich, and where the stock is | — |
| **Service ring** | Black | Slow | Quiet, no alarms, connects everything | Yes. This is the goods route |

Real malls have a back-of-house corridor ring joining every store to the
loading dock, because that is how stock arrives. **That ring is the game's
logistics layer**, and it is the answer to extraction: big loads go through
the back, because they always did.

So the gates do the work of a difficulty dial with no numbers in it:

- **Through the front with goods** → alarm. Fast route, lit concourse, but you
  have announced yourself to the whole floor.
- **Through the back** → silent, but the service ring is black, slow, and you
  are carrying.
- **Through the front with nothing** → free. Scouting is always permitted,
  which is a lovely rule: the mall does not mind you looking.

That last line is the one to keep. **Looking is free; taking is not.** It is
the retail logic and the horror logic at the same time, and it means a crew
can always reconnoitre without paying — which makes the decision to pick
something up the moment the run turns.

---

## 5. Locked stores, grilles and getting in

Chris: *"stores need opened sometimes, they are locked?"*

Yes, and it pairs with the alarms to make entry a real decision rather than a
door.

- **The rolling grille at the mall frontage.** Opening it is loud and slow —
  a mechanical sound the whole concourse hears. The obvious way in, and the
  worst one.
- **The service door from the back ring.** Quiet, but needs power or a key.
  `data/loot.json` already gates the `FreightLift` behind
  `SubBasementBreakerRestored`, so the vocabulary for this exists: restoring
  power opens routes rather than granting abilities.
- **Stores that are already open** because something came through them.

Entry and exit therefore use the same two channels in reverse, and a crew that
came in the loud way has not opened the quiet way out.

---

## 6. Deliveries: the mall restocks while you are in it

Chris: *"robot vendors arrive."*

A delivery is the mall doing the one thing it most obviously still does, and
it makes a run change shape mid-way:

- New stock on the floor where there was none, so the loot you scouted is not
  the loot that is there.
- Routes blocked by pallets and rolltainers that were not there ten minutes
  ago — **the service ring is the delivery route**, so a delivery closes the
  quiet way out.
- Something moving that is not an enemy and not a crewmate, which is its own
  kind of dread.

**And the rare version: a changeover.** Retail resets its floor overnight. A
run that lands during a seasonal changeover finds the mall rearranging itself
— fixtures in motion, half the stock in transit, the floor plan the crew
learned simply not true any more. That is the most frightening ordinary event
available to this game, and it costs one state flag plus prop placement.

---

## 7. The mall itself

Johnstown Galleria, close to as-built, with minor changes for play, plus a
park inside it like Richland Mall had.

- **The park** — real planting, benches, a quiet green room in the middle of a
  machine. `docs/ZONES.md` already derives the Ficus from mall landscaping, so
  the park is where it worked and the one place its presence needs no
  explanation.
- **The carousel** — the machine whose entire purpose is to go round forever.
  If one thing still runs, it is this. Audible across the concourse, and a
  landmark precisely because it never stops.
- **The fountain** — water, in a game about a flood.
- **The clocktower elevator** — a clock still keeping time in a building with
  nobody to be on time. The natural diegetic home for a value that only rises.
- **High skylights** — the reason the concourse is lit and the stores are not,
  which is the inversion the whole three-layer route depends on.

The four anchors sit at their real positions, including the slot that never
had one.
