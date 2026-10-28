# The valley — what the geography means for play

Status: **proposed**, 2026-10-28. Claude's half of the terrain brief Codex
proposed; he owns the technical pipeline (source selection, heightmap prep,
Landscape import, streaming, performance budget) and Chris owns how literal we
get, the bounded area, and everything in `Content/`.

**Read the timing section first.** Most of this is M7 work and saying so is the
most useful thing on the page.

## The approach is right, and one line of it is righter than it looks

USGS 3DEP as the source is correct and the licensing is clean — public domain,
no restrictions, no attribution burden. Studying real maps and photos to
understand how hills, roads, industry and neighbourhoods sit against each other
is research, not reproduction. Fictionalising names, roads, distances and
layout is already required by `DECISIONS.md` row 18 (*do not name Johnstown, or
any real fraternal order*), so the terrain plan and the setting rule agree
without anyone having to reconcile them.

**Sharpen one thing: do not use OpenStreetMap at all for anything that ships.**
Codex's "only after handling attribution and share-alike correctly" is the
right instinct and still too generous. ODbL's share-alike attaches to a
*derived database* — and a heightmap or a road network extracted from OSM and
baked into a shipped game is exactly the argument nobody wants to have with a
lawyer about a product they are selling. 3DEP gives us the terrain with no
strings. Looking at OSM on screen while authoring creates no derived database
and is fine. Importing it does. The distinction is worth keeping absolute,
because it costs us nothing: **we already have a public-domain source for the
only data we actually need.**

## The valley is already load-bearing, which is the real argument for this

This is not set dressing dressed up as rigour. The fiction's whole progression
is **topographic**, and it was written that way before anyone mentioned
terrain:

| Place | Where it sits | `ZONES.md` |
| --- | --- | --- |
| **Lanternhold** | Out past the farms, high and away | Safe because it is far |
| **The store / Galleria** | A hilltop near the airport | Zone one |
| **The solar farm** | The next hilltop over | Zone two |
| **The rail yard** | Where the hills give out | Zone three |
| **The works** | The valley floor | Zone four |
| **The city** | The Point, the bridge — "down in the valley where the water went" | The end |

**Height is safety. Depth is danger.** Row 38 says expansion must arrive as
more danger rather than more floor, and real terrain delivers that rule with no
words at all: the player descends for the whole game. You do not have to teach
a gradient. People already know that downhill is where the water goes.

Two existing design beats become nearly free if the terrain is real:

- **"Every zone shows you the next one."** From the Galleria roof, the ridge and
  the array. From the farm, the valley and the city haze. From the works, the
  Incline and the bridges. That is a sightline requirement, and sightlines
  across a real valley either work or they do not — you cannot fake it with a
  backdrop once the player can stand in two of those places.
- **The Incline carries you up and the rail carries you down.** Both are
  vertical-transport fiction that only reads if there is a real gradient for
  them to climb.

## Timing: almost none of this belongs in the first slice

**Row 3 is LOCKED: do not expand scope before the v1 loop passes its playtest
question.** The first slice is one sealed department store, interior, and it
does not touch terrain. Chris held M1 to finish M0 for exactly this kind of
reason — infrastructure before gameplay, and gameplay before world.

So the honest schedule:

| | What | When |
| --- | --- | --- |
| **A backdrop, not a landscape** | What you see *out* of the store | M6, with the art pass |
| A small exterior around Lanternhold | Codex's step 3 | After M4's gate |
| Real Landscape under a playable valley | Codex's step 1, properly | **M7+**, one district at a time |

**The one piece the first slice genuinely needs is small, and it is already
written.** `THE-STORE.md`'s fourth floor has the roof gone and is lit by the
storm — curriculum lesson six, *light is not always yours to control*. That
means the player looks **out**, and what they see has to say "this building
stands above a valley that is full of water and machines." That is a skybox, a
distant silhouette and a weather layer. **It is not a Landscape, it needs no
heightmap, and it should not wait for the terrain pipeline.** Getting the
pipeline built in order to put a shape on the horizon would be the most
expensive way to do the cheapest thing on the list.

Codex's instinct — *do not make the whole real region playable immediately,
that is how we get a huge pretty map with nothing in it* — is right and is
`ROADMAP.md` M6's warning in different words. The thing to protect against is
subtler: a terrain pipeline that exists will *invite* an open exterior, and an
open exterior is a different game. Every zone on the ladder is enclosed or
bounded on purpose, and the storm is what bounds them.

## The rule that will be discovered painfully if nobody writes it down

**Real terrain, fictional distances. Compress hard and compress deliberately.**

Real valleys are the wrong size for games. If the terrain is imported at true
scale, the mall and downtown sit miles apart, and a few real miles is either a
tedious walk or a loading screen — and `DECISIONS.md` O1 just settled a raid at
**10-12 minutes**, which is the whole night. A night cannot contain a true-scale
commute.

So the heightmap supplies **shape**: ridgelines, the river grade, which way
water runs, what you can see from where. It does not supply **distance**. Set a
compression factor once, write it into Codex's brief beside the scale figure,
and never let it drift per zone — inconsistent compression is how a world stops
feeling like one place, which is the only thing this exercise is for.

The test for any compression: **can a crew still see the next zone from the one
they are in?** That beat survives compression. True scale is not required for
it, and may actively prevent it — real haze over real distance hides exactly the
silhouette we want on the horizon.

## What I would hand Codex, and what I would not

**Would:** the topographic spine above as the requirement — six places, in that
descending order, each visible from the one before. That is enough to bound a
playable area and pick a region without anyone deciding how literal to be yet,
which is Chris's call.

**Would not:** a bounded area or a scale figure. Those depend on the
compression decision, and the compression decision depends on playtest data we
do not have — how far a crew will actually walk before it stops being tense.
M4 answers that. Picking a scale now means picking it twice.

## Open

1. **Chris's:** how literal — "Johnstown-shaped" or a looser valley that only
   rhymes with it? Row 18 forbids the name either way; it does not settle the
   shape.
2. **Mine, after M4:** the compression factor, argued from how far testers will
   actually cross.
3. Does Lanternhold's exterior need the real ridge behind it, or is its own
   hill enough? Cheaper if the latter, and nothing in the fiction demands more.
