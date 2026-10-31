# Prerequisite check — the authored-map seam

Read-only pass by Claude, 2026-10-31, ahead of Codex's seam work (relic actor
plus a DockBay game path that does not spawn the harness over it). **Source and
asset reads only. Nothing was edited, and `LLKTestGameMode.cpp`, the map and
the config were not touched.** Asset facts come from the external actor files
on disk; the editor was closed and anything needing it is marked as such.

**Headline: three of the four prerequisites are already in the map.** The
blocker is not missing actors — it is the harness being built on top of them.

---

## 1. Navmesh and patrol tags — both present

| | |
| --- | --- |
| `LLK_Encounter_NavBounds` | `NavMeshBoundsVolume`, `Brush_Add`, has a `RelativeScale3D` |
| `RecastNavMesh-Default` | present, with `bAllowWorldPartitionedNavMesh` |
| `LLK_Patrol_1`, `_2`, `_3` | bare `AActor`s, each tagged `LLK.Patrol` |

The tag matches exactly: `ALLKSweeper::PatrolPointTag` is
`FName(TEXT("LLK.Patrol"))` (`LLKSweeper.cpp:20`), and `ALLKSweeper` looks for
it with `ActorHasTag(PatrolPointTag)` at line 232. So **row 78's failure mode
does not apply to the authored map.** A Sweeper placed there has both a surface
to path on and a route to walk.

**What the editor would still have to confirm:** that the bounds volume's
extents actually cover the bay, and that the navmesh has been *built* rather
than merely configured. The volume and the Recast actor exist; whether the tile
data is current is not readable from the `.uasset` with any confidence, and I
am not going to infer it.

**The harness is a different story, and the GameMode already says so.** Its
floor and shelving are spawned at runtime, so no baked navmesh can describe
them. `Config/DefaultEngine.ini` declares **no `[/Script/NavigationSystem...]`
section and no `RuntimeGeneration` setting at all** — so navmesh generation is
static. This matters at the seam for a reason the GameMode's own comment does
not reach: while the harness is still built over the authored map, the authored
navmesh is correct for the authored floor and **blind to every harness counter,
case and shelf standing on it**. A Sweeper would path straight through them.
That is an argument for the seam, not a bug to fix first.

---

## 2. The extraction box — present, and the id matches

`LLK_LoadingDock` is a real `ALLKExtractionZone` in the persistent level, and
its `ExtractionPointId` property is serialised with the value `LoadingDock`.

`data/loot.json` authors `extraction.LoadingDock` and `extraction.FreightLift`
(lines 64 and 70), checked by `tests/balance.test.mjs:258`. **So the authored
id matches the data.** `FreightLift` has no zone in DockBay, which is correct
for one bay and worth knowing before anyone calls it from the console:
`LLK.CallExtraction FreightLift` has nothing to arrive at.

**The duplicate everyone would expect is already guarded.**
`LLKTestGameMode.cpp:262` wraps its own dock spawn in
`if (!WorldAlreadyHas(ALLKExtractionZone::StaticClass()))`, so on the authored
map the harness sees `LLK_LoadingDock` and spawns nothing. Two zones claiming
`LoadingDock` is not a hazard at this seam.

**Extents need the editor.** The zone's box half-extents are a serialised
component property; the harness sets 7 × 7 × 4 studs on its own spawn and its
comment says *"the real extents come with the authored dock"*. Whether the
authored one carries real extents or a default box is a read I cannot make
reliably from the file.

---

## 3. The uncommitted lines in `LLKTestGameMode.cpp` — not navmesh lines

The 108 uncommitted insertions are **three unrelated changes**, and calling
them navmesh work has been part of why nobody resolved them. Read only; the
file is untouched.

**a. A console variable, and a comment that came loose.**
`CVarHarnessSweeper` (`LLK.Harness.Sweeper`, default `0`) is declared inside
the existing anonymous namespace. In doing so the edit left
`UStaticMesh* LoadEngineCube()` un-indented with its body still indented, and
**the pre-existing doc comment that described the cube loader** — the one about
loading *"invisible-but-solid rather than failing"* — now sits above the new
CVar's own comment block, documenting the wrong thing. It compiles. It reads as
a mistake, and it is the kind that survives for months.

**b. The harness extraction zone** (≈50 lines), guarded as described above.
Spawns at `Floor.ExtractionPoint`, id `LoadingDock`, 7 × 7 × 4 studs, raised by
its own half-height.

**c. The Sweeper and its patrol markers** (≈40 lines), behind the CVar at
default off.

**The one real conflict with an authored-map path is in (c), and it is
unguarded.** The zone spawn checks `WorldAlreadyHas`; the patrol markers do
not. With `LLK.Harness.Sweeper 1` on the authored map, the harness spawns a
fresh tagged `AActor` at **every** `EnemySpawnPoints` entry, and `ALLKSweeper`
finds them by tag alongside `LLK_Patrol_1`–`_3`. The result is one route made
of authored positions and harness positions mixed together — which is row 63's
rule about machines appearing somewhere a player can learn, broken quietly,
because nothing errors and the Sweeper simply walks somewhere nobody designed.

Everything else in the diff is additive and world-local. Nothing in it writes
to `Content/`.

---

## 4. Test surface — the blind spot is total, and it is not where the names suggest

**No automation test loads or runs a world. Not one.** I grepped all 36 test
files for `UWorld`, `GetWorld`, `SpawnActor`, `LoadMap` and
`AutomationOpenMap`: the only hit is a *comment* in `LLKStormChargeTest.cpp`.

So the honest answer to "which would silently pass while covering nothing" is
that **all 44 would pass, and none of them covered the harness world either.**
The seam cannot break them because they were never looking. What changes is
that three of them currently *read* as map coverage and will read that way more
strongly once a real map is the thing being played:

- **`LLK.Harness.MapReadiness`** — the one whose name promises the most. Its
  fixture comment says it plainly: *"Built here rather than loaded."* It
  constructs a synthetic ready-map struct and breaks one field at a time. It
  tests the judgement, correctly and well, and asserts nothing whatsoever about
  `LLK_DockBay`.
- **`LLK.Harness.StoreFloorPlan`** — arithmetic over `FLLKStoreFloorPlan`'s
  authored numbers (sightline limits, counter heights, ramp angle). Every one
  of those numbers describes the *harness* layout. After the seam it is
  checking the geometry of a scaffold nobody plays in, and it will still pass.
- **`LLK.Harness.CanFail`** and **`LLK.Harness.Runs`** — same shape.

On the node side, two of the thirteen suites are about maps and neither is
about this one:

- **`tests/map-readiness.test.mjs`** (72 checks) is a prose-and-source
  agreement check between `LLKMapReadiness.h` and `docs/MAP-READINESS.md`. A
  substring scan, by design. It would pass against an empty map.
- **`tests/map-contract.test.mjs`** (6 checks) reads
  **`src/map/GalleriaBuilder.luau`** — the frozen Roblox prototype, last
  touched `1dbcdd8` on 2026-10-22. It guards the Relay Annex of a build that is
  no longer the game. It will pass forever regardless of what `LLK_DockBay`
  does.

### What the coverage gap actually is

Requirement 15 / `GeometryServerOnly` is still absent from
`LLKMapReadiness.h`, which still has fourteen — so **nothing would notice if
the next map went back to spawning its floor from a GameMode.** That is Claude's
and unlanded, and the seam is exactly the change that makes it matter. Row 72
is about this gap by name.

Two things the authored map lacks that are worth knowing before coverage is
written:

- **Zero relics.** None of the 168 external actors carries a
  `ULLKRelicComponent`, and outside tests the only place one is ever
  constructed is `LLKStoreFloorBuilder.cpp`. This is the seam's first job.
- **`LLK.TestLane.Open` and `LLK.TestLane.Blocked` are both present** (two
  actors each), so `LLK.CheckMap`'s sightline sweep has something to run
  against in the real map. Note the blocked pair is 900 cm = 32.1 studs, past
  Open's 30 — sized for the sightline check and, as
  `docs/FINDINGS-DOCKBAY-LANTERN-PASS.md` section 2 records, too long to
  demonstrate anything about light.

No light probe actors and no relic tags, which is consistent with both.
