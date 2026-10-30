# Handoff to Codex — the dock map must exist on the client, and nothing checks that

Written 2026-10-30 by Claude, after the first two-client PIE session. One
requirement, one check code, and a trap that costs a week if the authored dock
is built the way the harness was.

Chris ran the first listen-server session tonight: two keepers, one process,
`PlayNumberOfClients=2`, `PIE_ListenServer`. It half worked, and the half that
failed is not where anyone would look.

---

## What happened

Chris, from the host window: *"i was able to find the other character and bump
into them, i couldnt see them. on their screen, nothing rendered, but their
light could be seen on my server."*

Three separate facts, and only one of them is a defect.

**The teammate was invisible, and that is correct.** `ALLKCharacter` assigns no
mesh — not a missing one, none at all. The mesh is `Content/` and row 41 kept
it out of agent hands deliberately. An invisible keeper with a working collision
capsule is exactly what this build should produce today. Do not fix it.

**Their lantern lit the host's world, and that is the M1 proof.**
`ULLKLanternComponent::bLanternOn` is a `DOREPLIFETIME`; the client toggled it,
the server decided, the state replicated, and it had a visible effect on another
player's screen. Replicated gameplay state crossed the wire. That is the thing
M1 exists to demonstrate and it demonstrably works.

**The client's screen was black, and that is the defect.**

---

## Why the client rendered nothing

`BuildHarnessWorld()` is called from `ALLKTestGameMode::InitGame`
(`LLKTestGameMode.cpp:121`). **A GameMode exists only on the server.** Clients
never call it, so clients never build the harness. A client receives exactly
what is replicated to it and not one actor more:

| Spawned by the harness | Reaches a client |
| --- | --- |
| Keepers (`ALLKCharacter`) | yes — but no mesh, so invisible |
| Relics (`SpawnLoot`) | yes — `SetReplicates(true)`, the only place it is set |
| Sweeper (`ALLKSweeper`) | yes — `bReplicates = true` |
| Extraction zone (`ALLKExtractionZone`) | yes — `bReplicates = true`, and invisible anyway |
| **The 50m slab** (`SpawnBlock`) | **no** |
| **The 55 store blocks** (`SpawnBox`) | **no** |
| **The sun** (`ADirectionalLight`) | **no** |
| **The sky light** (`ASkyLight`) | **no** |

So the client's world was a handful of floating relic cubes and an invisible
teammate, lit by nothing whatsoever. Black is the correct render of that scene.

Bumping into each other still worked because movement is server-authoritative:
the server has the floor, so it kept both capsules standing on it and
replicated the positions down. Chris's teammate was walking around on a floor
that did not exist on their machine.

---

## The obvious fix is a trap — do not replicate the lights

`SetReplicates(true)` on `ADirectionalLight` and `ASkyLight` will **not** work,
and it will fail in a way that looks like success.

Intensity and colour are set on the light *component* at spawn
(`LLKTestGameMode.cpp:198-230`) and are not replicated properties. A replicated
light actor arrives on the client at default intensity and default white — the
harness authors a deliberately weak, cool sun precisely so the keeper's warm
lantern reads as the one human light in the shot. A default-brightness white sun
on the client washes the lantern out completely, which destroys the single thing
the harness exists to test, while appearing to have fixed the black screen.

Deterministic geometry and lighting want **local construction on every net
mode**, not replication. Replicating 56 static mesh actors is bandwidth spent to
duplicate something both ends could compute identically.

---

## The requirement this exposes, and why nothing caught it

`docs/MAP-READINESS.md` lists fourteen things a map must be true about and
`LLK.CheckMap` reports which ones it is not. **Every one of the fourteen is a
server-side property.** Floor collision, nav bounds, nav data, projection,
routes, occluding geometry, sightline length, the two test lanes — all of them
are measured by iterating the server's world and tracing against the server's
geometry.

So a map whose geometry exists only on the server **passes all fourteen**. The
harness passes `LLK.Harness.MapReadiness` today, and we have just watched a
client in that same harness see absolutely nothing. That is the state row 72
already warned about in another context: green reads as evidence, and a suite
that passes while guarding the wrong thing is worse than no suite.

Proposed as requirement 15, for whoever lands it:

| # | Requirement | Code when absent | What you would otherwise see |
| --- | --- | --- | --- |
| 15 | **Playable geometry and lighting that exist on a client**, not only on the server | `GeometryServerOnly` | A host who can play and a client who renders black. Every other check passes, so the failure reads as a renderer or GPU fault rather than as a replication boundary |

Writing the check is harder than writing the rule, because the natural place to
ask it is on a client and `LLK.CheckMap` runs with authority. The cheap honest
version is an inspection rather than a measurement: report `GeometryServerOnly`
when the world's floor and light actors were spawned at runtime and are neither
replicated nor built on all net modes. That is narrower than the real rule and
would have caught tonight exactly.

---

## What the dock map must do, and the good news

**Geometry and lighting belong in the `.umap`, not in a runtime spawn.** Level
actors load on every client natively — no replication, no GameMode, no
authority question, and none of this trap. If `LLK_DockBay` carries its floor,
its walls, its sun and its sky as placed actors, requirement 15 is satisfied for
free and cannot regress.

The failure mode only appears when something is **spawned at runtime from the
GameMode**. So the rule for the dock, in one line: spawn only actors that carry
replicated state — relics, Sweepers, the extraction zone — and place everything
a player needs to see.

This is also why the harness is not worth repairing properly. It describes
itself in its own log line every single run: *"Disposable greybox; the authored
store replaces all of it."* Fixing its replication builds a path the authored
dock will never use. Chris has assigned the maps to you (row 41, ratified
2026-10-30), so the correct resolution of this bug is that the harness stops
being the thing we play in.

If a renderable two-client harness is wanted in the meantime, the minimum that
actually works is local construction of geometry and lights on clients, with
relics, Sweeper and zone left server-spawned and replicated — and the local
build must then exclude the stateful actors, or a client gets one local relic
and one replicated relic standing in the same place.

---

## Status

Not started by Claude. `Source/LLK/Private/LLKTestGameMode.cpp` had 108
uncommitted lines of navmesh work in it when this was written and Codex was
assigned the harness fix, so row 27 applies — one writer per lane at a time, and
this file is a handoff rather than an edit.

Requirement 15 is **proposed, not landed**: `docs/MAP-READINESS.md` still lists
fourteen, and `ELLKMapCheck` has no `GeometryServerOnly`. Whoever adds it owns
updating both plus a test, and `docs/MAP-READINESS.md` is Claude's lane.
