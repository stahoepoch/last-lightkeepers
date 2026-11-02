# Fleet log

Append-only. One entry per session that changed something durable. Newest at
the bottom, so the file reads forward.

`git log` says what changed. This says **who was driving, why, and what it
cost** — which is the part three agents and a human cannot reconstruct a week
later.

**Entry format.** Date, party, one line of what happened, then only what a
future reader needs: what was decided, what was retracted, what is still
open. Not a changelog. If a commit message already explains it, cite the hash
and stop.

Seeded 2026-11-01. Earlier history is in `git log` and in the `FINDINGS-*`
files, which are better records than a retrospective summary would be.

---

## 2026-11-01 — Claude (MCP): the lighting instrument thread closed

Roughly ten rounds between the MCP session and the chat session. **Five
instrument defects found**, both parties wrong more than once. The procedures
that came out of it are now `ADVERSARY.md`.

- Capture geometry drift — fixed insets against a moving window (`a109611`).
- Offscreen GI omission; the occlusion/visibility gate.
- The staleness detector's false-rejection mode: fixed by differencing against
  a **fixed baseline** rather than the previous capture (`d1a92b9`).
- The frame counter's wrong operand: fixed as a **rate over the
  change-to-capture interval** (`9063269`, `215d134`).
- `e1eb630` retracts `e55c587` — the restart claim was wrong because the
  engine log is UTC and the machine is EST. The lantern numbers reproduced.

**Result that matters:** `023f675`. The exposure sweep's card half lands on
**EV100 5** (lock 4.0), exactly as the illuminance arithmetic predicted.
Glyph separation 175.8 at 0% clipped, against 104.2 at the current lock 1.0 —
so **the current lock is marginal**, not merely suboptimal. The plateau from
EV100 5→7 is flat and one-sided; there is no argument for going higher.

**Still open:** the orientation half. EV100 3 against 5 at the Bay B pose,
with Bay A's card as in-run witness. Chris judges "navigable but uneasy"
against "nothing is there." Not a measurement question.

**Also open:** `bThrottleCPUWhenNotForeground` was written nowhere, so it was
at the engine default — on — for **every capture this project has ever
taken**. Codex has toggled it off in the editor. Persistence across a restart
is unverified and the key is still not on disk.

## 2026-11-01 — Chris: the Epic Launcher asset pipeline

Blocked: "No compatible user projects found" with *Show all projects* checked,
so nothing redeemed on Fab could be added to the project.

Cause, from the engine source: `CreatedProjectPaths` in
`EditorSettings.ini` holds **parent directories**
(`SProjectDialog.cpp:1950` takes `FPaths::GetPath` twice), and the key was
absent. The `RecentlyOpenedProjectFiles` route cannot substitute for it —
`DesktopPlatformBase.cpp:1809` parses those entries as bare filenames while
the property is a `TArray<FRecentProjectFile>` struct, so double-clicking a
`.uproject` never registers its folder.

**Fix:** `CreatedProjectPaths=C:/Users/chris/Documents`. Confirmed working.

Habits set with it: one pack per commit, with file count and LFS bytes in the
message; resist bulk imports, because LFS cannot be undone without a history
rewrite.

## 2026-11-01 — Chris: the playtest that changed the core verb

He walked the lantern walk lab and found the pickup instant. His own
conclusion: *"holding in E for a few seconds to pick something up will work.
Less time if your light is on."*

Committed as `8f940c9`. **Light becomes a rate, not a gate** — the central
tension now comes out of existing systems, because holding still in the light
is what summons a Sweeper. Constraints: accumulate, never reset; drive it from
the debounced `bCurrentlyLit`; read gameplay radius, never visual light
(`d065469`); and make the ratio a real decision — roughly 4s dark against
1.5s lit.

## 2026-11-01 — Codex: the walk lab became playable

`a3cb4ad` shell, `2a9970e` relics and Sweeper patrols, then past what the
chat session had seen: `d91774f` held lantern and progressive relic
inspection, `362688e` relic searches visible and usable in darkness,
`a060e78` the walk lab turned into an enclosed emergency-lit store,
`7010501` a physical first-person lantern rig, `2b275ab` asset intake
documented, `c18f9ef` `GameFeatureData` registered, `0bd1534` and `7d2c6c7`
the keeper mannequin and the co-op lantern mounted to the hand.

## 2026-11-01 — Claude (chat): docs made findable, then audited

`0ef5ed5` — `docs/README.md` as an index with four authority tiers, and
`WORLD-SYSTEMS-BOARD.md`, `THE-VALLEY.md`, `THE-WORK.md` committed. All three
had been written and left untracked, which is why the board's contents kept
being re-derived in conversation.

Then this commit: `FLEET.md`, `ADVERSARY.md`, `DRIFT.md`, `LOG.md`.

**`DRIFT.md` section A needs Chris and nothing else can clear it:** rows 6 and
73 of `DECISIONS.md` are both LOCKED and contradict each other on whether
darkness slows the clock. Row 73 is the live one. Row 6 has no banner and sits
sixth, under "The core rules."

## 2026-11-01 — Claude (MCP): Boswell's first extraction run is playable

`cfae411`. A playable loop in `/Game/LLK_Boswells_FirstRun`, verified on
Unreal's own `Result: Succeeded`, npm tests green, and PIE registering the
Sweeper, the patrol and the LoadingDock zone.

It also cleared `DRIFT.md` section B's urgent item on its own: the map had
been sitting untracked — binary, inside the LFS path, owned under row 41 by a
single writer, one editor crash from gone. The register named the risk and the
lane owner committed it, which is the intended shape of row 41 rather than a
coincidence.

The eleven untracked root design documents are still outstanding, and C2 is
the reason to move them into `docs/` before committing rather than after:
committing a divergent duplicate makes the divergence permanent.

## 2026-11-01 — Claude (chat): the indifference beat, and the root tidied

Chris: *"Work on the project, get it in order… Let's at least get the
horrifying first mini play through done. U have the handoff."*

The handoff's own "finish next" list is art and asset migration, which needs
the editor this session does not have. The gap it could close is the one that
makes the slice horror rather than stealth: the Sweeper was an obstacle you
avoid.

**`ELLKSweeperState::Stock`.** On patrol, unnoticed, with stock in the wrong
place inside the authored `ContactRange`, a Sweeper files it. It wires
`Sweepers.attractedToDroppedLoot: true` — Chris's key, imported since M0.5,
asserted for presence by `tests/balance.test.mjs`, consumed by nothing until
now, and one of the gaps `FINDINGS-ENEMY-OPTIONALS.md` recorded. Being
noticed interrupts the job, which is the decision it creates: the only way to
stop a Sweeper taking your crate is to let it see you. It relocates and never
deletes.

Three gates, all off by default, so it cannot turn itself on in a map that
did not ask: the authored key, an actor tagged `LLK.StockBay`, and contact
range. **The map needs that one tag before any of this is visible.**

**Verified:** game target `Result: Succeeded` on all three changed TUs, the
test TU among them; `npm test` green. **Not verified:** the editor target
would not build — Live Coding was active and the editor was Chris's and open —
so `LLK.Enemies.SweeperBrain` has been compiled and never run, and nothing
has been seen in PIE.

Also: the eleven untracked root documents are in `docs/` and indexed, and the
repo root is no longer a second docs directory. One correction to my own
audit — `FIXTURE-LIGHT-CONTRACT.md` was **not** a divergent duplicate. The
two files are different documents that shared a filename, and I read a hash
inequality as divergence. `ADVERSARY.md` names that exact error in its own
words: a hash has no magnitude. Renamed to `docs/FIXTURE-LIGHT-FIELD.md`;
`DRIFT.md` sections C and C2 are struck with the reason.

## 2026-11-02 — Claude (chat): the editor target builds and the test actually passes

Chris closed the editor, which unblocked both jobs the MCP session had
correctly refused to fake.

- `LastLightkeepersEditor` Win64 Development: **`Result: Succeeded`**.
  `UnrealEditor-LLK.dll` relinked at 06:55 against sources last touched
  22:29-22:31, so the staleness the MCP session caught is gone — it checked
  for Live Coding patch DLLs before concluding, and there are none now either.
- `LLK.Enemies.SweeperBrain`, headless: **`Result={Success}`**, one test
  performed, zero events.

**And the green was checked rather than believed.** `ADVERSARY.md` says a
calibration that can only say pass is nearly worthless, so one new assertion
was inverted deliberately — `Begins.Mind.State` expected `Patrol` instead of
`Stock` — rebuilt, and re-run. It failed, by name and line:

```text
Error: Test Completed. Result={Fail} Name={SweeperBrain}
Error: all three gates open and it starts filing: The two values are not
       equal. [...LLKSweeperBrainTest.cpp(399)]
```

So the harness does execute the new blocks, and `WITH_DEV_AUTOMATION_TESTS`
has not quietly compiled them out — which is the failure mode that would have
made every assertion in blocks 10 through 13 a decoration. Reverted with
`git checkout --`, rebuilt, re-run green, and `git status` is clean, so the
revert is exact rather than approximately exact.

**Still unobserved:** nothing has been seen in PIE. The map has no
`LLK.StockBay` yet, so the Stock state cannot run in the slice. That is one
tag and it is the MCP session's, now that the editor is free.

---

## 2026-11-02 — Claude (MCP): the bay is tagged, and the floor cannot reach it

The tag the entry above was waiting for. `BSW_STOCK_BAY` — an empty `AActor`
matching the `LLK.Patrol` convention rather than a `TargetPoint` — placed in
`LLK_Boswells_FirstRun` at **(450, 450, 50)**, tagged `LLK.StockBay`, saved.
The spot was chosen against measurements, not by eye: bare floor with clear
headroom, reachable from the aisle at y=450, and **two** patrol legs cross the
line from `BSW_WRONG_CRATE` to it. Nearest patrol point 17.6 studs — outside
the Sweeper's 12-stud dark aggro and inside its 38-stud lit one, so the walk
back is safe in the dark and a gamble with the lantern up. Z is 50 because all
three relics are 100-tall boxes with centred origins, so 50 puts a filed
crate's base exactly on the floor.

**The beat was then observed end to end**, which closes the item above:
`began filing misplaced stock` to `filed LLKRelicActor_1 into the stock bay`,
1.93 s apart against the authored 2 s, and the relic read out of the running
PIE world at **(450, 450, 50)** — the tagged actor's own origin.

**Two things were corrected, and both were corrections of this project's own
documentation rather than of code.**

- **`LLK.Enemies.SweeperBrain` cannot validate a tag.** It was run, and it
  passes, 1/1 in 6.5 ms. But it has *zero* references to `UWorld`, `GEditor`
  or `ActorHasTag` in 483 lines and sets `bHasStockBay = true` in its own
  fixture, so it would pass identically with the map untagged. The brief asked
  for the tag and the test in one breath; they turn out to be independent, and
  reading that green as proof of the bay was the available mistake.
- **"Tag any actor and the behaviour turns on" is not true.** The brain also
  gates on `DistanceToMisplacedStuds <= Contact.ContactRange`, and
  `ContactRange` is 5 studs. The closest any patrol leg comes to any authored
  relic on this floor is **6.26 studs** — 35 cm outside. An idle 16-second run,
  about three full laps, produced no stock line at all; the observation above
  only exists because a relic was deliberately parked 3.59 studs from the
  Sweeper and then restored. Written up in `docs/MAP-READINESS.md`.

**Still open, and deliberately not acted on:** closing that 35 cm means moving
a patrol point or a relic, and both are route shape — one of the four things
Chris's scope says the map authors and the pawn refuses to invent. So the
number is recorded and the route is untouched. The design question that
follows is whose: the gate makes the trigger *dropped* loot, since a carried
relic is `IsTaken()` and excluded, so the intended beat is losing the crate
near a Sweeper rather than one tidying a shelf. Whether the authored stock
should also be filable is Chris's call.

**Trap 13** was added on the way: during PIE the scene tools answer about the
`UEDPIE_0_` duplicate, so a write issued while PIE runs edits a world that is
discarded on stop — and the read-back confirms it, because it reads the same
doomed world. One call earlier and the restore would have reported success
while leaving a test value in the committed map.

## 2026-11-02 — Claude (MCP) measured the floor; Claude (chat) had the gate wrong

`46b46ee`. `BSW_STOCK_BAY` tagged at (450, 450, 50) in Boswell's First Run,
picked against measurements rather than by eye: two patrol legs cross the
crate→bay line, nearest patrol point 17.6 studs — outside the 12-stud dark
aggro, inside the 38-stud lit one, so the walk back is safe dark and a gamble
lit. Z of 50 because the relics are 100-tall boxes with centred origins.

**The beat was observed end to end in PIE**: `began filing misplaced stock` →
`filed LLKRelicActor_1 into the stock bay`, 1.93 s apart against the authored
2 s, with the relic read back out of the running world at the bay's own
origin.

**Three findings, all of them corrections to things I had written.**

1. **The gate could not fire on the authored floor.** `ContactRange` is 5
   studs; the closest any patrol leg comes to any authored relic is **6.26**.
   Thirty-five centimetres, three laps, nothing. The PIE observation exists
   only because a relic was parked 3.59 studs away and then restored.
2. **"Tag any actor and the behaviour turns on" was false** — my line, in my
   own handoff. Tagging was necessary and not sufficient.
3. **`LLK.Enemies.SweeperBrain` cannot validate the tag.** Zero references to
   `UWorld`, `GEditor` or `ActorHasTag`; it sets `bHasStockBay` in its own
   fixture, so it passes identically with the map untagged. The tag and the
   test are independent and I should not have implied otherwise.

**The fix is not to nudge the floor.** Reusing `ContactRange` for both
noticing and reaching was the wrong operand dressed as a tidy saving — contact
range is the reach for touching a keeper. It now notices within
`aggroRadiusDark` (12, authored), walks there via
`ELLKSweeperMove::MisplacedItem` — which I had dropped as dead vocabulary, and
the floor says it was not — and files within `ContactRange` (5, authored),
bounded by `losesTargetAfterSeconds` so an unreachable relic produces a
warning instead of a Sweeper that silently stops patrolling. No invented
number, and **the map needs no change**: 6.26 is now asserted in the test
because it is the distance the real floor offers.

**And the beat had no cues at all.** `bStockStarted`, `bStockInterrupted` and
`bItemShelved` each only wrote a log line, so a player could watch a crate
move and have nothing to connect it to. `OnStockStarted`,
`OnStockInterrupted` and `OnStockFiled` now exist, matching `OnTelegraph`;
`OnStockFiled` broadcasts at the bay so a cue can point at where the crate
went. My guess that the cue was the problem was right and understated it —
the interrupt, the one moment a player causes on purpose, had no hook either.

**Verified:** `LastLightkeepers` game target `Result: Succeeded`.
**Not verified:** the editor target is blocked by Live Coding again — Chris
reopened the editor — so `LLK.Enemies.SweeperBrain` has NOT been run against
this change. The previous green covered the old gate and does not carry.

Also from that session: **trap 13**. During PIE the scene tools answer about
the `UEDPIE_0_` duplicate world, so a write while PIE runs edits a world
discarded on stop *and the read-back confirms it*. One call earlier and a test
value would have been left in the committed map with a success report on it.
