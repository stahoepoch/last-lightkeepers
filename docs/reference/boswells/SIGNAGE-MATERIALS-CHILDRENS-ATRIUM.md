# Children's floor through the atrium — signage and materials

Status: **brief**, 2026-11-14. Written for the blockout in
`Content/LLK_Boswells_ApparelPass.umap` at `83e5b60`. The map was not opened
for this pass. Measures below are the ones the build scripts saved. A later
writer applies them only while they alone hold that `.umap`.

The fourteen photographs in `source-photos/` decide composition, material
role, and which words are large. They are not textures, and they are not a
plan of this building. Several are later remodels: brand walls, digital
screens, and a fully lit sales floor stay out of the night. The era is
section 13 of `docs/THE-MALL-IS-STILL-TRADING.md`, 1992 through the late
1990s. The light rule is `docs/FIXTURE-LIGHT-CONTRACT.md`: navigation
lettering may emit; it does not join `LLKLightField`; a price card still
needs the lantern.

Letter only the words in the schedule. No vendor name from the photographs
goes on a mesh. Secret saver stays off the walls until Chris puts it on a
sign himself.

## The walk the signs have to explain

Upper floor at z 662.5. The children's slice is the upper west plate.

| Saved piece | Where it sits |
| --- | --- |
| South sage island | Center (−2500, −1750), 2800 × 550 cm. `BSW_KIDS_SOUTH_CARPET`. |
| North sage island | Center (−2500, −700), 2800 × 750 cm. `BSW_KIDS_NORTH_CARPET`. |
| Dark carpet edges | 12 cm strips at y = −2025, −1475, −1075, −325. |
| 23 rolling racks | x = −3650, −3150, −2650, −2150, −1650, −1250 and y = −1900, −1650, −900, −500, except the desk occupies (−2650, −1900). |
| Sales desk | `BSW_KIDS_SALES_DESK` at (−2650, −1900). |
| Column | `BSW_KIDS_CEILING_SALES_COLUMN` at (−2550, −1970). 44 cm square, from the floor at 662.5 up into the grid at 1100. |
| Grid ceiling | `BSW_KIDS_CEILING_TILE_FIELD`, center (−2500, −1150, 1100), 2800 × 1800 × 3 cm. Underside near z 1098.5. Seams every 100 cm at z 1098. Clear height over the carpet is about 436 cm. |
| Reserved walks | Fixtures stay inside x −3950…−1060, and wholly inside y −2050…−1450 or y −1150…−150. The cross aisles and the east bypass are the gaps. Signs in this brief do not add collision there. |
| Well | Escalator slopes at x = −250 (ascend) and x = 250 (descend). Upper guards leave a 300 cm opening at the north landing and one at the south. Smoked mirror canopy over the void, glass centers from x −390…390 and y −700…700 at z 1472. Roof underside at z 1488. |

From the desk the customer route runs east off the sage, across the tile
bypass, and along the west atrium guard to a landing opening. The south
island is babies, because the desk and the column are there. The north
island is kids. The well is the next room, not a third department.

## Sign schedule

Bottoms of hanging panels sit at or above z 863, which is 200 cm over the
upper floor. The built camera is 152 cm. A shorter eye still walks under
them. Nothing in this table is a gameplay light.

| Mark | Copy | Carrier | Place | Finish |
| --- | --- | --- | --- | --- |
| S1 | `BABIES` | Paint on the column's east face, the face turned toward the well. Stacked small caps, one letter per line, bottom to top. | Letters run z 820–1040 on the 44 cm face. No blade wider than the column. | Dark gray on the column's cream. Unlit. |
| S2 | `sales desk` | One hanging panel, 96 × 28 × 2 cm, non-colliding. | Center (−2650, −1860, 980), hung from the grid over the desk and clear of the column. Lowercase, one line. | Pale celery face, dark green-gray letters. Unlit. This is the plaque in `12-childrens-sales-desk.jpg`. |
| S3 | `KIDS` | Horizontal tube word, non-colliding, on the east edge of the north island. | Center (−1200, −700, 1040), inside the ceiling footprint, reading toward the bypass. | Coral tube, emissive only. No point light. Wide gothic caps. |
| S4 | `EXIT` | Two red faces, each 36 × 15 cm, non-colliding. | (−1200, −1700, 1040) for the south island and (−1200, −500, 1040) for the north island. Both face east, toward the tile. | Red translucent face, emissive, the same job as the far exit in `08-sales-floor-entrance.jpg`. |
| S5 | Upper-floor directory, the lines below, in that order. | One overhead panel, 80 cm wide and 150 cm tall, hung in the west bypass. No new post. | Center (−800, −400, 1040), west of `ROUTE_ATRIUM_GUARD_W` and east of the carpet. Bottom above z 863. | Cream board, dark condensed caps. A thin emissive edge is enough to find it under the route spots. |
| S6 | `2` | Painted numeral on the burgundy fascia at each escalator head. | On the art that replaces `ROUTE_ESCALATOR_ASCEND_HEAD` and `ROUTE_ESCALATOR_DESCEND_HEAD`. Do not move those slopes. | Paint. Not a lamp. |
| S7 | `SALE` | Four cards, each about 15 × 10 cm, clipped under the top tube of the east-end rack. | The four racks at x = −1250: `BSW_KIDS_ROLLING_RACK_-1250_-1900`, `_-1250_-1650`, `_-1250_-900`, and `_-1250_-500`. | White card, red caps. The tube shades the face from the ceiling spots. The lantern is what resolves the word. |

Directory lines, and no others:

```
CANDY
HEARING
OPTICAL
GIFT WRAP
RESTROOMS
KIDS
BABIES
TOYS
SEASONAL
BEDDING
CURTAINS
HOME
FURNITURE
```

`KIDS` on the directory is the department. The north-island tube is the same
word, so the sign you walk toward matches the board. Boys and girls are that
one department in section 13. They do not get separate neon.

A painted band, 20 cm tall, may run the east fascia of the kids ceiling:
coral, butter, then the blue already used on the canopy. Unlit. No figures,
no script word. That is the children's wall color from `11-childrens-department.jpg`,
reduced to a stripe.

Do not hang `TOYS`, `CANDY`, or a store name over this carpet. Those
departments have no socket on this slice. The directory is how they exist
from here.

## Materials

Start from the flat colors already saved. A texture may add weave, grout,
or a scuff. It may not change the role of the surface.

| Surface | Saved recipe | Dress |
| --- | --- | --- |
| Sage carpet | `M_BSW_KidsSageCarpet`, linear (0.21, 0.25, 0.15), roughness 0.94 | Low-contrast cut pile. No medallion. The two islands stay separate. |
| Carpet edge | `M_BSW_KidsCarpetEdge`, (0.065, 0.075, 0.065), roughness 0.72, 12 cm | A darker binding. This line is the department boundary a walker feels underfoot. |
| Tile bypass and landings | Undressed in the blockout | 30 cm warm off-white squares, roughness near 0.35, grout a shade darker. This is the hard promenade in `08`, `11`, and `12`. The black polished runner in `01` and `03` is the men's aisle. It does not cross this floor. |
| Rolling racks | White blockout meshes | White powder-coated tube, four casters, one hanging bar. Garments are solid pastels already blocked in: pink, mint, lavender, butter. Hangers are dark plastic. No printed faces, no licensed pattern. |
| Desk | `SM_BSW_KidsSalesDesk` | White laminate top, warm gray modesty panel. A closed drawer. No monitor and no register glow. |
| Column and grid | Ceiling cream (0.58, 0.57, 0.53), roughness 0.65. Seams (0.15, 0.16, 0.16), roughness 0.76 | Painted wrap and a 2×4 ft acoustic tile in a dark T-bar. Keep the 100 cm module. Sprinkler heads at a few intersections, unlit. Square diffusers only where an egress spot already hangs. |
| Kids fill | Six point lights, `BSW_KIDS_CEILING_FILL_*`, z 930, 8 cd | Leave them. They are visual fill under the low grid. |
| Route spots | `BSW_EGRESS_*`, 8 cd, aimed at the floor. Spots inside the kids footprint were moved to z 1050. | Leave the count, the aim, and the candela. Exit faces and neon do not add a light actor. |
| Canopy mirror | `M_BSW_SmokedMirror`, (0.16, 0.20, 0.22), metallic 0.92, roughness 0.16 | Segmented dark mirror, the soffit in `07-mirrored-atrium.jpg`. |
| Canopy frame | `M_BSW_DarkChrome` | The grid between those panes. |
| Canopy tubes | `BSW_CANOPY_NEON_*`. West and north use `M_BSW_NeonBlue`. East and south use `M_BSW_NeonAmber`. Emissive only, z 1447. | Keep. They are the colored edge of the mirror island in `06-apparel-neon-ceiling.webp`, not department names. |

Coral for S3 is the one new emissive: a warm tube in the family of
`M_BSW_NeonAmber`, dimmer than a sales-floor neon, with no accompanying
light component. Blue, violet, magenta, and occasional red remain the
store's neon set in `docs/THE-STORE.md`. The children's tube is that
occasional warm accent, used once.

## The well

`10-escalator-glass-ceiling.jpg` shows the escalator atrium from inside.
`13-mall-glass-entrance.jpg` shows its exterior glass entrance, while
`14-boscovs-glass-frontage.jpg` shows the separate anchor-store frontage.
Chris clarified that the chandelier belongs in the escalator atrium and that
the glass admits the outside at entrances. These are two exterior glazing
references, not evidence of a skylight cut through the store roof.

The art escalator keeps the saved slope footprints. Black steps, a glass
balustrade, and a burgundy fascia. The numeral in S6 sits on that fascia.
The slopes stay the collision.

The chandelier is one non-colliding actor at the well center (0, 0). Clear
glass rods in a vertical stack, roughness under 0.1, no lamp inside the
fixture. Highest rods just under the mirror at z 1472. Lowest rods near
z 1100, so the upper floor looks up into them and the lower floor sees them
through the void. It is a landmark, not a light source. A weak emissive in
the glass is enough for the emergency spots to catch it.

The smoked-mirror grid and `ANCHOR_FULL_ROOF` stay intact. The 24 canopy
panes are interior mirror, not exterior windows. Do not aim a light from
above the opaque roof or turn any mirror pane into a skylight.

The south mall entrance is the light path, and it is already saved on
`LLK_Boswells_ApparelPass`. `ROUTE_SOUTH_HEADER` is gone.
`ROUTE_SOUTH_LINTEL` keeps the roof joint at x −900..900, y −2720..−2680,
z 1468..1500. `MALL_GLASS_ENTRANCE` is a 4 cm face at x −900..900,
y −2880..−2876, z 280..1468, with `MALL_GLASS_RETURN_W` and `_E` closing
the 160 cm projection. Teal mullions sit on that outer face. The night
grille is unchanged at z 0..280. `ANCHOR_UPPER_SOUTH` is still one solid
slab. No light component was added. Photo 14 is not this bay: the verifier
requires that no `LOT_GLASS_` actor exist.

`Saved/Temp/verify_boswells_mall_glass_entrance.json` is the trace record.
From the inner face (0, −2670) to (0, 0), the lines at z 1120, 1280, and
1400 hit nothing, so they reach the rod stack under the mirror. From
(0, −3000, 1280) the first hit is `MALL_GLASS_ENTRANCE`. An eye at
(0, −1500, 814.5) reaches that glass and the rods. The low opening still
dies in the upper floor: (0, −2680, 280) to (0, 0, 1100) hits
`ANCHOR_UPPER_SOUTH`. A vertical line from (0, 0, 2000) to (0, 0, 1200)
hits `ANCHOR_FULL_ROOF`. The grille trace at z 100 still hits
`ROUTE_MALL_GRILLE_22`. Exposure is 0.5/0.5. The 31 egress spots and 6
kids fills are still 8 cd.

Any later storm spill stays visual-only, outside `LLKLightField`, and does
not move the exposure volume or those 8 cd lights. Fixed exposure will not
brighten this glass on its own.

## What the other photographs are for

| Frames | Use on this route |
| --- | --- |
| `11`, `12` | Short white racks, sage set back from the tile, plain column, hanging sales-desk plaque, low grid. |
| `01`, `03` | A column can carry a department word. Use the habit, not the men's copy, the wood capitals, or the dark stone floor. |
| `04` | A department word is a horizon, with a colored tube near it. Toys get that treatment when toys have a socket. |
| `06`, `09` | Mirror islands and a red/blue tube horizon. Already the canopy edge. Do not repeat that island over the sage. |
| `07` | The mirrored soffit around the well. |
| `02`, `05`, `08` | Density, a glass case, and a pale hanging plaque. Brand walls, candy bins, and ceiling screens are other departments or later renovations. |

## Done when

A person at the built eye height can stand at the east edge of either carpet
island, read `KIDS` or the column's `BABIES`, see `sales desk` over the
desk, and see the directory and the rod chandelier toward the well. `SALE`
does not resolve under the route spots. No new collision enters the reserved
aisles or the escalator slopes. The map's exposure lock and the 8 cd route
lights are unchanged.
