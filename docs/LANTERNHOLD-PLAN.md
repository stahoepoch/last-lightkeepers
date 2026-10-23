# Lanternhold — the real building

This is the Johnstown Masonic Temple, Valley Pike, read off Chris's own
floorplans and photographs rather than guessed at. **It supersedes the room
list in `src/map/LanternholdBuilder.luau`'s header**, which was built from a
programme described from memory and is wrong in most of its particulars.

Chris, handing these over: *"we are a very special temple, you'll never see
another one like it besides philly or england, we have a huge place and it's
ornate."*

> **This is my reading of four hand-drawn plans and seven photographs. Check
> it before anything gets built from it.** A misread plan is a week of
> building in the wrong place. Anywhere I was unsure is marked ❓.

---

## Reference material in this repo

`docs/reference/lanternhold/` holds Chris's own four floor plans and three
interior photographs. Look at them before building; this file is a reading of
them, not a replacement for them.

| File | What |
| --- | --- |
| `plan-bottom-floor.jpg` | arrival, banquet hall, kitchen |
| `plan-1st-floor.jpg` | ballroom, bar, secret vaults |
| `plan-2nd-floor.jpg` | lodge rooms, the Egyptian Room |
| `plan-3rd-floor.jpg` | museum, library, cigar room |
| `interior-lobby-counter.jpg` | terrazzo, stencil border band, beamed ceiling |
| `interior-stair-hall.jpg` | the Art Deco iron balustrade, terrazzo banding |
| `interior-stair-door-brass.jpg` | panelled door, brass backplate, stair treads |

## Checked against the record

Searched after reading the plans, and it confirms rather than corrects them:

- **J. C. Fulton and Son of Uniontown**, built 1927-1933, cornerstone 1931.
  The builder's header already had this and it is right.
- The Art Deco facade has **a tall central entrance bay with fluted pilasters
  that read like organ pipes** - which is the postcard exactly, and is the
  single most recognisable thing about the outside.
- The Tribune-Democrat describes the Egyptian Room as **"formerly the Blue
  Lodge Room"**, which is precisely how Chris's 2nd floor plan labels it.

That last point settles something the builder's header has carried as an open
conflict for weeks: it noted that Chris puts the lodge rooms on the second
floor while the Society of Architectural Historians puts the Egyptian room on
the third, and left it unresolved. It is a counting difference - Chris's four
plans are Bottom / 1st / 2nd / 3rd, so his 2nd floor is the third level up.
Same room, same floor. **Chris's plans are authoritative; he is in the
building.**

Sources: [SAH Archipedia](https://sah-archipedia.org/node/17247) ·
[Tribune-Democrat, National Register](https://www.tribdem.com/news/johnstown-masonic-temple-added-to-national-register-of-historic-places/article_98cb1201-68bd-4aec-8d0f-6ee7c8945e29.html) ·
[Tribune-Democrat, photo gallery](https://www.tribdem.com/cnhi_network/photo-gallery-for-future-generations-efforts-underway-to-get-johnstown-masonic-temple-onto-national-register/article_6eb4237c-56c8-11ef-811b-c3a66c2ba65a.html)

---

## The building itself

Art Deco / stripped classical, pale limestone, **flat roofed**. A plain block
with tall narrow window slots and a projecting entrance bay with vertical
fluting. Three floors over a raised basement, and the basement is at grade at
the back — the bottom floor has its own ramp entrance.

What the current build gets wrong: it is stone-and-brick with a pitched feel,
and the entrance reads as a hall rather than a deco frontispiece.

---

## Bottom floor — arrival

The floor people actually use, and the one the hub spawns on.

| Where | What |
| --- | --- |
| bottom right | **Entrance**, with a **ramp** |
| right edge, bottom | Room |
| right edge, middle | **two bathrooms**, stacked |
| right edge, upper | **Elevator** |
| top right | Room (drawn on an angle) |
| top centre | **Exit** — a small projecting vestibule |
| centre | **Stairs up** — one long run |
| left, upper two thirds | **Banquet Hall** |
| bottom left | **Kitchen** |

The circulation reads: in at the bottom right off the ramp, up the right-hand
edge past the rooms and bathrooms, stair core in the middle, and the banquet
hall opening off to the left with the kitchen behind it.

## 1st floor — the ballroom

| Where | What |
| --- | --- |
| whole left side | **Big Ballroom** (one corner drawn curved ❓ — apse? bay?) |
| top centre | Bathrooms |
| centre | **Bar** |
| top right | **Secret Vaults** — *"we have, you can walk in"* |
| right, upper | **Elevator** |
| right, middle | **Entrance from the steps outside** — the public front door |
| centre | **Stairs up / stairs down** |
| lower right | Tiny Room |
| bottom centre | Women's restroom |
| bottom right | Storage |

Note the front door is on **this** floor, not the basement. Two ways in to the
building, on two levels.

## 2nd floor — the lodge rooms

| Where | What |
| --- | --- |
| top left | **Balcony** (over the Blue Lodge room below ❓) |
| left | **Chapel Room** / **York Rite Room** |
| top centre | Preparing Room, Bathroom |
| top right | **OES / AMD Room** |
| centre | **Hallway**, running the depth of the floor |
| mid left | cross **hallway** |
| right, top | **Elevator** |
| right | Room, then **two stair runs** |
| right, lower | Room, Room |
| bottom left, large | **Blue Lodge Room — EGYPT**, with a balcony down its right side |
| bottom centre | Prepare Room |
| bottom right | **Gathering Area** |

## 3rd floor — the quiet floor

| Where | What |
| --- | --- |
| top left | Tiny Room |
| middle left | **Museum** |
| bottom left | **Library** (narrow) |
| bottom right, large | **Cigar Room**, with a **pool table** |
| top right | **Elevator** |
| right | **Stairs** |

---

## Materials, from the photographs

This is where "ornate" lives, and where the current build is furthest off. It
is a greybox of plain coloured boxes; the real thing is none of those words.

- **Floors: terrazzo**, cream-tan and polished, with **dark inlay bands** set
  in as borders and stripes. In every circulation photo. The builder already
  has `terrazzo` and `inlay` colours and barely uses the banding.
- **Walls: cream plaster**, plain, above a dark wood base.
- **A painted stencil border band** where wall meets ceiling — a green and
  gold geometric run. Present in the lobby photo. `C.stencil` exists for this.
- **Beamed and coffered ceilings**, plain plaster between the beams.
- **Dark stained wood**: doors, handrails, counters, the bar. Heavy panelled
  doors with **rectangular brass backplates and round brass knobs**.
- **Art Deco wrought iron balustrades** — a repeating circle-and-chevron
  motif, dark, with a dark wood handrail over. This is the signature of the
  stair hall and the current build has nothing like it.
- Stair treads are dark stone with **yellow nosing strips** (modern safety
  addition, but it is what the building looks like today).

### The Egyptian Room

The showpiece, and the reason the building is worth modelling at all.

- **Gold / ochre columns with lotus capitals**, painted with Egyptian figures
  in panels down the shaft.
- **Coffered ceiling on broad flat yellow-gold beams.**
- **Deep blue-purple patterned carpet** across the whole floor.
- **Teal-blue upholstered seating**, tiered, around the perimeter.
- In the middle: a **gold altar** with **four tall gold lamp standards**,
  each topped with a white flame-shaped globe.
- Painted panels and hieroglyph friezes on the walls between the columns.
- A **black-and-white checkerboard floor** is visible through a doorway.

---

## What this means for the build

1. **The four-level model is roughly right but the rooms are wrong.** The
   current build has basement / ballroom / lodge / museum, which matches, but
   almost every room within them is invented.
2. **The stair core is wrong.** The current build stacks one identical core on
   every floor, which is what makes it read as a warren. The real building has
   a stair in the middle on the lower floors and **two runs on the right** on
   the 2nd, with the elevator always to the right of them.
3. **Two entrances, on two levels** — the basement ramp and the front steps
   into the 1st floor. The hub currently has one.
4. **The Egyptian Room is the Blue Lodge Room on the 2nd floor**, bottom left,
   with a balcony. It is not a side room; it is the heart of the building.
5. **Relics belong on the altar in the Egyptian Room**, not on plinths in the
   lobby. That also answers Chris's "relics shouldn't be in the middle of the
   floor".
6. **There is a bar, a cigar room with a pool table, a library and a museum** —
   four natural homes for the progression sinks that currently have nowhere to
   live, and for the shop that does not exist yet.
7. **Secret vaults you can walk into**, on the 1st floor. That is a gift for a
   game about recovering relics and it should not be wasted on a cupboard.

## Open questions for Chris

- ❓ The curved line in the Ballroom's corner — a bay, an apse, a stage?
- ❓ Is the 2nd floor "Balcony" top-left overlooking the Ballroom below, or
  overlooking the Blue Lodge Room?
- ❓ Rough dimensions. The plans give layout but not size; is the footprint
  nearer 100 x 150 feet, or larger?
- ❓ Which floors should be open to players at the start, if the building
  unlocks as the town is restored?

---

## Measured, 2026-10-23: the rooms are mostly not rooms

Chris, on the upper floors: *"the upper rooms have way to many rooms, walls are
weird and there isn't the feeling this is like real, or walkable, it has so
many spots, and i know that is what the pictures showed you, but the flow is
pretty rough."*

Measuring it in a running session says the problem is not the number of rooms
and not the doors. Every room on every floor has at least two ways out — there
are no dead ends anywhere. What there is instead:

**20 of the 32 named rooms on the upper three floors have more than half their
perimeter standing open.**

| Level | Named rooms | More than half open |
| --- | --- | --- |
| 1st | 10 | 7 |
| 2nd | 14 | 9 |
| 3rd | 8 | 4 |

The worst of them are the ones with the names that should mean the most:

| Room | Perimeter open |
| --- | --- |
| Hallway2, StairHall2 | 93% |
| Bar | 91% |
| PrepareRoom | 88% |
| UpperHall3 | 86% |
| Bathroom2 | 81% |
| YorkRiteRoom | 74% |
| CigarRoom | 74% |
| ChapelRoom | 72% |

The 2nd floor has fourteen named rooms and five walls. So most of what the
builder calls a room is a rectangle of different carpet with a brass plaque
over it, standing in open floor. You walk past a plaque reading THE CHAPEL and
there is no chapel there — the label changes and nothing else does. That is
what "walls are weird" and "doesn't feel real" are describing, and it is also
why there are "so many spots": the floor is subdivided by flooring rather than
by architecture, so it reads as many places while walking like one.

### Two ways out of it, and they need Chris

1. **Fewer rooms.** Merge the labels down to the spaces that are actually
   enclosed. The 2nd floor becomes about six rooms instead of fourteen. Most
   faithful to "way too many rooms", least faithful to the plans.
2. **Real walls.** Wall the named rooms so they become the rooms the plans
   show. Faithful to the drawings, but it adds partitions to a floor he has
   just said has too many.

The likely answer is a mix: give real walls to the rooms that carry the
building — Egyptian, Chapel, York Rite, Museum, Cigar Room, Ballroom — and
fold the leftovers (TinyRoom1/3, PrepareRoom, PreparingRoom, EastRoom2/3, the
loose bathrooms) into the halls they already open onto. Which of those matter
depends on what each room is **for**, and the unanswered question from this
file still blocks it: there is no shop, and `data/loot.json` has four sinks
with nowhere to spend. A room with a job gets walls; a room without one gets
merged.
