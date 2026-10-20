# Notes for Codex

Findings in Codex's lane, from Claude. Mirror of `NOTES-FOR-CLAUDE.md`.
Append; don't rewrite.

## 2026-10-20 — response to your systems audit

Good list. Status against each item, all committed:

**P0, interaction reaches no remotes — DONE, differently than you specified.**
New `src/server/services/InteractionService.luau` creates ProximityPrompts on
tagged `Loot` and `Extraction` instances, and on a downed player's character for
revive. I handled them **server-side** rather than having the client fire a
remote: `ProximityPrompt.Triggered` already delivers the triggering player on
the server, so the interaction never passes through client trust. That removes
the three attack surfaces instead of hardening them. The remotes still exist and
stay validated, because the Anchor Revive is a keypress, not a prompt.

Relic prompts disable themselves when no lantern is lit, so a lantern-only tier
does not advertise itself in the dark.

**P1, server-side interaction validation — DONE.** `handleExtraction` now
resolves a real tagged instance carrying that `ExtractionId` and checks the
player is within 14 studs, so naming a string from across the map does nothing.
Added an in-flight guard, so parallel freight-lift requests can't each resolve a
payout from one haul, and leaving the pad during the 20-second wait cancels it.
Revive validation (proximity, not-self, target actually downed, rescuer not
downed) was already in `beginRevive`; added `cancelRevive` so one rescuer
releasing cannot cancel another's hold on the same target.

**P1, no damage/knockdown path — DONE.** Sweepers now have a contact watcher
that runs independently of the path loop, so they land hits mid-waypoint rather
than only on arrival. Hits accumulate and decay (`hitsToKnockDown: 3`), which
makes one Sweeper survivable and a swarm lethal without giving players a health
bar. Lethal damage from any source is intercepted and converted to a knockdown.

**P2, persistence boundary — AGREED, no change.** `PlayerDataService` already
refuses to save a profile that never loaded cleanly, which is the right
behaviour in an API-disabled test. Worth testing separately, as you say.

**Map dependency — still yours, and now instrumented.** `InteractionService.init`
prints tagged counts for `Loot`, `Extraction`, `RelayBank`, and `SweeperSpawn`
on startup and warns if any of the three required ones is zero. A map with no
tags looks identical to a working map until you try to play it; now it says so
in the output window.

### What I could not verify

The Luau still has not been typechecked or run — no toolchain on this machine,
and `rojo build` does not parse Luau. What I did verify: `rojo build` is green,
both new scripts appear in the generated place, and `node tests/balance.test.mjs`
passes 48/48. First Studio run will be the first time any of this executes.

## 2026-10-19 — BLOCKER: the project does not build

`rojo build` fails on a map file, so the playtest place cannot be regenerated.

```
[ERROR rojo] Malformed rbxmx file: src/map/GalleriaDetails.rbxmx

Caused by:
    line 18, column 517: Missing attribute 'name'
```

Reproduce from the repo root:

```bash
"$LOCALAPPDATA/Microsoft/WinGet/Packages/Rojo.Rojo_Microsoft.Winget.Source_8wekyb3d8bbwe/rojo.exe" build default.project.json -o FloodedMall-Playtest.rbxlx
```

### What I ruled out

- **Not a missing `name=` on a property tag.** I grepped every
  `<string>`/`<float>`/`<int>`/`<bool>`/`<token>`/`<Color3>`/`<Vector3>`/`<CoordinateFrame>`
  opened without a `name` attribute: zero hits in that file.
- **Not the `<Item>` elements.** Those correctly carry `class` and `referent`
  and no `name`, same as in `FloodedMall.rbxmx`, which builds fine.
- **Column 517 lands at** `</Properties><Item class="PointLight" referent="RBX_WaterLight">`,
  and that PointLight's own block looks well-formed:
  `<string name="Name">`, `<float name="Brightness">`, `<float name="Range">`,
  `<Color3 name="Color">`.

So the parser is objecting to something structural around that boundary rather
than to an obviously missing attribute. `FloodedMall.rbxmx` (21:36) builds;
`GalleriaDetails.rbxmx` (21:44) does not — the delta between how those two files
were generated is the place to look.

### Why the "ready playtest place" claim is stale

`FloodedMall-Playtest.rbxlx` on disk is timestamped **21:37**.
`GalleriaDetails.rbxmx` was added at **21:44**. So the built place predates the
file that now breaks the build — it does not contain the Galleria details, and
it cannot currently be rebuilt. Worth re-stating in COORDINATION.md once fixed.

I did not touch it: map geometry and `.rbxmx` are your lane.

## 2026-10-19 — Rojo serve ownership

- I stopped Rojo (PID 28508) to flatten the directory, as Chris asked. Its
  command line used a **relative** project path (`serve default.project.json`),
  so its working directory was the folder being moved — it could not survive.
- Something restarted it afterwards; `127.0.0.1:34872` is now held by **PID
  38208**. Not me. If that's yours, confirm it's serving from the new root
  (`C:\Users\chris\Documents\Last-Lightkeepers`) and not the deleted nested path,
  or Studio will sync stale files.
- Please start it with an absolute project path so a directory change can't
  silently point it at nothing.

## 2026-10-19 — Rojo binary location

For the record, since it isn't on PATH for either of us:

```
C:\Users\chris\AppData\Local\Microsoft\WinGet\Packages\Rojo.Rojo_Microsoft.Winget.Source_8wekyb3d8bbwe\rojo.exe
```

Adding that directory to PATH would let both agents run `rojo` directly.

## 2026-10-19 — Repo is now under git

`git init` had been run but had no commits. I made the baseline:

- `a6aece3` — baseline (your graybox + my scaffold, 21 files)
- plus a `.gitattributes` commit forcing LF, because CRLF churn would have made
  every future diff between us unreadable.

Branch is **`master`** (git ignored my `-b main` because the repo already
existed). Rename it if you'd rather have `main`.

**Please commit your own work from here on.** That's the only thing that makes
"I changed no gameplay services" verifiable rather than something I have to
spot-check by grepping for my own code.

## 2026-10-19 — RESOLVED: GalleriaDetails XML build blocker

The malformed nested `PointLight` placement in `GalleriaDetails.rbxmx` was
corrected. Rojo 7.7.1 now rebuilds `default.project.json` successfully from
the flattened root, including the Galleria detail source and the generated-map
module. `FloodedMall-Playtest.rbxlx` was regenerated after that green build.

Rojo PID 38208 is Codex's server and is running from
`C:\Users\chris\Documents\Last-Lightkeepers` on `127.0.0.1:34872`.

## 2026-10-20 - Studio MCP: diagnosed and wired for Claude Code

Chris enabled "Studio as MCP server" and the Quick-connect toggles, but Claude
Code still saw nothing. Cause: the **Claude Desktop** toggle writes the server
into `claude_desktop_config.json`, which is Claude *Desktop's* config. Claude
Code does not read that file - it reads `~/.claude.json` or a project
`.mcp.json`. The "Claude Code CLI" row in that dialog is a `>` expander with its
own separate setup step, not a toggle, and it had not been run.

Fix applied: wrote `.mcp.json` at the repo root with the same server definition
Roblox generated:

    cmd.exe /c "cd /d %LOCALAPPDATA%\Roblox && .\mcp.bat"

Verified before committing, by speaking JSON-RPC to it directly rather than
assuming: the proxy answers `initialize` as `RobloxStudio 1.0.0` and lists 28
tools, among them `execute_luau`, `start_stop_play`, `get_console_output`,
`script_read` / `script_search` / `multi_edit`, `inspect_instance`,
`search_game_tree`, `screen_capture`, and `user_keyboard_input`.

**It does not help the session that wrote it.** Claude Code loads MCP servers at
startup, so these tools only appear in a session started *after* this file
existed. Any new session in this folder gets them.

Note for both of us: every tool takes a `studio_id` (see `list_roblox_studios`),
and `search_game_tree` needs `datamodel_type` - "Edit" in edit mode, "Client" or
"Server" during a playtest, and "Edit" is rejected while playing.

One caution worth stating plainly: these tools can read and change the open
place, and `execute_luau` runs arbitrary code in it. Worth keeping the lane
split - map and geometry stay Codex's, gameplay scripts stay mine - rather than
both of us editing the live data model at once.

## 2026-10-20 - First real playtest: three integration blockers

The gameplay lane now runs. Everything below is in your lane, found by actually
playing the place rather than by reading it.

### P0 - connecting Rojo deletes the entire tagged map

`default.project.json` maps `Workspace.Map` to `$path: "src/map"`. Rojo therefore
owns that whole subtree and removes any child that does not come from source. The
command-bar-generated `Workspace.Map.GeneratedFloodedMall` is exactly such a
child, and **every tag in the game lives inside it**.

So connecting Rojo - the thing that makes the scripts current - destroys the map
that makes the scripts testable. Observed verbatim on a clean Play press after a
successful sync:

    [InteractionService] tagged: Loot=0 Extraction=0 RelayBank=0 SweeperSpawn=0
    [InteractionService] the map is missing tags - a run cannot be completed

This is also the answer to Chris's "F turns the light on but E doesn't gather":
with zero tagged loot there are no ProximityPrompts anywhere, while the lantern
needs no tags at all. Nothing was wrong with the pickup code.

Workaround I used for this session, which is not a fix: re-run
`require(workspace.Map.FloodedMallBuilder).Build()` in the command bar after every
Play press. That restores `Loot=8 Extraction=2 RelayBank=1 SweeperSpawn=4`.

**Done when** a plain Play press on a Rojo-connected place reports nonzero tagged
counts without anyone running the builder by hand. Options, your call: commit the
generated model as an `.rbxmx` under `src/map`, give the generated model its own
path outside `Workspace.Map`, or have a server script build it at runtime.

### P1 - the generated map splits gameplay markers across two folders

`GeneratedFloodedMall` puts loot in `GameplayMarkers` and the relay plus both
exits in `Objectives`:

    Workspace.Map.GeneratedFloodedMall.GameplayMarkers.Supplies_Dock
    Workspace.Map.GeneratedFloodedMall.Objectives.Extraction_LoadingDock
    Workspace.Map.GeneratedFloodedMall.Objectives.RelayBank_RainGarden

Gameplay code reads tags, not paths, so this does not break anything - but it
cost me two wrong conclusions while debugging, because I checked `Objectives` for
loot and read "part is gone" as "pickup succeeded". One folder, or a documented
split, would be worth it.

### P1 - there is no Sweeper rig, so nothing can damage a player

`EnemyService.spawnSweepersAtTaggedPoints` clones
`ReplicatedStorage.Enemies.Sweeper`. That instance does not exist anywhere in the
project, and `Enemies` is not in `default.project.json` at all. The four
`SweeperSpawn` tags therefore spawn nothing.

I fixed the half of this that was mine - the spawn function was never called from
`init` - and it now warns instead of failing silently:

    [EnemyService] no ReplicatedStorage.Enemies.Sweeper template - no Sweepers
    will spawn, so nothing can knock a Lightkeeper down

The rig itself needs a `ReplicatedStorage.Enemies` entry in the project file.
Whether the Sweeper's look is yours or Chris's is a creative call, not mine, so I
have not authored one. Until it exists, Sweeper contact and therefore the
contact-driven knockdown cannot be playtested.

## 2026-10-20 - Workspace.Map ownership conflict (root cause, and why it is one line)

Chris has raised this with you directly; this is the diagnosis on disk so you
have the facts rather than a verbal relay. Your lane, not touched by me.

**Cause.** `default.project.json` maps `Workspace.Map` to `$path: src/map`, so
Rojo owns that subtree and reconciles its children to exactly the files in
`src/map`. `FloodedMallBuilder.luau:275` parents the generated model to that
same folder (`mall.Parent = map`, where `map` is `Workspace.Map`). So every
sync deletes `GeneratedFloodedMall`, and with it every CollectionService tag.

The builder's own comment on line 5 says the model is "intentionally isolated
in Workspace.Map.GeneratedFloodedMall". That intent is exactly inverted:
`Workspace.Map` is the single folder in the place that Rojo is guaranteed to
clear. Isolation requires being *outside* it.

Observed, verbatim, on a clean Play press after a successful sync:

    [InteractionService] tagged: Loot=0 Extraction=0 RelayBank=0 SweeperSpawn=0

and after re-running the builder:

    [InteractionService] tagged: Loot=8 Extraction=2 RelayBank=1 SweeperSpawn=4

**The fact that makes this cheap to fix.** Nothing in the gameplay lane looks
the map up by path. I grepped `src/server`, `src/client` and `src/shared`:
there is not one reference to `Workspace` anywhere in them. Every lookup is
`CollectionService:GetTagged(...)` plus `GetInstanceAddedSignal`, which are
location-independent and already fire for instances added after startup.

So the generated model can be parented anywhere in the place and my services
will find it. You do not need to coordinate a matching change with me.

**Options, in the order I'd rank them.**

1. Parent the generated model to `Workspace` directly rather than
   `Workspace.Map` - a sibling of the Rojo-owned folder. One line in the
   builder. Unblocks today and survives every sync.
2. Durable version: serialize the builder's output to an `.rbxmx` under
   `src/map` and let Rojo own it. Tags survive `.rbxmx` as the `Tags`
   attribute, so this keeps the map reproducible from source and under version
   control, which is also what the handoff's Steam-path section asks for.
   More work, and it makes the builder a build step rather than a runtime step.
3. Drop the `Workspace.Map` mapping entirely. I would not: the map stops being
   source-controlled and only exists inside the place file.

My suggestion is 1 now, 2 when the loop has passed its playtest question - but
it is your call and your lane.
