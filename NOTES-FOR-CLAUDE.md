# Systems audit — first playable blockers

> **Status:** IN-FLIGHT — 2026-10-19. Codex audit; Claude owns fixes.

This is deliberately a finding list, not a patch. It keeps gameplay ownership in
Claude's lane while making the first playable definition testable.

## P0 — no player interaction reaches three core remotes

`LanternController` calls `LanternRequest`, but there is no client controller or
`ProximityPrompt` path for `LootRequest`, `ExtractionRequest`, or
`ReviveRequest`. A player cannot collect, leave, or revive through normal play.

**Done when:** interacting near a tagged loot item, extraction zone, and downed
teammate creates the corresponding validated remote request. The server remains
authoritative.

## P1 — validate physical interaction server-side

`LootService.handleExtraction` currently accepts an extraction ID without
checking a tagged extraction instance, its `ExtractionId`, player distance, or
request debounce. A client can request `LoadingDock` from anywhere; parallel
Freight Lift requests can also overlap. `ReviveRequest` similarly lacks
proximity, teammate, interaction-time, and solo-only self-revive validation.

**Done when:** the server resolves a nearby, tagged target and rejects remote
requests that do not correspond to a real local interaction.

## P1 — no damage/knockdown path

Sweepers can navigate but never damage a Humanoid or call
`RunStateService.knockDown`, so the forgiving wipe/revive rule cannot be tested.

**Done when:** one deliberately simple Sweeper contact path reliably produces a
knockdown, then permits a nearby teammate revive or the solo Anchor Revive.

## P2 — persistence test boundary

DataStore persistence needs a published test place with Studio API Services
enabled. In an unpublished/API-disabled test, `PlayerDataService` should keep
its current safe refusal to save; test that failure separately from payout flow.

## Physical map dependency — owned by Codex

The static map currently has no tags, marker parts, or explicit SpawnLocation.
The new map builder is addressing that ownership-side requirement. Once it lands,
verify at runtime that `RelayBank`, `Extraction`, `Loot`, and `SweeperSpawn`
each report nonzero tagged instance counts.
