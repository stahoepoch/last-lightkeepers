# The opening's navmesh was never built, so the machine has never been able to take a step toward the boy

> **REOPENED the same hour, and the status below was wrong when it was
> written.** The navigation build survived about three minutes. Codex wrote
> `Tools/rehabilitate_boswells_emergency_lighting.py` at 15:49:11 and the map
> was saved at 15:49:26, which dropped the navmesh - 2,605,317 bytes back to
> 2,300,902, and the next boot read `0 of 2 navigation data actor(s) carry any
> built tiles` again. A second build at 16:0x was destroyed the same way
> within minutes, by `Tools/build_boswells_service_core.py`.
>
> **This is systemic, not an accident.** A commandlet world holds empty
> navigation data, so ANY tool that loads this map and saves it writes that
> emptiness over the built tiles. Every dressing, lighting and fixture script
> in `Tools/` silently un-navigates the level. Filed as **X11**, which is now
> the row that matters; this document's own "fix" is not durable until that
> one closes.
>
> Rebuilding was abandoned rather than retried a third time: Codex is actively
> authoring and running map tools minute by minute, and a navmesh cannot be
> landed into a map somebody else is saving.

**Status: found, built twice, and destroyed twice on 2026-11-21**, after
Chris asked for the build to be done here. Navigation is now built into
`Content/LLK_Boswells_OpeningPass.umap` and the running game says:

```
LLKSweeper_0 can reach the keeper: 7145 cm of path over 2758 cm straight
  (2.59x), 42.5 s at 168 cm/s.
```

**That line also settles a question this document had filed as separate.** The
machine stands on the upper plate at z=753 and the boy begins on the lower
floor at z=75. A 2.59x detour over 71 m of path means Recast does carry an
agent between the two floors - the escalator ramp is navigable, and the
two-level connection is no longer theoretical.

**The map holding the fix is uncommitted, and it is not only mine.** That blob
also carries another writer's uncommitted dressing from 2026-11-21, which was
already on disk when the navigation went in. Nothing of theirs was overwritten
- a byte-identical backup was taken first, verified by SHA-256, and restored
once - but committing that file lands their work too, which is Chris's call.

---

## What the running game says

Added to the startup report in `ALLKTestGameMode::ReportThreatReachability`
and read out of `/Game/LLK_Boswells_OpeningPass` in a `-game` process:

```
LLKSweeper_0 awake. Speed 168 cm/s, telegraph 1.0s, notices a dark keeper
  at 12 studs and a lit one at 38.
Warning: LLKSweeper_0: path query returned 1, which is neither success nor
  failure. Why: 0 of 2 navigation data actor(s) carry any built tiles,
  machine projects onto the mesh NO, keeper projects NO.
  THE NAVMESH HAS NO BUILT TILES.
Warning:   machine at (-850, -1300, 753), keeper at (1600, -2370, 75)
Warning:   nav data RecastNavMesh-Default covers (0, 0, 0) to (0, 0, 0)
             - EMPTY BOUNDS
Warning:   nav data AbstractNavData-Default covers (0, 0, 0) to (0, 0, 0)
             - EMPTY BOUNDS
Warning:   bounds volume NavMeshBoundsVolume_1 centred (0, -2600, 700)
             extent (4500, 5500, 1800) - keeper is INSIDE it
```

## What that means, in one line

The map has a `NavMeshBoundsVolume` in the right place and a `RecastNavMesh`
actor to hold the result, and **the result was never generated**. Zero tiles.
Nothing in the opening can path anywhere.

## Why nobody noticed

**Every component is correct and every log was clean.** The Sweeper wakes,
reads its family from `DT_Enemies`, sets its speed, and reports its notice
radii. Its brain is a pure state machine with every transition present -
Patrol, Telegraph, Pursue, Search, Stock - and `LLK.Enemies.SweeperBrain`
passes. It can notice the keeper. It can telegraph at him. Then `Move::Keeper`
asks the navigation system to move it, and there is no navmesh, and it stays
where it is.

Nothing in the chapter depends on navigation except the one thing that is
supposed to be frightening. The lantern, the grille, the capture sequence,
the lights and the fixtures all work without a single path query, which is
why the opening reviews as functional.

This is the fourth time this project has shipped a correct system over absent
geometry: the crate in a recess the Sweeper could never reach
(`FINDINGS-SWEEPER-CANNOT-REACH-STOCK.md`), the fixtures whose collision never
imported (`HANDOFFS.md` M16), the lantern 189 cm above a 150 cm reach
(M17), and this.

## The bounds volume is not the problem

Worth stating because it is the obvious suspect and it is innocent. The volume
covers x ∈ [-4500, 4500], y ∈ [-8100, 2900], z ∈ [-1100, 2500]. The keeper at
(1600, -2370, 75) is inside it. The machine at (-850, -1300, 753) is inside it.
Recast was told where to build and did not build.

Note the machine's z of **753** against the keeper's **75**: the Sweeper stands
on the upper plate and the boy begins on the lower floor. So even once tiles
exist, whether a path connects those two levels is a separate question this
finding does not answer - the escalators are presentation over a collision
ramp, and whether Recast will carry an agent up them is unmeasured.

## `IsNavigationBuilt` is a false green, and it caught me first

The first version of this diagnostic reported **"navmesh built YES"** beside
bounds of `(0,0,0)-(0,0,0)`. `UNavigationSystemV1::IsNavigationBuilt` answers
"is anything pending", not "do tiles exist" - and an unbuilt navmesh has
nothing pending precisely because nobody ever asked for it to be built.

The corrected instrument counts navigation data actors whose bounds are valid
and non-empty, and demotes `IsNavigationBuilt` to a logged note. **Anything
that gates on `IsNavigationBuilt` is gating on nothing**, which is worth
knowing beyond this map: `LLKMapReadiness` has a `NavmeshNotBuilt` check, and
if it is implemented on that call it will pass on an empty navmesh.

## Related: the readiness checker has never been run against a real map

`LLKMapReadiness` already has the checks this finding needed -
`NavBoundsMissing`, `NavDataMissing`, `NavmeshNotBuilt`,
`NavBoundsDoesNotCoverSpawn`, `SpawnOffNavmesh`, `EnemySpawnOffNavmesh`,
`NoRouteFromSpawn`, `RouteUnverified` - eighteen check kinds in all, and a
struct deliberately designed so that *"a route that was not queried is not a
route that failed"*.

`LLKMapReadiness::Observe(UWorld*, const FLLKStoreFloorPlan&)` is the only
half that reads a real world. **It has no callers.** `LLKMapReadinessTest`
exercises `Evaluate` against hand-built `FLLKMapObservations` structs, so the
pure half is covered and the world-reading half has never executed anywhere,
in any map, once.

That is filed as its own row rather than fixed here, because pointing it at a
dressed map would also fire its harness-convention checks - tagged enemy
spawns, test lanes - which `LLK_Boswells_OpeningPass` never adopted, and a
diagnostic that reports six irrelevant findings is a diagnostic nobody reads.

## How it was built, and the two things in the way

Both will recur, so they are recorded rather than summarised.

**1. The resave commandlet cannot build an async-loaded map in UE 5.8.** The
first attempt logged:

```
UNavigationSystemV1::Build Navigation NOT building because navigation build
  is locked (flags: 0x20).
```

`0x20` is `ENavigationBuildLock::AsyncLoadLock`. The commandlet async-loads the
map, which takes that lock, and the lock comes off via a core ticker that needs
**sixteen ticks, a time delay, and zero remaining asset compiles**
(`NavigationSystem.cpp:5971`, `RegisterDelayedUnlock`). The commandlet calls
`FNavigationSystem::Build` immediately and never pumps that ticker, so the lock
is still held, the build refuses - and the commandlet still exits **0**.

The lock is taken only when
`bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically` is true. It is
`UPROPERTY(config)`, so it can be overridden on the command line without
editing anyone's ini:

```
-run=ResavePackages -PACKAGE=/Game/LLK_Boswells_OpeningPass -BuildNavigationData
-ini:Engine:[/Script/NavigationSystem.NavigationSystemV1]:bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically=False
```

Safe here because the commandlet already calls
`FStaticMeshCompilingManager::FinishAllCompilation()` before building, so it
does its own waiting for the geometry.

**2. The save failed once on a sharing violation, and the file changed
anyway.** `MoveFile ... (Error Code 32)` six times, then
`LogSavePackage: Error: Error saving`, `0/1 packages were resaved` - and the
map had still grown by 308 KB. The holder was almost certainly a hard-killed
`-game` verification process of mine. The file was restored from the verified
backup rather than trusted, and the build re-run once nothing held the map.

**And `0/1 packages were resaved` is not the test.** It read 0 on the
successful run too. The resave loop and the navigation build's own
`CheckoutAndSavePackage` pass over `ANavigationData` packages are different
code paths, and the second is what writes the map. The boot log is the test.

## What closes this

1. ~~Navigation built and saved into the opening map.~~ **Done twice and
   undone twice.** It builds correctly and does not survive another writer's
   save. **X11 is the real closure condition**: either every map-saving tool
   in `Tools/` rebuilds navigation as its last step, or navigation is built
   once at the end of a dressing session with nothing else touching the map.
2. ~~A `SUCCESS` path query from the machine on the upper plate to the boy on
   the lower floor.~~ **Done:** 7145 cm, 2.59x, 42.5 s at 168 cm/s.
3. **Still open, and now the only thing left: whether being hunted through
   that store is frightening.** No instrument reports on that, and nothing in
   this document is evidence for it.

   Two numbers deserve a human judgment, and both are newly measurable rather
   than newly wrong. **42.5 seconds** is how long the machine takes to arrive
   from where it spawns - which may be exactly right for an opening where the
   player is working out the lantern, or far too slow to ever matter. And the
   route is **2.59x** the straight line, so it arrives the long way round
   rather than from where the player last saw it. Neither is a defect. Both
   are now facts to tune against instead of guesses.
