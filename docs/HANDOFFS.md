# Handoffs — the open items, their owners, and how each one is proved closed

Written 2026-11-02 by Claude, on Chris's instruction: *"i definitely need to
make sure what you do and what codex does gets integrated and you guys are able
to communicate effectively."*

`docs/LANES.md` already says who owns which path and how the three of us hand
work over. That part works. **This file exists because of the half that does
not: nothing in the project records when a handed-over item is finished.**

---

## The failure this file fixes, with today's evidence

Findings get written down. Closures do not. So a document whose whole thesis is
a live problem stays on disk reading as a live problem after the problem is
gone, and the next reader — agent or human — plans against a world that no
longer exists.

Three measured instances, all found in twenty minutes on 2026-11-02 while
looking for something else:

| Document | Says | Actually |
| --- | --- | --- |
| `FINDINGS-ENEMY-OPTIONALS.md` | *"Every family-specific enemy number is missing from the build"* — 170 lines, present tense, titled as live | **Closed 2026-10-31 in `1ae1db9`.** Its own check now passes: `every authored enemy number reached the generated DataTable`, control group green |
| `roster-sense-audit-UNBUILT.md` + `.patch`, in the session notes folder | written and unbuilt, parked because Live Coding refused the build | **Landed 2026-10-31 in `9ddc7af`.** `HasReachingSense` and `AuditSenses` are both at `HEAD`; a 3-way apply of the patch today was a no-op |
| `FINDINGS-EMISSIVE-LIGHTS-NOTHING.md` | *"this project has no Lumen"* | **Answered 2026-11-01 in `aa8b8a5`**: `r.DynamicGlobalIlluminationMethod=1`, `r.ReflectionMethod=1`, SM6. The *verification* is still open — see M1 |

**This is not a tidiness problem, and the cost is specific.** Reading
`FINDINGS-ENEMY-OPTIONALS.md` today tells you that `AggroRadiusDark` never
reaches the build. `e0efb06`'s new Stock gate is
`Senses.DistanceToMisplacedStuds <= Def.AggroRadiusDark.Get(0.f)`. If that
field were still being dropped, the fix we spent 2026-11-02 building and
verifying would be **inert in play** — `.Get(0.f)` would hand it zero, nothing
would ever be within zero studs, and the automation test would stay green
regardless because its fixture assigns the field by hand.

That was a complete, coherent, alarming conclusion, and it was **wrong**. It
survived only until I ran `node tests/enemies.test.mjs`, which took nine
seconds, and whose control group was green.

**I first wrote that off as "the same wrong-operand shape this repo keeps
naming." That was too generous to it, and the correction is `5b5499b`'s.**
Every other error on this project's adversary list is a *reasoning* error —
the wrong operand, a hash read as a magnitude, a counter read as a redraw —
and you catch those by thinking harder about the argument. **Not one link in
the chain above was a mistake.** The input was a document describing a world
that had stopped existing two days earlier, and arguing with it would not have
helped, because it was internally consistent.

That is its own failure class and it is now a section of
`docs/ADVERSARY.md` — "The wrong vintage" — with the defence: *a document is
evidence about when it was written, never about now.* **Read it alongside this
file.** It carries the asymmetry that makes this ledger load-bearing rather
than tidy: a stale document saying *"this is broken"* costs a visible,
self-limiting cycle of alarm, which is what happened here and was over in nine
seconds. A stale document saying *"this is fine"* costs a defect nobody goes
looking for. **The second is worse and attracts no attention at all** — which
is why the closure half of the rule below matters more than the finding half.

### So the rule, and it is one line each way

- **Finding something in another lane:** write the symptom, the evidence, the
  file and the line, and hand it over. Already the rule (`LANES.md`, "Review
  findings, in any direction"). Unchanged.
- **Closing something handed to you:** write the closure *where the finding
  lives* — a status line at the top of that document, naming the commit. Not
  only in your own commit message, which the person who filed it will never
  see.

Every `FINDINGS-*.md` now carries a status line as its second line. If you
close one of these, change that line. It costs a sentence and it is the only
thing standing between the next reader and a confident wrong plan.

### One thing that makes this harder than it needs to be

All three of us commit as `stahoepoch`. Of the last 60 commits, 49 carry
`Co-Authored-By: Claude Opus 5` and 11 carry nothing — and those 11 are
indistinguishable from each other, so history cannot say whether Chris or Codex
did a given piece of work. `1ae1db9` and `9ddc7af` are both in that group.

**Proposal, Chris's call:** Codex adds a trailer of its own. One line in its
commit template, and `git log` becomes readable by lane for the first time.

---

## The one row that is a shipped-quality defect, not an unfinished feature

Everything else on this page is work not yet done. **C2 is different, and it
was buried in a notes file until the session that landed the delegates argued
it should not be.** It was right.

`DECISIONS.md` row 16 is LOCKED: every enemy telegraphs with sound before
sight, distinguishably. That is an accessibility requirement and a build
requirement. **It has been unmet in play since `OnTelegraph` landed** - in
Boswell's the Sweeper announces itself with nothing at all, because the
delegate was declared, broadcast, and never bound.

Why it stayed invisible is the part worth generalising: **both halves of the
mechanism existed and read as finished.** A declaration and a `Broadcast` call
look exactly like a working cue to anyone reading the file, and the compiler
has no opinion about a delegate nobody subscribed to. Nothing in the project
could have told us - no test, no log line, no warning. The same shape covers
any delegate, any interface, any event this project adds from here: **a
broadcast with no subscriber is silence that compiles.**

`8738b57` binds all four to `ALLKDebugHUD`, which makes the beat judgeable by
playing it. It is a debug readout and not the cue, and it does not close C2.

---

## Live items, by owner

Each row names the evidence and **how to prove it closed**, because "done" is
the claim that has cost this project the most.

### Chris

| | Item | Why it is his | Proof of closure |
| --- | --- | --- | --- |
| **C1** | **The Steam App ID.** The reserved `1903488` / `1903489` / `1903490` and store item `1409731` are packages and a store item, not an App ID | Steamworks dashboard; nobody else can read it | Online Subsystem Steam configured with a real App ID, and a friends-invite that connects |
| **C2** | **The four Sweeper cues - and row 16 is a locked requirement it breaks.** See the section above. `8738b57` binds all four to the debug HUD, so the beat is now judgeable; the authored cue is still absent | Row 41, sound and effect are his. Row 16 is LOCKED and is accessibility, not polish | An authored sound on each, `OnStockInterrupted` first: it is the only one the player causes on purpose, and its success looked exactly like nothing happening. Closed when the debug readout can be deleted without losing the beat |
| **C3** | **`IMC_LLK` and the seven `IA_LLK_*` actions.** All seven or none — `IsComplete()` falls back wholesale | `Content/` is his | `.\Tools\validate_input_assets.ps1` passes. It fails today, by design, rather than accepting the fallback |
| **C4** | **The relic outline material.** Stencils 1/2/3/0 are already correct in play | His "make it WHY" note | The 2→3 transition reads: a thing you identified visibly stops being available when you walk away with the lantern |
| **C5** | **M1.5, the greybox ground floor.** Codex and Claude reached this independently: the next blocker is content, not code | Art direction | Five of the six acceptance questions in `THE-STORE.md` judgeable on the blockout. Also gates row 63's traversal work, which needs real counter heights |
| **C6** | **The singular/plural title**, open since `PLATFORM.md` | Published under his account | The store page says one thing and the repo agrees |
| **C7** | **Two renderer/editor decisions**: `bThrottleCPUWhenNotForeground`, and whether `ExtendDefaultLuminanceRange` flips to 1 | Values and feel are his; the files are Codex's | Decided either way and written into `DECISIONS.md`. "Still open" is itself a cost — it blocks the exposure work |
| **C8** | **Audio bus structure** | `LANES.md` responsibility table | Busses exist before mixing, rather than after |
| **C9** | **The fixture light contract asks for something light cannot do.** Its rule - ambient fixtures must "never look bright enough to read a relic under" - has no candela solution: the inspection ceiling is below 0.5 cd and the orientation floor is ~2 cd, so any fixture broad enough to light the aisle lights a price card directly. Lower candela, barn doors and floor albedo were all measured and all failed | `FINDINGS-FIXTURE-LIGHT-BAND.md`, four levers tabulated. The contract is LOCKED, so this is his to amend | The contract names **geometry** as the mechanism rather than intensity - a shelf lip reaching far enough to block the fixture's direct ray to the readable face, measured at **~23 cm** for the lab's pose, which cut text contrast sevenfold while leaving the bay wall untouched. **CLOSED 2026-11-02 — ratified by Chris with two corrections, both of which improved it.** He kept the player-facing promise, *"no fixture may look bright enough to read by"*, as the design intent rather than letting the traces replace it: the traces are how the promise is built, not the promise. And he added the exception the draft was missing — self-luminous wayfinding may be readable, but may not carry relic, product, lore or objective information. Without it an EXIT sign is a readable face nothing shades, so a correctly built aisle would have failed its own test. His one-line rule: *"Fixtures orient; geometry keeps inspection detail dark. Navigation signage may emit; game-relevant information still requires the lantern."* |

### Codex

| | Item | Evidence | Proof of closure |
| --- | --- | --- | --- |
| **X1** | **`run_automation.ps1:101` fails a green run on any narrowed filter.** It requires a marker only `LLKHarnessTest.cpp:36` emits, so `-TestFilter LLK.Enemies.SweeperBrain` exits 1 while the log says `Result={Success}`, `1 tests performed` | Handed over 2026-11-02 in `NOTES-FOR-CODEX.md`, with the log | That command exits 0 on a green run, and still exits 1 when no test reports at all |
| **X2** | **Value verification at import time.** `1ae1db9` fixed the instance; the *class* is still undetected. `VerifyTable` compares `Table->GetRowNames()` against expected row names and nothing inside the rows, which is why `DT_Enemies` could verify clean while 17 of 32 numbers were missing | `LLKImportDataCommandlet.cpp:331`, and §4 of `FINDINGS-ENEMY-OPTIONALS.md` asked for exactly this | A deliberately dropped field fails the import, not a later test. Proven red before it is called done |
| **X3** | **The scan cannot run in CI.** `Content/Data/` is gitignored, so on a fresh checkout `tests/enemies.test.mjs` reports that it compared nothing | Stated limit, same §4 | Either CI generates the assets before the scan, or X2 makes the scan redundant. X2 is the better fix and subsumes this |

Codex also owns, from the responsibility table and not yet started: the
performance budget, crash reporting, and build/packaging for a tester who
cannot launch the editor.

### Claude

| | Item | Evidence | Proof of closure |
| --- | --- | --- | --- |
| **M1** | **Verify emissive now lights something.** Lumen landed in `aa8b8a5`; the config's own comment warns the settings "risk being enabled in config and inert in the running editor, which would look exactly like the no-GI behaviour they are meant to replace" | `FINDINGS-EMISSIVE-LIGHTS-NOTHING.md`, open half | A measurement in `LLK_LightLab` showing the lantern core's emissive lighting a surface it does not own. Needs the editor open and the MCP bridge started |
| **M2** | ~~The fixture light contract still has no band~~ **- withdrawn 2026-11-02, as the wrong row.** It asked for "a numeric band in the contract, and a test that fails when a fixture crosses it", and all three clauses are wrong: no band exists, the contract is **LOCKED by Chris**, and fixture intensities live in the map rather than `data/`, so no headless test can read one | `FINDINGS-FIXTURE-LIGHT-BAND.md` measures the gap at ~4x under Lumen - inspection ceiling below 0.5 cd, orientation floor ~2 cd - and records three failed levers | **Closed by being correctly re-stated.** The decision is Chris's, as **C9**. What was mine was reading the contract before writing a proof of closure against it |
| **M5** | **Ladder 3's lantern reading was taken on the wrong cone.** "25 cd reads as a carried lantern" came from a 72 deg outer cone; `data/lantern.json` authors **110 deg**. `a060e78` adopted the 25 anyway | `FINDINGS-FIXTURE-LIGHT-BAND.md` conditions section against `data/lantern.json:44` | Ladder 3's three rungs re-run at 110 deg, or the authored cone changed to match what was measured. Needs the editor and the bridge, so it waits with M1 |
| **M3** | **Status lines on every `FINDINGS-*.md`** | This file's whole argument | Done 2026-11-02, in the same commit as this file |
| **M4** | **The orientation half of the exposure sweep (Bay B)** | Longstanding, not acted on | The sweep covers orientation as well as intensity |

---

## Grok, and the lane that does not exist yet

Chris, 2026-11-02: *"grok is basically musing with quests, and story and
progression."*

**There is no lane for that, and the collision is real.** `docs/**` is Claude's
in the `LANES.md` table, and quests, story and progression live there —
`QUESTS.md`, `CAMPAIGN-SHAPE.md`, `PROGRESSION-GAP.md`, `REWARDS.md`,
`SPINE.md`, `THE-WORK.md`. Grok writing into those today is two writers in one
file, which is the rule the whole table exists to enforce. Grok appears nowhere
in `LANES.md`, `FLEET.md` or the README; the only trace of it in the repo is as
the author of two files under `research_notes/`.

**Proposal, needing Chris's ratification, because a lane change is his:**

- **Carve the narrative set out of `docs/**` and give it to Grok**: `QUESTS.md`,
  `CAMPAIGN-SHAPE.md`, `PROGRESSION-GAP.md`, `REWARDS.md`, `SPINE.md`,
  `THE-WORK.md`. I keep the systems docs, the findings, the traps,
  `DECISIONS.md` and this file. One owner per file, as before.
- **The constraint that makes it safe:** a quest or a reward that needs a *rule*
  is a note to me, never a number and never a C++ change — same shape as
  `NOTES-FOR-CHRIS.md`. Values stay in `data/*.json`, Chris's. The story may ask
  for a mechanic; it does not get to specify one.
- **The live question to point it at first**, already written and sitting
  unanswered since 2026-10-26: `PROGRESSION-GAP.md` — *"a finished run pays out,
  and there is nothing to spend it on."* Materials have been accumulating in
  every profile since banking landed. That is the gap between a loop and a
  game, it is pure design, and it needs no engine.

**And a fourth party breaks the notes pattern.** Three parties needed three
`NOTES-FOR-*.md` files; four need six, and nobody will keep six current. This
file is the replacement: one ledger, owners in columns. If Grok gets a lane,
its handoffs are rows here rather than a seventh file.

---

## Keeping this current

The ledger is worth exactly as much as its last update. Two habits, and they
are cheap:

- **Close a row where it lives.** Edit the row, name the commit, and change the
  status line on the finding it came from.
- **Read at `HEAD` before reporting.** `git log --oneline origin/master..HEAD`
  first — already in `LANES.md`, and the three stale documents above are what
  skipping it costs.

If a row here is older than the thing it describes, that is this file failing
in exactly the way it was written to prevent. Fix it rather than working
around it.
