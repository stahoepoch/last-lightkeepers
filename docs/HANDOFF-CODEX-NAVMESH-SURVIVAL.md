# Handoff — Codex — X11: every map-saving tool in `Tools/` deletes the navmesh

Written 2026-11-21 by Claude, on Chris's instruction to relay this. One
change, in your lane, with the command that works and the two traps that make
it look like it worked when it did not.

**The short version.** `LLK_Boswells_OpeningPass` had no navigation data at
all. I built it; the game logged the Sweeper reaching the keeper for the first
time. Three minutes later your emergency-lighting pass saved the map and the
navmesh was gone. I rebuilt it; `build_boswells_service_core.py` destroyed it
again within minutes. **This is not an accident either time, and it is not
about those two scripts.**

---

## 1. Why any map save removes navigation

A commandlet world holds **empty** navigation data. It never builds tiles,
because nothing asks it to. So a tool that loads a map and saves it serialises
that emptiness over whatever was built before, and the level silently stops
being navigable. Every dressing, lighting, fixture and repair script in
`Tools/` does this.

Measured, in file sizes and mtimes:

| time | event | `LLK_Boswells_OpeningPass.umap` |
|---|---|---|
| 15:46 | navigation built | 2,605,317 |
| 15:49:11 | `rehabilitate_boswells_emergency_lighting.py` written | — |
| 15:49:26 | that tool saves the map | **2,300,902** |
| 15:52 | boot | `0 of 2 navigation data actor(s) carry any built tiles` |
| 16:0x | navigation rebuilt | 2,613,340 |
| 16:08:37 | `build_boswells_service_core.py` saves | **2,398,704** |
| 16:08:54 | boot | empty again |

The ~300 KB that comes and goes is the tile data.

## 2. What the game says, both ways

From `ALLKTestGameMode::ReportThreatReachability`, which now runs at startup in
every map. With navigation:

```
LLKSweeper_0 can reach the keeper: 7145 cm of path over 2758 cm straight
  (2.59x), 42.5 s at 168 cm/s.
```

Without it:

```
Warning: LLKSweeper_0: path query returned 1 ... 0 of 2 navigation data
  actor(s) carry any built tiles ... THE NAVMESH HAS NO BUILT TILES
```

So you do not need to take my word for the state of any map — boot it and
read one line.

## 3. The command that works, and the trap that makes it exit 0 while refusing

```
UnrealEditor-Cmd.exe <project> -run=ResavePackages
  -PACKAGE=/Game/LLK_Boswells_OpeningPass
  -BuildNavigationData
  -ini:Engine:[/Script/NavigationSystem.NavigationSystemV1]:bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically=False
  -unattended -nopause -nosplash
```

**Without that `-ini:` override the build refuses and the commandlet still
exits 0.** It logs:

```
UNavigationSystemV1::Build Navigation NOT building because navigation build
  is locked (flags: 0x20).
```

`0x20` is `ENavigationBuildLock::AsyncLoadLock`. The commandlet async-loads
the map, which takes that lock, and the lock is released by a core ticker that
needs **sixteen ticks, a time delay, and zero remaining asset compiles**
(`NavigationSystem.cpp:5971`, `RegisterDelayedUnlock`). The commandlet calls
`FNavigationSystem::Build` immediately and never pumps that ticker.

The flag is `UPROPERTY(config)`, so the override needs no edit to
`Config/*.ini`. It is safe here because the commandlet already calls
`FStaticMeshCompilingManager::FinishAllCompilation()` before building, so it
does its own waiting for geometry.

## 4. The second trap: the success line is not the success line

`[REPORT] 0/1 packages were resaved` reads **0 on a successful build**. The
resave loop and the navigation build's own `CheckoutAndSavePackage` pass over
`ANavigationData` packages are different code paths, and the second one is what
writes the map. Do not gate on that number. Gate on the boot log, or on the
file growing by a few hundred KB.

## 5. What I am asking for, and why option 1 is better

**Option 1, preferred: every tool that saves a map rebuilds navigation as its
last step.** Then the level is navigable after a normal dressing run and
nobody has to remember anything. It costs about 45 s of static-mesh
compilation plus a fraction of a second of tile generation on this map.

**Option 2: navigation is built once at the end of a dressing session**, with
nothing else touching the map afterwards. This works until the next script
runs, which is why it is second.

**Closes when a boot reports a reachable keeper after an ordinary dressing
run** rather than after a hand-run build.

## 6. Three things that are mine, not yours

Recorded because they cost real time today and two of them are my errors.

- **I built into a map that was dirty under you**, which is the collision
  `LANES.md` exists to prevent. I knew the file was dirty when I started. The
  verified backup and restore below is the only reason that was recoverable.
- **One save attempt hit a sharing violation** — `MoveFile ... (Error Code
  32)`, six retries, then `Error saving` — from a `-game` process I had
  hard-killed. The map changed anyway, so it was restored from a
  SHA-256-verified backup rather than trusted, and rebuilt once nothing held
  the file. If you see a map change size while a save reports failure, that is
  what it looks like.
- **I filed `M18` claiming `LLKMapReadiness::Observe` has no callers. That is
  wrong.** `LLK.CheckMap` is a registered console command at
  `LLKMapReadiness.cpp:574` and calls it. My grep excluded the file the caller
  lives in. The row is corrected.

## 7. One thing for me, found while checking the above

`LLK.CheckMap` on this map reports five findings, and the navmesh is **not**
one of them by name:

```
NoEnemySpawnPoints - no actors tagged LLK.EnemySpawn
SpawnOffNavmesh - the keeper spawn does not project onto navigable space -
  usually a PlayerStart sunk into the floor, floating above it, or on a counter
SightlineTooLong - longest sightline 80.0 studs, limit 34.0
TestLaneOpenMissing - 0 actor(s) tagged LLK.TestLane.Open, need exactly 2
TestLaneBlockedMissing - 0 actor(s) tagged LLK.TestLane.Blocked, need exactly 2
```

`NavmeshNotBuilt` does not fire, because two navigation data actors exist and
`IsNavigationBuilt` returns true — the false green that
`HANDOFF-CODEX-NAVIGATION.md` already recorded on 2026-10-29. So an empty
navmesh surfaces as `SpawnOffNavmesh`, whose own text blames a PlayerStart
sunk into the floor. That diagnosis sends the reader to move the spawn, which
is not the problem.

Three of the other four are harness conventions this map never adopted. Both
of those are `Source/LLK` and mine to fix; they are why nobody runs this
command on a dressed map.
