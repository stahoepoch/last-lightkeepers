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
| `storm-charge.json` | `ULLKStormChargeData` | `baselineRatePerSecond`, the `lantern` block including the **diminishing** curve (row 12), the **`run` block** (`targetSeconds`, `targetBandSeconds`, `earlyExitSeconds`, `hardCeilingSeconds` — added 2026-10-28, see below; `targetBandSeconds` is a two-element `[low, high]` array, not a scalar), `extractionCall`, `thresholds` (an array of **two** since 2026-10-28), the **`warden` block** (`chargeValue`, `arrivalBandSeconds` — a two-element `[low, high]` array — `lightSensitivity`, `effect` — row 54 gave the Warden its own dampened arrival meter, so it is **not** a threshold and must not be imported as one), `anchorRevive` |
| `lantern.json` | `ULLKLanternData` | `charge`, `knockdown`, `revive`, `sweeperContact`, `movement` |
| `loot.json` | `ULLKLootRulesData` | `rules`, `burden`, `banking`, `extraction` — everything in the file except `tiers` |
| `workshop.json` | `ULLKWorkshopRulesData` | `currency` — a string array, today `["Supplies", "Components"]`. **Added to this contract 2026-10-28**; it was the one top-level key in the eight files with no home here, so the importer would have hard-errored on it per rule 2. Like `loot.json`, this file is **both** shapes: `upgrades` is a DataTable, `currency` is a singleton. Each entry must be a `Tier` name that exists in `loot.json` → `tiers`, so it parses as an enum per rule 4 rather than as free text |

## `FLLKEnemyDef` is heterogeneous, and that needs deciding here

Added 2026-10-28. This is the one thing in the contract that would have stopped
M0.5 with a question, which is the exact failure M0.4's done-criterion names.

The six enemy rows do **not** share a field set. Five fields are universal, one
is near-universal, and eighteen belong to a single family each:

| Scope | Fields |
| --- | --- |
| **All six** | `formerJob`, `behaviour`, `counterplay`, `moveSpeed`, `audioTelegraphSeconds` |
| **Five of six** | `health` — absent on the **Warden**, and that absence is a rule, not an omission (row 7: unkillable, drops nothing) |
| Three | `sightRange` (Stockers, Window Walkers, Ficus) |
| Two | `wakeRange`, `wakeRequiresActiveLantern` (Window Walkers, Ficus) |
| Sweepers | `aggroRadiusDark`, `aggroRadiusLit`, `swarmCallRadius`, `attractedToDroppedLoot` |
| Stockers | `sightConeDegrees`, `losesTargetAfterSeconds` |
| Glimmers | `scanRange`, `scanSeconds`, `scanRequiresActiveLantern`, `tagSeconds`, `tagRevealsPositionToAllRobots`, `tagAddsStormCharge`, `tagGivesWardenPermanentWaypoint` |
| Warden | `killable`, `dropsLoot`, `spawnStage`, `retargetIntervalSeconds` |

**The decision: one struct, two tiers of strictness.** Not per-family structs —
a DataTable has one row type, and splitting into six tables would scatter the
roster that `docs/ZONES.md` treats as one thing.

- The **five universal fields are required**, and rule 3 applies in full: a
  missing one is a hard error.
- The **family-specific fields are `TOptional`**, and rule 3 is *deliberately*
  relaxed for them — this is the only exemption in this contract, and it is
  named here so nobody later "fixes" it by defaulting them.
- **Silent defaults stay forbidden regardless.** An absent `TOptional` must
  read as *absent*, never as zero. `health` on the Warden is the case that
  proves why: defaulting it to 0 would turn "cannot be hurt" into "dies
  instantly," which is row 7 inverted by a type system.
- Because optionality costs rule 3's protection, the importer **validates by
  family instead**: given the row name, the fields in the table above must be
  present, and anything outside its row is an error. That restores the
  guarantee rule 3 was giving, keyed on the row name rather than on the struct.

`health` is the one judgement call worth stating: it is required for five and
absent for one, so it is `TOptional` with the Warden as its only sanctioned
absence. If a second enemy ever lacks health, that is a design change and
should be argued in `DECISIONS.md`, not absorbed by the importer.

## The rules the importer must follow

These are the parts that matter. The rest is mechanical.

**1. Keys beginning with `_` are documentation and analysis. The importer
STRIPS THEM EXPLICITLY — it does not merely fail to read them.**
Every file carries them — `_comment`, `_values`, `_curveChoice`, `_rungState`,
`_scopeNote`, `_dutyProfiles` — and between them they hold the reasoning for
most of the project's balance decisions, including who approved what and on
which date. They are load-bearing for humans and meaningless to the engine.
Never import, never warn about, never strip from the *source* file.

**Tightened 2026-10-28 on Chris's instruction**, and the distinction is the
whole point: *"the Unreal importer should explicitly strip or reject
underscore-prefixed analysis fields, not merely happen to ignore them…
`_dutyProfiles` belongs to tests and tuning reports; runtime should only
receive actual gameplay values."*

"Happens to ignore" and "must reject" are different guarantees, and only one of
them survives somebody later writing a generic `JsonObjectToUStruct` call. So
the contract is now three obligations, not one:

| # | The importer must | Why |
| --- | --- | --- |
| 1a | **Strip every `_` key before the struct conversion runs**, as a deliberate pass over the parsed object | A generic deserialiser handed the raw object would try to match them, and rule 2 would then fail the build on the project's own documentation |
| 1b | **Report what it stripped**, as a count per file in the import log | A silent strip cannot be audited. If a real gameplay key is ever typo'd with a leading underscore, the count is the only thing that would reveal it |
| 1c | **Treat an `_` key reaching runtime as a hard error**, not as data to coerce | This is the one Chris is actually protecting against: an engine-side reader finding `_dutyProfiles` and treating profile metadata as configuration. `UCLASS`/`USTRUCT` definitions must not declare any field whose JSON name begins with `_` |

**The count is not written down here on purpose.** This page used to say
"fifty-four of them across the eight files," which was true the day it was
typed. `tests/balance.test.mjs` now counts them on every run and prints the
total, which is rule 1b satisfied on the data side as well as the engine side —
and one fewer number living in prose where it can go stale. Today it prints 69.

**`_dutyProfiles` is the live example and the reason this got tightened.** It
holds three named playstyles with a contract each (`DECISIONS.md` row 55), it
is read by `tests/balance.test.mjs` and `tests/pacing.mjs`, and it is exactly
the shape of thing an engine-side reader would mistake for tuning
configuration — it has `duty` numbers in it that look like gameplay values and
are not. Nothing in the runtime should know those profiles exist.

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

**7. The file set is closed, the same way the key set is.** Rule 2 makes an
unrecognised *key* a hard error. An unrecognised *file* must be one too. If
`data/` contains a `.json` the importer has no validator for, the import fails
and names it - it does not import the seven it recognises and declare success.

**Added 2026-10-28 after finding this open, and CLOSED the same day in
765df60.** The history is kept in the past tense below because the failure
mode is worth remembering, not because the hole is still there.

`Tools/import_data.py` walked a hardcoded list of the eight filenames, so a
ninth file in `data/` was not rejected: it was invisible. Dropping a junk
`data/keepers.json` into a copy of the data directory and running the gate
printed **"Data contract passed"** and exited 0, having never looked at it.
Nothing was broken on the day, because the eight on disk were exactly the
eight in the list.

It would have broken soon and predictably. `DECISIONS.md` row 49 makes
contained keepers the next contract type, and the obvious shape for it is
`data/keepers.json`. The failure mode is the quiet one this whole page exists
to prevent: somebody adds a data file, the gate stays green, the Node suite
never sees it either, and the file silently is not part of the game until
someone notices months later that an asset was never generated from it. A
green gate over an unread file is worse than no gate, for the same reason a
green test suite that cannot fail is worse than no suite.
**How it was closed, and it is tighter than what I asked for.** Codex made the
directory the input: `load()` globs `data/*.json` and compares that set against
the contract's list in **both** directions. An unrecognised file is now a hard
error, and so is a *missing* one - named upfront rather than discovered
part-way through a per-file read. I only raised the extra-file half; the
missing-file half is his addition and it is the better guarantee, because a
deleted data file and an unread one fail the same way for the same reason.

Verified against the live tooling rather than taken on report, all three ways:
a junk `data/keepers.json` now gives `unrecognised contract file(s):
keepers.json` and exit 1; removing `town.json` gives `missing contract
file(s): town.json` and exit 1; the real eight still pass with exit 0.
`Tools/test_import_data.py` asserts the first of those, so the guarantee has a
red-path test and not just a changelog entry.

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

**Settled by Chris the same day, so the flag is gone.** The target is **10-12
minutes**, six minutes is the earliest smart extraction call rather than a
target, and 6-10 is retired everywhere. O1 is closed. `targetBandSeconds`
carries the band so that `tests/balance.test.mjs` can read it instead of
keeping its own copy — the duplication that let four sources disagree three
ways is now structurally impossible for this number.
