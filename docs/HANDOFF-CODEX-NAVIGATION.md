# Handoff to Codex — the navigation half of `LLK.CheckMap`

Written 2026-10-29 by Claude, on Chris's instruction. One task, two files, and
a deliberate ceiling on what it is allowed to become.

Read `docs/MAP-READINESS.md` first — it is the contract this completes, and it
already lists every check code and the reason each one is on the list. Read
`docs/LANES.md` for why this is a handoff at all rather than something I did.

---

## The task in one sentence

Make `LLK.CheckMap` able to answer **"can a Sweeper actually walk from the
keeper spawn to where I want to put it"**, and when it cannot, say which of
four specific things is wrong.

Today it cannot ask. It reports `RouteUnverified` on every map, including
perfect ones, because a navmesh query needs the engine's `NavigationSystem`
module and `Source/LLK/LLK.Build.cs` is yours.

## What this is, and what it must not become

**It is a map-readiness diagnostic.** Chris, 2026-10-29: *"Make it a
map-readiness diagnostic, not a permanent CI gate yet. It should help Chris see
why a greybox is not ready without pretending a generated harness floor is the
future store."*

So, concretely:

- **Not a CI gate.** Do not add it to `.github/workflows/**`, do not make a
  failing check fail a build, and do not make it return a non-zero exit code
  from anything. It prints. A gate on a map that is deliberately half-built
  would train everyone to ignore it, and the map is *supposed* to be
  half-built right now.
- **Not a judgement about the harness floor.** `FLLKStoreFloorBuilder` spawns a
  code-built greybox at runtime from engine cubes, and that floor has no
  navmesh and never will — a `NavMeshBoundsVolume` is a level actor and the
  harness floor has no level. A clean run of `LLK.CheckMap` against the harness
  is **expected to report `NavBoundsMissing`**, and that is not a bug in either
  of them. The check is for Chris's `Content/` map. If you find yourself
  tempted to special-case the harness so it looks ready, stop: the thing to
  avoid is a check that calls a scaffold ready, because then "ready" stops
  meaning anything about the real store.
- **Not a repair.** No asset is written, no level is saved, no navmesh is
  built, nothing in `Content/` is touched, and no actor is spawned, moved or
  modified. `LLKMapReadiness::Observe` is read-only with respect to the world
  and must stay that way. If a check needs the navmesh built in order to
  answer, it reports that it is not built — it does not build it.

---

## The two changes

### 1. `Source/LLK/LLK.Build.cs` — one line

Add `"NavigationSystem"` to `PublicDependencyModuleNames`. The module is loaded
by default at runtime, so this is a compile-time link and not a new runtime
dependency: `LLKMapReadiness.cpp` already finds `NavMeshBoundsVolume` and
`NavigationData` actors by class name precisely because it could not include
their headers.

Once it is linked, **delete the `IsClassNamed` workaround** in
`LLKMapReadiness.cpp` and use the real types. It exists only to avoid the
dependency, it says so in its own comment, and a string compare against a class
name is a thing that breaks silently when a class is renamed.

### 2. `Source/LLK/Private/Harness/LLKMapReadiness.cpp` — one function

This is in my lane, so it is an **explicitly invited, bounded edit**: fill in
one function and nothing else in the file. I will not touch that function while
it is yours. Row 27 is one writer per lane at a time, and this is how we both
stay inside it.

The seam is marked in the source:

```cpp
// HANDOFF(Codex): NavigationSystem. ...
```

at `Source/LLK/Private/Harness/LLKMapReadiness.cpp:311`, inside `Observe`,
immediately after the nav bounds coverage loop. Everything it needs is in scope
there: `World`, `KeeperSpawn`, `EnemySpawns`, and `Obs`.

**What to fill in.** Three `TOptional` fields on `FLLKMapObservations`. Leaving
one unset is a legitimate answer and means *nobody could ask* — see "unset is
not false" below.

| Field | Set it to |
| --- | --- |
| `Obs.NavigationBuilt` | whether the navmesh has finished generating |
| `Obs.SpawnOnNavmesh` | whether the keeper spawn projects onto navigable space |
| `Obs.RouteFromSpawnReaches` | whether a complete path reaches **every** enemy spawn (see the designated-position note) |

`NavigationBuilt` and `SpawnOnNavmesh` do not exist yet, and neither does the
`SpawnOffNavmesh` check code. **Those are mine** — tell me and I will land the
contract side (the two fields, the enum value, the `Evaluate` branches and the
pure tests) in a commit of its own that touches no nav API, so that when you
start, the struct you are filling in already has the right shape. That commit
is one short step and is not blocked on anything.

---

## The four things it must be able to name

Chris's list, mapped to the codes in `docs/MAP-READINESS.md`:

| What is wrong | Code it reports | Status |
| --- | --- | --- |
| No bounds volume | `NavBoundsMissing` | **works today** |
| Navmesh not built | `NavDataMissing` today; `NavigationBuilt` makes it exact | the field is new |
| Start off-navmesh | `SpawnOffNavmesh` | the code is new |
| Route blocked | `NoRouteFromSpawn` | the branch exists, the query does not |

The distinction between the second and third is worth keeping sharp, because
they look identical in the viewport and have different fixes. *Not built* is a
volume that exists and has not generated — usually a level saved before the
volume was added, or a check that ran before generation finished. *Off-navmesh*
is a generated navmesh that does not include the spawn point — usually a
`PlayerStart` sunk into the floor, floating above it, or standing on a counter.

The existing `NavBoundsDoesNotCoverSpawn` is a **box test** against the
volumes' bounds and is already documented as necessary-and-not-sufficient. Do
not delete it when `SpawnOffNavmesh` arrives: a spawn point outside the volume's
box and a spawn point inside the box but off the mesh are, again, two different
afternoons. The box test also still answers when the navmesh is not built.

---

## The API, verified against the pinned engine

Checked by reading `UE_5.8/Engine/Source/Runtime/NavigationSystem/Public/` on
2026-10-29, rather than from memory. Line numbers are that header's:

- `UNavigationSystemV1::GetCurrent(World)` — the subsystem, or null. Null is
  `RouteUnverified`, not a failure.
- `NavSys->IsNavigationBuilt(World->GetWorldSettings())` —
  `NavigationSystem.h:775`.
- `NavSys->ProjectPointToNavigation(Point, OutLocation, Extent)` —
  `NavigationSystem.h:718`. Pass a deliberate `Extent` rather than the default
  and write down why you chose it: a generous extent will happily project a
  spawn that is two metres in the air and report it fine, which is the exact
  failure this check is for. A keeper's capsule is 88cm half-height, so
  something on that order is defensible and a 500cm extent is not.
- `UNavigationSystemV1::FindPathToLocationSynchronously(World, Start, End)` —
  `NavigationSystem.h:535`, returns `UNavigationPath*`.
- `UNavigationPath::IsValid()` and `::IsPartial()` — `NavigationPath.h:74` and
  `:77`.

**`IsPartial()` is the one that matters and the one that is easy to miss.** A
blocked route does not return a null path; it returns a *partial* path that
gets as close as it can. A check that tests only `IsValid()` passes on every
unreachable destination in the building, which would make this whole task a
check that always says yes. So:

```
reaches = Path != nullptr && Path->IsValid() && !Path->IsPartial()
```

### Timing: the check must run after generation, not after load

Navmesh generation is asynchronous, so "the level is loaded" is not "the
navmesh is ready", and a check that runs on `BeginPlay` will report an
unbuilt navmesh on a map that builds one a frame later. Two acceptable
answers:

1. **Report it honestly.** `IsNavigationBuilt` is false, so `NavigationBuilt`
   is `false`, and the finding says the navmesh is not built *yet*. Since
   `LLK.CheckMap` is a console command a human types, this is usually enough:
   by the time anyone types it, generation has finished.
2. **Wait for it**, via `OnNavigationGenerationFinishedDelegate`
   (`NavigationSystem.h:444`), if you add a non-interactive caller later.

Prefer the first. Do not add a `Sleep`, a frame-count wait or a retry loop to
`Observe` — a function that blocks is a function that will eventually block
something, and `Observe` is the half that is meant to be callable from
anywhere, including a future commandlet.

### One piece of friction you will hit immediately

`LLKMapReadiness::Observe` takes a `const UWorld*`, deliberately, because it
changes nothing. `UNavigationSystemV1::GetCurrent` takes a non-const `UWorld*`
(`NavigationSystem.h:1177`), as does `FindPathToLocationSynchronously`, which
takes a `UObject*` world context.

**Change the signature rather than casting the constness away.** `Observe`,
`Check` and the console command can all take `UWorld*`; the const was my
statement of intent, and intent that costs a `const_cast` at the one place it
matters is worth less than the cast costs. Nothing else in the project calls
either function, so widening them is a two-line change with no callers to
chase. Say in the comment that the non-const is the nav API's requirement, so
the next person does not "tidy" it back and find out why.

### The designated Sweeper test position

Chris asked specifically for the path from `PlayerStart` to *"the designated
Sweeper test position"*. Today the check tests **every** actor tagged
`LLK.EnemySpawn`, which is a superset and is the right default — a route to
one spawn point and not the others is a map that will still waste an
afternoon.

If one of them needs to be the designated one, the convention to use is a
second tag `LLK.TestRoute.Sweeper` on that actor, and the rule is: test every
`LLK.EnemySpawn`, and report the designated one first in the finding's detail
so it is the one Chris reads. Do not make the designated tag *required* — a map
with no designated position should still be checkable.

---

## Unset is not false, and this is the part not to optimise away

`RouteFromSpawnReaches` is a `TOptional<bool>` with three meanings, and
`Evaluate` already reports all three differently:

- **unset** → `RouteUnverified`: nobody asked.
- **false** → `NoRouteFromSpawn`: somebody asked and the answer was no.
- **true** → silence.

`LLK.Harness.MapReadiness` asserts that an otherwise-perfect map with an unset
route reports **exactly one finding** and that it is `RouteUnverified`. That
test is there to stop the obvious shortcut: defaulting the field to `true` so
clean maps go quiet. A check that answers "fine" when it did not run is worse
than no check, which is the same reason `ULLKEnemyRosterSubsystem` has no
fallback roster. Row 66 in `docs/DECISIONS.md` records the trade.

So if the nav system is null, or the navmesh is not built, or there are no
enemy spawns to path to — leave the field unset. Do not set it to `false`
either: *could not ask* and *asked and got no* are different sentences and
Chris needs to be told which one happened.

---

## Done when

1. `npm test` still passes with the same totals it has now: 7, 160, 297, 89,
   12, 29, 27, 61, 6. `tests/map-readiness.test.mjs` reads the source of
   `LLKMapReadiness.cpp` and guards that the document and the code still agree
   about the check codes, the eye height, the tags and the command name, so it
   is the suite most likely to notice a rename.
2. The in-engine tests still pass — 24 today. `LLK.Harness.MapReadiness` tests
   `Evaluate`, which has no world and no nav API, so it must keep passing
   unchanged; if it needs editing, something has moved into the pure half that
   does not belong there.
3. In PIE on a map with a built navmesh and a reachable spawn, `LLK.CheckMap`
   reports **no route finding at all** — not `RouteUnverified`.
4. On the same map with the `PlayerStart` raised three metres into the air, it
   reports `SpawnOffNavmesh` and nothing about the route.
5. On the same map with a wall closing off one `LLK.EnemySpawn`, it reports
   `NoRouteFromSpawn`.
6. Nothing in `Content/` is modified. `git status` after a run is clean.

Point 4 and point 5 are worth doing deliberately rather than trusting: both of
the bugs the last perception commit fixed were found by breaking something on
purpose and watching whether anything noticed.

## Out of scope

- Patrol routes, patrol authoring, and anything an enemy *does* with a path.
  That is M2.6 and it is mine.
- Building, generating or fixing the navmesh.
- Any change to `Content/`, including resaving a level.
- Making this a CI gate.
- Deciding whether the greybox is good. It reports whether it is *ready*.
  Those are Chris's two separate questions and only the second one is ours.
