# Saving a map without deleting its navmesh

**Status: established and proven on 2026-11-21**, on Chris's instruction to
settle this before committing the functional map and elevator pass. Three
steps, each with a process exit code rather than a log line somebody has to
read.

Written because the opening map's navigation was built and destroyed twice in
one hour by ordinary tool saves, and both times the only evidence was a
warning nobody was looking at. `docs/FINDINGS-THE-OPENING-HAS-NO-NAVMESH.md`
has that history; `HANDOFFS.md` X11 has the ask; this file is the procedure.

---

## The problem, in one sentence

A commandlet world holds **empty** navigation data, so any tool that loads a
map and saves it serialises that emptiness over whatever was built before.

## How bad it was, measured

The gate below, run against every Boswell's map on 2026-11-21:

| map | gate | finding |
|---|---|---|
| `LLK_Boswells_OpeningPass` | **1** | `NavmeshNotBuilt` |
| `LLK_Boswells_RoutePass` | **1** | `NavmeshNotBuilt` |
| `LLK_Boswells_ApparelPass` | **1** | `NavmeshNotBuilt` |
| `LLK_Boswells_Anchor` | **1** | `NavmeshNotBuilt` |
| `LLK_Boswells_FirstRun` | **1** | `RouteUnverified` |

**Four of five had no navmesh at all.** Nothing could path in any of them.
That reframes the Sweeper's whole history, including
`FINDINGS-SWEEPER-CANNOT-REACH-STOCK.md` — which was measured in `FirstRun`,
the one map that does have tiles.

---

## Step 1 — save the map

Whatever the dressing, lighting or fixture tool already does. No change here.

## Step 2 — build navigation

```
UnrealEditor-Cmd.exe <project> -run=ResavePackages
  -PACKAGE=/Game/<MapName>
  -BuildNavigationData
  -ini:Engine:[/Script/NavigationSystem.NavigationSystemV1]:bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically=False
  -unattended -nopause -nosplash
```

**The `-ini:` override is not optional.** Without it the build refuses and
the commandlet still exits 0:

```
UNavigationSystemV1::Build Navigation NOT building because navigation build
  is locked (flags: 0x20).
```

`0x20` is `ENavigationBuildLock::AsyncLoadLock`. The commandlet async-loads
the map, which takes that lock, and the lock comes off via a core ticker
needing **sixteen ticks, a time delay, and zero remaining asset compiles**
(`NavigationSystem.cpp:5971`). The commandlet calls `FNavigationSystem::Build`
immediately and never pumps that ticker. The flag is `UPROPERTY(config)`, so
the override needs no edit to anyone's ini. It is safe because the commandlet
already calls `FStaticMeshCompilingManager::FinishAllCompilation()` first and
does its own waiting for geometry.

**Do not gate on `[REPORT] 0/1 packages were resaved`.** It reads 0 on
success. The resave loop and the navigation build's own
`CheckoutAndSavePackage` pass over `ANavigationData` packages are different
code paths, and the second is what writes the map.

## Step 3 — prove the tiles exist

```
UnrealEditor-Cmd.exe <project> /Game/<MapName> -game -unattended -nullrhi
  -nosplash -nopause -ExecCmds="LLK.CheckMap strict"
```

**Exit 0 means ready. Exit 1 means findings, and the log names them.** The
bare `LLK.CheckMap` stays a human instrument that changes nothing and never
exits; only `strict` ends the process, because a diagnostic that can kill the
editor you typed it into is one you stop typing.

`strict` forces the exit deliberately. With an unforced exit the command
logged "exiting 1" and the process returned **0** — the unforced path ends in
`PostQuitMessage`, and nothing pumps a message loop in a headless
`-game -unattended` run, so the code was dropped. A gate that always passes is
worse than no gate.

The same boot also prints the stronger fact, from
`ALLKTestGameMode::ReportThreatReachability`:

```
LLKSweeper_0 can reach the keeper: 7145 cm of path over 2758 cm straight
  (2.59x), 42.5 s at 168 cm/s.
```

That is a real path between the machine where it stands and the keeper where
he spawns. Tiles existing does not prove it; this line does.

---

## Proven, both directions

On `LLK_Boswells_OpeningPass`, same command, the only change being navigation:

| | gate | what the log said |
|---|---|---|
| before step 2 | **1** | `NavmeshNotBuilt - 2 nav data actor(s) and NONE with any built tiles` |
| after step 2 | **0** | `Map readiness: ready.` plus the reachability line above |

The map went 2,398,704 → 2,722,113 bytes. The ~320 KB is the tile data.

## Two things this workflow does not do

- **It does not make the opening frightening.** 42.5 s to arrive and a 2.59x
  route are now measurable, and whether they are right is a judgment in
  control that no instrument reports on.
- **It does not survive someone skipping it.** Until step 2 is part of every
  map-saving tool (X11), the next dressing run un-navigates the level again,
  and step 3 is how you find out in twenty seconds instead of a playtest.

## If a save fails with error code 32

`MoveFile ... (Error Code 32)` is a sharing violation: another process holds
the `.umap`. A hard-killed `-game` process will do it. **The file can change
size while the save reports failure**, so do not trust it — restore from a
backup whose SHA-256 you checked, wait for the handle to clear, and rebuild.
Taking that backup before step 2 costs nothing and is the only reason the
first clobber here was recoverable.
