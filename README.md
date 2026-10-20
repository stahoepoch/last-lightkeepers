# Last Lightkeepers — Roblox scaffold

Week-one graybox skeleton. Source of truth for design is the project handoff
(`last-lightkeepers-claude-handoff-2026-10-19.pdf`); this repo implements it and
does not reinterpret it.

## Layout

```
default.project.json     Rojo sync map
data/                    Balance values as plain JSON — the portable layer
  storm-charge.json      Baseline rate, lantern multiplier, thresholds, Anchor Revive
  loot.json              Tiers, visibility rules, banking, extraction
  enemies.json           Sweepers, Stockers, Glimmers, Warden
src/shared/
  Config.luau            Requires the JSON, exposes stormRate() / thresholdAt()
  Types.luau             RunState, LightkeeperState, Haul, RunSnapshot
  Remotes.luau           Remote creation + lookup; every remote is a *request*
src/server/
  init.server.luau       Entry point: wires services, starts the run
  services/
    RunStateService      Storm Charge, lanterns, knockdown/revive, banking, wipe
    LootService          Spawning, visibility, pickup, extraction payout
    EnemyService         Sweepers implemented; others stubbed with config in place
    PlayerDataService    DataStore profiles, workshop materials, contributions
src/client/
  LanternController      F / gamepad Y to toggle; server echoes authoritative state
  HUDController          Storm Charge meter with threshold ticks, run-end summary
```

## Getting it into Studio

```bash
rojo serve
```

Then connect from the Rojo plugin in Studio. The place starts empty of geometry —
Day 1 of the roadmap is building the crude mall by hand.

**Rojo note:** `data/*.json` is synced as JSON modules (a `.json` file becomes a
ModuleScript returning the decoded table). That needs Rojo 7+. If your version
doesn't support it, rename them to `.luau` returning a table — `Config.luau` is
the only file that requires them, so it's a one-line change per file.

## Tagging the map (CollectionService)

Day 1–3 need these tags applied to parts you build:

| Tag            | On                          | Attributes                              |
| -------------- | --------------------------- | --------------------------------------- |
| `Loot`         | each pickup part            | `Tier` (string), `Amount` (number)      |
| `RelayBank`    | the mid-run relay volume    | —                                       |
| `Extraction`   | loading dock, freight lift  | `ExtractionId` = `LoadingDock`/`FreightLift` |
| `SweeperSpawn` | Sweeper spawn points        | —                                       |

`Tier` must be one of `Supplies`, `Components`, `Relics`, `RestorationCrates`.

## Tuning anchors in place

From the handoff, as initial placeholders only:

- **8 minutes** to the first threshold in permanent darkness (`baselineRatePerSecond: 1.0`, first threshold at `480`).
- **~2.5×** baseline with one active lantern (`perLanternBonus: 1.5`).
- Thresholds at 480 / 720 / 960 → in darkness 8 / 12 / 16 min; with one lantern 3.2 / 4.8 / 6.4 min.

`maxMultiplier: 5.0` caps a fully-lit four-crew. **This cap is untested and is
the most consequential number in the file** — a four-person lit run currently
reaches the Warden in about 3.2 minutes. Test it before trusting it.

## Deviations from the handoff, and why

1. **`RunService` → `RunStateService`.** The handoff's Studio layout names a
   server module `RunService`, which collides with Roblox's own `RunService`.
   Same responsibility, non-colliding name.
2. **Loot tier/examples mapping is inferred.** The loot table is still collapsed
   in the rendered PDF, so `data/loot.json` records my reading
   (Supplies → workshop, Components → gadget crafting, Relics → cosmetics and
   landmark contributions). Correct it there if I guessed wrong.

## Tests

```bash
node tests/balance.test.mjs
```

48 invariant checks over `data/*.json` — no Roblox, no Rojo, no Luau toolchain
needed. They assert that the numbers still match the handoff: the 8-minute dark
anchor, the 2.5× lantern rate, exactly one lantern-only loot tier, banked loot
fully surviving a wipe, the Warden unkillable, the locked Glimmer scan/tag
values, and that a lone Sweeper cannot stack a knockdown.

They do **not** test gameplay code. The Luau is still untypechecked — see
`rokit.toml` for the one command that fixes that.

## Not built yet

Day 4 onward is deliberately incomplete, per "build one enemy before the full
roster": Stockers, Glimmers, and the Warden have config and hooks but no
behaviour. Lantern charge is tracked but never drains. The Lanternhold hub,
workshop UI, and Community Kitchen landmark are untouched.

## Flooded Mall vertical slice — 2026-10-19

The first playable level source lives in `src/map/FloodedMall.rbxmx`. It is a
fictional Johnstown-inspired mall graybox: two-level concourse, upper galleries,
Food Court overlook, sunken Rain Garden, stormfall water feature, relay bank,
Loading Dock exit, and deeper Freight Lift exit.

`FloodedMall-Playtest.rbxlx` is a generated ready-to-open Studio place. It was
built successfully with Rojo 7.7.1; edit source and rebuild rather than editing
that generated file by hand.

Rojo is serving locally at `127.0.0.1:34872`. In Studio, install the Rojo plugin
if needed, then connect to that address. The first Studio-only follow-up is
applying the three CollectionService tags and two extraction attributes listed
in `src/map/README.md`.

The visual layer is separated from the floorplan in
`src/map/GalleriaDetails.rbxmx`: teal roof trusses, translucent glass gable,
white upper-gallery rails, shallow blue stormwater, planted Rain Garden beds,
and an amber relay beacon. This keeps the Johnstown-inspired mood readable
without using real mall branding or a copied layout.

When Studio MCP is connected, use `src/map/FIRST_PLAYTEST.md` for the first
walkthrough. It protects the only question that matters at this stage: does a
new player understand where to go and why to risk turning on light?

## Generated vertical slice

`src/map/FloodedMallBuilder.luau` is the authoritative, repeatable first-run
map build. The server calls it automatically on every Play session, before it
registers loot, extraction, relay, and enemy tags. This matters because Rojo
owns `Workspace.Map` and removes the generated model on sync.

To refresh the slice while staying in Edit mode, run:

```luau
require(workspace.Map.FloodedMallBuilder).Build()
```

It rebuilds only `Workspace.Map.GeneratedFloodedMall`, so a rebuild cannot
delete hand-authored work. The generated slice includes its own SpawnLocation,
tagged RelayBank and Extraction parts, eight typed Loot markers, four
SweeperSpawn markers, the main concourse/storefront shell, upper galleries,
Rain Garden, and storm lighting. The static `.rbxmx` sources remain as a
pre-MCP preview; do not run both map approaches as overlapping production
content after the Studio workflow is live.

## Rules this code enforces — do not relax them casually

- Storm Charge only ever rises. `addStormCharge` rejects non-positive amounts.
- Baseline loot is always visible; only `Relics` check lantern state.
- Banked loot survives a wipe outright; carried loot is halved. Disconnect uses
  the same rule. There is no code path that sells a way around this.
- The client never sends a reward. Every remote is a request the server validates.
