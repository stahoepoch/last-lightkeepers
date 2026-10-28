# The store — one building, furnished top to bottom

Status: **proposed**, 2026-10-26. Chris: *"it's just like one department store
that you're in, so that department store would be massively stocked and feel
like a real department store and not just Roblox… furnishing one building from
top to bottom and making it a busted up broken old and kind of dilapidated
style place that we go into and salvage."*

This narrows `DECISIONS.md` 4 (*one mall, and it is the Galleria*) rather than
reversing it. The Galleria stays the zone and everything already built stays
built. **The playable interior for version one is one anchor store.** The
concourse and the rest of the mall are what the store's service doors open onto,
earned — see `docs/QUESTS.md`.

Chris, the same day: *"I think the whole mall is definitely going to be
something, but we should start small."* This document is the small.

## Which store

`DECISIONS.md` 19 already names the four anchors with no real trademarks in
them: **Boswell's, Sayers, Penhurst, Bellevue.** Recommend **Boswell's** — it
reads as the oldest and the most local of the four, which is the one a town
would have mourned, and the only one whose name wants an apostrophe and a
founder behind it. Chris's call; the rest of this page holds whichever it is.

Four floors, a basement, and a roof that is not an exit (`data/loot.json`
already says so).

## The ruin is vertical, and that is the whole art direction

The flood came up and the roof came down. So the building rots in a gradient,
top and bottom toward the middle:

| Level | What happened to it | How it reads | How it plays |
| --- | --- | --- | --- |
| **Basement** | Took the water and kept it | Black water, floating stock, silt line | No sightlines, no dry footing, the best Components |
| **Ground** | Took the water and gave it back | A tide mark at chest height on every fixture, everything below it ruined and everything above it intact | Legible, open, the floor you learn the building on |
| **Second** | Dry, sealed, untouched | The most *intact* floor in the game — stock still folded, lights still in their sockets | Uncanny rather than ruined. The scariest floor, because nothing is wrong with it |
| **Third** | Water came through the roof slowly | Stained ceilings, swollen floors, furniture showrooms gone soft | Soft footing, collapsed sightlines, staged rooms |
| **Fourth** | The roof failed outright | Sky through the deck, weather inside, plants in the toy department | Half-outdoor, lit by the storm, so your lantern is less useful and you are more visible |

Chris's "busted up, broken, old, dilapidated" is doing two jobs here. It is the
look, and it is **why the good things are awkward to reach** — a store that
fell apart evenly would just be a dark shop.

**One rule for the whole building:** the ruin is always *something that
happened*, never texture. Water has a level. A collapse has a direction. If a
prop is broken and you cannot say what broke it, rebuild it intact.

## The floors

### Basement — receiving, bargain, and the plant

Receiving bay and the loading dock. The bargain basement. The boiler room, the
switchgear, the sub-basement breaker (`data/loot.json` already gates the
`FreightLift` behind `SubBasementBreakerRestored` — that quest was written into
the data before it was written down anywhere).

**Yields:** Components, richest in the game. Wiring, tools, fixtures, motors.
**Light:** nothing ambient at all. The only floor where the lantern is not a
choice.
**Why you come:** because the exits and the power are both down here, so every
quest that makes the building *work* ends in the dark.

### Ground — cosmetics, jewellery, the well

Glass counters, mirrored columns, perfume cases, the jewellery department, the
main escalator well, the sealed street entrances with the water still against
them.

**Yields:** Relics, concentrated. Small valuable things were kept in locked
glass on the ground floor of every department store that ever existed, and the
locks are the hold.
**Light:** your lantern comes back at you off glass and mirror. The one floor
where light *betrays its own carrier* geometrically rather than by rule.
**Why you come:** first floor in, last floor out, and the richest relic density
outside the Keeping Room.

### Second — menswear, womenswear, fitting rooms, shoes

The intact floor. Racks still full, stock still folded, the fitting-room
corridor with its mirrors and its curtains, the mannequin islands.

**Yields:** Supplies in bulk — fabric, leather, thread — and the hall's bedding.
**Light:** no ambient, long straight aisles, excellent sightlines when lit.
**Why you come:** it is the most generous floor in the building and the most
populated, because Window Walkers were display mannequins and this is where
they worked. Lighting an aisle here is the clearest version of the game's one
decision.

### Third — housewares, furniture, linens, the lunch counter

Kitchenware, appliances, lighting fixtures, bedding, and the furniture
gallery: a dozen staged showrooms, each a complete little room with a price
card, now soft and stained.

**Yields:** the hall. Everything Lanternhold is missing is on this floor.
**Light:** showroom walls cut the floor into rooms, so sightlines are short and
you are never sure what the next doorway holds.
**Why you come:** see *The store is the hall inverted*, below. This floor is the
answer to Chris's question.

### Fourth — toys, the auditorium, the offices

The toy department, the portrait studio, the credit and accounts offices, and
the store auditorium — department stores of this size had halls where the town
held things, and that is not a liberty, it is what they were for.

**Yields:** Relics with *names on them* — ledgers, account cards, staff
photographs, the auditorium's own programmes. The writing floor.
**Light:** the roof is gone, so the storm lights it. Your lantern adds little
and you are visible from a long way off.
**Why you come:** the story, and the auditorium is the deepest single room in
the store with one way in.

## The building is the tutorial

**Chris, 2026-10-27: "The department store is the tutorial."** `DECISIONS.md`
row 43, LOCKED. There is no training level, no tutorial mode and no narrator.
This building teaches the whole game by being played, and that is now the
first thing its layout is judged on.

The headline is his. The curriculum below is **proposed** — it is the order the
floors above already imply, written down so the grey-box can be built against
it instead of discovering it later.

### Why the ruin gradient is already a syllabus

The floors were designed as an art direction, but they happen to run from
*legible* to *hostile* in exactly the order a player needs to meet those
conditions. Nothing below invents a floor. It assigns each one a lesson and
says what the exam is.

| Order | Floor | The one new idea | How the floor examines it |
| --- | --- | --- | --- |
| 1 | **Ground** | There are things here and you can carry them | Storm light through the windows means baseline loot is visible with the lantern off. The player loots before they learn light costs anything |
| 2 | **Ground**, deeper in | **Light reveals value and attracts danger** | The first relic sits where the sightlines are longest. You see the thing you woke, and you have room to go dark and live |
| 3 | **Second** | Not everything that is wrong looks wrong | The most intact floor. It teaches no mechanic. It teaches distrust, and it is where the Window Walkers stand in the display windows, exactly where they worked |
| 4 | **Third** | The building can take away what you learned to rely on | Swollen floors, staged rooms, collapsed sightlines. Sound stops arriving from a direction you can use |
| 5 | **Basement** | Sometimes the choice is made for you | No ambient light at all. The keeper who learned to run dark finds the floor where running dark is not available — and the best Components are down here |
| 6 | **Fourth** | Light is not always yours to control | Roof gone, lit by the storm. Your lantern adds little and you are visible anyway. The solar farm's whole inversion, three zones early, in one room |
| 7 | **Basement**, again | Leaving is the loudest thing you do | Extraction is at the loading dock, so the last lesson is taught by the floor that taught lesson five |

### The three rules that make it teach

**One new idea per floor, and the floor is the exam.** If a floor introduces
two things, the player learns neither and blames the game. If a floor
introduces nothing, it is set dressing and belongs after M6.

**A trade-off has to be felt both ways, somewhere survivable.** A prompt can
teach a control; nothing but experience teaches *light reveals value and
attracts danger*. So the player has to turn the lantern on, get the relic, be
noticed, and get away — all on the ground floor, where that sequence is
survivable. The ground floor is the only place in the building where the wrong
answer is affordable, which is the entire reason it comes first.

**It has to teach four people talking at once.** `DECISIONS.md` row 36 is 1–4
private co-op, which defeats text, quiet moments and anything resembling a
cutscene. Every lesson above is carried by arrangement, light and sound,
because those are the only channels that survive a voice call. The corollary,
and it is uncomfortable: **the building must also teach the solo player**, who
has no veteran in the lobby to explain it. A lesson that only lands because
someone's friend already knew it has not been taught.

### The hole in rule two, found the same day

Rule two says a trade-off has to be felt both ways. Research into tutorial-free
teaching says something sharper that breaks it: the player has to see the rule
applied to **their own** action. Portal's developer commentary says it, and a
2026 study on self-modeling (N=60) measures it.

In four-player co-op the default outcome is that **the player who lights the
lantern and the player who meets the enemy are different people.** Neither one
learns the rule. One got a relic and no consequence; the other got a
consequence and no cause. Play it ten times and the crew still cannot tell you
what the lantern does, because nobody ever held both halves.

This was the sharpest single risk the research turned up against this page, and
it is a hole in my own proposal rather than in Chris's decision.

**The fix is already a locked rule, and it is being broken in the prototype.**
Enemies converge on the *brightest position*, not the nearest player. If that
holds, the lantern holder is the one who gets found, every time, and the lesson
is personal by construction — cause and consequence land on the same pair of
hands. The archived `EnemyService` retargets the Warden to the **nearest**
player instead, which until now looked like a fidelity bug. It is not. It is
the bug that would stop the building teaching, and the UE5 build must not
inherit it.

**The corollary nobody will like: solo teaches better than co-op.** Alone, the
player necessarily holds both halves, so the ground floor's lesson cannot
miss. That makes the solo path the one to design the teaching against, with
co-op inheriting it — the opposite of the usual order for a co-op game, and the
reason the brightest-position rule carries so much weight.

### What this costs the roadmap

`ROADMAP.md` M1.5 asks for a grey-box floor you "can get lost in." Row 43
raises that: the grey-box is now judged on whether it **teaches**, not on
whether it is navigable, and a floor that cannot be walked by someone who has
never seen it has failed its only job.

It also puts a clock on the whole thing. A tutorial that is also the first real
level has to land inside Steam's two-hour refund window — see **O7**. The
lessons above are deliberately front-loaded for that reason: ideas one and two,
which are the game, are both on the ground floor.

## Massively stocked without eight thousand unique models

The honest version of "feel like a real department store and not just Roblox"
is not more unique assets. It is **repetition with intent**, which is exactly
what a real store is: a hundred of one thing, priced, signed, and arranged by
somebody whose job that was.

Every department gets four kits and nothing else:

1. **Fixtures** — the department's own furniture. Round rack, four-way rack,
   glass case, gondola shelf, wall standard. Five meshes, every department.
2. **Merchandise** — one or two meshes *per department*, instanced in bulk and
   recoloured. A rack of coats is one coat mesh ninety times. This is where the
   density comes from and it is nearly free.
3. **Ruin** — the gradient above. Silt, tide line, fallen ceiling tile, swollen
   board, water.
4. **One landmark object** that exists nowhere else in the building, and which
   is how players name the place out loud. The escalator well. The fitting-room
   mirror corridor. The furniture gallery's lit bedroom. The auditorium stage.

**Signage carries the realism, not geometry.** Department names, price cards,
sale banners, a directory board by the escalator, floor numbers on the stair
doors. Text and decals are cheap, and they are the single biggest difference
between a shop and a Roblox room. They are also wayfinding, which the slice
needs anyway (`docs/VERTICAL-SLICE.md`, element 1).

**The test:** stand anywhere in the building with the lantern off and you should
be able to say which department you are in. If you cannot, the dressing is not
done.

## The store is the hall inverted

This is the answer to the thing Chris actually asked — *"there's just so many
things to loot, so what's the point of having them… there needs to be reasons
to get things and have things and do things and go out."*

**Lanternhold is an unfinished building. A department store is a building full
of the things that finish buildings.** Beds, lamps, tables, crockery, curtains,
rugs, a stove, the chairs for a meeting room. You are not carrying abstract
loot out of an abstract box. You are stripping one building to dress another,
and both are rendered.

So: **a restoration crate is stamped with the department it came from, and a
room at home is restored by crates from the department that would have supplied
it.** The dormitory wants bedding from the second floor. The kitchen wants the
third. The meeting room wants the auditorium's chairs, four floors up and one
way in — which is why the hardest room at home is the last one to open.

This adds **no fifth currency** — `docs/REWARDS.md` forbids that, and it still
holds. A crate stays a crate and still answers exactly one question (*what does
home look like?*). It gains one attribute: where it came from. What it buys is
a route decision, which is the one thing the middle game was missing: **you go
to a particular floor tonight because of a particular room at home.**

Chris's call, and it is one field on the crate spawn plus a cost line per room.

## What this does not change

- **Light reveals value and attracts danger.** Untouched, on every floor.
- **Storm Charge only rises.** Untouched.
- **The Galleria is still the zone and still Codex's build.** The concourse is
  not deleted; it is what the store's service doors earn you.
- **No new enemy.** Six stays six. Every floor above is populated out of the
  existing roster, by the job rule in `docs/ZONES.md` — which is why a
  department store was the right first building: four of the six already worked
  here.

## For the map lane

Geometry is **Chris's alone** now — `docs/LANES.md`, and `DECISIONS.md` row 41:
no agent opens a `.uasset`, because binary does not merge. The `COORDINATION.md`
tag-and-`src/map/` arrangement this section used to name governs only the frozen
Roblox archive (row 39). Everything on this page is a spec, not an instruction.
The two pieces that belong to systems rather than map: the crate's department
stamp, and the per-room cost at home (`HubService`, and the rooms that open —
`docs/VERTICAL-SLICE.md` element 5).
