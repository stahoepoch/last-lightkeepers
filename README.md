# Last Lightkeepers — Roblox prototype (archived 2026-10-27)

> **This repo is no longer the product.** On 2026-10-27 Chris chose Steam,
> PC-first, with no parallel Roblox version — `docs/DECISIONS.md` row 1,
> LOCKED. The Roblox project is kept as a design reference and prototype
> archive (row 33): nothing here is deleted, and nothing here is advanced.
>
> **Start at `docs/PLATFORM.md`.** It says what the decision is, what moves to
> the PC build (the 3,075 lines of design docs and the eight `data/*.json`
> value files — all of it engine-neutral) and what does not (15,080 lines of
> Luau, and every Roblox API it touches).
>
> The rest of this README describes the archived prototype accurately. It is
> still the fastest way to understand what each system does.

Week-one graybox skeleton. Source of truth for design is the project handoff
(`last-lightkeepers-claude-handoff-2026-10-19.pdf`); this repo implements it and
does not reinterpret it.

## Layout

```
default.project.json     Rojo sync map
data/                    Balance values as plain JSON — the portable layer
  storm-charge.json      Baseline rate, lantern multiplier, thresholds, Anchor Revive
  loot.json              Tiers, visibility rules, extraction-only payout
  enemies.json           Sweepers, Stockers, Glimmers, Warden
src/shared/
  Config.luau            Requires the JSON, exposes stormRate() / thresholdAt()
  Types.luau             RunState, LightkeeperState, Haul, RunSnapshot
  Remotes.luau           Remote creation + lookup; every remote is a *request*
src/server/
  init.server.luau       Entry point: wires services, starts the run
  services/
    RunStateService      Storm Charge, lanterns, knockdown/revive, extraction-only wipe
    LootService          Spawning, visibility, pickup, extraction payout
    EnemyService         Sweepers, Glimmers, and Window Walkers; Stockers and Warden remain future work
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
| `RelayBank`    | the mid-run relay volume    | — (utility only; never secures loot)    |
| `Extraction`   | loading dock, freight lift  | `ExtractionId` = `LoadingDock`/`FreightLift` |
| `RunStart`     | optional run-start marker   | — (used to choose a far-side exit)      |
| `SweeperSpawn` | Sweeper spawn points        | —                                       |

`Tier` must be one of `Supplies`, `Components`, `Relics`, `RestorationCrates`.

## Extraction rule

Every run has exactly one active `Extraction` candidate. The server chooses at
random from candidates at least 70% as far from `RunStart` (or the first
`SpawnLocation`) as the farthest candidate, so an exit beside spawn cannot be
selected when a far-side option exists. The selected candidate receives
`ActiveThisRun = true`; all other extraction prompts remain off.

There is no banked safety net: a wipe or disconnect without extraction pays
nothing from that run. Both current exits are held commitments (Loading Dock:
20 seconds; Freight Lift: 30 seconds). Add at least three well-separated,
tagged candidates to give future maps genuine route variety.

## Tuning anchors in place

Two of the handoff's three placeholders have since been replaced by Chris's own
numbers. What is in `data/storm-charge.json` now:

- **~2.5×** baseline with one active lantern (`perLanternBonus: 1.5`). Still the
  handoff's figure, and locked: no change to the curve may move it.
- Thresholds at **270 / 480 / 700** → in darkness 4:30 / 8:00 / 11:40; with the
  lantern lit about half the time 2:34 / 4:34 / 6:40. Retuned by Chris on
  2026-10-26, replacing the handoff's 8-minutes-to-first-threshold anchor,
  which left a dark crew with no escalation inside any run it would play.
- The multiplier uses the **`diminishing`** curve, Chris's call on 2026-10-27:
  2.50 / 3.40 / 3.94 / 4.26× for one to four lanterns, self-limiting at 4.75×.
  Each further lantern adds less than the one before it but never nothing, so
  no keeper's light is free.

`maxMultiplier: 5.0` is **inert** — the diminishing curve needs no clamp. It is
kept only as the value `linearCapped` would use if the curve were switched
back. Under that clamp a third and fourth lantern cost 5.00× alike, which made
a four-player run the shortest one in the game; that is why it was retired.

`node tests/pacing.mjs` prints the whole grid, both curves side by side.

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

Invariant checks over `data/*.json` plus static wiring checks over the Luau — no
Roblox runtime needed. They assert that the numbers still match the handoff: the 8-minute dark
anchor, the 2.5× lantern rate, exactly one lantern-only loot tier, extraction
being required for all run loot, the Warden unkillable, the locked Glimmer scan/tag
values, the one-time four-second teammate revive, and that a lone Sweeper
cannot stack a knockdown.

They do **not** replace a Studio playtest. The Luau is still untypechecked — see
`rokit.toml` for the one command that fixes that.

## Toolchain and CI

Install the pinned tools once from the repository root:

```bash
rokit install
```

Then run the same checks used by CI:

```bash
npm test
stylua --check src
selene src
luau-lsp analyze src
rojo build default.project.json --output Last-Lightkeepers-validation.rbxlx
```

Formatting or lint warnings in a file owned by another lane should be reported
to that owner rather than mass-reformatted in a shared working tree.

## Not built yet

Stockers and the Warden still have config and hooks but no behaviour. The
Lanternhold hub, workshop UI, and Community Kitchen landmark are untouched.

Glimmers are now in the vertical slice: at PatrolsHunt, one appears near a lit
Lightkeeper, keeps sight for two seconds, then tags that player for twenty
seconds. Going dark cancels the scan. PatrolsAlert also widens nearby Sweeper
pressure, so a crew that stays dark still sees escalation during a normal run.

Window Walkers are also physical now: at PatrolsAlert, one spawns from the
nearest existing enemy marker and remains still. Nearby lantern light with a
clear line of sight wakes it; it announces itself, then walks directly toward
the lit Lightkeeper. Turning the lantern off or breaking sight makes it stop
rather than path around the mall.

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
