# Zones — the ladder in toward the city

Status: the **scope and the spine are LOCKED** — Chris, 2026-10-27,
`docs/DECISIONS.md` row 38: *"We will have all the levels, including the whole
mall. It starts off small, progressesion. Danger."* So the full ladder below is
the game, not a wishlist, and "danger is a function of how close to the city
you get" is the progression rather than a flavour note.

Individual zone details below remain **proposed** (2026-10-20) and are still
covered by **O2** — the terrain, the local enemies and the inversions are
reasoned, not ratified. Read `docs/PREMISE.md` first.

**Zone one is now one building, not the whole mall.** Chris, 2026-10-26: *"the
whole Galleria can be the eventual world, but the game begins with one sealed
department store and earns its expansion."* The Galleria below stays exactly as
written and exactly as built - it is what the store's loading dock gate opens
onto, at the end of the ladder in `docs/QUESTS.md`. Read `docs/THE-STORE.md`
for the building itself.

**Nothing past zone one gets built yet, and that is a build order, not a
scope.** Row 38 settles the scope: every level here is coming. The handoff
gates even the solar farm behind the playtest question - *do testers kill their
lanterns, bank their haul, and argue for one more room?* - and that gate still
holds, because a second map cannot fix a loop the first one did not make
frightening. Build in this order; plan for all of it.

## The spine

**Danger is a function of how close to the city you get.**

That is the whole progression, and it needs no explanation to be legible. You
are safe in Lanternhold because you are far away. Everything worth having is
closer in. The city is the end of the road, down in the valley where the water
went.

| | Place | Terrain | What it does to light |
| --- | --- | --- | --- |
| Hub | **Lanternhold** — the hall, out past the farms | Safe | Yours, and warm |
| 1a | **The store** — one sealed anchor inside it | Enclosed, vertical, four floors and water | Light is scarce. You choose to make it |
| 1b | **The Galleria** — hilltop mall near the airport, earned | Enclosed, flat, short sightlines | Light is scarce. You choose to make it |
| 2 | **The solar farm** — the next hilltop over | Open, flat, long sightlines | Darkness is scarce. You hunt for it |
| 3 | **The rail yard** — the line where the hills give out | Linear, long sightlines | Cover sits between you and where you are going |
| 4 | **The works** — the old mill on the valley floor | Vertical, tangled | Both, at different heights |
| - | **The hospital** — a wing, not a zone | Airless, no sightlines | Nothing visible far enough to plan around |
| 5 | **The city** — the Point, the bridge, the old hall | Dense, flooded, final | — |

Each zone should invert the previous one's core constraint. That is what keeps
a second map from being the first map with new wallpaper.

**And it is now a requirement, not a preference.** Row 38 locks expansion as
the way danger arrives: progression is not unlocking more square footage, it is
being asked to stand somewhere worse. Every zone added has to be more dangerous
than the one before it in a way the player can name - not a higher stat, a
different problem. A zone that fails that test should not ship, however
finished it looks.

## Every zone shows you the next one

From the Galleria roof: the ridge wind farms and the array on the next hill.
From the farm: the valley and the city haze below.
From the works: the Incline and the bridges.

The player always has the next place on the horizon before they can reach it.
Free foreshadowing, and it makes the world feel like one continuous descent
rather than a level select.

## Enemies come from the job the place did

The generative rule, and it is already true of the existing roster: Sweepers
were floor cleaners, Stockers stacked shelves, Glimmers were maintenance
drones, Window Walkers stood in display windows, the Ficus was mall
landscaping.

**Deepened 2026-10-28 by `DECISIONS.md` row 47, LOCKED.** This section had the
rule one level too shallow. It is not only that enemies inherit a job — **the
*place* is trying to finish the last job it was built to do**, and the enemies
are how it does that. Chris: the storm wakes broken infrastructure, and a
Lightkeeper is dangerous because they are *an unrecognized person inside an
active system* — carrying equipment away, entering restricted areas, turning
on power, disrupting an emergency protocol.

So every zone on the ladder now arrives with a second question attached, next
to the inversion test:

> *What was this building built to protect or keep running — and why does a
> Lightkeeper violate that job?*

And the honesty test for the answer, in his words: a hospital enemy *"should
not feel like a mall security guard in scrubs; it should feel like an
emergency-care system trying to 'help' in a way that becomes horrifying when
nobody is there to override it."* The full table of what each place thinks it
is doing is in `docs/THE-NIGHT.md`.

**The boundary that keeps this from becoming a bestiary:** no machine needs a
murder motive. They contain, remove, restrain, reroute, retrieve and lock
down. The flood, the storm, the dark and the inability to extract are what make
those verbs lethal.

**Staples** — everywhere, because every site employed them:

- **Sweepers** — cleaning crews existed at every one of these places.
- **Glimmers** — so did maintenance.
- **The Warden** — not really an enemy. It is Storm Charge with a body, so it
  belongs wherever the meter does.

**Locals** — derived from that site's work:

- **Galleria**: Stockers, Window Walkers, the Ficus. All retail - and all three
  of them already worked in a department store, which is why one was the right
  first building. The ladder introduces five of the six one rung at a time.
- **Solar farm**: whatever tends an array — sun-trackers, cable crawlers,
  something that washes panels.
- **The works**: heavy things. Loaders, pourers. The first machines you meet
  that were never customer-facing.
- **The city**: civic. Traffic, transit, enforcement.

The player learns to read a place by what hunts them in it. It also means new
enemies are *derived* rather than invented, which keeps them coherent.

Current approved roster is five: Sweepers, Stockers, Glimmers, Window Walkers,
the Warden. The Ficus is a sixth pending Chris's confirmation — concept art
exists for it under `assets/enemies/`.

## Zone 2 — the solar farm

Where they charge. Going there is the first time you are not scavenging around
the machines but walking into the thing that feeds them.

**The inversion.** The mall is a dark box and your lantern is the only light in
it. The farm is an open field of surfaces built to catch and hold light, lit by
the array and by the storm hitting all that conductive surface. The question
flips from *can I afford light* to *can I find anywhere actually dark.*

**The trap, and the fix.** Pillar three says light is useful on every run and
never free. If the farm is simply lit, the player never chooses to light up and
the signature decision quietly disappears — a good stealth level that is not
quite this game.

The fix is a rule already locked: **only your own lantern reveals relics.**
Their light does not. So the ambient glow exposes you without showing you
anything worth having. You begin the zone already half-visible, and relics
still cost amber on top of blue. That does not invert the decision, it
**compresses** it: same choice, less safety to start from, higher stakes.

**Storm Charge gets a rhythm.** In the mall it is a smooth climb. Here,
lightning hits the array and the meter jumps — sky flash and sound first, per
the telegraph rule, then a step change. Periods of breathing room punctuated by
"that just cost us ninety seconds." The locked rule does not bend; it still
only rises.

**Cover and safety stop being the same thing.** Panel rows are the obvious way
to move unseen, and a conductive array is the worst place to be during a
strike. The farm constantly asks you to choose between hidden and safe, which
the mall never does.

**Loot is power, not goods.** Cells, inverters, copper, transformers. Which
matters because the Signal Tower needs power and the Garden Atrium needs light:
zone one restores the hall's comfort, zone two restores its *capability*.

**Amber reads harder here, not softer**, against a field of electric blue. The
colour-blind readability rule gets stronger in the zone with more light.

## Zone 4 — the works

The descent into the valley, and the inversion of the farm: after enclosed-flat
and open-flat, **vertical and tangled**. Catwalks, furnace halls, pipe
galleries, things above and below you.

It tests a muscle neither earlier zone does, it is the reason the valley exists
at all, and it justifies a heavier enemy family — this is where heavy machines
were built and maintained.

## The rail yard — the zone whose reward is access

Chris, 2026-10-20: keep it. On reflection it is not an alternate to the works
at all; it does a different job, and it is the only place on the ladder whose
payout is **infrastructure rather than loot**.

The line runs along the river, where the hills give out into the valley — so it
sits naturally between the farm and the works, and it is how you get further in
once it is working. Clear it and power it and the line opens; that is the
handoff's Transit Lift landmark doing its job, with the Incline carrying you
*up* to the hilltops and the rail carrying you *down* the valley.

It also plants the train three zones before you escape on one.

**What it tests.** Brutally long sightlines straight down the tracks, with
cover that is mostly *between* you and where you want to be. After the farm's
open flat and before the works' tangle, the yard is linear — a place where the
only safe route is the one you can see all the way along, and therefore the one
that can see you.

**Hazards on fixed paths.** A yard still working is full of things that move
predictably and kill you anyway: a shunter running its loop, blind, on rails.
That is a different fear from an enemy that hunts, and pairing the two is
something no other zone on the ladder does. Predictability as a hazard rather
than a comfort.

**Locals** (per the job rule): shunters, couplers, signal drones.

## The hospital — the dark mirror

Chris, 2026-10-20: keep it. Not as a full zone — as a **short, dense map** or a
wing of the city, because it only has one idea and that idea is airless.

**What it tests.** No sightlines at all. Corridors, bays, stairwells, doors on
both sides. It is the exact opposite of the farm and the yard: nothing is
visible far enough away to plan around, so Stockers are useless here and
anything that swarms is terrifying.

**Loot.** The richest medicine in the game — the pharmacy in the Galleria is
the small version of this — and it should hold the highest relic density
anywhere, because of what it means.

**Why it means something.** The order's founding fact is that *their hall
became a hospital.* It held, and strangers lived through the night in it. This
one is a real hospital, built for exactly that, and it did not hold.

That is the most loaded room in the setting and it should be handled with the
same care as the Keeping Room — it is powerful if it is deliberate and ugly if
it is careless.

**Locals.** Care units. Things that were built to be gentle.

## Scope, plainly

That is now six locations plus the hub, and the handoff gates everything past
zone one. Writing them down is not approving them. The order they get built in,
and whether half of them ever do, depends entirely on whether the loop earns
it — and the loop has not answered its question yet.

## Zone 5 — the city

The Point, the bridge that held, and the old hall.

The campaign's end is not recovering a vault — the records left with the order
decades ago (see `docs/PREMISE.md`). It is reaching **the hall that sheltered
strangers through the first flood, and relighting it.**

Which is, finally, what a Last Lightkeeper is for.

**Raised 2026-10-28 by `DECISIONS.md` row 46.** Relighting the hall is now a
step rather than the summit. Chris: *"End goal, rebuild civilization. Find the
library downtown, recover information, museum, history. Reclaim the past."* So
the city holds two more places than this page listed - the **library** and the
**museum** - and the campaign's end is knowledge rather than a building. The
order has kept records since the first flood; the city kept its records in a
library. `docs/THE-NIGHT.md`.

Escape by train or by the Incline — both of which the player has been looking
at from three zones away.

## Open questions

- Is the Ficus a sixth enemy or a Stocker under a working name?
- Does the farm need its own locals in a first pass, or do staples plus the
  terrain inversion carry it?
- Is the city one zone or several? It is the only entry here that might not fit
  a target run. **O1 closed 2026-10-28:** the target is **10-12 minutes**
  (`data/storm-charge.json` `run.targetBandSeconds`), the early exit is six,
  and the hard ceiling is thirteen - only enough for a last call and holdout.
  The city is still the one entry on this ladder that might not fit a night.
