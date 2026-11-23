# Where LADIES, MENS and ELECTRONICS go on Boswell's ground floor

**Proposal, 2026-11-23.** Chris: *"Propose, follow the internet and what those
layouts suggest, they show floorplans and where the areas sit plus where people
walk around to and from."*

`b3dc935` laid the carpet where the stock stands. That exposed the real gap: the
map does not record where LADIES, MENS or ELECTRONICS are. Their stock is
generic `BSW_APPAREL_LOWER_*` — 104 actors spread over the whole floor — so each
department rests on a single sign, and the LADIES sign stands inside the JUNIORS
racking. This proposes the layout, from the building we already have plus how
real stores are arranged.

---

## 1. What the building already decided

Read out of `LLK_Boswells_OpeningPass` rather than assumed:

| Thing | Where | Source |
|---|---|---|
| Sales floor | x −4350..4350, y −2700..2090 (87 × 48 m, 4,167 m²) | `ANCHOR_FULL_FLOOR_LOWER`, service wall |
| **Front door** | **south**, grille at x −900..900, y −2700 | `ROUTE_MALL_GRILLE_*`, `ROUTE_SOUTH_HEADER` |
| Cash wraps | south wall, x −3500..−3100 and 3100..3500, y −2250 | `DEPT_CHECKOUT_WEST/EAST` |
| Back-of-house wall | y 2090..2110, full width | `ROUTE_SERVICE_WALL_W/M/E` |
| Two service doors | x −3400..−3000 ("STOCK / SALES"), x 3000..3400 ("SALES FLOOR") | `ROUTE_SERVICE_HEADER_W/E` |
| **Receiving dock** | **north**, pad x −600..600, y 2680..3300 | `ROUTE_DOCK_RECEIVING_PAD`, `DOCK_SIGN "RECEIVING"` |
| Player starts | (400, 3340), facing −139° — on the dock pad, outside | `ANCHOR_PLAYERSTART_ENTRANCE` |
| Main aisle | x −900..900, full length | `MAIN_AISLE_HALF_X` |
| Cross aisle | y −500..500, full width | `CROSS_AISLE_HALF_Y` |
| Atrium, escalators | x −600..600, y −900..900, open to the roof | `ROUTE_ESCALATOR_ASCEND/DESCEND` |
| JUNIORS, 477 rails | x −4291..−1248, y −1895..−381 (south-west) | `audit_boswells_departments.py` |

**Two facts settle most of the layout.** The customer entrance is in the south
wall and faces the Galleria; the dock is in the north wall behind a back-of-house
corridor. So the store was arranged for somebody walking in from the south — and
the player walks in from the north, through the goods door. Every department
ordering below is for the customer. The player gets it backwards, which is the
point.

## 2. What real floorplans suggest

- **The loop is already the plan.** A racetrack layout runs one continuous main
  aisle around the floor with departments as islands off it, so every department
  gets main-aisle exposure and the shopper steps off wherever they like. Its known
  failure is dead interior corners. Boswell's has had the racetrack since
  `f5f2a43`; what it lacks is anything in the islands.
- **Destination departments pull traffic; impulse departments catch it.** The
  planning advice is to put high-demand destination categories where reaching them
  walks the customer past everything else, and to treat the first few metres
  inside the door as a decompression zone that sells little by design, with the
  high-velocity goods at the first decision point past the threshold.
- **Juniors belongs at the mall door.** Historically the main floor took the
  small, impulse, high-margin goods — cosmetics and accessories — while apparel
  sat deeper or upstairs. In a single-level mall anchor the equivalent of that
  front-of-house position is the concourse end of the floor. JUNIORS is already
  there and already has the stock, so this proposal does not move it.
- **Men's is a destination, and sits apart.** Selfridges puts beauty on the
  ground and men's on the first floor; Japanese department stores put cosmetics at
  grade and men's higher; the 1890s Siegel-Cooper put menswear on the main floor
  and womenswear above. The constant across all of them is not a floor number — it
  is that men's gets its own zone with its own approach, never a shelf inside
  womenswear.
- **Bulky boxed goods go next to receiving.** Store planning puts the stockroom
  against the dock with a straight-through flow to the departments, and bulky
  items need extra floor area. Electronics in a store this age is cartons on
  pallets; it belongs where the pallet jack has the shortest trip.
- **Keep the mezzanine shallow.** Boscov's — the live reference for this kind of
  store — said plainly that people do not like to traverse from floor to floor,
  and their newest store put all the retail on one level with offices and
  stockroom above. Boswell's upper deck should keep HOME, TOYS, BEDDING and
  FURNITURE (browse, bulk, nobody's errand) and should never hold a department the
  chapter needs the player to reach.

## 3. The proposal

Four quadrants off the racetrack, with shoes as the hinge on the east side:

| Department | Block (cm) | Area | Why there |
|---|---|---|---|
| **JUNIORS** | x −4350..−900, y −2150..−500 | 570 m² | Unchanged. At the mall door, and the only zone where the map's stock and its signs already agree. |
| **LADIES** | x 900..4350, y −2150..−500 | 570 m² | The prime block: biggest, straight across the main aisle from juniors, both visible through the grille from the Galleria, each with a cash wrap already built at its own corner. |
| **SHOES** | x 900..4350, y 500..1150 | 224 m² | On the cross aisle between ladies and men's, because shoes serve both. Its existing mark (2262, 719) is already inside this block. |
| **MENS** | x 900..4350, y 1150..2090 | 324 m² | Its own quadrant, back wall, beside the "SALES FLOOR" service door. A destination department reached by walking the loop. |
| **ELECTRONICS** | x −4350..−2300, y 500..2090 | 326 m² | Behind the "STOCK / SALES" door — the shortest straight line from the dock, which is what cartons need. |
| *unassigned pad* | x −2300..−900, y 500..2090 | 224 m² | **Left open on purpose.** Candidates: housewares at grade, a seasonal or toy pad, ladies' coats and petites. The map has no stock that names any of them, and inventing a department to fill a hole is how the old carpet got its SHOES. Chris's call. |

The front band (y −2700..−2150) stays selling-free: grille, decompression, and
the two cash wraps. The atrium and both aisles stay as laid, because the aisle
width is now shared with the bulkhead ceiling from `86cbdfd`.

## 4. What this does to the walk

The customer's route is: south door, juniors and ladies left and right, shoes
mid-floor, men's and electronics at the back.

**The player's route is that film run backwards.** Dock pad, in through the
goods door, along the back-of-house corridor, out of the east service door at
x 3200 — and the first selling floor he ever sees is menswear against the back
wall, straight racks and nothing soft. Then shoes at the cross aisle. Then the
long walk down the main aisle with ladies on his left and juniors on his right,
getting smaller and brighter and more domestic the closer he gets to the grille
and the Galleria beyond it.

That ordering is free: it falls out of entering a store through the wrong door.
It is worth knowing before any later change moves the dock or the grille.

## 5. What this proposal does not claim

- Nobody has played it. The blocks are sized from the map and from how real
  stores are arranged, not from walking the floor.
- It does not say what LADIES or MENS racking should *look* like. The 104
  `BSW_APPAREL_LOWER_*` actors are one generic rail dressed five ways; a men's
  department that reads as men's needs different fixtures, not the same rail in a
  different block.
- The LADIES sign at (−2850, −2050) still stands in the JUNIORS racking. If this
  layout is accepted that sign moves to the south-east block; the contradiction is
  resolved by the decision, not by any rule.
- The carpet still has colour and roughness and no weave texture.

## Sources

- [Retail space design considering revenue and adjacencies using a racetrack aisle network (Auburn)](https://eng.auburn.edu/~aesmith/files/RetailSpaceDesign.pdf)
- [Racetrack / loop store layout](https://www.artisticdisplayny.com/blog/loop-store-layout/)
- [9 Effective Retail Space Planning Strategies](https://www.expandastands.com/feeds/blog/retail-space-planning)
- [Inside the Decompression Zone at Store Entry](https://type-magazine.com/retail/store-layout-decompression-zone/)
- [Compartment Couture: New York City Department Stores 1850–1930 (FIT)](https://fashionhistory.fitnyc.edu/compartment-couture/)
- [Selfridges floor directory](https://www.london-tourist-guide.com/en/shopping/department-stores/selfridges-in-oxford-street.html)
- [Japanese department stores](https://www.japan-guide.com/e/e2072.html)
- [Family-owned Boscov's ready for Rochester debut](https://rbj.net/2025/10/29/boscovs-opens-rochester-store-greece-ridge/)
- [The proper planning of store-rooms](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC5238196/)
- [Retail stockroom shelving](https://mh-usa.com/retail-stockroom-shelving-solutions/)
