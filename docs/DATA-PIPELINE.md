# Data pipeline — `data/*.json` into Unreal

Roadmap task **M0.4**. Written by Claude, 2026-10-27, as the contract Codex
implements in **M0.5** (`Tools/import_data.py`). Owner of this document:
Claude. Owner of the importer: Codex. Owner of every number inside the JSON:
Chris.

## Why the JSON survives the engine change

Eight files, 100% text, zero engine dependencies, and 98 existing assertions in
`tests/balance.test.mjs` that already encode the game's balance rules and run
on plain Node with nothing installed. That suite is the cheapest quality
mechanism this project has and it keeps working unchanged. Nothing about this
pipeline may break it.

It also preserves the ownership rule that has worked all along: **Claude owns
the schema, Chris owns the values.** A number that moves into a `.ini`, a
Blueprint default or a C++ constant leaves Chris's lane and enters ours, which
is how a balance decision quietly becomes an engineering one.

## The flow

```
data/*.json                 text, in git, reviewed, diffed, tested
    │
    ├─→ tests/balance.test.mjs        (Node, no engine, 98 assertions) ── gate
    │
    └─→ Tools/import_data.py          (Codex, M0.5)
            │
            └─→ Content/Data/*.uasset     generated. Never hand-edited
                    │
                    └─→ ULLKBalanceSubsystem      (Claude, M1)
                            │
                            └─→ every gameplay system reads from here
```

Nothing in `Source/` parses JSON at runtime. Nothing in `Content/Data/` is
authored by a human. The generated assets are build output that happens to live
under `Content/`, and they are the single exception to "Chris owns `Content/`"
in `docs/LANES.md` — because nobody authors them, nobody owns them.

## Two kinds of file, two kinds of asset

The eight files are not the same shape, and forcing them into one is where this
would go wrong.

**Collections** — a map of named things. These become `UDataTable`s, row name =
the JSON key:

| File | Top-level keys that become rows | Row struct |
| --- | --- | --- |
| `enemies.json` | `Sweepers`, `Stockers`, `Glimmers`, `WindowWalkers`, `Ficus`, `Warden` | `FLLKEnemyDef` |
| `loot.json` → `tiers` | each tier | `FLLKLootTierDef` |
| `ledger.json` → `rooms` | each room | `FLLKRoomDef` |
| `ledger.json` → `rungs` | each rung | `FLLKRungDef` |
| `relics.json` → `relics` | each relic | `FLLKRelicDef` |
| `workshop.json` → `upgrades` | each upgrade | `FLLKUpgradeDef` |
| `town.json` → `landmarks` | each landmark | `FLLKLandmarkDef` |

**Singletons** — one block of settings, not a list. These become one generated
`UDataAsset` each. A single-row DataTable would also work; a DataAsset reads
better at the call site and that is the whole point of it:

| File | Asset | Holds |
| --- | --- | --- |
| `storm-charge.json` | `ULLKStormChargeData` | `baselineRatePerSecond`, the `lantern` block including the **diminishing** curve (row 12), the **`run` block** (`targetSeconds`, `earlyExitSeconds`, `hardCeilingSeconds` — added 2026-10-28, see below), `extractionCall`, `thresholds` (an array), `anchorRevive` |
| `lantern.json` | `ULLKLanternData` | `charge`, `knockdown`, `revive`, `sweeperContact`, `movement` |
| `loot.json` | `ULLKLootRulesData` | `rules`, `burden`, `banking`, `extraction` — everything in the file except `tiers` |

## The rules the importer must follow

These are the parts that matter. The rest is mechanical.

**1. Keys beginning with `_` are documentation. Skip them, always.**
Every file carries them — `_comment`, `_values`, `_curveChoice`, `_rungState`,
`_scopeNote` — and between them they hold the reasoning for most of the
project's balance decisions, including who approved what and on which date.
They are load-bearing for humans and meaningless to the engine. **Fifty-four of
them across the eight files**, counted today — `ledger.json` has 13,
`storm-charge.json` 12, `lantern.json` 11. Never import, never warn about,
never strip from the source.

**2. An unrecognised non-underscore key is a hard error.** Not a warning, not a
skip. If `data/enemies.json` grows a field that `FLLKEnemyDef` does not have,
the import fails and the build fails with it.

This is the most important rule here. The entire value of a separate data layer
is that it cannot silently disagree with the code. A warning gets scrolled
past; a failure gets fixed. The failure message should name the file, the key
and the struct it did not match.

**3. A missing field is also a hard error**, unless the struct marks it
optional. Silent defaults are how a balance value becomes zero without anyone
noticing — and a zero storm charge rate is a game with no clock in it.

**4. Strings that are enums parse strictly.** `storm-charge.json` has
`"curve": "diminishing"`, with `linearCapped` retired and kept in the file as
`_curveOptions`. That maps to a `UENUM` with exactly those two values, and an
unknown string fails the import. Row 12 is LOCKED and the retired shape stays
implemented, so switching back must never need a code change — which only holds
if both values are real enum entries.

**5. The importer is idempotent and total.** Running it twice changes nothing.
Running it rebuilds every asset from scratch rather than patching. Partial
imports are how two assets come to disagree about the same number.

**6. The importer validates shape, not design.**
`tests/balance.test.mjs` is the design gate and stays the design gate — it is
what knows that no keeper's light may be free, that a room cost and a rung must
agree in both directions, and that no relic may appear in a progression cost.
Do not reimplement any of that in Python. Run both: the Node suite gates the
values, the importer gates the shape.

## How C++ reads it

`ULLKBalanceSubsystem` — a `UGameInstanceSubsystem`, Claude's, landing in M1.
It loads the generated assets once and exposes typed accessors. It is the
direct descendant of the Roblox project's `Config.luau`, which did exactly this
job and is worth reading first: one module requires the data, exposes
`stormRate()` and `thresholdAt()`, and no other file ever touches the raw
values.

The rule it enforces: **no gameplay class reads a DataTable directly.** One
loader, typed accessors, and the storm charge curve implemented in exactly one
place.

## Open for Codex

1. **Commit the generated assets, or build them on demand?** Committing keeps a
   fresh clone playable without running Python; building on demand keeps binary
   churn out of LFS. Recommendation: **build on demand**, with the import as a
   pre-build step, because a generated binary in LFS gets a new object every
   time a number moves and the whole point of `data/` is that numbers move
   often. Codex's call — it is the LFS bill.
2. **Where the row structs live.** They are Claude's (`Source/LLK/Data/`), but
   the importer needs to read them to know the field names. Either the importer
   parses the headers, or both sides read a generated manifest. A manifest is
   less clever and will break less.

## The `run` block, added 2026-10-28

Recorded here because its absence was a defect in this contract, not an
oversight in the data.

**Run length did not exist in `data/` at all.** It lived only in
`src/shared/Config.luau` as `targetMaxSeconds`, which is frozen Luau (row 33)
and does not port — so the single most-argued-about number in the project was
unreadable by the engine this contract exists to feed. A data layer that cannot
express the run's length is not a portable layer.

The keys also fix the naming that caused the argument. `targetMaxSeconds` held
a *ceiling* under a name that says *target*, which is why four sources carried
three numbers. The block separates them permanently:

| Key | Meaning |
| --- | --- |
| `targetSeconds` | What a normal run should feel like. Row 48 makes this the night's window too |
| `earlyExitSeconds` | The loading dock. Reliable, pays less |
| `hardCeilingSeconds` | A safety valve nothing may exceed. Never a goal |

**One value needs Chris and is flagged in the file.** O1 says the target is
6–10 minutes; `GAME-THESIS.md` and `STEAM-PRODUCTION-PLAN.md` both say 10–12,
and they are newer. The block carries 10–12, because it is his latest statement
twice over and because this file's own tuning line has said "a normal 10–12
minute run" since 2026-10-26 — but the older figure is still sitting in O1 and
one of the two has to be retired by him rather than by an importer.
