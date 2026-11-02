# Asset intake — bringing outside work into Last Lightkeepers

This is the durable path for Marketplace, Fab and Epic sample assets. It keeps
the playable project small, reviewable and reproducible while still letting us
use bought or free licensed work.

## The rule

**Never copy `.uasset`, `.umap`, `.uexp` or `.ubulk` files in Explorer.**
Unreal's **Asset Actions → Migrate** command is the import mechanism: it
collects dependency closure and writes assets into the target project's
`Content/` directory in a form the Asset Registry understands.

One person or agent owns each binary asset while it changes. An intake project
is a staging area, not a shared authoring space.

## Two routes

| Asset state | Route |
| --- | --- |
| Compatible with UE 5.8 | Add it through Epic Launcher to an intake project, inspect it, then migrate only the selected assets into `/Game/ThirdParty/<PackName>/`. A tiny, already-reviewed pack may be added directly to Last Lightkeepers only when its entire contents are intended to ship. |
| Version-locked or sample-only | Add it to an **engine-matched intake project** outside this repository, open it there, then migrate selected assets and dependencies into Last Lightkeepers. |

The project root `C:/Users/chris/Documents` is registered with Unreal so
compatible assets now appear in Epic Launcher's **Add to Project** picker.

## Intake project convention

Create one disposable staging project per engine line, outside the game repo:

```
C:/Users/chris/Documents/LLK_AssetIntake_5_8/
C:/Users/chris/Documents/LLK_AssetIntake_5_6/   # only if an asset is locked to 5.6
```

Do not commit an intake project. Its job is to receive vendor content, let us
inspect it, and provide Unreal's migration command.

## Every import follows this checklist

1. **Name the need first.** Example: `first-person arms and a lantern hold
   pose for Toy Department Escape`. Do not import an entire pack because it
   looks interesting.
2. **Record licence and source.** Keep the licence/receipt outside the cooked
   build under `assets/<kind>/licences/`; add a row to `assets/MANIFEST.md`
   only when game content starts referencing the pack.
3. **Inspect in the intake project.** Check Unreal version, required plugins,
   skeleton compatibility, scale, collisions, material complexity, and whether
   the content is genuinely useful.
4. **Migrate a named selection.** In the Content Browser select the assets,
   choose **Asset Actions → Migrate**, review the dependency report, and choose
   this repository's `Content/` directory. Put vendor work under
   `/Game/ThirdParty/<PackName>/`; never scatter it into `/Game/`.
5. **Exclude demo maps by default.** Demo levels bring their own lighting,
   post-process volumes, maps and assumptions. Migrate individual meshes,
   materials, textures, animations or Blueprints only when they are explicitly
   needed.
6. **Make plugins explicit.** A required plugin is recorded in
   `LastLightkeepers.uproject` with its intended enabled state. Never rely on
   an engine-default plugin silently loading.
7. **Commit in one isolated change.** One pack/selection, its manifest row,
   licence record and any required code integration. Check Git LFS before
   pushing; third-party content can be large and LFS history is durable.

## First intake: Game Animation Sample

The first vertical-slice need is not a full character pack. It is a temporary
first-person arm rig with a reliable hand socket/hold pose so the lantern is
visibly carried instead of camera-mounted.

Use the Game Animation Sample only as a source for the smallest viable set:

- a UE5 mannequin-compatible skeletal mesh or first-person arms;
- its skeleton and required physics/material assets;
- one idle/locomotion pose suitable for holding an object; and
- the socket or bone reference that lets C++ attach the lantern at hand height.

Do not migrate its showcase maps, cameras, UI, GameMode, input setup or its
whole animation database. The lantern remains gameplay logic in C++; imported
assets supply only the body and animation presentation.

## Definition of done for an imported pack

- The asset loads in a fresh editor restart.
- The packaged/gameplay path references it from a documented `/Game/ThirdParty`
  location.
- Its licence and source are recorded.
- Required plugins are explicit.
- Demo lighting and maps are absent from gameplay maps.
- Git LFS shows the assets as tracked, and the import has its own commit.
