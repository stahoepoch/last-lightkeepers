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

## The eleven things, and what breaks without each

Each row names the check code `LLK.CheckMap` reports, so a failure message and
this table are searchable by the same word.

| # | Requirement | Code when absent | What you would otherwise see |
| --- | --- | --- | --- |
| 0a | **An `APlayerStart`** | `NoPlayerSpawn` | Nothing. Every other requirement below is measured from the keeper spawn, so this one failing suppresses the rest rather than being reported alongside them |
| 0b | **At least one actor tagged `LLK.EnemySpawn`** | `NoEnemySpawnPoints` | Requirements 4 and 5 silently pass, because a route to nowhere is a route nobody needs |
| 1 | **A floor with blocking collision** under the whole playable area | `FloorHasNoCollision` | Keepers fall through on spawn, or stand on nothing and every trace misses |
| 2 | **A `NavMeshBoundsVolume`** enclosing the playable area | `NavBoundsMissing` | No navmesh is ever built. An enemy with a perfect brain never takes a step |
| 3 | **Built nav data** (a `RecastNavMesh` actor in the level) | `NavDataMissing` | The volume exists but nothing was generated — the usual cause is that the level was never saved after the volume was added |
| 4 | **Nav bounds that enclose** the keeper spawn and every enemy spawn point | `NavBoundsDoesNotCoverSpawn` | Enemies spawn off-mesh and are inert in a way that reads exactly like a broken behaviour tree. The usual version of this is a volume that covers the shop floor and stops short of the back of house, which is where the spawn points are |
| 5 | **A walkable route** from the keeper spawn to every enemy spawn point | `NoRouteFromSpawn` / `RouteUnverified` | The first Sweeper is placed somewhere it can see you and never reach you. `RouteUnverified` is not a pass — see *The route check is honest about not knowing* below |
| 6 | **Sight-blocking geometry above eye height** — shelving, gondola runs, the internal wall | `NoOccludingGeometry` | Perception degenerates to raw distance. The whole floor becomes one room and light stops being a trade |
| 7 | **No unbroken sightline longer than `MaxSightlineStuds` (34)** from the keeper spawn | `SightlineTooLong` | A lit keeper is hunted from 38 studs by something they had no way of seeing. `FLLKStoreFloorPlan` already carries this rule; this is it checked against real geometry instead of against arithmetic |
| 8 | **One open test lane**, tagged `LLK.TestLane.Open` | `TestLaneOpenMissing` / `TestLaneOpenBlocked` | Nothing proves the visibility source can return `Clear`. A trace that always reports blocked makes every enemy blind and passes every "is it safe" test |
| 9 | **One blocked test lane**, tagged `LLK.TestLane.Blocked` | `TestLaneBlockedMissing` / `TestLaneBlockedClear` | Nothing proves the visibility source can return `Blocked`. A trace that always reports clear makes every enemy see through walls |

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
