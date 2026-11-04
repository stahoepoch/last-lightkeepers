# Marketplace art is a dependency, and the repo was 19 GB

> **Status: DONE 2026-11-03.** Written while importing the Warehouse pack on
> Chris's *"import and make sure we don't have too much space."* The space
> problem turned out not to be the pack.

## The 19 GB

`.git` was **19 GB**. The project's own `Content/` is 42 MB.

`.git/lfs` held **6,622 objects**, of which the current tree referenced **220**,
the largest being 742 KB. The four biggest objects on disk were **3.2 GB,
2.6 GB, 1.2 GB and 1.2 GB**, and `git lfs ls-files --all` found no commit in
any branch referencing them. They were orphaned local cache, not history.

```
git lfs prune
  6622 local objects, 220 retained
  Deleting objects: 100% (6402/6402)
```

**19 GB → 44 MB**, with `.git` as a whole at 130 MB. Nothing in history was
touched; LFS prune only removes local copies of objects no commit needs. Worth
re-running occasionally — editor saves of `.umap` and `__ExternalActors__`
churn LFS objects fast, and this is where they pile up.

## What was imported, and what it costs

From the full pack in the Fab vault — **365 meshes, 1.8 GB** — against a
project that had 3 meshes from it:

| Folder | Assets | On disk |
| --- | --- | --- |
| `Meshes/{Racking, Lamps, Dock, Crates, Cardboard_Boxes, Pallets, Handling_Equipment, Fire_Fighting_Equipment}` | 101 meshes | **21 MB** |
| `Materials/` (all) | 247 | 4.9 MB |
| `Blueprints/` (all) | 23 | 2.7 MB |
| `Textures/` (selected sets only) | 26 | **302 MB** |

**Meshes are nearly free and textures are the entire cost.** 101 meshes cost
21 MB; 26 textures cost 302 MB, with single normal maps at 25–28 MB. The
pack's whole 1.8 GB is 1.7 GB of textures.

So the rule for any further import from this pack: **take meshes, materials
and blueprints freely; take texture sets one at a time and only when something
in the map actually uses them.**

## Why it is gitignored

`/Content/Warehouse_Big_Pack/` is in `.gitignore`.

- It is licensed art, re-downloadable from the Fab library that already holds
  it. Versioning it buys nothing a download does not.
- 302 MB of 4K texture sets, for a greybox store the fixture-light contract
  caps at 2 candelas. Nobody will ever see a 4K crate normal in that dark.
- Committing it would undo a third of the 19 GB reclaim above and make every
  clone and CI run pull 330 MB. GitHub's free LFS allowance is 1 GB of storage
  and 1 GB of bandwidth a month.

## The risk that creates, and what covers it

This is exactly the trap `HANDOFFS.md` row X3 records against `Content/Data/`:
a gitignored dependency goes missing and the thing depending on it reports
success anyway. **Unreal does not error on a missing mesh.** It renders nothing
and carries on, so a clone without this pack opens Boswell's with invisible
racking and no dock, and nothing explains why.

`tests/asset-deps.test.mjs` draws the distinction that matters:

- **pack wholly absent** — reports plainly, exits 0, and names the Fab listing.
  A fresh clone is not broken; it just does not have the art.
- **pack partially present** — **fails**, because that is a half-finished copy
  or a corrupt import, and it is the case that costs an evening.

Proved in all three states before it was committed: green at 8/8, the absent
case exiting 0 with instructions, the partial case exiting 1.

## Still in the library, not imported

- **Game Animation Sample 5.8 — 7 GB, 3,890 animations.** Epic's motion-matching
  locomotion set. Relevant because the live complaint is that *everything moves
  too fast*: the player walks at 448 cm/s, which is 3.2x human walking speed.
  This sample is the reference for what tuned human locomotion feels like. It
  is a 7 GB project, so the question is which parts to take, not whether to
  take it whole. **Not a decision to make while a 200 cm aisle gap is the
  newest thing in the map.**
- **Concrete Shelter / Bunker Constructor — 12 GB.** Modular concrete. Nothing
  in a department store wants it today; plausible for back-of-house, service
  corridors or the dock exterior later.
- **The rest of the Warehouse pack**: 264 meshes not taken, including
  `Building/Walls` (26), `Building/Roof` (18), `Air_Conditioning` (13, with
  lit and unlit ceiling-panel variants), `Pipes`, `Stairway`, `Electric`,
  `Drums+Jerrycans`. The roof and ceiling-panel sets are the obvious next
  candidates, because Boswell's currently has **three** ceiling surfaces at
  z 586, 604 and 614 and no agreement about which is real.
