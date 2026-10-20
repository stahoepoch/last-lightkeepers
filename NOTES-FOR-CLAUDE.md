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

## 2026-10-20 — runtime blocker in the first Studio playtest

The generated Flooded Mall is structurally ready in Studio: `Loot=8`,
`RelayBank=1`, `Extraction=2`, and `SweeperSpawn=4`, with both extraction IDs
present. The player spawns and the lantern/HUD client path runs.

However, Output reports `You must publish this place to the web to access
DataStore` from `ServerScriptService.Server.services.PlayerDataService`, line 9,
followed by `Requested module experienced an error while loading` at
`ServerScriptService.Server`, line 16. As a result, no `ProximityPrompt`
instances are created in the running client, including at the relay and exits.

Please make the API-disabled local-playtest path non-fatal: continue startup
with persistence disabled, while retaining the current fail-closed rule that a
profile which did not load cleanly must never be saved. This is not a map tag
or placement issue.

## 2026-10-20 - Answered: the DataStore blocker, and what the playtest found

The API-disabled path was already non-fatal before this session started:
`PlayerDataService` wraps `GetDataStore` in a pcall and warns, keeping the
fail-closed rule that a profile which never loaded cleanly is never saved. The
errors in the note above came from a stale build, not from current source.

Fixed in this lane during the first real playtest (all verified running, not read):

1. **Solo knockdown wiped the run instantly, making the Anchor Revive dead code.**
   `knockDown` ran its whole-crew-down check and called `wipe()` inside the same
   call that downed the last player. With a crew of one that is always true, so a
   solo player was hard-wiped on their first knockdown - including with a full
   lantern charge - and `anchorRevive` could never run, because it bails on
   `run.finished`. The wipe now defers while any downed player can still pay for
   an Anchor Revive, via a shared `canAnchorRevive` predicate. A crew with no
   charge and no anchors left still wipes.
2. **The relay was never wired when the map arrived late.** `LootService.init`
   scanned `RelayBank` once; the generated map is tagged after services start, so
   banking silently did nothing. Now watched with `GetInstanceAddedSignal`, like
   loot and extraction.
3. **Sweepers never spawned.** `spawnSweepersAtTaggedPoints` was never called
   from `init`. Now called, plus a watcher for late tags, plus a loud warning for
   the missing rig (see NOTES-FOR-CODEX.md - the template does not exist).
4. **The Anchor Revive refused silently.** `ReviveController` suppressed R from a
   locally re-derived price that did not know about `anchorRevivesLeft`. The
   snapshot now carries `yourCanAnchorRevive` and `yourAnchorRevivesLeft`, the
   client gates on the server's answer, and the HUD says which cost is unpaid.
5. **The run-end summary was unreadable.** Extraction does not stop the run tick,
   so the next snapshot overwrote the payout line within 250ms. The HUD now
   freezes its label once a run has ended.

Known gap left deliberately unfixed, documented at the function:
`InteractionService` hides relic prompts crew-wide rather than per-player.
`ProximityPrompt.Enabled` is a property of the instance, so a server-created
prompt cannot be shown to one player and hidden from another. Pickup is still
correct - `isVisibleTo` re-checks the acting player - so this leaks a prompt, not
loot. Closing it means moving relic visibility to the client, which is a real
change rather than a one-liner.

Still unreached, honestly: teammate revive needs a second player, and Sweeper
contact needs a rig that does not exist yet.

## 2026-10-20 - Sweepers exist now. One open item for Chris.

The Sweeper rig is built procedurally in `src/server/services/SweeperRig.luau`
and created at `EnemyService.init`, so no project mapping and no `.rbxmx` - both
Codex's lane - were needed. Contact knockdown is verified in a running game for
the first time: a Sweeper noticed a dark player at 10 studs, closed to under 3,
and downed them at t=4.6s, which is three contacts at the configured 1.5s
cooldown plus the 1.0s telegraph.

**RESOLVED same day - the telegraph has audio.** Chris had no sound to give, so
rather than leave an accessibility gap open I sourced one instead of inventing
one: "Servo Motor 7 (SFX)", asset `9118959688`, from the Pro Sound Effects
library Roblox licenses for creator use. Free, and it loads in an unpublished
place. Short servo bursts, which is what a former floor cleaner should sound
like noticing you.

Verified rather than assumed, which is the part that matters for an `rbxassetid`:
searched the Creator Store, then loaded it in a running playtest and read back
`IsLoaded = true` and `TimeLength = 1.11s`. That 1.11s covers
`audioTelegraphSeconds` of 1.0, so the warning is still sounding when the first
hit becomes legal rather than having finished before it. In play the telegraph
fired at t=0.45s on target acquisition and the knockdown landed at t=4.95s.

The rule is now enforced in timing *and* in fact. The id sits in one named
constant with its provenance beside it, so swapping the Sweepers' voice is a
one-line change; everything around it - timing, falloff, range - is independent
of the choice. `SweeperRig` also warns if a future sound is shorter than
`audioTelegraphSeconds`, because a warning that ends before the hit it announces
is worse than none.

One interpretation worth a second pair of eyes: the handoff says "before sight",
and I gated *contact* rather than detection. A Sweeper telegraphs when it
acquires a target and again if a player walks into an idle one, and may not hit
until the delay has elapsed. That guarantees sound precedes damage in every
path. Gating detection itself would mean a Sweeper that sees you but cannot act
for a second, which reads worse and is not what the sentence requires.

## 2026-10-20 - The two-player pass. All three co-op behaviours verified.

Chris got Studio's Clients and Servers running with 2 players, which turned out
to expose three separately addressable Studio instances - one server and one per
client - so each player could be driven independently. Everything below was
previously impossible to test and is now confirmed in a running game.

**Relic visibility is genuinely per-player.** Both players standing at
`Relic_Waterfall`, Player 1 lit, Player 2 dark:

    Player 1 client : relic prompt Enabled = true
    Player 2 client : relic prompt Enabled = false   (while seeing activeLanterns = 1)
    Server          : relic prompt Enabled = false

Player 2's snapshot shows `activeLanterns = 1`, so the crew-wide fact is visible
to them and the per-player decision still comes out right. Under the old
crew-wide toggle Player 2 would have seen the prompt. That closes the gap for
real rather than by argument.

**Teammate revive works, and takes exactly as long as configured.** Player 2
downed, Player 1 three studs away: 4.04s from request to standing, against
`teammateHoldSeconds` of 4.

A measurement caution worth keeping: the first run of this measured 7.78s and I
nearly reported a double-count bug. The extra 3.7s was my own tool round-trip
between arming the watcher and firing the request. Re-measured with
`workspace:GetServerTimeNow()` taken on the client immediately before firing, it
is 4.04s. Time co-op interactions inside one call, or with a synced clock.

**The crew wipe fires, and the solo-only gate is what allows it.** Both players
downed with `crewSize` 2: neither could Anchor Revive, so the wipe ran. Player 1
carried Components x2 and the summary read

    "Lanterns burned out - half of what you carried is gone  |  Components x1"

which is `unbankedKeptOnWipe` 0.5 applied correctly, while Player 2 - carrying
nothing - got "nothing recovered". Per-viewer payouts are right. Snapshots
stopped, confirming `run.finished`. This is the runtime verification 2bb8b84
was committed without.

### Open, not fixed: the revive costs twice what it should

The RevivePrompt is created with `HoldDuration = teammateHoldSeconds` (observed
live: `hold=4s`), and `Triggered` only fires once that hold completes. The server
then runs its own 4s through `stepRevives` (observed: 4.04s from request to
standing). So a real player holds E for 4s and then waits another 4s - about 8s
for a "4-second proximity hold".

Both halves are observed; the 8s total is inferred, because simulated input does
not reach a ProximityPrompt with a non-zero hold and I could not press the key
for real. Chris can confirm it in two windows in about ten seconds.

It is left unfixed because the three fixes trade off differently and the choice
is a feel decision, not a correctness one:

- Prompt taps (`HoldDuration` 0), server keeps the 4s. Matches the documented
  "4-second proximity hold... walking away cancels it" and keeps timing
  server-authoritative, but loses the prompt's built-in progress bar, so the
  rescuer gets no feedback that anything is happening.
- Prompt keeps the 4s hold and `Triggered` stands them up immediately. Keeps the
  progress bar, but moves the timing to the client - a modified client could
  revive instantly. Proximity is still server-checked.
- Prompt taps and the HUD draws the progress from a new snapshot field. Correct
  on both axes, and the most work.
