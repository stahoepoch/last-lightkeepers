# Finding — Boswell's reads as a stockroom, and the art is the only thing saying so

Chris, 2026-11-04, after walking the dressed circuit: *"it looks like the
stockroom and not a dept. store."*

He is right, and the diagnosis is not a criticism of the dressing pass. The
pass placed the kit it had, carefully and measured. **The kit is a warehouse.**

---

## Why this was the inevitable outcome

The pack is `Warehouse_Big_Pack`. Every piece placed is industrial by
construction:

| Placed | What it is in a real building |
| --- | --- |
| `SM_Upright_Frame`, `SM_Load_Beam_3_5m`, decking | pallet racking — a warehouse aisle |
| rack signs | bay/aisle location labels, for pickers |
| `SM_Pallet_Truck`, wheel stoppers, parking poles | dock equipment |
| `SM_Crate_Stack_1`, cardboard box piles | stock in transit |
| warehouse lamp housings | high-bay industrial pendants |

Nothing in that list appears on a department store's sales floor. Pallet
racking is the single most legible "this is a warehouse" object there is — it
is 106 cm deep, two levels, open-frame, and designed to be seen by staff.
A sales floor hides all of it.

## The sharpest contributor: the relics are dressed as crates

The relics are the most-looked-at props in the game. You hunt them, lean in to
read them, pick them up and carry them to the dock. There are two in the map,
`BSW_WRONG_CRATE` and `BSW_DISPLAY_RECEIVER`, and the dressing pass gave both
`SM_Crate_Stack_1` — a stack of warehouse crates, chosen because the
alternative was the 100 cm grey cubes they had been.

That was a clear improvement and it is still wrong, because **the fiction
already knows what these things are.** From `data/relics.json`:

> Counter lanyard, laminated · Ledger, water-swollen · Ring, still boxed ·
> Pedometer, strap perished · Child's drawing, taped · Arcade tokens, six
> hundred · Security handset

Every one of those is a department store object. A boxed ring is currently a
pallet of crates. **The writing is a department store and the art is a
warehouse**, and the art is what Chris is looking at.

## What a sales floor actually needs

Roughly in order of how much each buys:

1. **Gondola shelving** — low, double-sided, 45–60 cm deep, with end caps.
   This is the one that does the most work: it replaces the racking as the
   aisle-defining object, and because it is *low*, it changes sightlines
   completely. A store you can see across is a different horror space from a
   warehouse you cannot.
2. **Glass display cases and a counter.** The dressing pass placed one toy
   counter and a dead display case against cubes; as real props they are what
   says "things were sold here."
3. **Small props at human scale** — the relics first. Seven authored objects,
   none of them a crate.
4. **Clothing rails, mannequins, signage.** `docs/ZONES.md` already has
   mannequins as an enemy family, so the rails have a second job.
5. **Ceiling and fixtures.** Troffers rather than high-bay pendants. The
   Warehouse pack's `Air_Conditioning` set has lit and unlit ceiling-panel
   variants, which is the nearest thing in hand.

## Two ways forward, and they are not exclusive

**Re-zone what we have.** The warehouse kit is not waste — Boswell's *has* a
stockroom, and a dock, and back-of-house corridors. Everything placed belongs
somewhere, just not on the sales floor. That reframes the pass as "the
back-of-house is dressed first," which is honest and costs nothing.

**Source a retail kit for the floor.** This is the one that answers Chris's
sentence. Gondola shelving plus a counter plus a display case is a small,
specific shopping list rather than another 1.8 GB pack, and
`ASSET-DEPENDENCIES.md` has the rule that came out of the last import: **take
meshes, materials and blueprints freely; take texture sets one at a time and
only when something in the map uses them.** 101 meshes cost 21 MB and 26
textures cost 302 MB.

## What this finding does not claim

- **It is not a reason to undo the dressing pass.** The measurements it made
  are sound, the 200 cm escape gap survived contact with real geometry, the
  route probes are byte-identical, and the dock is no longer a debug box.
- **It is not a lighting finding.** The store is lit at 2 candelas by the
  fixture-light contract, and at that level the *silhouettes* are what read.
  That is precisely why rack frames are so recognisable and why swapping them
  for gondolas would land hard.
- **It is not urgent before the relics.** The relic props are seven small
  objects and the most-seen art in the game. They are the cheapest change with
  the largest effect, and they do not need a new pack at all.
