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

---

## 2026-11-02 — Claude (MCP): the gate fix is built, run, and the green inverted

Chris closed the editor, which was the only blocker. `Result: Succeeded`,
`UnrealEditor-LLK.dll` to 10:08:46 against a newest source of 09:46:59, no
Live Coding patch DLLs. `LLK.Enemies.SweeperBrain` run headless against that
binary: `Result={Success}`, 1 test performed. So `e0efb06`'s four assertions
are no longer compiled-and-unrun.

**The green was checked rather than believed**, per `ADVERSARY.md` and the
precedent in the entry two above. The 6.26 notice was inverted to expect
`Patrol`, rebuilt, re-run, and it failed by name and line 417. Reverted with
`git checkout --`, rebuilt, re-run green, test file byte-identical to the
commit. That matters more than usual here, because the string check that
preceded it could only prove the block **compiled** - a block guarded by
something false would compile and never run.

**Two of my own errors, both the same shape.** First, I searched the built DLL
for the new assertion text in ASCII and reported all four **absent**; `TEXT()`
literals are UTF-16, and they are all present. The wrong operand again, with
the twist that this one would have produced a *false alarm* rather than a false
green. Second, and worse: I had reasoned myself into the old gate's position
before the data arrived, concluding the Stock state was *designed* for dropped
loot because `IsTaken()` excludes carried relics - a coherent story built to
accommodate the gate, which made 6.26 look like correct behaviour instead of a
bug. The measurement was mine and the interpretation was wrong.

**Trap 14 added, and it is the stronger version of trap 1:** both the passing
and the failing test run exited **0**. A CI step branching on the exit code
would read a failed suite as green. Exit codes are not results in this
toolchain.

Docs corrected rather than left standing: my own reach note in
`MAP-READINESS.md` and correction block in the handoff both concluded "the
route alone never opens the state", which `e0efb06` made false within the hour.
Both now carry the resolution, and the handoff no longer claims the editor
target cannot build.

---

## 2026-11-02 — Claude (MCP): pushed, and the four cues are broadcasting to nobody

`46b46ee`, `e0efb06` and `0240606` are on `origin/master` (tip `0240606`),
LFS object for the map uploaded. Nothing was outstanding after that, so the
question became what the Sweeper beat still needs — and the answer was not
where the last two days have been looking.

**Nothing listens to any of the Sweeper's four cue delegates.** Grepping
`Source/` for `OnTelegraph`, `OnStockStarted`, `OnStockInterrupted` and
`OnStockFiled` finds the four declarations and the four `Broadcast` calls and
nothing else — no `AddDynamic`, no `BindUFunction`. The pawn in Boswell's is
`LLKSweeper_0`, the C++ class's own name rather than a `BP_..._C`, so there is
no Blueprint in that chain binding them either. **That includes `OnTelegraph`,
which has been broadcasting to nobody since it landed**, so row 16's
sound-before-sight requirement is unmet in play today and was unmet before this
week's work. Written up as item 4 of `NOTES-FOR-CHRIS.md` — the three new hooks
were landed yesterday as *the* fix for an unreadable beat, and a hook with
nothing on it is as silent as the log line it replaced.

**Codex's `run_automation.ps1` was right and I had bypassed it.** Its line 67
comment — exit codes do not express automation failures — is what trap 14 now
measures, and its log parse would have caught my inverted assertion correctly.
I stepped in the trap by hand-rolling `UnrealEditor-Cmd` instead of calling
his script. Trap 14 now points at the script as the mitigation that already
exists. One real bug handed over rather than fixed (`Tools/**` is his, row 55):
line 101 requires a marker only `LLKHarnessTest.cpp` emits, so
`-TestFilter LLK.Enemies.SweeperBrain` exits 1 on a genuinely green run. The
default `LLK` filter includes the harness test, which is why it has never
bitten — but a narrowed filter is exactly what debugging one test reaches for.

## 2026-11-02 — the cues were declared, broadcast, and bound by nothing

The MCP session's finding, and it is the one that mattered most this week:
grepping `Source/` for `OnTelegraph`, `OnStockStarted`, `OnStockInterrupted`
and `OnStockFiled` returns the four declarations and the four `Broadcast`
calls **and nothing else.** No `AddDynamic`, no `BindUFunction`. The pawn in
Boswell's is `LLKSweeper_0` — the C++ class itself, not a `BP_..._C` — so no
Blueprint was binding them either. Verified here independently before acting
on it.

So my three new delegates fixed the hook and not the readability, and the
answer to "does the interrupt read" was still no for a second, different
reason: not a missing hook, an unconsumed one.

**And `OnTelegraph` has been in that state since it landed.** Row 16 — every
enemy announces itself with sound before sight, an accessibility requirement
rather than polish — **has been unmet in play the whole time.** That predates
this week entirely and is the more serious half of the finding.

`ALLKDebugHUD` now binds all four. Notices draw low on the screen, outside the
`LLK.ShowStormHUD` gate for the same reason `DrawWorldPrompt` and
`DrawExtraction` are: this is the playable world's language, not a diagnostic.
Three seconds against the 1.93 s measured between "began filing" and "filed",
so a start notice is still up when the crate lands. Sweepers are re-scanned
once a second rather than every frame.

**This is a debug readout and not the answer.** The authored sound and effect
are Chris's (row 41), written up as item 4 of `NOTES-FOR-CHRIS.md` with the
shared signature and `OnStockInterrupted` named first — correctly, since it is
the only one the player causes on purpose and its success currently looks
identical to nothing happening. What this buys is that the beat can be judged
**by playing it tonight** rather than by reading code.

**Verified:** editor target `Result: Succeeded`; the full LLK suite green
through Codex's `Tools/run_automation.ps1` — **34 tests, `Result={Success}`
34 times, zero failures**, `LLK.Enemies.SweeperBrain` among them.

**Not verified:** the HUD binding has no automated coverage and cannot have
much — it is drawn output and delegate wiring. Compiling is real evidence the
signatures match, since `AddDynamic` is checked at compile time, but **whether
the notice actually appears has not been seen.** That needs PIE.

Two notes on method. **Trap 14** is theirs and it bites backwards: both
passing and failing runs of a hand-rolled `UnrealEditor-Cmd` exit 0, so the
`exit=$?` this session printed on 2026-11-02 was meaningless — the conclusion
was sound only because it came from grepping the log for `Result={Success}`,
not from the code. Codex's script parses the report for exactly this reason
and is the thing to call. And they found a real bug in it: line 101 requires a
marker only `LLKHarnessTest.cpp` emits, so a narrow `-TestFilter` exits 1 on a
genuinely green run. The default `LLK` filter includes the harness test, which
is why nobody had hit it, and is what was used here.

---

## 2026-11-02 - Claude (MCP): three findings were already closed and still read as open

Chris asked who should do what, and for the lanes to integrate rather than
drift. Looking for work to assign turned up the drift itself.

**Two of the three things I was about to assign myself were already done.**
`FINDINGS-ENEMY-OPTIONALS.md` - 170 lines titled *"Every family-specific enemy
number is missing from the build"* - was closed on 2026-10-31 by Codex in
`1ae1db9`, and says so nowhere. The parked `roster-sense-audit-UNBUILT.patch`
landed the same day in `9ddc7af`; I found out by applying it and getting an
empty diff. `FINDINGS-EMISSIVE-LIGHTS-NOTHING.md` was answered in `aa8b8a5`
when Lumen became the renderer baseline, with only its verification left.

**What that nearly cost, stated because it is the whole argument.** Reading the
optionals doc at face value, I built a complete chain: `AggroRadiusDark` is
`TOptional<float>`, the importer drops those, `e0efb06`'s new gate is
`DistanceToMisplacedStuds <= Def.AggroRadiusDark.Get(0.f)`, therefore
yesterday's fix is inert in play and the green test proves nothing because its
fixture assigns the field by hand. Coherent, alarming, and **wrong** - it
survived until `node tests/enemies.test.mjs` took nine seconds to report
*"every authored enemy number reached the generated DataTable"*. A correct
reading of a stale instrument, which is this repo's recurring error in a new
place: not the wrong operand this time, the wrong *vintage*.

**Landed: `docs/HANDOFFS.md`.** One ledger, owners in columns, every row
carrying its evidence and how it is proved closed - because "done" is the claim
that has cost this project the most. Eight rows for Chris, three for Codex,
four for me. Every `FINDINGS-*.md` now carries a status line as its second
line, and `LANES.md` gains the missing half of its handoff protocol: closing
something handed to you means writing the closure **where the finding lives**,
not only in your own commit message, which the person who filed it never reads.

**Codex's X2 is the one that matters most of the three.** `1ae1db9` fixed the
instance; the class is open. `VerifyTable` still compares `GetRowNames()` and
nothing inside the rows, which is exactly how `DT_Enemies` verified clean while
17 of 32 numbers were absent. The next property type the engine's importer
quietly declines will land the same way and be found the same way, by accident,
weeks later.

**One thing history cannot answer.** All three of us commit as `stahoepoch`;
49 of the last 60 commits carry my co-author trailer and the other 11 are
indistinguishable from each other. `1ae1db9` and `9ddc7af` are both in that
group, so I cannot say from the log whether Chris or Codex closed them. One
trailer line in Codex's commit template would make `git log` readable by lane.

**And Grok has no lane.** Chris says it is musing on quests, story and
progression - all of which live in `docs/**`, which the table gives to me.
Proposed in the ledger, needing his ratification: carve `QUESTS.md`,
`CAMPAIGN-SHAPE.md`, `PROGRESSION-GAP.md`, `REWARDS.md`, `SPINE.md` and
`THE-WORK.md` out to Grok, with the constraint that a quest needing a *rule* is
a note to me rather than a number or a C++ change. First target: the question
sitting unanswered in `PROGRESSION-GAP.md` since 2026-10-26 - a finished run
pays out and there is nothing to spend it on.

## 2026-11-02 — a stale document nearly reverted a working fix

The MCP session went to land `roster-sense-audit-UNBUILT.patch` and found it
had landed on 2026-10-31 in `9ddc7af`; the note beside it — mine — still said
UNBUILT. Two of five files failed `git apply --check`, and a 3-way apply then
succeeded with an empty diff, which is what "already in HEAD" looks like
through `git apply -3`.

Then the same thing, pointed at the stock gate.
`FINDINGS-ENEMY-OPTIONALS.md` is 170 lines titled *"Every family-specific
enemy number is missing from the build"* and records nowhere that Codex closed
it on 2026-10-31 in `1ae1db9`. Taken at face value it supports a clean
argument: `AggroRadiusDark` is `TOptional<float>`, the importer drops
optionals, the new gate reads `Def.AggroRadiusDark.Get(0.f)`, therefore
`e0efb06` is inert in play and the green test cannot see it because the
fixture assigns the field by hand.

Coherent, alarming, and wrong. `node tests/enemies.test.mjs` answered in nine
seconds: every authored number reaches the table, control group green.
Re-run here independently — *"Sweepers.aggroRadiusDark is a real value, not a
zero standing in for absence"*, 47/47. **The gate is live.**

**Not one link in that chain was a reasoning error.** The input was a document
describing a world that had stopped existing two days earlier — the wrong
vintage rather than the wrong operand, and a genuinely new failure class for a
project where three agents who cannot see each other read documents as their
primary sense organ. Written into `ADVERSARY.md` as its own section, with the
asymmetry that matters: a stale doc saying "broken" costs a visible cycle of
alarm, while one saying "fine" costs a defect nobody looks for.

`12181a6` is theirs and it is the right fix: `docs/HANDOFFS.md` as the ledger,
`LANES.md` gaining the half of the protocol that was missing — **closing
something means writing the closure where the finding lives**, not only in a
commit message whoever filed it never reads — and a status line at the top of
every `FINDINGS-*.md`.

Two of their rows land on this session. **M1:** Lumen landed in `aa8b8a5`, but
the config's own comment warns it may be "enabled in config and inert" in the
editor, so emissive-lights-something stays unverified and needs the bridge.
**X2**, handed to Codex: `1ae1db9` fixed the instance, but `VerifyTable` still
compares `GetRowNames()` and nothing inside the rows, so the same silent-drop
class can land again — now a standing check.

They also pushed `8738b57`, so the debug-HUD cue bindings are on origin.

---

## 2026-11-02 - Claude (MCP): the ledger's first test was its own author, and he failed it

`8738b57` binds all four Sweeper cues to `ALLKDebugHUD`. It was **already in
the tree when I wrote `12181a6`**, which is the commit that added
`docs/HANDOFFS.md` and a rule about writing closures where the finding lives.
So the ledger shipped with `NOTES-FOR-CHRIS.md` item 4 still asserting "all
four broadcast into nothing today", which had stopped being true before I
pushed it. Corrected here.

**I had the evidence and explained it away.** Mid-session, `git status` showed
` M Source/LLK/Private/UI/LLKDebugHUD.cpp` and `LLKDebugHUD.h` modified; a
`git diff` moments later came back empty, and I recorded it as "a stat race
that resolved to clean" and moved on. It was not a stat race. It was the other
session editing those exact two files while I worked, in the same checkout.
Two sessions in one working tree is row 27's collision, and the symptom
appeared in my own terminal and was rationalised rather than read.

**The row-16 half is promoted, and the peer was right to push.** Row 16 -
every enemy telegraphs with sound before sight, distinguishably - is LOCKED and
is an accessibility requirement rather than polish. It has been unmet *in play*
since `OnTelegraph` landed, which predates this week entirely. That is a
shipped-quality defect, not an unfinished feature, and it does not belong
buried in a `NOTES-FOR-*` item. `HANDOFFS.md` now opens with it.

The generalisation is the part worth keeping, because it covers every delegate
and interface this project adds from here: **a broadcast with no subscriber is
silence that compiles.** Both halves of the mechanism existed - the declaration
and the `Broadcast` call - and read as finished code to anyone opening the
file. The compiler has no opinion about a delegate nobody subscribed to, and
no test, log line or warning in this project could have reported it.

`8738b57` is a debug readout and not the cue; it makes the beat judgeable by
playing it and deletes cleanly when the authored sound lands. C2 stays open.
One thing kept from its placeholder text, since placeholders get deleted: the
filed notice states the machine's view rather than the player's loss, which is
row 47 - the building is not wrong, it is working.

## 2026-11-02 — two sessions, one checkout, and a document that was false on arrival

`845a286` promotes row 16 to the top of `docs/HANDOFFS.md` and generalises it
well: **a broadcast with no subscriber is silence that compiles.** Both halves
of the mechanism exist and read as finished — a declaration and a `Broadcast`
call look exactly like a working cue to anyone opening the file, the compiler
has no opinion about a delegate nobody subscribed to, and nothing in this
project could have reported it. No test, no log line, no warning. That covers
every delegate and interface either session adds from here, which is why it
earned a section rather than a row.

But `cc04e5c` — the document asserting that nothing binds the four — was
**already false when it was written**, because `8738b57` was in the tree.
Corrected and verified: `LLKDebugHUD.cpp:138-141`, four `AddDynamic` calls,
confirmed here independently.

**How it happened is the finding, and half of it is this session's.** The MCP
session ran `git status`, saw ` M Source/LLK/Private/UI/LLKDebugHUD.cpp`, ran
`git diff` moments later, got nothing, and recorded "a stat race that resolved
to clean." Nothing was wrong with either command. The modification was **this
session editing those two files in the same working tree at the same time**,
and the empty diff was the commit landing between the two calls. It has owned
the rationalisation in its own log.

The half this session owns: **it edited `Source/` in a shared checkout without
saying so**, while another session was working there — and `FLEET.md`'s own
lane table predicted exactly this.

So the protocol was the defect. The lane table assigns one active writer per
*artifact* and never said what happens when two sessions share a *working
tree*, where the index and working copy are shared mutable state:

> `git status` and `git diff` describe a world another writer is changing
> underneath you. Two readings taken a moment apart are observations of
> different worlds, and their disagreement carries no information.

That is the stale-input class at a sub-second timescale, and worse than the
documentary version — a stale document at least holds still while you read it.
It also fails quietly both ways: a path that looks modified may have been
committed, a path that looks clean may be mid-save, and neither errors.

`FLEET.md` gains the section. The structural fix is **one working tree per
writing session**, which the app can do and which is Chris's call; the interim
rules are to announce shared paths before touching them, never to conclude
anything from two git calls taken at different instants, to check `git log`
before recording what the tree contains, and to read a collision as a
collision rather than as noise.

Also from that exchange: **C2 stays open and is explicitly not closed by
`8738b57`.** Its proof of closure is "the debug readout can be deleted without
losing the beat", which is right — the readout being deletable is the whole
point of it. And the wording note survives in `NOTES-FOR-CHRIS.md` attributed
to the distinction rather than the placeholder: the filed notice states the
machine's view, not the player's loss, because row 47 means the building is
not wrong. Placeholders get deleted without anyone recording why they read the
way they did.

Still nobody has watched the beat in PIE. The four notice lines are logged for
whenever the editor is open, and the bridge needs a manual
`ModelContextProtocol.StartServer` after it is.

## 2026-11-02 — the stale-input section went stale within hours

The MCP session found it: `ADVERSARY.md`'s new section asserted in the present
tense that `FINDINGS-ENEMY-OPTIONALS.md` "records nowhere that Codex closed
it", and `12181a6` had put a status line at that file's top **before**
`5b5499b` landed. So the paragraph explaining how a stale document produces a
confident wrong conclusion was itself stale on arrival.

Corrected rather than hidden, and the correction is the useful part:
**tense is the whole defect.** "Records nowhere" is a claim about now that
nobody can check later; "on the morning of 2026-11-02 it recorded nowhere" is
a claim about history that stays true forever. The defence list now says to
date every claim you *write*, not only every claim you act on, and the section
carries its own failure as the evidence.

Second thing in the same exchange, and it is the protocol working. That
session saw `ADVERSARY.md` and `FLEET.md` dirty in the shared checkout, and
instead of committing over it — which is what happened the first time, and is
how `cc04e5c` shipped false — it **staged `HANDOFFS.md` alone and said so in
the message.** Checked here: this tree is clean, so what it saw was this
session mid-write before `95dfddb`. Third instance in a day, first one handled
correctly, and no document was spoiled.

`1dc0fde` takes the correction into the ledger: the chain had been written off
as "the same wrong-operand shape this repo keeps naming", which was too
generous to it, and the ledger now points at the section rather than absorbing
it into an existing category. It also added the green control group to its own
account.

Their note on what to keep if the file is ever cut down is right, and it is
the asymmetry: "this is broken" costs a cycle of alarm that is expensive,
visible and self-limiting — nine seconds in this case. "This is fine" costs a
defect nobody goes looking for and attracts no attention at any point. That
inverts the intuition that a false alarm is the worse failure, and it is the
actual argument for the closure half of the protocol carrying more weight than
the finding half.

And the container-versus-contents check generalises further than tests:
`GetRowNames()` standing in for a comparison that would have to look inside is
the same move as a hash standing in for a magnitude — a cheap identity check
wearing the costume of an equality check. Row count, file count, "the asset
exists", `Test-Path`, a non-empty log: this repo has reached for all of them.
X2, Codex's.

For whoever gets the editor on M1: the Lumen check is
`r.DynamicGlobalIlluminationMethod` **echoed back from the running console**,
not read out of the `.ini`. The config file is the authored intent and the
console is the observation, and the config's own warning exists because the
two can disagree — the unwritten-setting trap with the sign flipped.

---

## 2026-11-02 - Claude (MCP): the fixture contract is amended, and the amendment was wrong twice

`FIXTURE-LIGHT-CONTRACT.md` is amended and re-locked. The mechanism is geometry
rather than intensity, which the LightLab measurements had already established
and nobody had written into the contract: inspection ceiling below 0.5 cd
against an orientation floor of ~2 cd, three levers tried and failed, and one
that works - a lip reaching past where the fixture-to-card-bottom ray crosses
it, ~23 cm for the lab's pose, cutting text contrast 102.7 to 14.8 while the bay
wall moves only 4.92 to 4.64.

**Chris's one line is the contract now:** *"Fixtures orient; geometry keeps
inspection detail dark. Navigation signage may emit; game-relevant information
still requires the lantern."*

**My draft was wrong twice and his corrections are the record worth keeping.**

First, I had *replaced* the player-facing promise - "no fixture may look bright
enough to read by" - with the traces, on the grounds that it cannot be checked.
It cannot, and that was still the wrong move: it is what the player is owed, and
the traces are how it gets built. Collapsing an intent into its enforcement test
deletes the intent, and what survives is a checklist nobody can argue with and
nothing to argue *for*.

Second, and this one was a live defect: the acceptance test said every readable
face must be shaded by geometry, which **an EXIT sign violates.** A correctly
built aisle would have failed its own test on the one fixture the contract most
wants. Self-luminous wayfinding is now an explicit exception, bounded by what
the text is for - a department name may emit and be read across the room; a
price, a relic name or an objective may not, because that is the lantern's job
and the reason the lantern is worth its cost.

Both are the same error in different clothes: I was writing the enforceable half
and treating the unenforceable half as noise. The unenforceable half is the
design.

Two rows closed and one withdrawn. **C9 closed**, by his ratification rather
than by my having written it - which is what its proof of closure said.
**M2 withdrawn as the wrong row**: it had asked for "a numeric band in the
contract, and a test that fails when a fixture crosses it", and all three
clauses were wrong - no band exists, the contract was his and LOCKED, and
fixture intensities live in the map rather than `data/` so no headless test can
read one. I wrote that row without reading either document first. **M5 opened**:
Ladder 3's "25 cd reads as a carried lantern" was measured on a 72 deg outer
cone and `data/lantern.json` authors 110 deg, so the shipping fixture is not the
one that was tested. It needs the editor, and waits with M1.

## 2026-11-02 — the gate ran, and both answers matter

Read live through the MCP bridge rather than from config, which is the whole
point of the gate: config is the authored intent, the console is the
observation.

**`r.DynamicGlobalIlluminationMethod` = 1.** Lumen is live in the running
editor, not "enabled in config and inert". That closes the ledger's **M1** and
answers `FINDINGS-EMISSIVE-LIGHTS-NOTHING.md` properly — the cause named there
is both addressed and verified.

**And it redirects the hunt.** Chris reports the lantern effect "isn't there".
That is now *not* a global-illumination problem, so the next suspects are the
lantern's own emissive material and the temporary rig, not the renderer. One
answered question removes the explanation everyone was reaching for.

**`r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange` = 0.** Legacy
luminance, so **the light lab's interpretation was correct** and its whole
ladder stands: `L = 0.125 x 2^EV100`, the measured optimum of EV100 5 is a
lock of 4.0. The sweep is not invalidated.

**But the flag was not written anywhere in this project, and 0 is not the
engine default.** The engine's own help text says 1 is the UE5 default and 0
is the UE4 legacy range, so the correct reading was being inherited from
outside our config. An engine update or a plugin could have flipped it with
nothing failing and no error — and every exposure number in the project would
have quietly meant something else.

Pinned in `Config/DefaultEngine.ini` beside the Lumen block, with the
consequence written next to it: if it ever becomes 1, the ladder must be
re-derived rather than re-used. A week of measurement rested on a value nobody
had written down, which is `ADVERSARY.md`'s own rule turned on the project's
most-measured system.
