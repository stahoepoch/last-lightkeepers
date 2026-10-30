# Map readiness — what a map must have before an enemy is put in it

Written 2026-10-29 by Claude, as the support slice for **M2.5**. Chris owns the
actual `Content/` map (row 41); this file owns the list of things the map has to
be true about, and `LLK.CheckMap` owns saying which ones it is not.

The first version of the floor is allowed to be **deliberately ugly**. Chris,
2026-10-29: *"The first version can be a deliberately ugly playable department
store greybox. Once that passes, we replace its shell with
purchased/commissioned assets."* Nothing below asks for art. Every item is
something a grey box either has or does not have, and the point of writing it
down is that the alternative is discovering it by way of a Sweeper that stands
still, which is a bug that looks like six other bugs.

---

## The fourteen things, and what breaks without each

Each row names the check code `LLK.CheckMap` reports, so a failure message and
this table are searchable by the same word.

| # | Requirement | Code when absent | What you would otherwise see |
| --- | --- | --- | --- |
| 0a | **An `APlayerStart`** | `NoPlayerSpawn` | Nothing. Every other requirement below is measured from the keeper spawn, so this one failing suppresses the rest rather than being reported alongside them |
| 0b | **At least one actor tagged `LLK.EnemySpawn`** | `NoEnemySpawnPoints` | Requirements 4 and 5 silently pass, because a route to nowhere is a route nobody needs |
| 1 | **A floor with blocking collision** under the whole playable area | `FloorHasNoCollision` | Keepers fall through on spawn, or stand on nothing and every trace misses |
| 2 | **A `NavMeshBoundsVolume`** enclosing the playable area | `NavBoundsMissing` | No navmesh is ever built. An enemy with a perfect brain never takes a step |
| 3 | **Built nav data** (a `RecastNavMesh` actor in the level) | `NavDataMissing` | The volume exists but nothing was generated — the usual cause is that the level was never saved after the volume was added |
| 3a | **Nav data that has finished generating** | `NavmeshNotBuilt` | An enemy that is inert for one specific second and fine afterwards, or inert forever if the check that reported it ran at load. Generation is asynchronous |
| 4 | **Nav bounds that enclose** the keeper spawn and every enemy spawn point | `NavBoundsDoesNotCoverSpawn` | Enemies spawn off-mesh and are inert in a way that reads exactly like a broken behaviour tree. The usual version of this is a volume that covers the shop floor and stops short of the back of house, which is where the spawn points are |
| 4a | **A keeper spawn that projects onto the navmesh** | `SpawnOffNavmesh` | Every route query fails from a start point that is not on the mesh, which reads as the whole floor being unreachable rather than as one actor being 30cm too low |
| 5 | **A walkable route** from the keeper spawn to every enemy spawn point | `NoRouteFromSpawn` / `RouteUnverified` | The first Sweeper is placed somewhere it can see you and never reach you. `RouteUnverified` is not a pass — see *The route check is honest about not knowing* below |
| 5a | **Enemy spawn points that project onto the navmesh** | `EnemySpawnOffNavmesh` | A route finding that blames the building for one actor 30cm into the floor. Every path to an off-mesh point comes back partial |
| 6 | **Sight-blocking geometry above eye height** — shelving, gondola runs, the internal wall | `NoOccludingGeometry` | Perception degenerates to raw distance. The whole floor becomes one room and light stops being a trade |
| 7 | **No unbroken sightline longer than `MaxSightlineStuds` (34)** from the keeper spawn | `SightlineTooLong` | A lit keeper is hunted from 38 studs by something they had no way of seeing. `FLLKStoreFloorPlan` already carries this rule; this is it checked against real geometry instead of against arithmetic |
| 8 | **One open test lane**, tagged `LLK.TestLane.Open` | `TestLaneOpenMissing` / `TestLaneOpenBlocked` | Nothing proves the visibility source can return `Clear`. A trace that always reports blocked makes every enemy blind and passes every "is it safe" test |
| 9 | **One blocked test lane**, tagged `LLK.TestLane.Blocked` | `TestLaneBlockedMissing` / `TestLaneBlockedClear` | Nothing proves the visibility source can return `Blocked`. A trace that always reports clear makes every enemy see through walls |

**The four navigation answers are a chain, not four questions.** No bounds
volume or no nav data, navmesh not built, spawn off the mesh, route blocked:
each one makes the ones after it meaningless, so `LLK.CheckMap` reports the
first real failure and stops. A route query against an unbuilt navmesh returns
false, and reporting that false alongside the real problem is how a check with
four findings sends somebody to fix the wrong one. The acceptance criteria in
`docs/HANDOFF-CODEX-NAVIGATION.md` pin this: a `PlayerStart` raised three
metres reports `SpawnOffNavmesh` **and nothing about the route**.

**The chain's first link is the counts, and it was missing for a day.** When
the real nav queries landed (`c8ff2d1`), a map with no `NavMeshBoundsVolume`
started reporting `SpawnOffNavmesh` next to `NavBoundsMissing` - and the spawn
finding is the one that looks actionable, so it is the one somebody would have
acted on, moving a `PlayerStart` that was in exactly the right place. The cause
is worth knowing before trusting any nav query:
`UNavigationSystemV1::IsNavigationBuilt` returns **true when there is nowhere
to build navigation** (`NavigationSystem.cpp:2555`), and true when navigation
is disabled in world settings, because in both cases nothing was asked to
generate. So "is it built" cannot answer "does a mesh exist"; only
`NavBoundsVolumes` and `NavDataActors` can, and `Evaluate` now checks those
before reading any query result.

**Requirement 5a is why "one more projection" was not the whole fix.** The
route query paths **to** each enemy spawn's raw position, so an enemy spawn
that is itself off-mesh comes back as a partial path. Before 5a existed the
only truthful thing to report that as was `NoRouteFromSpawn` - correct, and
about the wrong thing. Codex declined to change the query alone to claim
"misplaced actor", which was right: a query cannot report a finding the
contract has no code for, and making the route query *say* something more
specific than the contract can represent is how a diagnostic starts lying.
The code, the observation and the chain link now exist; the projection that
fills the count is the one item left in the handoff.

**Requirement 4 is necessary and not sufficient, and says so in its name.** It
checks that the spawn points fall inside the *bounds of the volumes*, which is
a box test — a point can sit inside the box and still be off-mesh because it is
on top of a counter or inside a wall. Catching that needs a nav projection,
which needs the module named in the handoff below. The box test is still worth
running: the failure it catches (a volume that does not reach the back of
house) is the common one, and it is the one that is invisible in the viewport
because volumes are only drawn when selected.

Rows 8 and 9 are the pair that matters, and they are a pair on purpose. A
single lane cannot tell a working trace from a stuck one: whichever answer it
gives, a trace hardwired to that answer passes it. Two lanes with opposite
expected answers cannot both be satisfied by a constant.

### How to mark the enemy spawn points

Empty actors tagged `LLK.EnemySpawn` on the **Actor Tags** list. Requirements
4 and 5 are checked against each one, and M2.6 will read the same tag rather
than carry its own list, so a spawn point that passes this check is a spawn
point the first Sweeper will actually use.

`FLLKStoreFloorBuilder` already picks three positions for the code-built
greybox — the service route, the back of jewellery, and the mezzanine — chosen
because each is somewhere a keeper *arrives at* rather than starts in. Those
are the places to put the tagged actors in the real map.

### How to place the two test lanes

Both are ordinary empty actors, placed in pairs, tagged on the **Actor Tags**
list (not component tags):

- `LLK.TestLane.Open` — two of them, with nothing whatsoever between them.
  Across a clear stretch of concourse is ideal, at least 10 studs apart so a
  trace of zero length cannot pass by accident.
- `LLK.TestLane.Blocked` — two of them, with a shelf run, wall, or the
  mirrored columns squarely between them.

`LLK.CheckMap` traces between each pair at eye height and says so when a lane
disagrees with its own name. Fewer or more than two actors per tag is itself
reported, because a lane with one end is not a lane.

### Eye height is 152 cm, which is 5.43 studs

The camera sits 64 cm above the capsule's centre
(`Source/LLK/Private/Characters/LLKCharacter.cpp:30`) and the capsule's default
half-height is 88 cm, so a keeper's eye — and therefore every sightline trace —
is **152 cm above the floor**.

That is the number a sight break has to beat, and it is why the floor plan's
shelving is 6 studs (168 cm) and its counters are 3.2 studs (90 cm). The
shelving clears eye height by half a stud. **A 5-stud shelf would not**, and
would produce a floor that reads as full of cover and occludes nothing.

### The route check is honest about not knowing

A navmesh path query needs the engine's `NavigationSystem` module, and
`Source/LLK/LLK.Build.cs` is Codex's lane (`docs/LANES.md`). So the runtime
checker reports three distinct route states and never collapses them:

- **`NoRouteFromSpawn`** — somebody asked and the answer was no.
- **`RouteUnverified`** — nobody asked. Reported as a finding, not as a pass,
  for the same reason `ULLKEnemyRosterSubsystem` has no fallback roster: a
  check that returns "fine" when it did not run is worse than no check.
- Silence — asked, and there is a route.

`FLLKMapObservations::RouteFromSpawnReaches` is a `TOptional<bool>` so those
three states are three values rather than two values and a convention. Filling
it is the one line of this contract that is a handoff, written out at the
bottom of this file.

---

## Occlusion: how perception consumes a visibility answer

`LLKPerception::SenseReaching` deliberately traces nothing — it answers *which
of this family's senses reaches this keeper*, which is necessary and not
sufficient. M2.5 supplies the sufficient half, and it arrives through an
interface rather than through perception learning about `UWorld`:

```
ILLKVisibilitySource            // Enemies/LLKVisibility.h
  ELLKVisibility VisibilityBetween(From, To) const
      -> Clear | Blocked | Unknown

FLLKWorldVisibility : ILLKVisibilitySource
  // the real one: a line trace on ECC_Visibility, at eye height

LLKPerception::Notice(Enemy, Perceiver, Keeper, LitThreshold, Visibility)
  -> FLLKNotice { Sense, Visibility, bool bNoticed }
```

Three things about that shape, each of which is the reason for it:

**`Unknown` is a third answer, and it never notices.** A source with no world,
a trace against a floor with no collision, and a map with no geometry all
produce `Unknown`. Treating it as `Clear` would make an enemy see through
unbuilt walls; treating it as `Blocked` silently would make the whole roster
inert and look like a design problem. So it is its own value, it does not
notice, and it is reported in `FLLKNotice` so the debug HUD can say
*unverified* rather than *safe*.

**Passing no source at all is an error, not a default.** `Notice` with a null
source logs an error and notices nothing. There is no "assume clear" path,
because that path would be the one that ships.

**The sense is computed before visibility is consulted.** `FLLKNotice` carries
the sense that *would* have fired even when the wall stops it, because "why did
it not notice me" needs answering as much as row 5's "why did it". It also
keeps the two halves separately testable: the sense rules have no world, and
the visibility rules have no enemy data.

**What this is not.** It is not a perception system with memory, hearing,
peripheral falloff or last-known-position. Those are M2.6 and later, and they
belong to the enemy, not to this function.

---

## The boarding route, which is not a check yet

Added 2026-10-30 with M3's extraction rules (row 75), and deliberately NOT a
fifteenth check.

`ULLKExtractionSubsystem` needs one piece of geometry and only one:
`RegisterBoardingRoute(PointId, Anchor, RadiusCm)` - a position and a radius
per exit, which is the part of the dock a crew has to hold. Row 74 keeps the rig
on the service-road side and the driver out of the interior, so nothing else
about the dock is a rule.

**What happens today, with no map:** the hold cannot advance, because an
unmeasured route is not an empty one. The subsystem says so once per call, by
name:

    EXTRACTION CANNOT COMPLETE: no boarding route registered for LoadingDock,
    so the hold will never advance.

That is the same discipline as `LLK.CheckMap` - name the missing thing rather
than let somebody discover it by watching a timer not move - applied at the
moment it matters rather than as a map-load check. **It becomes a check here
when the greybox has a dock to check**, and it will want `NoBoardingRoute` as
its code. Writing the check before there is a dock would mean every greybox
reports a missing route for the whole of M2.5, which is how a warning stops
being read.

## Running it

```
LLK.CheckMap
```

Typed into the console in PIE or in a packaged build. It prints one line per
finding with the code from the table above and the place it was found, or says
the map is ready. It never changes anything.

The same report is returned by `LLKMapReadiness::Observe` + `Evaluate`, so a
commandlet or an automation test can assert on it instead of reading log text.
`Evaluate` takes an observations struct and no world at all, which is what lets
`LLK.Harness.MapReadiness` test the judgement separately from the gathering.

---

## Handoff to Codex

One item, and it is small:

**Add `"NavigationSystem"` to `PublicDependencyModuleNames` in
`Source/LLK/LLK.Build.cs`.** With it, `LLKMapReadiness.cpp` can fill
`RouteFromSpawnReaches` through `UNavigationSystemV1::FindPathToLocationSynchronously`
and requirement 5 stops reporting `RouteUnverified`. The call site is marked
with `// HANDOFF(Codex): NavigationSystem` and already has the branch; without
the module it resolves the nav *volume* and nav *data* by reflected class path,
which is why requirements 2, 3 and 4 work today and 5 does not.

Until then `RouteUnverified` appears on every clean map, which is correct and
is meant to be annoying.
