# The opening's navmesh was never built, so the machine has never been able to take a step toward the boy

**Status: measured in a running Development game on 2026-11-21, not fixed.**
The fix is a navigation build saved into `Content/LLK_Boswells_OpeningPass.umap`,
which is another writer's lane and currently dirty with their work. This
document is the evidence and the exact numbers; it changes nothing.

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

## What closes this

1. Navigation built and saved into the opening map, so
   `RecastNavMesh-Default` reports non-empty bounds at boot. The startup
   report goes quiet by itself when that happens.
2. Then the separate question: a `SUCCESS` path query from the machine on the
   upper plate to the boy on the lower floor, with a path length and a time at
   168 cm/s. Until that line appears, "the machine can reach him" is unproven
   even with tiles present.
3. Then, and only then, the thing none of this proves: that being hunted
   through that store is frightening. No instrument reports on that.
