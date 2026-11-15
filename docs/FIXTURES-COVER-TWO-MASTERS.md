# Two fixtures that are actually cover

Status: **source meshes and import script delivered, 2026-11-14. Not imported,
not placed, no map touched.** `LLK_Boswells_ApparelPass.umap` is Codex's and
was not opened. Asset lane only, per Chris's instruction the same day.

## The three numbers everything here is sized against

From `FINDINGS-KIDS-FLOOR-STEALTH.md`, measured in the live editor rather than
assumed:

| | Height above floor |
| --- | --- |
| Sight is traced at, standing | **88 cm** (capsule centre, not the eye) |
| Sight is traced at, crouched | **40 cm** |
| The keeper actually looks out from | **152 cm** |

And the trace channel is **`ECC_Visibility`**, not `ECC_WorldStatic` — *"a
fixture that does not block that channel is not cover no matter how solid it
looks."* That sentence is the reason the import script exists at all.

The children's rolling racks block only **88–130 cm**, so crouching behind one
drops the traced point *below* the cover and makes the keeper more visible.
**The racks are not changed and should not be.** They are honest modelling of
an open rack on castors, and they should keep reading as open. What the floor
lacked was anything solid to the floor.

## The two masters

| | `SM_BSW_FoldedGoodsTable` | `SM_BSW_ServiceCounter` |
| --- | --- | --- |
| Footprint | **160 × 90 cm** | **186 × 71 cm** at the top, 180 × 57 carcass |
| Height | **80 cm** | **102 cm** |
| Period | 30-inch soft-goods table | 40-inch service counter |
| Hides a crouched keeper | **yes** | **yes** |
| Hides a standing keeper | **no — this is the point** | **yes** |
| Keeper can see over it | yes | **yes, by 50 cm** |
| Parts | toe recess 0–10, painted plinth 10–64, laminate top 64–72, four folded stacks 72–80 | toe recess 0–10, carcass 10–96, painted apron 88–96, laminate top 96–102 |
| Materials | `white_steel`, `steel`, `oat`, and `denim` / `cream` / `burgundy` / `mint` stock | `charcoal`, `cream`, `white_steel`, `dark_wood` |

**The table is the fixture that teaches crouching.** It is deliberately below
chest height along its whole silhouette, so nobody can mistake it for
something to stand behind — that misreading is precisely what the racks cost.
Solid from the floor up, so there is no band to drop beneath.

**The counter is cover you can watch over.** At 102 cm it blocks the chest at
88 and sits fifty centimetres under the eye at 152. The findings found that
affordance by accident in the racks — *"you can stand behind a rack, see the
machine over it, and remain unseen… worth making deliberate before someone
fixes the trace to use eye height and deletes it."* This is that, on purpose.

## Where the files are

| File | What it is |
| --- | --- |
| `assets/models/boswells/make_fixture_kit.py` | The generator. **Extended, not replaced** — the two fixtures join Codex's existing five rather than starting a second kit with a second palette and a second `.mtl` |
| `assets/models/boswells/SM_BSW_FoldedGoodsTable.obj` | Source mesh, centimetres, Z up, origin at floor centre |
| `assets/models/boswells/SM_BSW_ServiceCounter.obj` | Source mesh, same convention |
| `assets/models/boswells/BoswellsFixtures.mtl` | The existing shared palette, unchanged |
| `assets/models/boswells/import_fixtures.py` | Editor Python: imports both, boxes them, sets the collision profile, builds four material instances. **Opens no map** |

## The checks, and which have actually run

| Check | Where | Run? |
| --- | --- | --- |
| Generator refuses to write a fixture that breaks the cover rule | `make_fixture_kit.py` `cover_check` | **yes** — `cover ok: table 0-80, counter 0-102` |
| The committed OBJs are still those heights | `tests/asset-deps.test.mjs` | **yes** — 15/15, inside 658 checks green |
| Import lands at the authored height | `import_fixtures.py` `assert_it_is_cover` | **no — needs an editor** |
| Simple box collision exists | same | **no** |
| Default profile blocks `ECC_Visibility` | same | **no** |

The generator's guarantee only covers the run that produced the file, which is
why the test re-checks the committed mesh: these are committed art, not build
output, and somebody will hand-edit one eventually.

**The import half is unproven.** There is no editor on the bridge that wrote
this. A clean run of `import_fixtures.py` is the first evidence; this document
is not.

## What is deliberately not here

**No placement.** Where these go is a level decision in Codex's map, and two
writers in one `.umap` is how binary work gets lost.

**No texture set.** The materials read as laminate, painted metal and folded
stock through base colour, roughness and metallic on parameterised instances.
Making them read as *worn* laminate under a lantern at two candelas needs
authored maps, and that is the next asset pass rather than something to fake
with a tint.

**No change to the racks**, no fixture heights touched elsewhere, and no row
in `DECISIONS.md` — the cover rule these are built against is still RF5 on
`RULINGS-OPEN.md`, open and Chris's.
