# Boswell's is the south-west end and the crossing is 396 m

**Status: OPEN, and partly inferred rather than measured.** Written 2026-11-10
to close the research task Codex named as blocking the next map pass —
*"identifying which end of the actual Galleria is Boswell's and which is
Sayers, then anchoring the chapter geometry to that"* — and to answer
`THE-MALL-IS-STILL-TRADING.md` §12 item 7, open since 2026-11-06.

Real names identify the real building. Row 19 does not move: the player sees
Boswell's and Sayers.

## Method, and its one weak joint

Queried the Overpass API for OSM way **617829322**, the same way §8 measured,
and for named features within 900 m of its centroid. Projected into local
metres about that centroid with §8's own constants — latitude metres =
degrees × 110540, longitude metres = degrees × 111320 × cos(latitude) — so
every number here is directly comparable to §8's.

The ring came back as **42 vertices, 355 m east-west by 359 m north-south**,
which reproduces §8's *"about 355 m by 359 m"* exactly. That is the control:
the projection and the source agree with the earlier read before anything new
is asked of them.

**The weak joint, stated up front: Sears and Bon-Ton are not in OSM.** Only
two anchors are mapped as features. So the north-east assignment below is
**inferred** from TLP-1's adjacency as recorded in §8, not read off a source.
The inference is sound and it is still an inference.

## What came back

| Feature | Local metres (east, north) | Source |
| --- | --- | --- |
| Ring centroid | (0, 0) = 40.295044, −78.833524 | way 617829322 |
| **Boscov's** | **(−91, −156)** | node 8914627982, `shop=department_store` |
| **JCPenney** | **(−87, −25)** | node 8914627983, same tagging |
| Ring's southernmost vertex | (−91, −223) | way geometry |
| Ring's north-east extreme | (176, 136) | way geometry |

Two things fall straight out:

- **Boscov's sits at the south-west end**, with the ring's southernmost
  vertex 67 m directly below its node — same easting to the metre.
- **JCPenney is 131 m north of Boscov's**, bearing 2°, which is the long wing
  §8 read off the lease plan.

## The inference, and why it holds

§8's reading of TLP-1 gives the order along the public walk:

> *"One end holds two anchors across a short concourse. Sears on one side,
> Bon-Ton on the other."* … *"The walk jogs into an elbow court."* … *"A
> straight wing follows… JCPenney is the anchor on the opposite side of the
> wing."* … *"Boscov's is the anchor past that court."*

So Boscov's and the Sears/Bon-Ton pair are at **opposite** ends, with
JCPenney on the wing between them. Boscov's is measured at the south-west
end. Therefore:

> **Boswell's is the south-west end. Sayers is the north-east end.** The
> crossing runs south-west to north-east, with Penhurst on the wing partway
> along it.

§10 independently describes the same walk from the 2026 directory model —
*"the upper walk runs from Boscov's face, along the wing, around the elbow,
and out to the far end anchor"* — and notes that the far end box carries no
lease string, which is consistent with a closed, separately-owned Sears that
a leasing model would stop drawing.

## The number chapter one needs

| | |
| --- | --- |
| Boscov's node to the ring's north-east extreme | **396 m**, straight line |
| On foot | **longer**, because the plan doglegs. Not derivable from an exterior ring |

**I could not get a walked length out of this and am not going to estimate
one.** A convex-hull pass over the 42 vertices leaves 32 of them concave, so
the polygon is a detailed outline rather than a clean dogleg with one notch,
and every "via the crook" path I could construct from it came out within 4%
of the chord — which is a tell that the method is measuring the hull and not
the walk. The walk length needs TLP-1 scaled in the editor at its own 100 ft
bar, which §8 step 2 already asks for.

396 m is still the useful figure, because it is the chapter's scale.

### What 396 m means against what is built

| | Long dimension | Floor area |
| --- | --- | --- |
| The furnished Boswell's room | 28 m | 504 m² |
| `LLK_Boswells_Anchor` plate | 87 m | 4,698 m² |
| **The crossing** | **396 m** | see below |

The reassuring arithmetic: a 396 m concourse at a plausible 12 m width is
**about 4,750 m²** — within 2% of the Anchor plate's 4,698 m². So the middle
of chapter one is **roughly one Boswell's plate of floor, shaped as a corridor
instead of a room.** That is a very different authoring job from a second
anchor, and a much smaller one than "five times the mall."

The concourse width is the number that decides it, and §8 already flags it as
*"the first number the shell owes"* because the sheet never prints it. 12 m is
a stand-in here for an order-of-magnitude check and **must not be authored.**

## What is still unknown

- **The concourse width.** Everything above treats it as unknown. Scale TLP-1.
- **Which of the pair is Sayers.** That end holds two anchors across a short
  concourse — Sears and Bon-Ton. Chris's crossing names Sayers and its auto
  centre, and §8's secondary writeup lists *"Sears 125,800 plus a 6,000 sq ft
  auto center"*, so Sayers is the Sears of the pair. What the *other* box at
  that end becomes is unassigned; row 19 has Penhurst and Bellevue spare, and
  §10 notes the 2026 model stopped drawing one of them.
- **The auto centre's position.** Not in OSM. A building query within 260 m of
  the north-east extreme returned only the mall ring itself and three
  unrelated parcels 250 m further north, one of them a Tractor Supply. So the
  auto centre is either inside the traced ring or unmapped, and placing it is
  a question for the scaled sheet and the aerial, not for this file.
- **Whether the anchors are separate polygons.** They are not in OSM; the ring
  is one building. §12 records that Boscov's, the former Bon-Ton and the
  former Sears are independently *owned*, which is a title fact and not a
  geometry one.

## What this unblocks, and what it does not

**Unblocks:** the chapter can be anchored. Boswell's at the south-west end is
where `LLK_Boswells_Anchor` already is in fiction, so nothing built has to
move; the crossing runs away from it to the north-east; Penhurst goes on the
wing; and the two remaining sealed volumes are the far-end pair and the fifth
pad on the grass in the crook, which §8 already places.

**Does not:** this is not a licence for a large map pass. The pass still waits
on the concourse width, and that waits on the 100 ft bar in the editor.

## Turned over

**Operand.** This reading, against §12 item 7 staying open, and against
anchoring chapter one's geometry to a guess about which end is which.

**Floor.** The ring is a floor: 42 vertices, reproducing §8's 355 × 359 to the
metre. The two anchor nodes are a floor. The north-east assignment is **not**
a floor — it is an inference from a lease plan read off a raster, and it is
the thing to check first.

**Clock.** OSM as of this query, 2026-11-10.

**Falsifier.** A scaled TLP-1 whose Boscov's box is not at the south-west end
kills this outright. A 1990s directory, a mall map, or a photograph of the
Sears entrance with a compass-fixable feature in it would settle the
north-east assignment properly and should replace the inference. Any
centimetre derived from this file falls immediately: these are metres from a
traced ring, and §8's rule stands that the 100 ft bar wins where a wall
disagrees.

**Upside down.** The crossing is 396 m of building and about one Boswell's
plate of floor. The thing that makes chapter one big is not how much there is
of it — it is that it is all one corridor, and you have to go the whole way
down it with a lamp you are afraid to turn on.
