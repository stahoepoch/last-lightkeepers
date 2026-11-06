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

---

## 8. The measured shell

Status: **proposed**, 2026-11-03. Research for the build. Not a `DECISIONS`
row. Real tenant names in this section are how the building is identified.
Row 19 still holds in the level: the anchors the player sees are Boswell's,
Sayers, Penhurst, and Bellevue.

Chris, the same day: build the Galleria, anchors and all, as a massive first
level. Row 4 already says the mall is this one. Row 28 already says version
one is one sealed department store, and the rest is earned. Row 38 already
says the whole mall is coming, and that more floor is not how danger arrives.
The three sentences fit together as one building: the first level is the
whole roof, blocked in, and the night that is furnished is still Boswell's.

### What was actually read

- Lower-level lease plan, sheet TLP-1, dated 04/21/16, Zamias title block,
  Gemini Property Management, 500 Galleria Drive. One page. The drawing is a
  raster. Its own note says every dimension is approximate and subject to
  field verification, and that a name on the sheet may be a proposed lease.
  The public file is
  `Galleria Lower Level Revised Lease Plan 3-29-17.pdf` on the Zamias
  Squarespace host. It does not go in the repo.
- The PDF text layer is a broken overlay. Anchor names were read off the
  drawing. Unit numbers that also appear in a stale directory (Shoe Dept.
  Encore 178, Rare Collectibles 195, the 140 kiosk) were used as a check that
  the drawing and the directory are the same building.
- OpenStreetMap way `617829322`, name Johnstown Galleria, 42 vertices.
  Measured here in a local projection: latitude meters = degrees × 110540,
  longitude meters = degrees × 111320 × cos(latitude), origin at the ring's
  centroid `40.295044, -78.833524`.
- Esri World Imagery for that centroid, plus the 2021 Atlantic Retail
  offering for the former Sears at 540 Galleria Drive (five pages, including
  a bird's-eye with a JCPenney label and a Sears outline).
- An upper-level lease plan was not found. A building section was not found.
  Edina, York, and any other Galleria plan were rejected. York is the sister
  mall and a different building. Richland Mall is the demolished one across
  the highway. Its footprint does not merge with this one.

### The roof

The footprint is a dogleg, two stories, long axis roughly northeast to
southwest, with a glass spine on the white concourse roof and a ring road
around the lot. US 219 is to the west. The inner crook of the dogleg is grass.

| Measure | Value | Source |
| --- | --- | --- |
| Gross leasable area | 894,646 sq ft (83,120 m²), two floors | Wikipedia, and repeated in later coverage |
| Roof area | 42,856 m², 461,299 sq ft | OSM way `617829322`, projection above |
| Axis-aligned extent of that ring | about 355 m by 359 m | Same. This is the box around a rotated dogleg, not the length and the width of the walk |
| Longest traced walls | 132.5 m (435 ft) at bearing 65°; 149.7 m (491 ft) at bearing −149° | Same |
| Faces in the anchor range | several of 72–82 m (236–269 ft) | Same |
| Two floors of the roof | about 923,000 sq ft gross | Arithmetic on the roof area |

The 894,646 and the 923,000 are both kept. One is leasable area, the other is
a traced roof counted twice. The gap is common area, single-story entries,
and tracing error. Neither number is a wall.

The Sears offering, 540 Galleria Drive, says three things that do not agree
with each other: a two-story building of 125,824 sq ft, dimensions about
234 ft by 234 ft, and an aerial callout of 122,146 sq ft on the same flyer.
A 234 ft square is 54,756 sq ft on one floor and 109,512 sq ft on two equal
floors, which is neither of those areas. The greybox follows the roof ring.
The 234 ft figure is a face to check against the 72–82 m walls, not a box to
extrude.

A secondary writeup lists Sears 125,800 plus a 6,000 sq ft auto center,
JCPenney 101,000, Bon-Ton 81,200, and Boscov's 101,300. Those four-and-an-auto
sum to 415,300. A 2018 outparcel brochure says the anchors were about 458,176
sq ft together. The two totals are left in conflict. They are not demising
dimensions.

### The lower level, as the sheet draws it

TLP-1 has no north arrow. "North West Consumer" is a tenant on the Boscov's
side of the court. Adjacency is what the sheet is good for.

- **One end holds two anchors across a short concourse.** Sears on one side,
  Bon-Ton on the other. Small inlines sit between them, including a bank
  space at the Sears vestibule. Both anchor interiors are blank on this sheet.
  Bon-Ton shows a sprinkler room and an electric room on its service side.
- **The walk jogs into an elbow court.** The kiosks there are printed around
  30 ft wide (25.58, 30, 30, 30, 30) with areas 996, 1,200, 1,200, and 2,100.
  GameStop and GNC are in that cluster. Fuji Japanese Steakhouse is 6,298 sq
  ft. Escalators and an elevator are drawn. This 30 ft is a kiosk width. It
  is not the depth of the inline shops.
- **A straight wing follows.** One printed run is 170.58 ft, which is 5,199 cm.
  Shoe Department Encore is 17,998 sq ft on that wing, and a golf tenant is
  8,601. Books-A-Million is 5,435, with Kay, FYE, and AT&T on the same straight
  wall. Dividing 17,998 by 170.58 gives 105.5 ft, and the drawing marks depths
  of about 100 ft on that wing. That division is a check inside one sheet, not
  a survey. JCPenney is the anchor on the opposite side of the wing. The text
  layer carries 75 ft and 70 ft notes at that frontage. The JCPenney interior
  is undivided.
- **Boscov's is the anchor past that court.** The court in front of it has a
  stage, an escalator pair, and an entry vestibule. The anchor interior is
  blank.

No concourse width is printed. No floor-to-floor height is printed. The upper
level is a second floor the current directory still lists, and it has no
sheet here.

### Where the fifth anchor goes

The 2016 sheet draws four anchors and no empty fifth room. Wikipedia records
that the mall opened with space for a fifth anchor that was never filled.
The Tribune-Democrat, 9 July 2008, as returned in search text, says the Gemini
purchase included the land between Sears and the food-court entrance, where a
fifth anchor had been sought since 1992. The live article page was a captcha
on this pass, so that sentence is a search excerpt, not a re-read. It matches
the grass in the inner crook of the roof, between the Sears/Bon-Ton end and
the food-court elbow. A Deadmalls walkthrough (secondary) puts the vacant
parcel on the back of the mall and says it was never even parking.

Steve & Barry's, late 2007, was a lower-level store of just over 20,000 sq ft.
That is an inline, not the missing anchor.

The filled slot in our fiction sits on that land, attached at the elbow. Its
walls are not on TLP-1. No side length is published for it until someone
traces the pad against the scaled plan.

### Lengths the greybox may use

Unreal is centimeters. One foot is 30.48 cm. `StudsToCm` (28) converts Roblox
data rows. It does not convert this building.

| Length | Centimeters | Standing |
| --- | --- | --- |
| The plan's own scale bar, 100 ft | 3,048 | The only scale the sheet declares |
| The printed straight run, 170.58 ft | 5,199 | One wing segment, not the whole mall |
| Kiosk width, 30 ft | 914.4 | Food-court cluster |
| Wing shop depth, about 100 ft | 3,048 | Marked on that wing; agrees with the Shoe Dept. division above |
| Lower story, 18 ft | 548.6 | **Stand-in for the floor-to-floor only.** The upper concourse is a vault above that. Forbidden as a vault, jump, or counter number |

### How to block it in

A new level, when `Content/` is free. The name to reach for is
`LLK_Galleria_Shell`. `LLK_Boswells_FirstRun` stays the furnished night. It is
not the file this shell gets written into.

1. Origin at the centroid above, or at a corner Chris picks. Write down which
   axis is north. The roof ring is the exterior. Two plates.
2. The lower-level sheet is a reference in the editor, held locally, scaled
   until its 100 ft bar is 3,048 cm. Trace the public walk and the four anchor
   boxes off that scaled sheet. The first number the shell owes is the
   concourse width, read after the scale is set, because the sheet never
   prints it.
3. Grilles down on every anchor face. Boswell's is the end anchor whose one
   grille faces the concourse and whose dock faces the lot. Which real end
   that is remains Chris's call. The play reason is one gate. The other three
   real anchors are sealed volumes at the adjacencies above: the anchor across
   the short concourse, the long-wing anchor, and the anchor past the court.
   The filled slot is the fifth volume, tagged unmeasured, on the land between
   the Sears end and the food court.
4. The upper floor keeps the same four anchors. A 2017 directory photo shows
   a second shop grid over the lower one. Trace that adjacency. Do not copy
   the lower ceiling up. The upper concourse is the high room: an arch under
   the glass, open down through the center court and the food court. The rise
   of the arch is unmeasured. Section 9. Section 10 reads the live map. The
   upper plate is the balcony around those openings, plus the paired
   escalators on the far wing. It is a cut plate. Copying the lower plate up
   would fill the wells. The lower plate in that model also fills the
   long-wing anchor's footprint, and the upper walk stops at that face.
5. Glass over the concourse only. The aerial shows the spine, and the upper
   walk is the floor under it. That glass is why section 7 can say the
   concourse is lit and the stores are not.
6. The service ring from section 4 is the goods route. TLP-1 does not draw a
   continuous back corridor. It draws service rooms at the anchors. The ring
   stays a proposal inside the shop depth until a sheet shows the wall.
7. Fountain and clocktower elevator stay as this page already wanted them. A
   2026 Post-Gazette snippet describes both as original, and the article body
   was paywalled this pass, so they are not upgraded to a surveyed fact. The
   2016 sheet draws a stage in the Boscov's court. A center-court fountain is
   also in a secondary walkthrough, which says it came out in the 2010s. The
   carousel and the interior park remain the Richland borrow. They are not on
   this sheet.

A dogleg a few hundred meters long is one persistent level. World Partition
is a later tooling call. The cost arrives when the concourse is glazed and
stocked, not when it is twenty boxes.

### Turned over

**Operand.** Playable night-one floor, against the furnished Boswell's
interior.

**Floor.** A volume seen through a closed grille is shell. A volume the crew
can walk is the night.

**Clock.** The shell's clock is the tracing. The fun clock is still the
seven-line sitting in the existing store. A story height of 18 ft is on
neither clock.

**Falsifier.** The shell claim falls if a played crew leaves the store and
spends the night in an empty concourse. It also falls if the grille shows a
flat wall where the second anchor mass should be. A length falls if it
disagrees with TLP-1 once the 100 ft bar is scaled, or with way `617829322`
re-measured in the same projection.

**Upside down.** The biggest first level is the one that teaches the mall by
refusing to let you into it. Four stocked anchors on night one is a larger
empty map. The building is massive because you can see that it continues. The
job is still one store.

---

## 9. The upper floor, and the sister mall

Status: **proposed**, 2026-11-03. Same rule as section 8. Real names identify
the buildings. They do not enter the level.

Chris: the second floor has high arching ceilings. Look at the website, and
at the sister mall, York Galleria. Use both.

### What the Johnstown site says about the section

`johnstowngalleria.com` is a living directory, not a plan. It does settle
which floor the public rooms are on.

- The map filters **Level 1** and **Level 2**.
- The food court is the upper level. Southern City, Napoli Pizza, Rayne's
  Backyard BBQ, Rosalinda's Express, and Annelesse's are all listed there.
- Center court is a different room from the food court. Italian Oven is
  "upper level, center court."
- The escalators from that food court land on the lower level at the
  information booth. StatTech's own page puts the shop at the bottom of those
  escalators, behind the booth. Island Cuisine is lower level, near the booth.
- Gardners Candies is "Second Floor — Main Corridor." The live model agrees,
  and puts that shop on the upper walk. Mister Jay's page says "upper level,
  Strip." The same model draws Mister Jay's on Level 1, in a large bay beside
  the long-wing anchor. The sentence and the drawing disagree. The drawing is
  the plan. Section 10.
- A January 2026 mall post still navigates by "the Sears exit." The closed
  anchor is still how the building is walked.

A directory photographed in the mall in April 2017 (Flickr, Nate Henderson)
shows both floors on one board. Sears and Bon-Ton share one end. JCPenney
runs the long wing. Boscov's is the far end. The lower diagram matches TLP-1,
including the large bay numbered 172. The upper diagram is a second set of
shops, numbered in the 200s, over the same anchors. That photo is glare and
it has no scale bar. It gives adjacency. It does not give feet.

The roof aerial already showed a glass spine. The upper walk is the floor
under that glass. The lower walk has a ceiling, except where a court is open
through both floors. Chris's arch belongs on the upper concourse, over the
food court and the center court, and along the second-floor corridor. The
rise is not on any sheet. A greybox that caps the upper walk at the 18 ft
stand-in has ignored the one thing he said the second floor is.

The Pittsburgh Post-Gazette, 21 September 2026, as returned in search text,
calls it "the airy, two-level mall" and says it opened with a fountain and a
clock-tower elevator. The article body was paywalled this pass. A 2019
Deadmalls walkthrough says the circular fountain came out in the 2010s.
Tribune-Democrat reporting in 2024 and 2025 says the current owner repaired
the fountain, and the children's play area. Both can be true in sequence.
The fountain and the clocktower elevator stay. The carousel and the interior
park are still the Richland borrow.

### York Galleria, as inspiration

York Galleria, 2899 Whiteford Road, Springettsbury Township, opened in 1989.
Zamias built it. Johnstown, three years later, is the sister. Deadmalls says
so on camera, and a 2022 walkthrough calls them twin sisters. Same original
four anchors: Boscov's, Bon-Ton, JCPenney, Sears. Two floors. A skylit
concourse. The food court is on the upper level there too.

A directory photographed at York on 2 July 2012 (Wikimedia Commons, Mike
Kalasnik, CC BY-SA 2.0) draws the same corner order as the Johnstown board:
Sears at one end, Bon-Ton across the short side, JCPenney along the long
wing, Boscov's at the far end. The food court is labeled on York's upper
level, between Bon-Ton and Boscov's. A round center court sits in the middle
of the walk. Escalators and an elevator are marked. That is the family
resemblance. It is not a plan of our building.

York is the smaller sister. Wikipedia gives 764,514 sq ft. A 2022 leasing
flyer says about 765,000. Another directory says 756,715. Leave the conflict.
None of those numbers is a Johnstown wall. York's former Sears upper floor
was offered at about 70,000 sq ft, with the lower floor taken by a casino.
One floor of an anchor is a large room. It is not the whole store.

What York is good for, because the photos and the tours are easier to find
than Johnstown's:

- The upper floor is the tall, daylit floor. A Yelp note from a visitor calls
  the ceilings high and distinctive, with enough glass that the concourse
  takes daylight. Walkthroughs point at the skylight over the food court, a
  hex pattern overhead, and a terrazzo compass in the floor that you can read
  from the upper rail and miss from below. That is the section we are
  missing: a well, a rail, and a vault.
- The vertical landmark is an elevator in a strong color. York's opened a
  dark red and was later painted purple. Johnstown's version is the clock
  tower. One object, not both.
- Inside the department store, York's Boscov's still had mirrored ceilings,
  chandeliers, and orange and purple neon on a 2022 visit. A 2018 walk of
  the Johnstown Boscov's is described the same way: the 1992 interior, neon,
  woodgrain, chandeliers. That is Boswell's, and `THE-STORE.md` already
  locked the mirrored ceiling islands and the broken neon. It is the store,
  not the concourse.
- Deadmalls' own distinction: Johnstown trades York's pastel for a subdued
  green and white. The sister's neon palette stays in the department store.
  The concourse of our mall is the green-and-white arch and the glass.

What York is not good for: its footprint, its square footage, its entrance
statues, its pastel, or the circle in the middle of its directory. Johnstown's
2016 sheet draws a stage in the Boscov's court. If our center court wants a
circle, that is a later art call, after someone stands in the real one.

### What the shell owes now

The four measurements from section 8 stand. Add three from this pass.

1. The rise of the upper arch, in centimeters, from the upper floor to the
   crown of the glass. Chris has the shape. Nobody has the number.
2. Which courts are open through both floors. Section 10 names three openings
   in the live model: the food-court escalator, the center well with the
   elevator, and the paired escalators on the wing toward the unnamed end
   anchor. They are still unscaled.
3. A scaled upper sheet. Until it exists, the 2017 directory photo is the
   adjacency, and TLP-1 remains the only scaled drawing.

**Falsifier.** The arch falls if a second-floor photo shows a flat ceiling at
the height of the shop fronts, with the glass only in a lantern at the court.
The sister claim falls if York's anchor order is laid over the Johnstown roof
and the corners disagree. On the two directories they do not. A York length
used as a Johnstown wall falls immediately.

**Upside down.** The high ceiling is the thing that makes the concourse
worse, not prettier. Daylight from an arch is ambient light, and ambient
light is how the building sees you. The stores under it stay dark. The second
floor is the bright floor. That is the trade section 3 already wanted from
Christmas, built into the architecture.

---

## 10. The live map, and the missing walk

Status: **proposed**, 2026-11-04. Same rule. Real names locate the real
building. They do not enter the level. Nothing in this section is a foot.

The directory at `johnstowngalleria.com/map/` is a Verge3D model, Verge3D for
Blender 4.12, loaded from `galleria mall 2.gltf` under
`/wp-content/uploads/verge3d/322/`. The page around it is only the tenant
list. The drawing is the model. It was read here and it stays out of the
repo. Its units are the artist's units. A length taken from them fails.

The model has two walk plates and three anchor volumes.

- `floor.l1` is one filled polygon. It is the lower walk, including the
  floors of the openings and the footprint of the long-wing anchor.
- `floor.l2` is the upper walk, a cut plate in the Mall Second Floor group.
  On Level 2 the lower plate stays visible through the cuts. That is the
  missing walk the page shows.
- About seven tenths of the lower plate has upper floor over it. About three
  tenths does not. A little over half of that gap is the long-wing anchor's
  own footprint, baked into the lower plate. The rest is the openings.
- A thin sliver of upper plate hangs past the lower plate. It is an edge, not
  a room.

The upper walk runs from Boscov's face, along the wing, around the elbow, and
out to the far end anchor. Shops line it, numbered in the 200s, which matches
the 2017 directory photo. The walk you can stand on is the rim. The cuts are
where you look down.

Three openings are big enough to be courts. The rest are stair punches.

1. The food-court escalator. One escalator, on the Boscov's wing. The cut is
   a wide opening beside it. The upper food tenants, the restrooms, and
   Gardners Candies sit on the upper walk along the far side of that opening.
   StatTech sits on the lower level near the same escalator, which is the
   landing the website already described. Bay 172, the old large lower bay,
   now labeled as the roller rink, sits under the upper walk on this wing.
   Bay 178, the other large lower bay, sits under it too. Their areas in the
   model are still in the 2016 ratio: 17,998 sq ft over 8,601 sq ft is about
   2.09, and the two meshes are about 2.07. The inline outlines are that
   lease grid. The anchor boxes are not. They are simplified, and this ratio
   does not size them.
2. The center well. It sits at the elbow, and the one elevator sits in it,
   with a stair beside it. Italian Oven is on the upper walk next to this
   opening, which is the website's "upper level, center court." This is the
   clock-tower elevator's candidate. The 2023 tour names a glass elevator and
   a clock tower. One object in the model, one object in the tour. Not yet a
   measured match.
3. The paired escalators, on the wing that runs toward the far end anchor.
   Two holes, with upper floor remaining on either side, so the balcony
   continues past them to the end.

Seven stairwells, three escalators, one elevator. Four public entries, two
tagged to each floor, and the upper pair is not stacked on the lower pair.
The upper doors are at the food-court side and at the long-wing anchor. The
lower doors are in the crook under that anchor and at the far end.

The long-wing anchor is JCPenney, drawn as its own volume and also filled by
the lower plate. The upper walk stops at its face. There is a stair at that
corner and an upper entry. You reach the anchor twice, once from each floor.
You do not cross its roof as concourse.

Boscov's is the box at the other end. The walk stops at its face. The far end
is a third anchor-sized box. Its lease string is empty. The blend object is
named Casino. The directory has no such tenant. York's former Sears lower
floor is a casino, and the sister is where that word is real. Here the word
is a file label. It is not a room in our building, and it is not evidence of
which closed anchor the box is. Sears and Bon-Ton are not labels in the file.
The 2016 sheet and the 2017 board still draw both, facing each other at one
end. This 2026 model draws one unnamed end box and leaves the other out.
Separate ownership explains why a leasing model might skip a closed box. It
does not say which box is the one still drawn. A January 2026 mall post still
navigates by the Sears exit, so the building still has that end even when the
mesh omits a volume. The shell keeps both end anchors from TLP-1. The fifth
pad, the grass in the crook, is still outside this model and still unmeasured.

Each court opening is smaller than bay 172. The long-wing gap is the size of
that anchor's footprint in the model, on the order of two and a half times
bay 172, and it is the anchor, not a court. No centimeter comes from any of
these. The 100 ft bar on TLP-1 is still the scale.

### What the photographs show

Three title frames, opened as stills. The videos were not watched through.

- Ace's Adventures, 26 October 2018. The frame looks down a two-level
  concourse. A teal steel truss and glass form a vault over the full width.
  An escalator rises to an upper rail. The lower floor is open. The shop
  fronts sit off to the sides under a lower soffit.
- Exploring Retails With Andrew, 23 January 2023. The frame is from the upper
  rail. The same truss runs away down the concourse. Food-court tables are on
  the upper level to one side. An escalator drops through a well. That is the
  map's upper food court and one of its openings, in a photograph of this
  building.
- Deadmalls, Jack Thomas, filmed 2019. The frame is inside a shop. The shop
  ceiling is a flat grid. The concourse outside shows the upper rail. The
  vault is the concourse ceiling. It is not the shop ceiling.

Chris's arch is this truss. It is steel and glass, over the open concourse,
with the upper walk as the balcony under it. The rise is still unmeasured.
The 2023 narration, as auto-transcribed, also names skylights, a clock tower,
a glass elevator, and a salmon and turquoise interior. Speech-to-text is not
a photo. The frame that was opened shows the truss. Deadmalls' older line,
green and white against York's pastel, stays on the record beside it. Ace's
frame is teal structure and pale columns. Three witnesses, one vault. York's
pastel still does not cross over.

The section 9 falsifier, a flat ceiling at the shop-front line with glass
only in a lantern, is not what these frames show. What would still kill a
vault-the-whole-way claim is a photograph down a side wing whose ceiling is
that flat soffit. These frames are the center and the food court.

### What the shell does with it

The upper floor of `LLK_Galleria_Shell` is the cut plate. Three openings:
food-court escalator, center well with the elevator, paired escalators on the
far wing. The lower floor is the filled walk, including the floors of those
openings. Glass and the truss span the concourse. The balcony is the upper
walk under that glass. Shop ceilings stay low. The long-wing anchor is a
sealed volume the upper walk stops at. Both end anchors stay, from TLP-1,
including the one this leasing model stopped drawing. The fifth pad stays on
the grass, tagged unmeasured. Row 28 does not move. One anchor is the
furnished night. The wells are how you see the rest.

**Operand.** The upper walk a crew can stand on, against a second copy of the
lower plate.

**Floor.** An opening you can look through is shell. An opening you can cross
on the upper floor is a filled well, and the map says those three are open.

**Clock.** The tracing clock gains the live model as adjacency. It gains no
new length. The fun clock is still the sitting in Boswell's.

**Falsifier.** The balcony falls if a scaled upper sheet shows a full upper
plate with no food-court opening, no center well, and no escalator pair. The
vault falls if a second-floor photograph down a wing shows a flat ceiling at
the shop fronts. A foot derived from the glTF units falls immediately. The
blend name Casino, used as a tenant or as a way to pick which closed anchor
the end box is, falls immediately.

**Upside down.** The missing floor is the part that makes the mall readable.
A solid upper plate would hide the lower concourse and shut the sightline the
daylight uses. The holes are where the bright floor sees the dark one. The
anchor you cannot cross, twice, once from each level, is a bigger wall than a
corridor that sneaks over its roof. The end box the leasing model gave up on
drawing is still a dark mass in the shell, because the building still walks
to that exit and the 2016 plan still has the room.

---

## 11. The cook's order

Status: **proposed**, 2026-11-04. This is the order the shell is built in.
Sections 8, 9, and 10 are the ingredients. The greybox those ingredients
already produced is `docs/reference/galleria/`. Nothing in this section is a
decision, and nothing in it is a license to open `LLK_Boswells_FirstRun`.

### What is on the bench

| Ingredient | Where | Use it for |
| --- | --- | --- |
| Lower-level lease sheet TLP-1, 100 ft bar | Held locally, not in the repo | The scale. 100 ft is 3,048 cm. Any length that disagrees with the sheet after that bar is set is the sheet's. |
| OSM way `617829322` | Section 8, the projection named there | The roof ring. Not a floor plan. |
| Live directory model | Section 10. The glTF stays out of the repo | Which floor is a balcony, where the three openings are, where the four public doors and the stairs are. |
| Bays 172 and 178 | Section 10 | The diagram's scale, 708.91 cm a unit, and only until TLP-1 is on screen. |
| Greybox | `docs/reference/galleria/GalleriaShell_Greybox.obj` | A walkable sequence at that scale. `REF_100FT` proves the import. |
| Marker list | `docs/reference/galleria/GalleriaShell_Markers.csv` | Exit lights, stair lights, escalator ends, the court, three dock faces, one extract candidate. |
| Extraction rules | `ALLKExtractionZone`, `ExtractionPointId`, rows 74 and 75, `docs/MAP-READINESS.md` | One volume, id `LoadingDock`, sized to a bay mouth. The actor already exists. |
| Emergency-light shape | Chris's line in `DONTFUCKUPTHELIGHTS.md`, and `docs/FIXTURE-LIGHT-CONTRACT.md` | Exits, both ends of every escalator, the stairs, the court, a sparse concourse run. Signs render. They do not join `LLKLightField`. |

Still not on the bench: the arch rise in centimeters, the concourse width, which roof mass is which anchor against north, the fifth pad's walls, and which closed anchor the unnamed end box is. The shell can be blocked without them. It cannot be called measured until the 100 ft bar has been checked in the editor.

### The order

1. Chris names which real end is Boswell's. The candidate extract sits on the Boscov's outer face until he does. The other two outer faces are marked `DOCK_` so the change is a move.
2. A new level, `LLK_Galleria_Shell`. The furnished night stays `LLK_Boswells_FirstRun`.
3. Import the OBJ at scale 1. Read `REF_100FT`. Then lay TLP-1 under it and break one length. The sheet wins.
4. Four anchor volumes, grilles down. The fourth box is traced from TLP-1, because the greybox does not have it. The fifth pad is a tagged empty on the grass, unmeasured.
5. Emergency lights on the marker list: four doors, seven stairs, three escalators at both floors, the court elevator. Shops and anchor floors stay dark. No candela is authored here.
6. One `ALLKExtractionZone`, `ExtractionPointId` = `LoadingDock`, on the service face of the anchor Chris named. `FreightLift` has no room in this building. The plans show no sub-basement, and that exit wants `SubBasementBreakerRestored`. `Rooftop` is already not an exit.
7. A Sweeper does not go in this level until `docs/MAP-READINESS.md` passes. The shell is a building you can see. The night is still the one store.

### What this commit does not do

It does not make the level. It does not set a candela, a concourse width, or which end is ours. It does not copy the leasing mesh, the lease PDF, or York's plan into the repo. Row 28 does not move. Row 19 does not move. `StudsToCm` does not convert this building.

The first anchor's own section is section 12. It does not change this order.

---

## 12. The first anchor, as Boscov's, played as Boswell's

Status: **proposed**, 2026-11-06. This is the first anchor. The real building is the Johnstown Boscov's at 570 Galleria Drive. The store the player sees is Boswell's. Real names stay labels. Row 19 does not move. Nothing here resizes `LLK_Boswells_FirstRun`, and nothing here is a foot.

### What the building is

It opened 19 September 1992, before the mall. The address is 570 Galleria Drive, Johnstown, Richland Township, Pennsylvania 15904. The directory unit is 570. The phone on the company's own page is (814) 266-4300.

That page lists one elevator item, a wheelchair-accessible elevator, plus a wheelchair-accessible entrance and a wheelchair-accessible parking lot. The services it names are a military discount, Optical, a Hearing Aid Center, the online registry, and in-store and curbside pickup. Optical and hearing are rooms. Their floor was not found. The registry is a service, not a room that can be placed. A restaurant, a hair salon, and a travel desk turn up in aggregator "about" text and do not turn up on that services list. They stay out of the plan.

The building is owned apart from the mall. Wikipedia's Galleria article says the Boscov's, the former Bon-Ton, and the former Sears buildings are independently owned. On 8 October 2022 Deanna Buda, assistant manager, told the Tribune-Democrat that the building is owned separately by the Boscov's company. On 10 February 2017 the same paper reported that Pax Mall Realty, then the owner of the store at 570 Galleria Drive, had appealed a $2.5 million assessment. The 2017 owner is not treated as the current owner. Ownership is not a wall.

A public listing aggregator gives the lot at that address as about 2.2 acres, built in 1992. The county card's building square footage was not retrieved. The lot is not the floor plate. 2.2 acres is about 96,000 sq ft of land. A single floor of 160,000 sq ft would not sit on it. Two floors whose combined area is 160,000 would, on a plate of about 80,000. The lot does not decide between the two published areas below.

### The section you can block

Two selling floors, and a third level the riders call offices.

- Buda, 8 October 2022 (Tribune-Democrat, Russ O'Reilly; the Yahoo reprint carries the quotes the paywall hides). The second floor had been under renovation since June and was due that November. "When it's finished, the candy counter will return." The first floor was to start the following spring. She remembers being a kid in this store, "looking at the chandelier, the mirrored ceiling, and going to the candy counter."
- Remembering Retail, caption posted 4 May 2026. Sean Selznick filmed the candy counter in March 2026, on the second floor. The caption says the remodels took the chandeliers and the neon down, and the counter stayed. The caption was read. The footage was not watched.
- WJAC, 2 July 2019, Sydney Jaxtheimer. The angel tree at the Johnstown store "is on the second floor by the courtesy desk." Store manager Rick Bausher is named. Wilkes-Barre puts a courtesy desk on a fourth floor, and Scranton puts a tree on a second floor of a different building. Those stores do not move this desk.
- The elevator. elevaTUBE, filmed 12 October 2017, published 3 March 2020: "This is the only anchor that serves 3 floors, while the rest of the mall has 2." The same note wishes an auditorium onto the top floor beside the elevator and the executive offices. That is a wish. It is not a room that was found. Grute's Trains, Movies and Elevators, filmed 14 February 2026, published 2 March 2026: "This store is actually 2 floors as the third floor is offices." Both descriptions say the machine was installed in 1992, and a comment on jaemre288's ride of 25 May 2025 says the same. One car was ridden, across several visits. The company's page confirms an elevator and does not count the cars. The mall's center-well car is a different machine.
- The name "RT 300A" is the riders' label. A comment on the 2017 ride says a 300A was not introduced until 1993. A hobby model guide dates that model to 1993–2004, and the same guide says its own pages are out of date. What holds is a Schindler hydraulic, with a 1992 install claimed by the people who rode it. The model number stays disputed.
- A comment on the May 2025 ride says "The 3 button is a down button." The footage was not watched. The panel has a 3. Which plate that 3 is, is not read off the comment.
- The escalators. Three rides, titles in the plural, all at this store. NEOH Escalators, filmed 14 January 2024. Bhedinskis717, 9 April 2023. Grute, filmed 14 February 2026, description "Installed in 1992." None of the descriptions count the machines. A plural title can be one criss-cross well. Two separate wells would be another store's plan, and Colonie Center's two escalator sets and two elevators do not cross over. Block one well. Count it when a frame is counted. Both ends take an emergency light once the well has a place, the same rule the concourse already uses.

What stays blank:

- No Johnstown department map was found. Men's, women's, shoes, home, toys, and jewellery are not assigned to a floor here. A review that puts children's, toys, home, and electronics upstairs, and clothing, perfume, and jewellery downstairs, is tagged Annville, Pennsylvania, on a page that also carries the Johnstown name. Annville is a different store. The split does not enter.
- A stair that lands on the candy counter was looked for and not found. Candy is on the second selling floor. The path to it is still open.
- Gardners Candies is a mall shop on the second-floor corridor. Sweet Spot is a mall shop that opened into a walled-up food-court bay. Neither one is this counter.

`THE-STORE.md` already locks mirrored ceiling islands as Boswell's signature, and it already wants an escalator well. Buda's memory is that room, in this building, from someone who later ran the floor. The 2022–23 remodel is the real store moving on. Boswell's keeps the older room.

`THE-STORE.md` also stacks four sales floors, a basement, and a roof. That stack is the proposal of 26 October 2026. The witnesses on this building give two selling floors and an office level. This section does not retire that proposal. It is the pressure on it. A fourth sales floor, and a basement under this anchor, are filled inventions relative to what was found. `FreightLift` still wants a sub-basement these plans do not have.

The furnished night stays the room that already exists. `FINDINGS-BOSWELLS-GEOMETRY.md` measured it at 2,800 by 1,800 cm, about 5,400 sq ft. One plate of the box below is about nine times that room. This section does not stretch the night up to the box.

### The numbers, and they disagree

| Figure | What it is |
| --- | --- |
| 160,000 sq ft | Tribune-Democrat, 21 September 2017, Mark Pesto. The lede calls the 1992 store "the 160,000-square-foot store," the first store to open in the Galleria and the chain's first location west of Harrisburg. The body past that sentence was not read. |
| 101,300 sq ft | Section 8's secondary writeup, in the same list as Sears 125,800 plus a 6,000 sq ft auto center, JCPenney 101,000, and Bon-Ton 81,200. Section 8 already says that list is not a set of demising dimensions. It was not re-measured off TLP-1. The anchor interiors on that sheet are blank. |
| 49,389 sq ft | Section 10's bounding rectangle of the directory box, at the bay scale of 708.91 cm a unit. 200.55 ft by 246.27 ft. Two plates are 98,778. Three full plates are 148,167. The up-facing fan in the mesh is smaller, and it is not the floor. |

98,778 sits beside 101,300. 148,167 does not reach 160,000. Three copies of the rectangle are still about 12,000 sq ft short of the newspaper, which is a quarter of one plate and not a reason to extrude a fourth. The two published areas stay in conflict. They are not averaged. TLP-1's 100 ft bar still wins if a wall disagrees with the bay scale. No square footage is authored into the level.

### What the shell can mark

Inside the existing Boscov's volume, when Content is open for `LLK_Galleria_Shell`:

1. A lower selling plate.
2. An upper selling plate. Candy and the courtesy desk belong to this floor. Their plan position is unknown, so they are tags, not walls.
3. An office level above them. It is shell. It is not a sales floor. Whether it is a full plate is unmeasured.
4. One elevator shaft, three stops, serving the two selling floors and the office level. Schindler hydraulic. One car witnessed.
5. One escalator well. The count is still open. Both ends are marked for a light.
6. The mall face, where the walk already stops.
7. The lot face, where `EXTRACT_LOADINGDOCK_CANDIDATE` and `DOCK_ANCHOR_BOSCOVS` already sit. Chris still names which end is Boswell's. Until he does, this face stays the candidate.

The grilles stay down. Row 28 does not move. The night is still `LLK_Boswells_FirstRun`.

**Operand.** The two selling floors a crew can walk, against a third plate copied up and dressed as more store, and against the four sales floors plus a basement in `THE-STORE.md`.

**Floor.** An office level you cannot shop is shell. A third sales floor is a filled invention. A basement under this box is the same invention until a sheet shows one. A department layout taken from Annville, Wilkes-Barre, York, Camp Hill, or Colonie falls on contact.

**Clock.** The elevator rides date the section. They do not date the area. No new foot until the 100 ft bar is in the editor.

**Falsifier.** A county card, or a scaled sheet, that shows one selling floor. A sheet that shows the 160,000 as the two-floor area of this rectangle. A panel with only two stops. A counted frame that shows two separate escalator wells, which would reopen the one-well block and still would not import another store's plan.

**Upside down.** The office floor is the part of the anchor that is not the store. The mirrored ceiling is how the lower floor sees a light it does not own. The grille on the mall face is a bigger wall than a corridor over the roof.
