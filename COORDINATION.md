# Working agreement — Last Lightkeepers

Three parties touch this repo. This file exists so two agents don't overwrite
each other's work. Agreed 2026-10-19.

## Lanes

| Party     | Owns                                                                                    |
| --------- | --------------------------------------------------------------------------------------- |
| **Claude** | Game systems: Storm Charge, lantern state, loot/banking, knockdown/revive, first Sweeper |
| **Codex**  | Integration and playability: Studio readiness, graybox/tag layout, Studio Assistant prompts, test checklist |
| **Chris**  | Creative authority: mall mood, Warden silhouette, Lanternhold look, balance values. Plays the first run. |

Claude stays out of map layout and visual polish. Codex does not change gameplay
logic. Neither agent expands scope, adds PvP, or alters a locked decision.

Nothing touching an account, publishing, payment, or public release happens
without Chris. No exceptions, no "it was only a test place."

## File ownership

One owner per path. If you need a change outside your lane, write it down for the
owner instead of editing it.

| Path                           | Owner  |
| ------------------------------ | ------ |
| `src/shared/*.luau`            | Claude |
| `src/server/**`                | Claude |
| `src/client/LanternController` | Claude |
| `src/client/HUDController`     | Claude — it renders run state; restyling is Codex's call |
| `data/*.json` (schema)         | Claude |
| `data/*.json` (values)         | Chris — Claude proposes, Chris decides |
| `default.project.json`         | Codex  |
| Map geometry, `.rbxmx`, tags   | Codex  |
| Studio prompts, checklists     | Codex  |
| `README.md`                    | Either — append, don't rewrite |
| `COORDINATION.md`              | Either — append, don't rewrite |

## The one real collision

Codex's lane description says it will "audit the Luau, fix the service
boundaries." The boundary rule says "Claude changes logic; Codex integrates."
Those two statements conflict, and `src/` is exactly where a silent overwrite
costs the most.

**Resolution:** Codex audits and reports; Claude applies. Findings go in
`NOTES-FOR-CLAUDE.md` as a list — file, line, what's wrong, why. Claude fixes
them in its own lane and notes what it did. If an audit finding is genuinely
integration-only (a Rojo path, a tag name, a `default.project.json` property),
Codex changes it directly; that is its lane.

This keeps the audit valuable without two writers in one file.

## Studio access — corrected 2026-10-19

An earlier version of this section said no agent can author in Studio directly.
**That was wrong**, and the correction matters enough to keep the record visible.

Studio now ships a **built-in MCP server**, and Roblox publishes an official
plugin that connects Claude Code and ChatGPT/Codex to it
(`Roblox/roblox-studio-agent-plugin`). The older standalone community server was
archived in April 2026 because this replaced it. With it enabled, a connected
agent can inspect the data model, read and edit scripts, execute Luau, run
playtests, capture the viewport, and simulate input.

Enabling it is Chris's click, once: in Studio, **Assistant → … → Settings → MCP
Servers → Enable Studio as MCP server**.

Roblox's own warning applies and is not boilerplate: an MCP client can read and
change the open place, so only connect clients you trust.

### Current state

- **Rojo 7.7.1 is installed and serving** on `127.0.0.1:34872` (Codex, verified:
  port listening on PID 28508, `FloodedMall-Playtest.rbxlx` built).
- **Rojo is not on Claude's PATH.** `command -v rojo` finds nothing from Claude's
  shell, and the binary isn't in `~/.rokit/bin`, `~/.aftman/bin`, or
  `~/.cargo/bin`. Claude therefore cannot run `rojo build` to check its own work.
  Codex: say where the binary lives, or add it to PATH.
- **Connecting the Rojo plugin in Studio is still a human click.**
- **Luau is still unverified.** Rojo does not typecheck. Nothing here has been
  compiled or linted. `luau-lsp`, `selene`, and `stylua` are all absent; `cargo`
  and `winget` are present, so installing them is possible with Chris's say-so.

## Git, and why it matters now

The repo is **not under version control**, and that already cost something: when
the session moved folders, every file's mtime was rewritten, so timestamps can no
longer distinguish "Codex edited this" from "the copy touched this."

Claude spot-checked its four most recent edits (`isLanternOn`, the `ExtractionId`
warning, `loadFailed`, the lantern echo handler) and all four are intact — so
Codex's "gameplay code intentionally not changed" holds up. But that was a spot
check, not a diff, and the next one may not be so clean.

**Nothing else should be built overnight until `git init` has run.** Without a
baseline, two agents writing to one tree have no way to prove what changed.

## Agents do not run unattended

Neither agent works while Chris sleeps. Both act only when invoked, and stop
when the turn ends. Scheduling exists but has not been set up, and nobody should
plan around overnight progress that will not happen.

## Logging

Append a dated line per working session. Newest at the bottom.

- 2026-10-19 — Claude: 15-file scaffold (Rojo map, balance JSON, shared
  Config/Types/Remotes, four server services, two client controllers). Luau
  unverified — no toolchain to check it against.
- 2026-10-19 — Claude: this file. Confirmed Studio running (PID 15380), no Rojo,
  repo not under git.
- 2026-10-19 — Codex: installed Rojo 7.7.1, added the Rojo-ready Flooded Mall
  graybox (`src/map/FloodedMall.rbxmx`), and built `FloodedMall-Playtest.rbxlx`
  successfully. Rojo serve is live at `127.0.0.1:34872`. Gameplay service code
  was intentionally not changed.
- 2026-10-19 — Claude: flattened the repo to
  `C:\Users\chris\Documents\Last-Lightkeepers` as Chris asked (stopped Rojo PID
  28508 first; its relative project path could not survive the move). Made the
  git baseline. Then built Days 4-5 of the systems lane: lantern charge drain,
  knockdown with movement lock and death interception, teammate revive as a
  proximity hold, solo Anchor Revive split out and now actually paying its cost,
  Sweeper contact knockdown with decaying hits, per-viewer HUD snapshots, and
  48 balance invariant tests that run on plain Node. `rojo build` green.
- 2026-10-20 — Claude: closed the P0 from Codex's audit. Server-side
  ProximityPrompts for loot, extraction and revive; extraction now validates a
  real tagged instance plus range, with an in-flight guard; Anchor Revive
  keybind; startup warns on an untagged map. Build green, tests 48/48.
- 2026-10-19 — Claude: verified Codex's claims (port listening, both map files
  present, gameplay markers intact). Corrected the Studio-access section: Studio
  has a built-in MCP server and an official Claude Code plugin, so the earlier
  "no agent can author in Studio" claim was wrong. Flagged that Rojo is not on
  Claude's PATH and that the repo still has no git baseline.
- 2026-10-19 — Codex: project flattened to the real root and Git initialized
  (no commit). Added `GalleriaDetails.rbxmx` plus the first-playtest script;
  rebuilt successfully with 27 total place parts and both Rain Garden lights.
  No Claude-owned gameplay code changed.
- 2026-10-19 — Codex: read-only first-playable audit recorded in
  `NOTES-FOR-CLAUDE.md`; interaction, server proximity validation, and the
  knockdown path are the P0/P1 systems blockers. Map construction continues in
  Codex's lane; no system code was changed.
- 2026-10-19 — Codex: added the authoritative generated-map slice
  `src/map/FloodedMallBuilder.luau` (spawn, two exits, relay, eight loot
  markers, four Sweeper spawns, tags/attributes, Galleria shell, Rain Garden,
  and lighting). Formatted with StyLua and Rojo build is green. Lune parses the
  module then stops at the expected `game` global absence outside Studio.
- 2026-10-19 — Codex: resolved the transient `GalleriaDetails.rbxmx` structural
  build error Claude recorded in `NOTES-FOR-CODEX.md`; regenerated the place
  from a green Rojo build at the flattened root. Rojo serve PID 38208 is the
  current, root-bound server.

## Commit hygiene with two agents in one tree (learned the hard way)

**Do not use `git add -A`.** Stage the paths you actually changed.

On 2026-10-20 commit `e65cee1` ("Record the Workspace.Map root cause for Codex")
says "Diagnosis only - map geometry and the builder are Codex's lane". That is
false about its own contents: a `git add -A` swept in Codex's in-flight fix to
`src/server/init.server.luau`, a README edit and wiring-test additions that
were sitting unstaged in the working tree at that moment. The code is good and
has been left alone; only the attribution is wrong, and it is recorded here
rather than rewritten, because another session was working in this tree at the
time and rebasing under it would have been worse.

The log is now slightly misleading about who wrote what in that commit. That is
exactly the failure COORDINATION.md was written to prevent, and it happened
anyway because the rule covered file ownership but not staging.

Rule going forward: `git add <explicit paths>`, and check `git status` before
committing when any other session may be live.

## 2026-10-20 - Lanes revised: one writer per lane

Three writers in one folder was too many, and two of them were the same lane.
The playtest session and this session are both Claude doing gameplay systems,
which is why work collided. Revised split, agreed by Chris and Codex:

| Party | Owns |
| --- | --- |
| **Playtest session** | Gameplay systems and runtime fixes. The only writer in `src/server/**`, `src/client/**`, `src/shared/**`. It can actually run the game, so it verifies its own work. |
| **Codex** | Map generation, Studio/Rojo integration, playtest artifact builds, verifying the generated map survives a sync. Will not touch InteractionService, client gameplay controllers, run state, loot or enemies while that session is active. |
| **This session** | Coordination and review. Holds the handoff and checks work against locked decisions. **Writes no gameplay code.** |
| **Chris** | Creative authority, all balance values in `data/*.json`, and every account, publish or release action. |

Why this session stopped writing code: everything it produces is unverified
until something else runs it, and that was the bottleneck all day. Its useful
contribution has been catching drift from the handoff - the solo-only Anchor
Revive being the example - not racing another session to the same files.

Staging rule still applies to everyone: `git add <explicit paths>`, never `-A`,
and check `git status` first. That is how Codex's fix ended up inside a commit
of this session's with a message that denied touching it.

## 2026-10-20 — Chris reassigned active lanes

Claude owns every mall/layout task. Codex must not edit `src/map/**`, Workspace
map objects, map tags, lighting, or playtest artifacts while Claude is building.
Codex owns the next enemy implementation pass, beginning with the existing
Glimmer specification in `data/enemies.json`. This is a direct reassignment by
Chris; map and enemy work remain separate.

## 2026-10-21 - Ownership is per FILE, not per folder

`src/map/` now has builders from two different authors: `FloodedMallBuilder`
and `GalleriaBuilder` from Codex, and `LanternholdBuilder` from the gameplay
session (afc5f9a). The folder-level rule above says map geometry is Codex's,
so on a strict reading that crossed a lane.

Nobody did anything wrong and nothing is being reverted. The gameplay session
built the hub from `docs/LANTERNHOLD.md` rather than inventing one, and it has
Studio open to verify with - which is why the pathfinding faults got caught at
all. But two writers in one folder is precisely the setup that caused the
earlier collisions, so the rule needs to be sharper rather than louder.

**The rule: whoever builds a thing owns that thing's file.** Not the directory.

| File | Owner |
| --- | --- |
| `src/map/FloodedMallBuilder.luau` | Codex |
| `src/map/GalleriaBuilder.luau` | Codex |
| `src/map/LanternholdBuilder.luau` | Gameplay session |
| `src/server/**`, `src/client/**`, `src/shared/**` | Gameplay session |
| `default.project.json`, Rojo and artifact builds | Codex |
| `docs/**`, `assets/MANIFEST.md`, `assets/audio/**` | This session |
| `data/*.json` | Chris |

A new builder belongs to whoever creates it. If you need a change in someone
else's file, write it in their notes file instead of making it.

This also keeps the real distinction intact, which was never really about
folders: **Codex owns getting things into Studio, the gameplay session owns
things that run.** A builder is geometry, but it is geometry expressed as code
that runs on the server, which is why the boundary was always going to be
blurry here.

### Two things from that commit that need someone

- **Nobody has seen Lanternhold.** Screen capture timed out on every attempt,
  so the lobby exists as 105 parts and six verified pathfinding routes, but no
  human or agent has looked at it rendered. Chris should walk it before anyone
  builds on top of it.
- **Rojo is not connected in Studio**, per that session - it has been pushing
  modules in by hand. Rojo itself is installed and serving on 34872; it is the
  Studio side that is not attached. That is Codex's lane and it is worth fixing
  before the by-hand pushes diverge from the repo.

## 2026-10-21 - What has actually RUN, and what has not

Established by asking rather than assuming, after the gameplay session reported
"nothing in src/server or src/client has ever actually run", which contradicted
its own earlier playtest. It corrected itself: that line was wrong, the earlier
playtest report is the accurate one, and it corrected me in the other direction
too. Recorded here because Chris is being shown a lot of work and the honest
line between verified and merely written is not obvious from the commit log.

**Has executed, in a live Studio session while Rojo was syncing:**
RunStateService, LootService, EnemyService, InteractionService,
PlayerDataService, the lantern and HUD client controllers, and the Sweeper
rig's audio. The measurements from that run stand: Storm Charge at 1.0/s dark,
the lantern at 2.5x draining 0.0342 per 5s against 0.0333 predicted, a
LoadingDock extraction paying Supplies x4, the teammate revive at 4.04s
measured client-side with GetServerTimeNow, and per-player relic visibility
proved with two real clients.

**Has never executed:** HubService, LiftService, the fall-damage change in
RunStateService, ExpeditionService and the Expedition client controller. All
written after the Rojo sync stopped.

**The map builders have executed many times** - injected through the Studio MCP
and run in a live datamodel. The pathfinding figures quoted for the mall and
Lanternhold are real measurements against real geometry, which is how the
escalator orientation, the sign band across the upper doorways and the stair
landing-pad lip were each found. I had called this "geometry that merely
exists" and that understated it. What it has *not* been is walked by a player,
and it has never run alongside the gameplay code, because the two cannot
currently be in the same place at once.

**Why the split exists:** the Rojo sync broke at an unknown point. The
signature was Workspace.Map holding the baked FloodedMall and GalleriaDetails
models with no GalleriaBuilder - Rojo not attached.

### Blocking, and it is Chris's

`RojoManagedPlugin.rbxm` is in `%LOCALAPPDATA%\Roblox\Plugins` (12:08, installed
with `rojo plugin install`, a local file drop - no Roblox account touched). It
has **not** loaded: the Plugins tab showed only the Terrain group at 12:10,
despite the Studio process having started 12:08:15. Studio also has a modal
"Studio is unable to connect, check your network connection" dialog open that
the session could not dismiss.

So the restart is still outstanding, and Chris has unsaved work in the place.
Until Studio restarts with the plugin loaded and Rojo is connected, nothing
written since the sync broke can run at all.
