# The valley — what the geography means for play

Status: **DECIDED, 2026-10-28** - `DECISIONS.md` row 57 locks the approach.
Chris answered this page's first open question the day it was written.

This is Claude's half of the terrain brief Codex proposed. He owns the
technical pipeline (source selection, heightmap prep, Landscape import,
streaming, performance budget); Chris owns the bounded area and everything in
`Content/`, and has now settled how literal we get.

**Read the timing section first.** Most of this is M7 work and saying so is the
most useful thing on the page. The one thing still open here is the
compression factor, and it waits on M4.

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

## Decided, 2026-10-28 - row 57

Chris's four rules, verbatim:

> Make it "Johnstown-shaped," not a literal reconstruction. Use real public
> elevation data for the valley, hills, river grades, and distant skyline.
> Keep the first playable exterior tiny: Lanternhold, the department-store
> approach, and a few visible-but-unreachable landmarks. Build the larger
> Valley later as unlocked districts, not one enormous open-world map.

That closes open 1 and open 3, confirms the OSM rule above, and matches the
timing table. Three notes on implementing it, because two of the four rules
are easy to read as more work than they are.

### "Real elevation data for the distant skyline" is not a Landscape

The skyline is the one piece the first slice genuinely needs - `THE-STORE.md`'s
fourth floor has the roof gone, so the player looks **out**. Use 3DEP to author
the silhouette and then **bake it**: the elevation data informs what the
horizon's shape is, and ships as a backdrop. It does not become a runtime
Landscape, and M6 must not wait on the terrain pipeline to get a shape onto
the horizon. Building the pipeline in order to do this would be the most
expensive way to do the cheapest thing on the list, which is the trap the
timing section above exists to flag.

### The store's exterior approach is not first-slice work

Row 3 is LOCKED and the first slice is one **sealed** department store,
interior. "The department-store approach" belongs to the first playable
exterior, which is Codex's step 3 after M4's gate - not M1. Worth stating
because the four rules read as one package and someone could reasonably take
the approach as part of the slice it approaches.

### The district unlock collided with row 50 - ANSWERED, row 58

**Unlocked districts gate content. Row 50 forbids gating co-op behind
progression.** Read strictly, row 50 is about party slots and districts are
fine. But its *argument* transfers exactly: if unlock state is per-player,
then a friend who has not opened the rail yard cannot join your rail yard
raid, and "sorry, you have not unlocked this level" is the same sentence row
50 banned, in a different costume. The refund maths is identical too - someone
who cannot play with the people they bought it with has two hours to ask.

This is not an argument against districts, which are right and are row 38
delivered. It is a question about where the unlock **lives**:

| Where unlock state lives | What happens to the friend who has not unlocked it |
| --- | --- |
| **Per-player** | Cannot join. Row 50's failure, renamed |
| **Per-host / per-lobby** | Joins anything the host has opened. Progression still means something to the host, and nobody is ever locked out of their own friends' game |
| Per-crew, shared | Works, but needs a persistent crew entity the game does not have and row 51 deliberately avoided |

**Chris chose per-host, 2026-10-28, and sharpened it past the table:**

> The host's Lanternhold decides which districts the party can raid. Friends
> can always join, even if they have not personally opened that district.
> Guests keep their own loot, relics, workshop progress, and personal
> restoration contributions - but they do not permanently inherit the host's
> district unlock just by visiting. They can experience the rail yard with a
> friend tonight, then later open it in their own Lanternhold.

The phrase that does the work is **what transfers is the night, not the
standing.** You have seen the rail yard and you still have to open it. That
keeps row 50 (nobody is told they cannot play with friends), keeps row 38 (the
danger ladder still has to be climbed), and keeps a guest's hub their own
story rather than a copy of the host's - which is the part a per-host rule
could easily have broken by granting the unlock on arrival.

Implementation rule: unlock state is host-scoped session data, never granted
to a joining client, and a guest's save records what they *earned* and not
where they were *allowed to go*. Row 58 has it.

**It opened one question underneath itself, now `DECISIONS.md` O8:** whose
ledger does an extracted restoration crate advance? `loot.json` has
`restorationCratesArePocketable: false`, so a crate is one physical object in
one person's hands, and nothing has ever decided who it pays. Crediting only
the carrier makes friends compete for the thing that advances their own hub;
crediting everyone makes a four-crew restore four hubs at a solo keeper's
rate. Not urgent - restoration is post-slice - but it must be settled before
any save-data shape is written.

## Open

1. **Mine, after M4:** the compression factor, argued from how far testers
   will actually cross.
2. **Tracked in `DECISIONS.md` O8, not here:** whose ledger an extracted
   restoration crate advances. It is a progression question rather than a
   terrain one, so it lives there.

**Closed by rows 57 and 58:** how literal the valley is (Johnstown-shaped, not
Johnstown), and whether Lanternhold's exterior needs the real ridge - the
"keep the first playable exterior tiny" rule answers that with its own hill -
and where district unlock lives.
