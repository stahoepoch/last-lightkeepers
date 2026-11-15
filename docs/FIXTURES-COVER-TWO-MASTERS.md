# Three fixtures that are actually cover

> **Updated 2026-11-14, same day.** A third master was added after the first
> two: `SM_BSW_WallMerchBay`, for the findings' second recommendation —
> *"give the floor more than one column. One dependable blocker in a
> department this size means one hiding place."* The file keeps its name so
> links to it do not rot.

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

**Three kinds of cover, and each one costs the player something different.**
That is the point of having three rather than three sizes of the same idea.

| | `SM_BSW_FoldedGoodsTable` | `SM_BSW_ServiceCounter` | `SM_BSW_WallMerchBay` |
| --- | --- | --- | --- |
| Footprint | **160 × 90 cm** | **186 × 71 cm** at the top | **240 × 74 cm** |
| Height | **80 cm** | **102 cm** | **190 cm** |
| Period | 30-inch soft-goods table | 40-inch service counter | double-sided merchandise gondola |
| Hides a crouched keeper | **yes** | **yes** | **yes** |
| Hides a standing keeper | **no — this is the point** | **yes** | **yes** |
| Keeper can see over it | yes | **yes, by 50 cm** | **no** |
| The trade | you must duck, and you can still watch | you stand, hide and watch — the best of the three | completely hidden and completely blind |
| Parts | toe recess 0–10, painted plinth 10–64, laminate top 64–72, four folded stacks 72–80 | toe recess 0–10, carcass 10–96, painted apron 88–96, laminate top 96–102 | plinth 0–10, carcass 10–190, four shelf bands per face, faced stock on both sides |
| Materials | `white_steel`, `steel`, `oat`, and `denim` / `cream` / `burgundy` / `mint` stock | `charcoal`, `cream`, `white_steel`, `dark_wood` | `charcoal`, `white_steel`, `steel`, six-colour faced stock |

**The bay is furniture, not structure**, which is the whole reason to add it.
`BSW_KIDS_CEILING_SALES_COLUMN` is the only dependable blocker in the
department today, and a column is where the building holds up the roof rather
than where the stealth needs a blocker. A gondola can be placed where the play
wants it.

**It is 240 cm long on purpose.** A long run breaks a sightline and makes the
player pick an end to go round, which is a decision. A narrow blocker is a
pillar you strafe past.

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
| `assets/models/boswells/SM_BSW_WallMerchBay.obj` | Source mesh, same convention |
| `assets/models/boswells/BoswellsFixtures.mtl` | The existing shared palette, unchanged |
| `assets/models/boswells/import_fixtures.py` | Editor Python: imports both, boxes them, sets the collision profile, builds four material instances. **Opens no map** |

## The checks, and which have actually run

| Check | Where | Run? |
| --- | --- | --- |
| Generator refuses to write a fixture that breaks the cover rule | `make_fixture_kit.py` `cover_check` | **yes** — `table 0-80 (duck), counter 0-102 (stand and watch), bay 0-190 (hidden and blind)` |
| The committed OBJs are still those heights | `tests/asset-deps.test.mjs` | **yes** — 47/47, inside 697 checks green |
| OBJ vertex bounds, read independently before Unreal saw them | MCP session control read, 2026-11-14 | **yes** — 160×90×80, 186×71×102, 240×74×190, all on z=0 |
| Import lands at the authored height | `import_fixtures.py` | **yes** — 1:1, no scale factor, heights exact |
| ~~Simple box collision exists~~ | ~~same~~ | **yes, and it proves nothing.** `box_elems` counts a struct |
| ~~Default profile blocks `ECC_Visibility`~~ | ~~same~~ | **yes, and it proves nothing.** `collision_profile_name` reads a string |
| **A trace fired at a placed fixture fails to see through it** | `place_fixtures.py` probes, commandlet, 2026-11-15 | **RUN, AND TWO OF THREE FAILED.** Counter blocks at 40 and 88 and is clear at 152, as designed. **Table and bay are missed at every height**, at every position, on simple and complex traces alike |

> **THE CAUSE NAMED BELOW IS WITHDRAWN, same day. It was Nanite, and the
> fallback I blamed was never the fault.** Varying one thing at a time in a
> full editor: Nanite on, clear; Nanite off with the collider *untouched*,
> blocks; Nanite off and then rebuilt with
> `StaticMeshEditorSubsystem.add_simple_collisions`, clear again. So the
> hand-written `KBoxElem` worked, and the engine's own build path is the one
> that produces a box which does not collide on these meshes. `962d506` flips
> the Nanite flag on the two failing meshes and nothing else, and verifies by
> trace with the counter as a control in the same process.
>
> **And Nanite is not a sufficient explanation either.** All three meshes
> carry identical Nanite settings - enabled, AUTO fallback, fallback percent
> 1.0 - and the counter blocks with Nanite *on*. So "Nanite breaks collision"
> cannot be the mechanism; something about these two meshes interacts with it.
> The difference is in built data and it is **unexplained**. `962d506` is a
> measured fix whose mechanism is not understood, which is worth having and
> is not the same as a diagnosis.
>
> **Kept below rather than deleted**, because the reasoning that produced a
> confident wrong cause is the part worth being able to re-read. It was built
> out of a true observation - the asset-level assert proves nothing - and a
> plausible mechanism that was never tested by varying it. Exactly the fault
> it was written to describe, one level further up again.

> **CORRECTION, 2026-11-15. Two of these three fixtures are not cover, and
> the two struck rows above are why nobody noticed.**
>
> A Visibility trace at placed instances finds `SM_BSW_FoldedGoodsTable` and
> `SM_BSW_WallMerchBay` missed at 40, 88 and 152 cm, at six positions across
> the department, on both simple and complex traces — including a trace fired
> straight down from 400 cm. `SM_BSW_ServiceCounter` blocks correctly
> everywhere, which is what proves the instrument was live.
>
> Both failing assets still pass every asset-level check today: `box_elems` 1,
> `CTF_USE_SIMPLE_AND_COMPLEX`, `BlockAll`, `QUERY_AND_PHYSICS`, correct
> colliding bounds. **The editor-side struct says cover and the physics scene
> disagrees.** The cause is that `import_fixtures.py` wrote a `KBoxElem`
> directly onto the body setup when `StaticMeshEditorSubsystem` was absent,
> and the engine never cooked physics data for it.
>
> **This is the `add_simple_collisions` returns -1 fault one level up: from an
> unchecked return to an unverified claim.** The first version asserted
> against collision it had never added. The second asserted against collision
> that existed as data and not as physics. Both times the assert was the thing
> that was too weak, and both times it read as rigour — the row below this one
> said *"a trace at a placed fixture is still the only thing that proves cover
> in play"*, and that sentence was describing a live defect rather than
> guarding against a future one.
>
> No headless repair exists, measured rather than assumed: `BodySetup` exposes
> no `InvalidatePhysicsData` and no `CreatePhysicsMeshes`, rewriting the
> element through the property setter leaves it clear, and
> `CTF_USE_COMPLEX_AS_SIMPLE` does not help because the complex trace is clear
> too. `StaticMeshEditorSubsystem.add_simple_collisions` is the engine's own
> build path and it is editor-only. **The repair needs the editor open; the
> placement needs it closed.**
>
> The hand-built fallback is deleted rather than fixed, because a fallback
> that cannot work converts *"this cannot run here"* into *"this ran and
> produced cover"*. `tests/asset-deps.test.mjs` now fails if it returns.

**Imported and verified 2026-11-14 by the MCP session, committed there as
`1ee88b8`.** ~~Verified~~ — **that word is withdrawn; see the correction
above.** The import ran and the assets exist; what was verified was their
configuration, not their collision. Four faults in `import_fixtures.py` had to be patched for it to
run at all — the source path walked above the repo, `StaticMeshEditorSubsystem`
is None outside a real editor, `BodyInstance` has no
`set_collision_profile_name` in Python, and `EditorStaticMeshLibrary.add_simple_collisions`
**returns -1 on failure without raising**, so the original would have carried
on and asserted against collision it never added. All four are fixed in the
committed script. The depth of the bay is **74 cm, not 70**: the shelf bands
stand 2 cm proud of the carcass on each face, and the bounds are what counts.

**State it at the level it was proven.** This is asset-level collision: the
mesh carries a box and defaults to a profile that blocks the channel. Nothing
is placed, so nothing has been sighted through. A trace at a placed fixture in
PIE is still the only thing that proves cover in play.

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
