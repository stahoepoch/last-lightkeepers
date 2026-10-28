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

## The runtime half of M0.5, built 2026-10-28

`Source/LLK/Public/Data/` now holds the typed contract gameplay will read.
Codex owns the editor-side writer that fills these; this is what it has to
fill.

| Header | Holds |
| --- | --- |
| `LLKDataTypes.h` | `ELLKLanternCurve`, `ELLKLootVisibility`, `ELLKExtractionReliability`, `ELLKEnemyFamily`, `FLLKSecondsBand` |
| `LLKEnemyDef.h` | `FLLKEnemyDef` - one row type, six rows |
| `LLKTableRows.h` | `FLLKLootTierDef`, `FLLKRoomDef`, `FLLKRungDef`, `FLLKRelicDef`, `FLLKUpgradeDef`, `FLLKLandmarkDef` |
| `LLKStormChargeData.h` | `ULLKStormChargeData` and its five blocks |
| `LLKLanternData.h` | `ULLKLanternData` and its five blocks |
| `LLKLootRulesData.h` | `ULLKLootRulesData` |
| `LLKWorkshopRulesData.h` | `ULLKWorkshopRulesData` |

`Source/LLK/Private/Data/LLKDataContract.cpp` compiles them and holds
`static_assert`s for the claims this page makes about the C++ side - that
every row type derives from `FTableRowBase`, that the retired `linearCapped`
curve is still a real enum entry (row 12), and that `Health` is still
optional (row 7). A claim in a document that nothing checks is a claim that
drifts.

**Verified, not assumed:** the module builds clean against the pinned 5.8.4
through the **Game** target, and all 15 reflected type names are present in
the linked binary. A script walked all eight JSON files - 146 distinct
non-underscore keys - and confirmed every gameplay key has a declared field.

### `TOptional` as a `UPROPERTY` is real on 5.8, and I checked before relying on it

This page decided `FLLKEnemyDef` would use `TOptional` for the
family-specific fields. That is only viable if UHT reflects it, which it did
not always. On the installed 5.8.4: `FOptionalProperty` exists in
`CoreUObject/Public/UObject/PropertyOptional.h`, UHT has a `CanBeOptionalValue`
property capability, and the engine itself ships `TOptional<bool>` and
`TOptional<float>` as `UPROPERTY`s. All 24 optional fields compiled and
`Module.LLK.gen.cpp` built, so UHT generated reflection for every one.

**Why it is worth the unusual type.** An unset `TOptional` is the type saying
*this family does not have this field*, which a `0.f` default cannot express.
That distinction is not hypothetical here: `lantern.json` is full of
deliberate zeroes - `downedWalkSpeed` is 0 because a downed keeper cannot
move, `bleedOutSeconds` is 0 because they wait indefinitely. If `Health` were
a plain float, the Warden's *absence* of health would read as zero health,
which any damage routine treats as already dead. Row 7 would become a bug.

### Rule 4, refined: an enum in code only when the vocabulary lives in code

Rule 4 says enum strings parse strictly. Building the headers forced a
distinction the rule did not make, and the refinement is worth stating:

- **`lantern.curve` is a `UENUM`.** Its vocabulary (`linearCapped`,
  `diminishing`) exists nowhere but in code and in row 12. A `UENUM` is the
  closed set.
- **`workshop.currency` is `TArray<FName>`, not a `UENUM`.** Its vocabulary is
  the row names of the loot tier table. A `UENUM` would be a second copy of a
  list the data already owns, and the two could drift - which is the failure
  rule 2 exists to prevent, reintroduced one level up. The closed set is still
  enforced at import; it is enforced against `loot.json` rather than against
  C++.

The principle: **strict parsing means the set is closed, not that the set
lives in C++.** Put it wherever it is already defined once.

### Three things the asset writer must do

1. **Row names come from the JSON, and `id` is not a field.** For `tiers` the
   row name is the object key; for every array-shaped collection it is the
   `id`. None of the row structs declares `Id`, because storing it twice
   invites the two to disagree.
2. **Normalise `loot.tiers.*.feeds` to an array.** The JSON permits a string
   or an array of strings, and `FLLKLootTierDef::Feeds` is always
   `TArray<FName>`. Normalising once in the writer is better than every reader
   handling both.
3. **The three `*Seconds` fields on a threshold are derived, not inputs.** They
   are published so a designer can read arrival times without running the Node
   suite. Writing one that disagrees with `value` makes the asset lie, and
   nothing in the engine would catch it - `tests/pacing.mjs` is what recomputes
   them.
## The asset manifest, reviewed 2026-10-28 - `019d431`

Codex's importer now emits `asset-manifest.json` beside the eight canonical
payloads, naming all eleven outputs: seven DataTables with their row struct and
source, four DataAssets with their class and an `exclude` list. **This closes
open question 2 below** - both sides read a generated manifest rather than the
importer parsing C++ headers, which was the recommendation.

**Checked rather than assumed.** All seven `rowStruct` names and all four
`class` names match the headers exactly. The `exclude` lists are complete: the
only two source files that produce both a table and an asset are `loot.json`
(excludes `tiers`) and `workshop.json` (excludes `upgrades`), and nothing else
in either file is dropped. `ledger.json`, `relics.json` and `town.json` are
tables only and have no other top-level keys, so no content disappears.

### The two things the manifest does not say, and the writer needs

Both concern a key named `id`, which means **opposite things on the two sides
of the seam**. This is the likeliest thing to be hardcoded wrong, so it is
written down before the commandlet exists rather than after it misbehaves.

**1. In a DataTable, `id` is the row name and is never a field.** For every
`"rows": "id-array"` entry the element's `id` becomes the `FName` row key and
must then be dropped - none of the seven row structs declares an `Id`, by the
rule already stated above. For `"rows": "object"` the object key is the row
name and there is no `id` to drop.

**2. In a DataAsset, `id` is an ordinary field and is written through.** Both
array-valued properties inside assets declare one:

| Source | Shape in JSON | C++ | What the writer must do |
| --- | --- | --- | --- |
| `storm-charge.thresholds` | array, elements carry `id` | `TArray<FLLKStormThreshold>`, has `FName Id` | Copy `id` straight into `Id`. Preserve order - the array is ascending by `value` |
| `loot.extraction` | **keyed object** (`LoadingDock`, `FreightLift`) | `TArray<FLLKExtractionPoint>`, has `FName Id` | **Promote the key into `Id`.** There is no `id` inside the element to copy |

That last row is the one with no precedent anywhere else in the contract: a
keyed object becoming an array, with the key moving into a field. Every other
keyed object in these files becomes table rows instead. If the writer reuses
its table path here it will produce two unnamed extraction points, and the
failure is quiet - `Id` would simply be `None`, and `FLLKExtractionPoint::Note`
would still read correctly in the editor, so it would look fine.

### The gate was built, broken on purpose, then hardened - `318084c`, `ab27741`

Codex closed the gap below the same evening, and went past it: the manifest now
carries `keyedObjectFields` on `DA_LootRules` and `derivedFields` on
`DA_StormCharge`, so the two conventions above are machine-readable rather than
only written here.

**A green suite proves nothing until it has been seen red** - M0.7's rule, and
it applies to a data gate as much as to an automation test. So each claim was
checked by deliberately breaking the thing it guards and confirming the exit
code moved. Results, 2026-10-28:

| Break | Before `ab27741` | After |
| --- | --- | --- |
| Delete a manifest entry (`DT_TownLandmarks`) | red | red |
| Remove the `extraction` key-promotion marker | red | red |
| Rename `FLLKLootTierDef` to `FLLKLootTier` | red | red |
| Rename `FLLKLootTierDef` to `FLLKLootTierDefRenamed` | **green** | red |
| Delete `USTRUCT(BlueprintType)` above a row struct | **green** | red |
| Delete `UCLASS(BlueprintType)` above a DataAsset | **green** | red |
| Remove the `derivedFields` declaration | **green** | red |

**Both holes were closed in `ab27741`, and the fix was re-broken to confirm
it.** The two green rows above were a substring test satisfied by a longer
name, and a check on declaration text rather than on reflection - so a type
could stop being a `USTRUCT` and the gate would not notice. Both now use the
anchored regex, which requires the reflection macro immediately above the
declaration. `derivedFields` was marked-but-unasserted in the previous commit
and is now a guard like the others.

Every row in the "after" column was produced by making the break, running
`Tools/test_import_data.py`, and watching the exit code move - not by reading
the diff. The suite is green at 151/297/89/6 with the tree clean.

**One sharpness worth knowing rather than fixing.** The `derivedFields` guard
compares the list exactly, so *reordering* the three field names fails too,
with a message about fields remaining "writer-derived from value" that would
not obviously explain why. Harmless today and arguably the right strictness;
if it ever confuses somebody, comparing sets instead of lists is the change.

What this now buys: the fast gate, which is the only thing CI enforces under
M0's two-tier rule, catches a renamed type, a lost output, a de-reflected
struct and a dropped conversion rule before the writer runs - rather than
leaving them to surface as a failed import, or worse, as an extraction point
with a `None` id that still looks correct in the editor.

### One gap worth closing cheaply

The manifest is now a **third** copy of the contract, after `data/*.json` and
the headers. `LLKDataContract.cpp` guards the C++ side with static assertions,
and the importer guards the JSON side, but nothing guards the manifest against
the headers: renaming `ULLKLootRulesData` in C++ would leave the manifest
naming a class that no longer exists, and the first sign would be a failed
import. A grep of the headers for each of the eleven names would catch it in
the fast CI gate that already runs on every push. Codex's call - it is his
file - but it is a few lines and it is the same drift argument that produced
rule 2. **Done the same evening in `318084c`** - see the section above for what
it does and does not catch.

## The `b` prefix will silently drop 13 booleans - checked in 5.8 source

Both APIs the writer needs are real on 5.8.4, and one risk they carry is
cleared: **`FOptionalProperty` is handled in both directions** by
`FJsonObjectConverter` (`JsonObjectConverter.cpp:1200-1203` on import,
`354-362` on export), so the 24 `TOptional` fields on `FLLKEnemyDef` will
round-trip. The `TOptional` decision holds end to end, not just at reflection.

**The naming does not.** Unreal matches JSON keys to properties by the
property's *authored name*, and for a bool that name keeps its `b`:

- `JsonObjectConverter.cpp:1338` - `GetAuthoredNameForField(Property)`
- `Class.cpp:2558-2562` - `GetAuthoredNameForField` returns `Field->GetName()`
- the DataTable path is no different: `DataTableUtils.cpp:447-455` builds its
  candidate list from `Prop->GetName()` and `GetAuthoredName()`, and
  `Field.cpp:608-616` routes `GetAuthoredName()` back to the same `GetName()`

So Unreal looks for `bKillable`. Our data says `killable`. Case does not
matter - the lookup is a case-insensitive `FString` map - but the prefix does.
**Twenty fields are affected**: 13 plain `bool b...` across the four DataAssets
and 7 `TOptional<bool>` on `FLLKEnemyDef`.

### The two halves fail differently, and one of them fails silently

| Path | On a missing key | Consequence |
| --- | --- | --- |
| DataTable (`CreateTableFromJSONString`) | `DataTableJSON.cpp:913` adds *"Row 'X' is missing an entry for 'bKillable'."* to the returned `TArray<FString>` | **Loud**, if the writer treats a non-empty return as failure. It should |
| DataAsset (`JsonObjectToUStruct`) | `JsonObjectConverter.cpp:1340-1354`: without `bStrictMode` it logs nothing and `continue`s | **Silent.** The field keeps its C++ default |

**The silent half is the dangerous one, because the defaults are right.**
Those 13 defaults were chosen deliberately to match the data -
`bDropsCarriedCrate = false` for row 59, `bRelicsRequireLanternLight = true`
for row 53. An asset where every boolean silently fell back to its default
would be **indistinguishable from a correct import** today. It would stay
invisible until the first time somebody changes a boolean in `data/*.json` and
the game ignores it - which is during balancing, months from now, and would
read as a gameplay bug rather than an import bug.

### What to do, and what not to

**Not `bStrictMode = true`.** It turns the silence into an error, but it also
demands that *every* property be present, which is wrong for a contract built
on `TOptional` - the Warden legitimately has no `health`. That trades a silent
wrong answer for a loud wrong failure.

**Emit the `b` in the canonical payload.** The payload is the engine-facing
form and already exists to carry exactly this kind of translation, the same
way the manifest carries `id` promotion and derived fields. `data/*.json`
stays human-facing camelCase; the seam does the work; nothing in C++ bends to
accommodate JSON.

**Then guard it in the fast gate**, because this is a name correspondence and
the fast gate is already where name correspondence is checked: every
`bool` / `TOptional<bool>` property in `Source/LLK/Public/Data/*.h` must have
a matching key in the emitted payload. That runs on every push with no engine,
and it is the same shape as the existing eleven-name check.

### Fixed in `3aad558`, and the fix is right - the guard is two samples

The importer now emits a second, Unreal-facing copy of every payload under
`unreal/`, with bool keys renamed to their authored `b...` form, and the
manifest names that directory. That is the correct shape: `data/*.json` keeps
`killable`, C++ keeps `bKillable`, and the seam translates.

**Verified by correspondence, not by sampling.** Deriving the bool properties
from the headers and checking them against the emitted payload: **20 declared,
20 present with the prefix, and no un-prefixed bool key survives.** The
translation is complete today.

**The guard was two samples, and is now a correspondence check** - `2886db4`.
It derives every `bool` and `TOptional<bool>` property from the headers and
requires each authored `b...` name to appear in the generated payload, with no
un-prefixed equivalent leaking in. Confirmed by breaking it twice:

| Break | Before `2886db4` | After |
| --- | --- | --- |
| Drop `soloOnly` from `BOOL_PROPERTY_KEYS` | **green**, and `bSoloOnly` silently kept its default | red - *"missing reflected boolean property key(s): bSoloOnly"* |
| Add a new `bool bProbeOnly` to a header | **green** - nothing knew to look for it | red - *"...: bProbeOnly"* |

The second is the one worth having. `BOOL_PROPERTY_KEYS` is still a
hand-maintained list, but it is no longer a list anybody has to *remember* to
update: adding a reflected bool to the contract now fails the fast gate by
name until the mapping exists. That is the difference between a convention and
a guard, and it is the same move rule 2 made for the data itself.

## Before the writer is built: what "run it twice" can and cannot prove

M0.5's acceptance says the writer must be **idempotent**, and the plan is to
run it twice and verify. The obvious verification - regenerate and diff the
bytes - **cannot pass under one of the two plausible designs**, for a reason
that has nothing to do with the data. Written here before the writer exists,
because it is a design decision rather than a bug to find afterwards.

**Checked in the 5.8 source on disk, not recalled.** Every `.uasset` carries a
`PersistentGuid` in its package file summary
(`CoreUObject/Public/UObject/PackageFileSummary.h:216`, editor-only data). Two
facts decide this:

- `CoreUObject/Private/UObject/Package.cpp:157-160` - *"Always generate a new
  unique PersistentGuid, required for new disk packages"*, then
  `PersistentGuid = FGuid::NewGuid()`. A **newly created** package gets a
  random guid.
- `LinkerLoad.cpp:1877` restores the stored guid when an **existing** package
  is loaded, and `PackageFileSummary.cpp:359` round-trips it verbatim for every
  modern file version. (The content-hash derivation below it is a legacy path
  for files older than `VER_UE4_ADDED_PACKAGE_OWNER`, and does not apply to
  anything 5.8 writes.)

**So the design choice decides whether byte-idempotence is even possible:**

| Writer design | Second run | Byte-diff across runs |
| --- | --- | --- |
| Delete `Content/Data/` and create fresh packages | 11 new random guids | **Always differs.** Never passes, however correct the data |
| Load the existing package and update it in place | guids preserved | Can be identical, and the diff means something |

The trap is not that one design is wrong - it is that the **failure looks like
a data bug**. A diff on eleven files after a supposedly deterministic
regeneration reads as non-determinism in the payloads, which is exactly what
this contract exists to rule out, so the instinct would be to go hunting in the
importer. The bytes that differ would be sixteen in a header.

**What to assert instead, in order of preference:**

1. **Load both generations and compare the data** - row counts, row names, and
   field values. This is what idempotence actually means here and it holds
   under either design.
2. **Byte-compare, but only if the writer updates in place**, and then it is a
   genuinely strong claim.
3. Never: byte-compare with the guid excluded by hand. A comparison with a
   hole in it is the kind of green that this milestone exists to distrust.

**This also bears on open question 1 below.** If generated assets are committed
and the writer creates fresh packages, every regeneration produces a new guid
in all eleven files, so each run adds eleven new LFS objects **even when no
number changed**. That is an argument for building on demand, or for updating
in place, and it is a cost that would otherwise be discovered as a slowly
growing repository.

## Open for Codex

1. ~~**Commit the generated assets, or build them on demand?**~~ **CLOSED
   2026-10-28 by Codex.** Build on demand; do not commit. Update existing
   packages in place when they exist. Prove idempotence **semantically** - load
   both generations and compare each asset's type, row names and counts, and
   typed field values - and never by a guid-masked byte comparison. That is all
   three recommendations taken, and the in-place rule is what makes the
   packages stable during iteration rather than churning a new guid per run.

   **One thing this decision requires that is not done yet, with the evidence.**
   `Content/Data/` is **not** ignored today, and `*.uasset` already carries
   `filter=lfs`. Verified by putting a file there: `git status` reports
   `?? Content/`, `git check-ignore` reports nothing, and `git check-attr`
   reports `filter: lfs`. Two consequences, both only appearing the first time
   the writer runs:

   - **M0.3's exit criterion breaks.** It is *"a full build leaves `git status`
     clean"*, and eleven generated assets would leave it dirty.
   - A `git add -A` commits eleven LFS objects, which is the exact churn this
     decision was taken to avoid.

   The fix is one line in `.gitignore` - `Content/Data/` - and it belongs in
   the same commit as the writer rather than before it, since an ignore rule
   for a directory that does not exist is a rule nobody can check. `.gitignore`
   is Codex's by lane (M0.3), and the note inside it still says this question
   is open, so that note wants updating at the same time.
2. ~~**Where the row structs live.**~~ **CLOSED 2026-10-28 by `019d431`.** The
   question was whether the importer parses the headers or both sides read a
   generated manifest. It is the manifest, which was the recommendation on the
   grounds that it is less clever and will break less. See the review above.

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
