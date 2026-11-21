# The store is the right size. The keeper is an adult walking 1.7x too fast

Written 2026-11-15, after Chris walked the children's floor for the first
time: *"the character is far too tall and too fast, maybe not too fast, but
maybe the dept store is too small. dimensions don't match my speed, like if I
were to go into Boscov's today, and video tape myself from place to place, I
know it would take longer to walk around than it does now."*

Three claims in that, and they do not all survive measurement. **Values in
`data/*.json` are Chris's; this proposes and does not change them.**

> **ADOPTED 2026-11-21, and this document is now history rather than a
> proposal.** `data/lantern.json` authors walk **5 studs = 140 cm/s** and
> crouch **3 = 84**, with sprint held at 12.5 = 350 so the worst-state escape
> margin survives. The running game logs `walk 140, sprint 350, crouch 84`.
> The height half was adopted on 2026-11-15 (145 cm keeper, every dimension
> derived from that one number in `LLKLanternFeel.cpp`).
>
> **Two things this document got wrong, kept here rather than quietly
> edited.** It proposed 5 studs and left crouch's 0.5625 ratio alone, which
> would have given crouch 79; the adopted 3 studs gives 84, so the ratio moved
> to 0.6 on purpose. And it treated sprint/walk as a number to preserve - the
> adopted pass moved it from 1.5625 to **2.5**, which is a bigger change in
> feel than anything proposed here and is still unjudged in a played route.
> **The tables below use the old 224 cm/s walk throughout.** Read them as the
> argument for slowing down, not as a description of the game.

---

## 1. The store is not too small, and this is worth settling first

Measured from the four slabs that carry the upper floor:

| slab | size | area |
|---|---|---|
| `ANCHOR_UPPER_SOUTH` | 87.0 × 18.0 m | 1,566 m² |
| `ANCHOR_UPPER_WEST` | 37.5 × 18.0 m | 675 m² |
| `ANCHOR_UPPER_NORTH` | 87.0 × 18.0 m | 1,566 m² |
| `ANCHOR_UPPER_EAST` | 37.5 × 18.0 m | 675 m² |
| **one floor** | **87 × 54 m footprint** | **4,482 m² = 48,244 sq ft** |

A real Boscov's runs 150,000–200,000 sq ft over two or three floors, so
48,000 per floor is squarely in the range. **The building is the right size.**
If it feels small, that is the pace, and pace is one number.

## 2. The walk is 1.72x an ordinary indoor walk

`data/lantern.json`: walk 8 studs = **224 cm/s = 2.24 m/s**.

An ordinary indoor walk is about 1.3 m/s. Someone actually looking at
merchandise moves at about 1.0. So the keeper strolls at nearly twice the
speed of a person in a shop, and the floor shrinks to match:

| route | today | browsing 1.0 | ordinary 1.3 | brisk 1.5 |
|---|---|---|---|---|
| across the children's department (28 m) | 12 s | 28 s | 22 s | 19 s |
| full floor, north–south (54 m) | 24 s | 54 s | 42 s | 36 s |
| full floor, east–west (87 m) | 39 s | 87 s | 67 s | 58 s |
| a lap of the upper floor (~250 m) | 1 m 52 | 4 m 10 | 3 m 12 | 2 m 47 |

That is the "I know it would take longer" feeling, and it is arithmetic
rather than taste. The `_retuneNote` already records this exact failure once:
at the old 4.48 m/s *"crossing the 28 m store took 6.2 seconds, which is why
the room read as too small. It was not too small; you crossed it like a
car."* Halving it was right and did not go far enough.

### The cheap part: walk can move on its own

**No enemy value has to change.** The escape invariant is about **sprint**,
not walk — `tests/movement.test.mjs` asserts that a clean sprint outruns every
enemy, and the only walk assertion is an upper bound (*"a stumbled walk does
NOT outrun a Sweeper, so walking it off is no option"*), which gets safer as
walk drops.

That is why this is not another 2026-11-04. That retune had to move all six
enemies because it changed sprint. This does not.

| walk | m/s | vs ordinary walk | 28 m dept | sprint/walk |
|---|---|---|---|---|
| 8 (today) | 2.24 | 1.72× | 12 s | 1.562 |
| 6 | 1.68 | 1.29× | 17 s | 2.083 |
| 5.5 | 1.54 | 1.18× | 18 s | 2.273 |
| **5** | **1.40** | **1.08×** | **20 s** | **2.500** |
| 4.5 | 1.26 | 0.97× | 22 s | 2.778 |

**Recommendation: walk 5 studs**, and drop crouch to 3 (0.84 m/s) so the
creep stays clearly under the walk. Sprint untouched at 12.5.

One thing that changes and should be chosen on purpose: sprint/walk goes
1.56 → 2.50. The gear change becomes much wider — you stroll, and running is
an event. Chris asked for more separation once already (*"running could be a
little faster"*), so this is probably the direction he wants, but it is a
feel decision and `tests/movement.test.mjs` asserts the old ratio and will
fail until the expectation is moved with a dated note, as it was last time.

## 3. He is right about the height, and fixing it breaks the cover kit

`LLKLanternFeel.cpp:7` sets `KeeperEyeHeightCm = 152`, and nothing calls
`SetCapsuleSize`, so the capsule is Unreal's default 88 cm half-height —
**a 176 cm adult**. `DECISIONS.md` row 89 says *"one player, one lantern, and
the boy is alone."* The boy is currently six feet tall.

**But shrinking him collapses the whole cover vocabulary**, and that is not
obvious until it is computed. Sight is traced between capsule **centres**, so
the keeper's traced point drops with him:

| boy | stand | crouch | eye |
|---|---|---|---|
| 176 cm (today) | 88 | 40 | 152 |
| 155 cm | 78 | 35 | 134 |
| 145 cm | 72 | 33 | 125 |
| 135 cm | 68 | 31 | 117 |

At **any** of those heights, every fixture on the floor becomes the same
thing:

| fixture | today | boy of 145 cm |
|---|---|---|
| 21 rolling racks (block only the 88–130 band) | cover standing | **not cover at all** |
| `COVER_NORTH/SOUTH` blockouts, 78 | duck behind | stand, hide, watch over |
| `SM_BSW_FoldedGoodsTable`, 80 | duck behind | stand, hide, watch over |
| `SM_BSW_ServiceCounter`, 102 | stand, hide, watch over | stand, hide, watch over |
| `SM_BSW_WallMerchBay`, 190 | stand, hide, blind | stand, hide, blind |

Two consequences:

1. **Ducking stops meaning anything.** The three-way vocabulary this floor
   was just built around — duck, stand-and-watch, hidden-and-blind — becomes
   two, and one of those is the bay alone.
2. **The 21 racks stop being cover.** Their blocking band was measured at
   88–130 cm; a boy's standing centre is below it. They would go from
   unreliable cover to no cover, which is a bigger change to the floor than
   anything placed today.

### Which is realistic, and that is the trap

A real eleven-year-old genuinely is hidden standing behind a real sales
counter. The simulation would be correct. It would also delete crouching as
a mechanic, on a floor that row 92 asks to be about hiding.

**This is `RULINGS-OPEN.md` RF5, which has been open since the kids-floor
findings**: whether to raise the traced point to eye height or author
fixtures that block from the floor up. Walking the floor has turned it from a
ballot item into the thing standing between here and a play pass. Tracing at
**eye** rather than centre would make a child's cover read the way a person
expects — you are hidden when your head is behind the thing — and would make
fixture heights mean what they look like they mean.

## 3b. The furniture is right. Measured, after Chris stood next to it

*"If you put yourself at the height of the counters and the clothes racks,
the person towers over it."*

Every fixture in the kit, against retail spec-book ranges:

| fixture | built | real | verdict |
|---|---|---|---|
| display / folded-goods table | 80 | 76–91 | in range |
| service counter | 102 | 91–107 | in range |
| sales desk *(body)* | 94 | 91–107 | in range |
| kids rolling rack | 145 | 152–183 | 7 cm under, and it is a *kids* rack |
| round rack | 156 | 122–152 | 4 cm over |
| clothing run | 164 | 152–183 | in range |
| four-way | 166 | 137–152 | 14 cm over |
| wall merchandise bay | 190 | 183–213 | in range |

**A correction to my own first pass**: I read the sales desk as 209 cm and
called it wrong by 102. It is not. The desk body tops out at **94 cm** and the
209 is a signage mast standing on two 3 cm posts above it — I had measured
the mesh's full bounds instead of the thing a person stands at. That is the
same mistake as this morning's rack union box, which reported a two-post
garment rack as a solid 146 × 72 × 145 block. Bounds are not geometry, and I
have now made that error twice in one day on the same floor.

So nothing in the kit needs resizing. **The towering is the person.**

### What the department looks like from each height

| fixture | cm | adult 176 (today) | boy 155 | boy 145 | boy 135 |
|---|---|---|---|---|---|
| display table | 80 | well below eye | well below eye | well below eye | well below eye |
| service counter | 102 | well below eye | well below eye | well below eye | just below eye |
| kids rolling rack | 145 | **just below eye** | about eye level | above your head | above your head |
| round rack | 156 | **about eye level** | above your head | above your head | above your head |
| clothing run | 164 | **about eye level** | above your head | above your head | above your head |
| four-way | 166 | **about eye level** | above your head | above your head | above your head |
| wall merchandise bay | 190 | above your head | above your head | above your head | above your head |

Read the adult column. **Every garment rack in the department sits at or just
below the keeper's eye**, so you stand in the middle of the children's floor
and look straight out across the tops of all of them. Nothing is a wall.
That is the towering, and it is not because the racks are short — they are
spec-correct — it is because an adult's eye is exactly at rack-rail height.

Read the boy column and the same floor becomes a different room: the racks go
over your head and the department turns into corridors of fabric you cannot
see across. Counters stay at chin height and tables at your waist, which is
what they are in life — you are *meant* to tower over a display table.

That is the store the fiction already describes, and the fixtures are already
built for it. One number is wrong, and it is the keeper's.

### Which makes RF5 the blocker, not a ballot item

The visual case for shrinking him is now unambiguous. The mechanical case is
the problem, and it is entirely the trace rule: sight is traced between
**capsule centres**, so a shorter boy has a lower traced point and §3's table
applies — every fixture collapses into "stand, hide, watch over" and the
racks stop being cover at all.

So the two halves disagree, and only because of where the trace is taken:

- **To the eye**, a boy among 156–166 cm racks is hidden by them. Correct and
  obvious.
- **To the trace**, a boy's centre at 72 cm is below the racks' 88–130 cm
  blocking band, so they hide nothing.

Tracing at eye height rather than capsule centre resolves both at once: the
racks become cover because they *look* like cover, and the counter stops
hiding a standing boy who is plainly visible above it. That is RF5, and it
should be settled before the height is set rather than after.

## What I would do, in order

1. **Walk 8 → 5, crouch 4.5 → 3.** One file, no enemy touched, and it is the
   complaint he actually felt. Cheap and reversible.
2. **Then walk it again**, before touching height. Pace and scale are easy to
   confuse for each other, which is exactly what happened here — and a floor
   that takes 20 seconds to cross instead of 12 may read as the right size
   with no other change.
3. **Then settle RF5**, because the height fix depends on it. Shrinking the
   boy without moving the traced point re-sorts every fixture on the floor
   and silently retires the racks.
4. **Then set the height**, and re-run the cover probes — `place_fixtures.py`
   already refuses to save a map whose fixtures do not block at the heights
   they claim, so the fixture kit will tell you itself what it has become.

Nothing here is done. Every number above is Chris's to set.
