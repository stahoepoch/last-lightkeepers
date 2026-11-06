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
that **all of them would pass, and none of them covered the harness world
either.** (Said as "all 44" when this was written; the suite is 33 — see
`docs/TOOLING-TRAPS.md`. The count was wrong; the blind spot was not.)
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
- ~~**`tests/map-contract.test.mjs`** (6 checks) reads
  **`src/map/GalleriaBuilder.luau`**.~~ **Resolved by deletion, noticed
  2026-11-05.** Both files are gone: the test went in `9675b61` (*"draw the
  engine boundary: npm test is the Unreal game again"*) and the builder with
  `src/` in `947a84b`. The concern was real and is now moot - it guarded the
  Relay Annex of a build that is no longer the game, and would have passed
  forever regardless of what `LLK_DockBay` did. **Worth keeping as the record
  of what a guard pointing at the wrong world looks like**, since that is the
  failure this page exists to describe and the suite has since grown guards of
  the same shape pointed at the right one.

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

---

# Part 2 — the editor-state questions, answered from the run logs instead

Second read-only pass by Claude, 2026-10-31, after the seam landed
(`70d2404`). **Nothing was edited. `Content/` is clean, 0 lines.**

## Why this did not go through the editor

Two things, both editor-side, and I took neither:

- **The MCP bridge is not serving.** `127.0.0.1:8000` refuses the connection.
  The plugin itself is healthy — the current log shows it registering 52
  toolsets at 19:58:36 — but the HTTP transport is started by hand:
  `ModelContextProtocol.StartServer` sits in `Saved/Config/ConsoleHistory.ini`.
  It has always been a per-session step, which is why it looked like a
  connection that had been verified once and then broke.
- **The editor is not on the bay.** `UWorld::...Initialize : World =
  /Temp/Untitled_1` — still the throwaway Open World template, as on 10-30. So
  a working bridge this session would have read the wrong world, and said so
  confidently.

What replaced it: four PIE runs today left logs, and a runtime log is a better
witness than a `.uasset` parse. Everything below is a quoted log line, a file
size, or a count.

## 1. The navmesh — not built, as far as play is concerned

Three independent facts, agreeing:

- **Nothing on disk carries tile data.** The `RecastNavMesh` external actor
  package is **3,879 bytes**, and there are **zero `NavigationDataChunkActor`
  packages** anywhere under `Content/__ExternalActors__/LLK_DockBay`.
- **What was saved gets thrown away on load.** At 18:48:02 and again at
  19:23:58: `LogNavigation: Warning: Recreating dtNavMesh instance
  (RecastNavMesh_UAID_...-Default) due mismatch in number of bytes required to
  store serialized maxTiles (serialized: 3, 2 bits) vs calculated required
  (54, 6 bits)`. Recast discards the serialized mesh and creates an empty one.
- **Play then finds nothing.** Every PIE start today — 18:50:49, 19:25:32,
  19:25:35: `LogCrowdFollowing: Warning: Unable to find RecastNavMesh instance
  while trying to create UCrowdManager instance`.

And nothing rebuilds it: `Config/DefaultEngine.ini` still declares no
`NavigationSystem` section and no `RuntimeGeneration`, so generation is static.

**So the honest answer is worse than "configured, not built".** It was built
once, for an area of three tiles, and it is now stale enough that the engine
refuses it and starts empty. A rebuild and a save fixes it; a rebuild alone
does not survive the editor closing.

## 2. The nav bounds — big enough to matter, exact fit still unread

`calculated required (54, 6 bits)` is derived from the current bounds volume
and the tile size, so the volume demands **54 tiles**, not the single tile a
default 200-unit brush would want. The volume is real and it is large.

What that does not settle is whether those 54 tiles of area sit over the
*playable* floor rather than out across the landscape. That is a geometry read
and it still needs the editor. Note also the gap it implies: 3 tiles were built
against a volume that now wants 54, so whatever was built covered roughly an
eighteenth of the authored intent.

## 3. The dock box extents — still unread, and a correction to Part 1

The half-extents still need the editor. But the logs turned up something that
matters more, and it **corrects section 2 above.**

Part 1 said the duplicate was "already guarded" and that on the authored map
"the harness sees `LLK_LoadingDock` and spawns nothing." That is not what
happened. At 19:24:07:

> `Warning: LLKExtractionZone_UAID_D843AEC613C11F1003_1827981198 is a second
> zone for LoadingDock - LLKExtractionZone_0 already is. Ignoring the new one`

`LLKExtractionZone_0` is the **harness** spawn. The `UAID` actor is the
**authored** `LLK_LoadingDock`. The harness went first and the authored dock
was rejected as the duplicate — the exact reverse of the guard's intent.
`WorldAlreadyHas` returned false because on a World Partition map the authored
external actor had not streamed in when the GameMode ran.

The seam removes this particular race by not spawning at all, so it is not a
live bug after `70d2404`. It is worth keeping because the guard never worked:
**any question of the form "does this world already have one?" asked from
GameMode init on a partitioned map is asked before the answer exists.**

---

# What would make the first authored-map playtest read as broken

## A. The Sweeper notices nobody, and has not all day

Every run today — 12:53:28, 12:53:42, 15:21:50, 15:23:39, 15:30:34, 19:24:07 —
logs the same line:

> `LLKSweeper_UAID_... awake. Speed 392 cm/s, telegraph 1.0s, notices a dark
> keeper at 0 studs and a lit one at 0.`

`data/enemies.json` authors `aggroRadiusDark: 12` and `aggroRadiusLit: 38`. The
log prints `Def.AggroRadiusDark.Get(0.f)`, so **both optionals are unset in the
row the subsystem read** — while `ULLKEnemyRosterSubsystem: DT_Enemies loaded,
6 families` reports success and the no-def warning never fires. Speed and
telegraph came through; these two did not.

This is not a cosmetic gap. `data/lantern.json`'s `_radiusNote` builds the
whole of row 5 on those two numbers: at radius 30, a lit keeper is noticed from
**8 studs beyond the edge of their own light**, so light reveals value and
attracts danger from the dark you just announced yourself to. That trade is
currently not in the build. Combined with no navmesh, the machine will stand
still and read as unfinished AI rather than as two unpopulated columns.

**Where to look** — stated as the next question, not as a diagnosis.
`DT_Enemies`'s row struct is `FLLKEnemyDef` (`Tools/import_data.py:29`), whose
family-specific fields are `TOptional<float>` UPROPERTYs
(`LLKEnemyDef.h:85`). The shared note of 2026-10-30 verified DT_Enemies' **row
set** against the JSON and did field-by-field comparison only for the four
DataAssets, so these columns have never been compared. The first question is
whether the JSON-to-DataTable import writes `TOptional` properties at all.
Nothing in the test surface would catch it: the automation tests assign
`AggroRadiusDark = 12.f` by hand, and `tests/balance.test.mjs` reads the JSON —
the gap is exactly between the two, which is section 4's shape again.

Not mine to fix: the commandlet and `Tools/**` are Codex's, `Content/` is
Chris's.

## B. An empty bay will look like broken telemetry

With the harness skipped the authored bay has no relics, so the run JSON will
carry `interactAttempts: 0`, `secondsToFirstRelicTaken: -1`, and `Describe`
will print `legibility NOBODY REACHED FOR ANYTHING. No interact was attempted.`

**That is the instrument working as designed** — it refuses to print a clean
zero for a run nobody reached in — and it is better said before the run than
explained after it.

## C. The bay is sixteen meshes on an open-world landscape

The 168 external actors break down as **128 `LandscapeStreamingProxy`, 16
`StaticMeshActor`**, and one each of `DirectionalLight`, `SkyLight`,
`SkyAtmosphere`, `ExponentialHeightFog`, `PostProcessVolume`,
`VolumetricCloud`, `SkySphere`, the nav bounds volume, the dock, the Sweeper, a
`PlayerStart`, the three patrol points and the four test-lane markers.

The lighting actors matter: **the seam does not leave the bay black**, which
was the obvious risk once the harness stopped supplying sun 0.35 / sky 0.10.
But the floor under a "dock bay" is landscape, and sixteen meshes is a sketch.
Zero `SpotLight` or `PointLight` actors, so there is no authored lantern-scale
light to read a relic by even once relics exist.

## D. The last pre-seam run, and one weakness it exposes

`Saved/LLKTelemetry/run-20261031-152532.json`, 15:25 local — after the
instrument landed (`8431529`, 14:25) and before the seam (`70d2404`, 15:58), so
it is the harness store, with relics:

| | |
| --- | --- |
| run | 84.5s, outcome `Unresolved`, extraction never called |
| light | `litFraction` 0.96 — the lantern was on almost the whole run |
| haul | 10 relics taken, **0 secured** |
| legibility | first take at **11.8s**, 22 interact attempts, 0 blocked, 1 abandoned |

M4's first question has an answer for this run: he did not kill his lantern,
not once, for 81 of 84 seconds.

**And the real data exposes a weakness in my own instrument.** 22 attempts, 10
takes, `refusedFraction` **0.0**. Twelve presses were allowed and produced
nothing — out-of-range reaches, which `LEGIBILITY-INSTRUMENT.md` deliberately
records as `None` so they stay visible in the event list instead of being filed
as a misunderstood rule. On synthetic test data that was a footnote. On a real
run it is **the majority of presses**, and `refusedFraction` 0.0 reads as "no
legibility problem" while 55% of what the player did achieved nothing.

The number is not wrong and the decision behind it still holds. It is just not
sufficient alone: the honest companion is a derived count of attempts that
produced no take. Flagging rather than changing — the instrument is landed, the
telemetry follow-ups were closed deliberately, and this is a new decision for
whoever owns it next.

---

## Still unread, and queued

- **The intensity ladder.** The A/B matrix only ever went down the point
  column; spot-at-low-intensity is the untested cell that decides whether the
  whole fix is one number. Needs PIE and temporary lights. **Not attempted** —
  Chris was about to press Play, and two writers in one viewport is not worth a
  measurement. Queued for when he is out.
- **`CaptureViewport` with a labelled coordinate grid.** Still the cheapest way
  to answer "does the bay read as a place at 152 cm", and still the thing Chris
  has never seen un-layered. Needs the bridge started and `LLK_DockBay` open.
- **Nav bounds against the playable floor, and the dock's box half-extents.**
  Editor reads, deferred again rather than inferred.
- **Row 80** in `docs/DECISIONS.md` — mine, and it was the blocker here that
  outlived the reason for it. **Written 2026-11-04**, reconstructed from the
  twelve `Source/` citations that had been depending on it while it did not
  exist.

**One correction to Part 1's closing claim:** requirement 15 /
`GeometryServerOnly` is **no longer missing.** It landed in `5f604c9` with
`LLKMapReadiness.h:54`, the check at `LLKMapReadiness.cpp:122`, two assertions
in `LLKMapReadinessTest.cpp`, and row 15 in `docs/MAP-READINESS.md`. The
readiness list is seventeen checks now, not fourteen. Row 72's gap is closed.
