# Every family-specific enemy number is missing from the build

Found 2026-10-31 by Claude, chasing why the Sweeper noticed nobody. **It is
not the Sweeper.** It is the entire family-specific half of `DT_Enemies`, and
it has been empty for as long as the asset has existed.

---

## 1. What is wrong

`data/enemies.json` authors 32 numbers across six families. The generated
`Content/Data/DT_Enemies.uasset` contains the 11 that are declared as plain
`float` on `FLLKEnemyDef` and **none of the 17 declared as `TOptional<float>`**:

| | |
| --- | --- |
| arrives | `moveSpeed`, `audioTelegraphSeconds` — every family |
| does not arrive | every `health`, both Sweeper aggro radii, `swarmCallRadius`, every `sightRange`, `sightConeDegrees`, `losesTargetAfterSeconds`, `scanRange`, `tagSeconds`, every `wakeRange`, `retargetIntervalSeconds` |

Measured by searching the asset for the IEEE-754 float32 of each authored
value. The split is total and falls exactly on the declared type, which is
what makes it a mechanism rather than a coincidence. Three small integers
(`2`, `32`, `0`) do appear by chance — `0.0` occurs 3,790 times — which is why
the check added below reports absence and never claims correct placement.

**What it costs.** `LLKPerception::SenseReaching` requires the fields of a
sense to be *set*, so with none set no family can notice anybody at any
distance. Six PIE runs today logged the consequence verbatim:

> `LLKSweeper_UAID_... awake. Speed 392 cm/s, telegraph 1.0s, notices a dark
> keeper at 0 studs and a lit one at 0.`

That is row 5's whole trade — at radius 30, a lit keeper noticed from **8
studs beyond the edge of their own light**, the asymmetry `data/lantern.json`'s
`_radiusNote` exists to argue for — absent from the build. It will read as
unfinished AI.

## 2. Which layer, and the answer is none of the three suspected

Not the struct. `TOptional<float>` genuinely reflects and serialises, and
`FJsonObjectConverter` handles `FOptionalProperty` in **both** directions
(`JsonObjectConverter.cpp:354` exporting, `:1200` importing). The header's
claim that this round-trips was right about everything except the one path the
importer actually uses.

Not the commandlet's logic. `LLKImportDataCommandlet.cpp:245` collects the
`Problems` array from `CreateTableFromJSONString` and hard-fails on any entry.
It was told nothing, so it failed on nothing. And `VerifyTable` compares **row
names only** — by design, which is why the data-contract pass of 2026-10-30
could verify DT_Enemies "row sets identical to JSON" and be entirely correct
while every value inside those rows was missing.

**It is the engine's DataTable importer.** `UDataTable::CreateTableFromJSONString`
routes property types its own cascade does not recognise into
`DataTableUtils::AssignStringToProperty`, which opens:

```cpp
if (InProp && IsSupportedTableProperty(InProp))
```

`DataTableUtils::IsSupportedTableProperty` (`DataTableUtils.cpp:407`, UE 5.8.4)
lists sixteen property classes. `FOptionalProperty` is not among them. So the
function **returns an empty error string without writing anything** — a silent
skip, by construction, with no diagnostic anywhere.

### The shape of the fix, for whoever owns the import

`FJsonObjectConverter::JsonObjectToUStruct` does handle optionals, so the
remedy is to stop routing this struct through `CreateTableFromJSONString` and
convert each row with `FJsonObjectConverter` before `AddRow`. One site.
Alternatively, extend `VerifyTable` to compare values rather than row names,
which would have caught this on the day it was introduced and would catch the
next type the importer quietly drops.

**Not done here.** `Tools/**` and `Source/LLKEditor`'s commandlet are Codex's
lane, and `Content/Data/` is generated. This document is the handover.

## 3. Why six playtests looked green

Three guards, each correct, with the gap between them:

- `tests/enemies.test.mjs` read `data/enemies.json` and still does. The
  authored file was never wrong.
- The automation tests assign `AggroRadiusDark = 12.f` **by hand** in six
  fixtures. The rule was never wrong either.
- `VerifyTable` checked row names at import, and the rows were all present.

So nothing in the project compared the number authored with the number
generated. That is the one comparison that would have caught it, and it did
not exist anywhere.

## 4. What landed, and what did not

**Landed: the missing comparison**, as a new section in
`tests/enemies.test.mjs`. It scans the generated asset for each authored
number and fails naming every one that is absent. Two things make it honest:

- **A control group.** The universal fields do import, so if one of *those* is
  missing the scan itself is broken — the asset format changed — and that is
  reported as a different failure. A check that cannot fail for its own
  reasons is a check nobody can trust.
- **Absence only.** A hit does not prove a value sits on the right row. The
  check can catch a dropped field and can never confirm a correct one.

Proven red before it was called done: it fails now, listing all seventeen,
with the control group green.

**Its limit, stated plainly:** `Content/Data/` is gitignored, so a fresh
checkout has no asset and the section reports that it compared nothing rather
than passing. That skip is the honest shape of the remaining hole, and value
verification at import time is what would close it in CI.

**Not landed: a runtime audit, because it could not be built.** The fix in
this lane is small — lift `SenseReaching`'s internal `bHasAnySense` expression
into `LLKPerception::HasReachingSense`, and have `ULLKEnemyRosterSubsystem`
log an error at load for every non-Warden family that has no reaching sense.
It repairs nothing; it stops the silence, so an inert enemy says why instead of
reading as unfinished AI. The roster's existing warning names two states that
"look identical from inside the game" — the enemies did nothing, and the
enemies never loaded — and this is the third one between them.

It is written, with tests, and it is **not committed**, because
`UnrealBuildTool` refuses to build while the editor holds Live Coding:

> `Unable to build while Live Coding is active. Exit the editor and game, or
> press Ctrl+Alt+F11`

Committing unbuilt C++ while another writer is mid-flight is the collision row
27 exists to prevent. The change is parked as a patch in this session group's
notes folder, `roster-sense-audit-UNBUILT.patch`, to be applied, built and run
in one step once the editor is free.

## 5. Two traps found on the way, worth more than their size

**`Build.bat` exits 0 on a failed build.** The Live Coding refusal above
printed `Result: Failed (OtherCompilationError)` and the script still exited
**0**. Anything trusting that exit code — a script, a hook, a CI step, an
agent — reads a failed build as a success.

**Automation results taken while the editor is open can come from a stale
binary.** With the build silently refused, the test run that followed used the
DLL already on disk, did not run the new assertions, and did not say so. Any
automation pass claimed while the editor was open should be re-taken with it
closed before it is relied on.

**Corrected 2026-10-31, same day:** the first version of this paragraph read
the stale binary off a test *count* — 33 reported against a suite said to be
44. That was wrong. The suite is 33, at every commit of the day and at `HEAD`,
and 44 was never a test-case count. The reliable signal is the red proof: in
that run `LLK.Enemies.PerceptionFailures` reported `Success` while
`HasReachingSense` was mutated to `return true;`, which a current binary cannot
do. `docs/TOOLING-TRAPS.md` carries the retraction and the receipts.
