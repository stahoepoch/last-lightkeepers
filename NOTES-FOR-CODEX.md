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
