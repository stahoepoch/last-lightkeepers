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
| **C7** | **Two renderer/editor decisions**: `bThrottleCPUWhenNotForeground`, and whether `ExtendDefaultLuminanceRange` flips to 1. **The exposure half no longer blocks the visual pass, 2026-11-02:** it read **0** live, legacy luminance, so the light lab's interpretation was correct and the ladder stands — `L = 0.125 × 2^EV100`, the measured optimum of EV100 5 is a lock of **4.0**, which is the value to seed the PostProcessVolume with. **It was inherited rather than authored** — the engine's help text says 1 is the UE5 default and 0 is the UE4 legacy range — so it is now pinned in `Config/DefaultEngine.ini` beside the Lumen block. An engine update could otherwise have flipped it with nothing failing. The throttle half is untouched | Values and feel are his; the files are Codex's | Pinning is not deciding: whether it *should* be 0 is still his, and a `DECISIONS.md` row is what closes this. If it ever becomes 1 the ladder must be re-derived rather than re-used, which is written next to the setting. The throttle still needs a decision and a home on disk |
| **C8** | **Audio bus structure** | `LANES.md` responsibility table | Busses exist before mixing, rather than after |
| **C9b** | **Three more moments the player cannot perceive**, found 2026-11-02 by `tests/cues.test.mjs`. Each is broadcast by the runtime and bound by nothing: **`OnDistressPulse`** - the Anchor Revive's "wakes the room" pulse, which is already a real source in the light field, so the player pays the cost and cannot perceive what they spent; **`OnExtractionFailed`** - the run ending badly with nothing but a HUD state to say so; **`OnReviveCancelled`** - a revive interrupted, with no confirmation to whoever caused or suffered it | `tests/cues.test.mjs`, which lists each in `SILENT` with its reason so the silence is a recorded decision rather than an accident | An authored cue on each, same as C2. Row 16's logic applies to all three: a mechanic whose result the player cannot perceive is not a mechanic |
| **M7** | **`tests/cues.test.mjs` — the guard for the class, not the instance.** Every `BlueprintAssignable` delegate in the runtime module must be bound in C++ or listed as deliberately silent, and the allowlist itself fails when it goes stale | Done 2026-11-02, in the suite as `test:cues`. 21 delegates, 21 broadcast, 15 bound, 6 declared silent. Proved red twice: removing a real subscriber fails by name, and giving an allowlisted delegate a subscriber fails the staleness check | **Closed.** It already earned its place: it found `OnInteractableChanged` and `OnLegibilityChanged` bound **only** inside `LLKRelicTest.cpp`, which a `.AddDynamic` grep reports as wired while a player hears nothing - the test-shaped version of the bug it exists to catch |
| **M9** | **One authored, replayable circuit through Boswell's - and the map is Claude's now.** TOYS front aisle, a lit orientation path through gondolas, toy counter with the misrouted crate, one Sweeper patrol with a visible side-step, stock-bay consequence, a signed dock, the extraction hold, the result screen. Priority is **scale and escape**: wider aisle mouths, at least one visible side-step per Sweeper leg, a real ceiling, light pools that lead somewhere, and the dock readable as a destination | Codex's assessment 2026-11-03, handed over by Chris: *"you can play a real loop now, but not yet a real department store."* Ownership moves to Claude under row 41, one owner per map; Codex keeps gameplay stable and verifies the run end-to-end after. `27b61d7` commits the map as found first, so this pass is a clean diff | **IN PROGRESS, five commits, and the editor was open for all of them.** `27b61d7` commits the map as found so the pass is a clean diff; `945b15f` fixes the world-partition defect and publishes `FINDINGS-BOSWELLS-GEOMETRY.md`; `7916fb9` cuts the 200 cm escape gap in both gondola rows; `9604934` opens the two crossings that walled the toy department off; `d399e40` dresses the circuit with 67 props and re-measures the gap against them (`FINDINGS-BOSWELLS-DRESSING-PASS.md`). **What is still open: the route plays but extraction has never been reached in a playtest, and the speed retune is a separate package (`PROPOSAL-MOVEMENT-RETUNE.md`) that this pass deliberately did not touch — so the circuit cannot yet be judged at its real traversal speed.** Two measured defects stay open: the east side aisle is closed at y 470 by `WL_DarkPocket_Partition` (60 cm against a 68 cm capsule), and `ToyBay_0` pokes 16 cm through the west wall. Done when the circuit plays as one run and Chris's feedback is *"tense"* or *"boring"* rather than *"the robot pinned me in a grey box"* |
| **X5** | **A Lumen noise control exists, and this project has not measured any noise.** `r.Lumen.ScreenProbeGather.Temporal.MaxFramesAccumulated` defaults to 10; 16 is the figure circulated for a closed, slow interior, trading a ghosting risk in motion and some VRAM for a quieter image | An 80.lv article Chris circulated 2026-11-02. `Config/*.ini` is Codex's lane | **Deliberately not applied, and that is the recommendation.** Nobody here has measured noise in Boswell's. Adopting a fix for an unobserved problem is the error this project keeps naming, and a ghosting risk in motion is a real cost to pay for it. Worth having on file for the day someone *sees* noise and can say where |
| **C11** | **The aisle is about ten times over, if the arithmetic holds.** The four `BSW_Emergency_Aisle` spots author **75 cd**; the lab's orientation floor is 2 cd and its inspection ceiling is below 0.5 cd. By inverse square that is **10x the floor** at the floor and **50x the ceiling** on a shelf label - and the 55 deg cone does not help, because candelas are per steradian. The three emergency points at 18 cd come out at 2.7x, which is a defensible authoring choice | `docs/FINDINGS-BOSWELLS-LIGHTING-AUDIT.md`, read from the live editor 2026-11-02. **Computed, not measured** - inverse square only, no cone attenuation, cosine falloff, albedo or exposure | A camera reading in the real room, which needs the editor window frontmost - the same block as check 4. Then an intensity that leaves a label unreadable without the lantern, which is the contract's central rule. **The number is Chris's**; this row only says the current one looks an order of magnitude high. **Ordering corrected 2026-11-03: this now waits on M9, not the other way round.** The 10x and 50x figures assume the 560 cm ceiling and the current open greybox; M9 adds a real ceiling and roof, changes aisle widths and re-pools the emergency lights, which moves every distance the arithmetic used. Measuring first would measure geometry about to be replaced, so the fixture intensity gets set as part of the map pass and the camera reading confirms it afterwards |
| **C12** | **A fourth `WL_` orphan, and the pattern is now the finding.** `WL_TEMP_OVERLIT_GEOMETRY_RIG` is a DirectionalLight at 50 in a sealed interior with `bAffectsWorld: false`, so it is inert and **not** a defect today | Same audit. The three before it were `WL_TempRobotBody` carving the navmesh, three `WL_` point lights doing most of the room's lighting, and `WL_DarkPocket_Partition` 20 cm from the stock bay | A sweep of `WL_`-prefixed actors across the authored maps, deciding keep or delete for each. Left in place here on purpose: the name says it is somebody's geometry rig, and deleting another writer's tool unasked is not how this works |
| **C10** | **Patrol point 1 has less clearance than the pawn has width.** Measured 2026-11-02 after the bay fix, because the bay's defect raised the same question about the route. All three points stand on open floor with clear columns to z=300 - but `Actor_11` at `(-250, 400, 100)` has **25 cm** to an obstruction at y=375 and 77.5 cm to one at x=-327.5, with +X and +Y open | `ACharacter`'s default capsule radius is **34 cm**, and the pawn is forbidden from changing it - `tests/enemies.test.mjs` bans `SetCapsuleSize`. So the Sweeper's capsule cannot centre on that point, and Recast insets the navmesh by the agent radius, which puts the point roughly 9 cm outside the walkable surface | **Probably benign, worth watching rather than fixing blind.** `ArriveToleranceStuds = 3` is 84 cm, so a goal projected onto the nearest poly still registers as arrival. If the Sweeper ever hesitates at that corner, this is the first thing to check. **Route shape is Chris's** - one of the four things the scope says the map authors and the pawn refuses to invent - so this was measured and not touched |
| **C9** | **The fixture light contract asks for something light cannot do.** Its rule - ambient fixtures must "never look bright enough to read a relic under" - has no candela solution: the inspection ceiling is below 0.5 cd and the orientation floor is ~2 cd, so any fixture broad enough to light the aisle lights a price card directly. Lower candela, barn doors and floor albedo were all measured and all failed | `FINDINGS-FIXTURE-LIGHT-BAND.md`, four levers tabulated. The contract is LOCKED, so this is his to amend | The contract names **geometry** as the mechanism rather than intensity - a shelf lip reaching far enough to block the fixture's direct ray to the readable face, measured at **~23 cm** for the lab's pose, which cut text contrast sevenfold while leaving the bay wall untouched. **CLOSED 2026-11-02 — ratified by Chris with two corrections, both of which improved it.** He kept the player-facing promise, *"no fixture may look bright enough to read by"*, as the design intent rather than letting the traces replace it: the traces are how the promise is built, not the promise. And he added the exception the draft was missing — self-luminous wayfinding may be readable, but may not carry relic, product, lore or objective information. Without it an EXIT sign is a readable face nothing shades, so a correctly built aisle would have failed its own test. His one-line rule: *"Fixtures orient; geometry keeps inspection detail dark. Navigation signage may emit; game-relevant information still requires the lantern."* |

### Codex

| | Item | Evidence | Proof of closure |
| --- | --- | --- | --- |
| ~~**X1**~~ | **CLOSED 2026-11-04, struck here 2026-11-05.** The marker requirement is gone from `run_automation.ps1` and a comment stands where it was saying not to reintroduce it. Both halves of this row's own proof were executed: `-TestFilter LLK.Enemies.SweeperBrain` exits **0** with exactly one `Result={Success}`, and `-TestFilter LLK.Enemies.NoSuchTestAtAll` still exits **1** with `No automation tests matched` and its evidence saved. Original: **`run_automation.ps1:101` fails a green run on any narrowed filter.** It required a marker only `LLKHarnessTest.cpp:36` emits, so a narrowed filter exited 1 while the log said `Result={Success}`, `1 tests performed` | `docs/HANDOFF-CODEX-LANTERN-ANCHOR.md` §2, which has the two runs in a table. It asked whoever next committed this file to strike the row and cite it, because `HANDOFFS.md` was dirty under another writer at the time — row 27, one writer per lane | **Closed.** A narrowed green filter exits 0; a filter that matches nothing still exits 1 with the reason named. The second run is the half that matters: a green exit code is only worth having if the failing case still fails |
| **X2** | **Value verification at import time.** `1ae1db9` fixed the instance; the *class* is still undetected. `VerifyTable` compares `Table->GetRowNames()` against expected row names and nothing inside the rows, which is why `DT_Enemies` could verify clean while 17 of 32 numbers were missing | `LLKImportDataCommandlet.cpp:331`, and §4 of `FINDINGS-ENEMY-OPTIONALS.md` asked for exactly this | A deliberately dropped field fails the import, not a later test. Proven red before it is called done |
| ~~**X4**~~ | **BUILT 2026-11-02, unverified in play.** The keeper now stumbles: `ALLKCharacter::ApplyStumble` is server-authoritative, replicated via `bStumbling` so an owning client is not corrected, on a timer rather than Tick because the presentation Tick early-returns for remote pawns, and composed into `ApplyMovementSpeed` as `Burden * Stumble` so all three speeds pay it. A repeat contact restarts the window instead of stacking; a downed keeper does not stumble. No impulse, because the movement component's own push may already be the shove and a scripted one is Chris's call. Game target `Result: Succeeded`, `tests/cues.test.mjs` now asserts the pawn acts on `bStumbleLanded` and was proved red by unwiring it. **Still Chris's: the two numbers** (`FallbackStumbleMultiplier` 0.7, `FallbackStumbleSeconds` 1.0, declared on `ALLKCharacter` in the house fallback pattern and reasoned against burden's 0.12 and the authored 1.5 s cooldown) **and whether there is a shove at all.** The row's own proof of closure needs PIE. Original: **The keeper-side stumble.** `8d6793f` builds trip-then-alert: the brain emits `bStumbleLanded` on every landed contact and the pawn logs it, but nothing applies it to the keeper. The seam is `ALLKCharacter::ApplyMovementSpeed`, whose own comment calls it *"the single function that assigns a speed, so there is no path to a movement value that forgot about the haul"* — a stumble multiplier composes there exactly as `Burden` does, on all three speeds for the same reason (you must not sprint out of a cost you took). The downed early-return is correct and should stay: a downed keeper does not stumble. **Rule: it costs distance and tempo, never input** — no camera lock, no input suppression, nothing to wait out; a control-removal the player cannot act through is a knockdown with a shorter timer. Reaching the keeper is `Target->...` from the pawn, which already calls `Target->GoDown` on the containment branch | `docs/PROPOSAL-SWEEPER-CONTACT.md`; seam at `LLKCharacter.cpp:506-529`. Left to Codex because that file has been uncommitted and actively edited all evening — splitting it would have been the `8196893` mistake again. **That reason expired 2026-11-02:** `bb50592` committed `LLKCharacter.cpp` and the tree is clean, so the file is no longer being edited under anyone. `ALLKCharacter::ApplyMovementSpeed` is `Source/LLK`, which is this lane, so the seam is mine to build the moment the number exists. **What it is actually blocked on is one value from Chris**, not ownership: this row already says duration and severity are his, and whether there is a shove at all is his, against burden's `0.04` / `0.12` as the scale reference. Inventing one is the thing the scope forbids | Two contacts visibly cost speed and line without ever taking the stick away, and the third still alerts the floor rather than stumbling. Duration and severity are **Chris's**, against burden's `0.04`/`0.12` as the scale reference. **Whether there is a shove at all is also his — and the movement research says one may already exist**, since the CMC ships physics push on with a large push factor, so capsule contact likely already displaces the keeper. Check that in PIE before adding an impulse |
| **X3** | **The scan cannot run in CI.** `Content/Data/` is gitignored, so on a fresh checkout `tests/enemies.test.mjs` reports that it compared nothing | Stated limit, same §4 | Either CI generates the assets before the scan, or X2 makes the scan redundant. X2 is the better fix and subsumes this |

Codex also owns, from the responsibility table and not yet started: the
performance budget, crash reporting, and build/packaging for a tester who
cannot launch the editor.

### Grok

Lane ratified 2026-11-05, `DECISIONS.md` row 83. Rows here are how a story beat
asks for a rule: name what the fiction needs to be true, and the owner of the
mechanism decides how it is built, or that it is not. **Not a number, not a
`data/` edit, not a C++ change.**

| | Item | Evidence | Proof of closure |
| --- | --- | --- | --- |
| **G1** | **The spend side of the loop, and it is the lane's first assignment.** `PROGRESSION-GAP.md` has been open since 2026-10-26: a finished run pays out and there is nothing to spend it on, while materials have accumulated in every profile since banking landed | The file itself, unanswered for ten days. It is the gap between a loop and a game, it is pure design, and it needs no engine — which is why it is the right first question for a lane that cannot press Play | A written answer to what the materials buy, in `PROGRESSION-GAP.md`. If the answer needs a rule it arrives as a row here; if it needs a number the number is Chris's. **A season, a streak, a battle pass or a fifth counter is the pattern `WORLD-SYSTEMS-BOARD.md` already refuses** — that refusal is the one piece of the board a new owner should read before writing |
| **G2** | **Nothing in the six files is ratified, and three of them have been quoted as though they were.** Row 83 handed over a writer, not an authority. `QUESTS.md`, `CAMPAIGN-SHAPE.md`, `PROGRESSION-GAP.md`, `REWARDS.md`, `SPINE.md` and `THE-WORK.md` are 1,149 lines of proposed design between them | The lane table before 2026-11-05 made these Claude's files, so some of their content is Claude's drafting and some is Grok's musing, and the files do not say which is which | A status line on each, in the shape `M3` put on every `FINDINGS-*.md`: what is proposed, what a `DECISIONS` row has ratified, and what is somebody's note. Until then no row in any of them may be cited as a decision |
| ~~**G3**~~ | **CLOSED 2026-11-05 by Chris: *"Move the section"*.** The board's hard-run section is now `docs/THE-HARD-RUN.md` and it is Grok's — moved byte-identical, which the commit proves, and `LANES.md` and `FLEET.md` both carry it. Original: **`WORLD-SYSTEMS-BOARD.md` is a Claude file holding Grok's research, which is two writers in one file** | It was live rather than hypothetical: on 2026-11-05 Claude rewrote every speed figure in that section against `data/` at `HEAD` under the board's own falsifier 4 (`35d73c2`). **And a rule nobody had cited made it sharper** — `FLEET.md` already said the board is *"append-only, anyone; never rewrite another's entry"*, so that commit was correct under the lane table and a rewrite of another's entry under the board rule, by substitution rather than a dated banner | **Closed, and the cost is now the standing rule.** The numbers in `THE-HARD-RUN.md` are Grok's to maintain. A finding against them is a row here under Grok — symptom, evidence, file and line — and the owner decides; Claude does not edit it to fix it, exactly as with `Tools/**`. Falsifier 4 still stands inside that file, so a stale number there still loses to `data/`; what changed is that fixing one is a handoff. **The cheaper option lost on a real argument rather than a preference:** leaving the research in a Claude file kept two live rules pointing opposite ways, and `35d73c2` had already done the thing they disagreed about |

### Claude

| | Item | Evidence | Proof of closure |
| --- | --- | --- | --- |
| **M1** | ~~**Verify emissive now lights something.**~~ **HALF CLOSED 2026-11-02.** `r.DynamicGlobalIlluminationMethod` read **1** live through the MCP bridge, so Lumen is *not* "enabled in config and inert" — the renderer half of `FINDINGS-EMISSIVE-LIGHTS-NOTHING.md` is verified, not just answered. **And it redirects the open problem:** Chris reports the lantern effect still "isn't there", which is now provably *not* a global-illumination fault. Remaining suspects are the lantern's own emissive material and the temporary rig | `FINDINGS-EMISSIVE-LIGHTS-NOTHING.md`; gate read in `3b209ae`. **Plus the anchor move, added 2026-11-02 on Chris's instruction:** `LanternAnchor` went `(38, 18, -4)` → `(42, 25, -19)`, **17.0 cm mostly downward**, and `LanternLight` attaches to it while the gameplay light field samples it, so the gameplay origin moved with the pose | The surface measurement is no longer the blocker and does **not** need `LLK_LightLab` — judge it in Boswell's during the visual pass, which is where the lantern actually has to look right. **Two things to read in that same sitting**, because the pose and the look are one pass: does the lantern light its own glass, and does a relic still read at the authored distance from the new anchor height. 17 cm is 2% of the 840 cm reach and nothing at range, but a **quarter of the 56 cm** at which `FINDINGS-FIXTURE-LIGHT-BAND.md` measures inspection — and the rule it brushes is the one saying cosmetic hand work must not move relic inspection. The radius is untouched: the field reads `GetGameplayRadiusCm()`, not `AttenuationRadius` |
| **M2** | ~~The fixture light contract still has no band~~ **- withdrawn 2026-11-02, as the wrong row.** It asked for "a numeric band in the contract, and a test that fails when a fixture crosses it", and all three clauses are wrong: no band exists, the contract is **LOCKED by Chris**, and fixture intensities live in the map rather than `data/`, so no headless test can read one | `FINDINGS-FIXTURE-LIGHT-BAND.md` measures the gap at ~4x under Lumen - inspection ceiling below 0.5 cd, orientation floor ~2 cd - and records three failed levers | **Closed by being correctly re-stated.** The decision is Chris's, as **C9**. What was mine was reading the contract before writing a proof of closure against it |
| **M5** | **Ladder 3's lantern reading was taken on the wrong cone.** "25 cd reads as a carried lantern" came from a 72 deg outer cone; `data/lantern.json` authors **110 deg**. `a060e78` adopted the 25 anyway | `FINDINGS-FIXTURE-LIGHT-BAND.md` conditions section against `data/lantern.json:44` | Ladder 3's three rungs re-run at 110 deg, or the authored cone changed to match what was measured. Needs the editor and the bridge, **and the editor window frontmost** - check 4, attempted 2026-11-02 and blocked at step zero. **One prediction to test when it runs, recorded now so it cannot be fitted afterwards:** in Unreal a spot light's intensity in *candelas* is luminous intensity per steradian, so widening the cone should **not** dim what is directly in front of it - it should widen the lit region and leave the centre pool alone. If that holds, the 25 cd reading transfers to 110 deg and this row closes as a non-issue; if it does not, the authored lantern is dimmer than the one that was measured. **M5 was opened assuming the second. The first is more likely, and assuming it was the error** |
| ~~**M6**~~ | **CLOSED 2026-11-02 by measurement in the live editor, and it was worth running.** The tag is attached and the bay has been moved from `(450, 450, 50)` to `(450, 390, 50)` to stop a filed crate clipping a wall. Detail in "What check 1 found". Original: **`BSW_STOCK_BAY`'s tag is unverified since the map was saved by another writer.** The greybox store room was written into `LLK_Boswells_FirstRun.umap` while that map was assigned to this lane. Nothing was lost — `BSW_STOCK_BAY`, `LLK.StockBay`, `LLK.Patrol`, `BSW_WRONG_CRATE`, `WL_PlayerStart`, `BSW_LOADING_DOCK_EXIT`, the Sweeper and fourteen relic references are all in the new 146,495-byte package | Byte search of the package, 2026-11-02. The name hits alone could not prove placement, so the transform was scanned too: **`(450, 450, 50)` is present once as a double-precision triple**, with the crate's authored `(300, 250, 90)` as a green control proving the scan can see transforms at all. The first attempt searched float32 and found nothing *including the control* - the wrong precision, caught by the control rather than reported as a loss. So the bay is still exactly where it was placed. What a byte scan cannot prove is that the **tag array** is still attached | `find_actors` by tag returns `BSW_STOCK_BAY` - **check 1 of "The editor sitting" below** - plus the harder question no scan can touch: **is the bay now inside the new gondola geometry?** |
| **M3** | **Status lines on every `FINDINGS-*.md`** | This file's whole argument | Done 2026-11-02, in the same commit as this file |
| **M4** | **The orientation half of the exposure sweep (Bay B)** | Longstanding, not acted on | The sweep covers orientation as well as intensity |
| ~~**M10**~~ | **CLOSED 2026-11-03 by `947a84b`. The engines are separated, and the green signal was pointing at the wrong world.** `npm test` ran 392 checks against Luau and a Rojo `DataModel`, and the pre-commit hook gated every commit in an Unreal project on two Roblox Lighting properties — it blocked an unrelated commit that day. 45 files removed, nothing lost: everything is addressable at the annotated tag `roblox-prototype-final`. The hook kept its job and changed its target, now running `tests/light-authority.test.mjs`. `DONTFUCKUPTHELIGHTS.md` was deliberately kept, marked retired, because `LIGHTING-IN-UNREAL.md` cites it as its ancestor and breaking that lineage is the exact failure this file exists to prevent | Chris, 2026-11-03: *"roblox is over with, so retire what we need to."* Codex's correction first: separate the engines, do **not** delete the prototype | `docs/RETIRED-THE-ROBLOX-PROTOTYPE.md`. **Left undone on purpose, and it is a real row for someone:** about twenty files under `docs/` still mention Roblox, Luau, Rojo or Studio. Most are historical and correct — `PLATFORM.md` and `DECISIONS.md` record the move itself — so a bulk search-and-replace would damage the history it is meant to clarify. Finding the ones that still speak in the present tense is a separate, careful job |
| ~~**M11**~~ | **CLOSED 2026-11-03 by `915329c`. `.git` was 19 GB and it was not the art.** `.git/lfs` held 6,622 objects; the working tree referenced 220, the largest 742 KB. The four biggest on disk were 3.2, 2.6, 1.2 and 1.2 GB and **no commit on any branch referenced them** — orphaned local cache, not history. `git lfs prune` took it to 44 MB, `.git` to 130 MB. Worth re-running: editor saves of `.umap` and `__ExternalActors__` churn LFS objects fast | Chris, 2026-11-03: *"import and make sure we don't have too much space."* The space problem turned out not to be the pack | `docs/ASSET-DEPENDENCIES.md`. The import rule it establishes: **meshes are nearly free and textures are the entire cost** — 101 meshes cost 21 MB, 26 textures cost 302 MB. So take meshes, materials and blueprints freely; take texture sets one at a time and only when the map uses them |
| **M12** | **A gitignored dependency goes missing and the thing depending on it reports success anyway — row X3 one layer out.** `/Content/Warehouse_Big_Pack/` is gitignored licensed art, and **Unreal does not error on a missing mesh**: it renders nothing and carries on, so a clone without the pack opens Boswell's with invisible racking and no dock and nothing says why. `tests/asset-deps.test.mjs` draws the distinction that matters — **wholly absent exits 0** and names the Fab listing, because a fresh clone is not broken; **partially present fails**, because that is the half-finished copy that costs an evening. Proved in all three states before it shipped | `915329c`, and the same trap this file already records against `Content/Data/` | **The dev case is closed. The cook case is open, and Chris called it correctly as a shipping concern rather than a blocker.** The dev check answers *"is the pack on this machine?"*; a release needs the stricter claim that **every asset referenced by a shipping map resolves at cook time**. Those are different assertions. That gate runs against the cooked output or the cook log, fails on any unresolved reference naming the actor and the asset, and treats "resolved to the placeholder mesh" as a failure too, since that is how a missing reference presents once baked |

---

## The editor sitting — everything that needs a viewport, in one pass

Written 2026-11-02 on Chris's instruction, because **M1, M5, M6 and the C2
judgement are four rows blocked on one thing**, and a list is cheaper than four
rows each saying "needs the editor". Nobody can do any of it from a terminal:
these need a running editor, the MCP bridge, and in two places a human looking
at a screen.

**Step 0, and it is the step that gets forgotten.** The bridge does not start
with the editor. In the editor console:

```
ModelContextProtocol.StartServer
```

Until that runs, `127.0.0.1:8000/mcp` refuses every connection and the four
checks below cannot be driven at all.

**It can be retired permanently, and it should be.** Epic publishes the MCP
workflow as a Claude Code plugin -
`EpicGames/unreal-engine-skills-for-claude-code-plugin` - and its setup
reference gives the setting this project has been doing by hand since
2026-10-31. Four lines in
`Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini`, which is
**per-user and gitignored**, so it is nobody's lane and costs no one else
anything:

```ini
[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]
bAutoStartServer=True
```

Checked 2026-11-02: that section is **absent**, which is why every session has
started with a manual `StartServer`. The two plugins the server needs,
`ModelContextProtocol` and `AllToolsets`, are already enabled in the
`.uproject`, so this one setting is the whole of what is missing. It takes
effect on the next editor launch.

**What the plugin does not cover:** photometry. Its skill says nothing about
`CaptureViewport`, `CaptureEditorImage` or measuring light, so
`docs/LIGHTLAB-PROTOCOL.md` section 9 remains this project's authority there -
including that the offscreen path has no global illumination and that a
stale frame comes back byte-identical rather than failing. Checked rather than
assumed, because an official skill that contradicted a measured finding would
be the worse outcome and was worth ruling out.

**Two traps that would make this list return wrong answers**, both already in
`docs/TOOLING-TRAPS.md`:

- **Trap 6 — `find_actors` answers about the level that is open.** If
  `LLK_DockBay` is the open level, a sweep for `BSW_STOCK_BAY` returns empty
  and that empty is about the wrong world, not about the tag. **Open
  `/Game/LLK_Boswells_FirstRun` first and confirm it.**
- **Trap 13 — during PIE the scene tools answer about the `UEDPIE_0_`
  duplicate.** Read freely while PIE runs; **write nothing**, because the edit
  lands in a world that is discarded on stop and the read-back confirms it by
  reading the same doomed world.

### What check 1 found, and the correction it forced

Run 2026-11-02 with the bridge up. `/Game/LLK_Boswells_FirstRun` confirmed as
the open level first, because trap 6 makes an answer from the wrong world look
exactly like an answer. No `UEDPIE` prefix on any returned `refPath`, so PIE was
not running and writing was safe — trap 13's own test, for free.

**The tag is attached.** `find_actors` by tag returns exactly one actor:
`BSW_STOCK_BAY`, tags `["LLK.StockBay"]`, transform `(450, 450, 50)` live,
matching the byte scan exactly.

**The floor was not clear, and the obstruction is not new.** Traces from the bay
origin at z=50:

| direction | first hit |
| --- | --- |
| down from z=300 | **300** — the floor at z=0, nothing in the column |
| −X, −Y | no hit within 600 cm |
| +X | **110** — the shelving run at x=560, already known from the original placement |
| **+Y** | **20** — a face at y=470 |

That face is **`WL_DarkPocket_Partition`**, bounds `(0, 372, 0)` to
`(500, 628, 280)`, and a reverse trace from y=700 hits at y=530 — so it is a
60 cm thick wall spanning y 470–530. The `WL_` prefix is walk-lab naming, so it
**predates the greybox**: this was mis-measured at placement time, not broken by
another writer. The original note claimed "bare floor with clear headroom" and
checked x but never traced +Y.

**Why it mattered.** `BSW_WRONG_CRATE` is `100 × 100 × 100`. Teleported to
`(450, 450, 50)` it spans y 400–500 against a wall face at y=470 — **30 cm of
interpenetration.**

**Fixed, in this lane, on this lane's own actor.** The bay is now at
`(450, 390, 50)` — 60 cm along −Y, the direction with over 600 cm of clear
floor. A crate there spans y 340–440 and clears the wall by **30 cm**; +X
clearance is unchanged at 110 cm to x=560, so the crate's 400–500 span clears
that by 60 cm; the column is clear from the floor to z=300. Verified after the
move: transform `(450, 390, 50)`, label `BSW_STOCK_BAY`, tag `["LLK.StockBay"]`,
and on disk the new triple appears once while the old one is gone.

**An overclaim of mine, corrected.** Before measuring, this was described as a
crate that would land "inside a shelf", be "unrecoverable", and "read as the
crate vanishing". The real defect was 30 cm of clipping into a partition, with
the crate still reachable from the −Y side, which has clear floor. Worth fixing
and not worth the alarm — and the alarm was written before the trace, which is
the wrong order.

### The four checks, cheapest first

| | Check | How | Closes |
| --- | --- | --- | --- |
| ~~**1**~~ | **DONE 2026-11-02 — tag intact, and the bay has been moved 60 cm.** `find_actors` by tag returns exactly one actor, `BSW_STOCK_BAY`, tags `["LLK.StockBay"]`. The floor check found a real defect and it is fixed: see "What check 1 found" below. Original question kept for the record — **is `LLK.StockBay` still attached, and is the bay still standing on open floor?** The transform is confirmed unchanged at (450, 450, 50), so this is the tag array - and the geometry now around it | `find_actors` by tag, then look at where it stands. **(450, 450) was chosen against an empty floor**, and the greybox added two gondola runs plus endcaps. The Sweeper does not walk a filed relic to the bay, it **teleports** it to that origin - so if the bay now sits inside a shelf, a filed crate lands inside the shelf and the run's objective is unrecoverable. In play it reads as the crate vanishing | **M6** |
| **2** | **Do the Sweeper's four cues read?** Lowest third of the screen, in PIE, with the debug HUD from `8738b57`: *"something has noticed you"* on telegraph, *"a machine is dealing with something of yours"* on start, *"it saw you, and left your crate where it was"* on interrupt, *"your crate has been put somewhere it belongs"* on filed | Play it. This is a human judgement, not a measurement — the question is whether a player could tell that lighting up is what saved the crate | the **C2** judgement. Not C2 itself: that needs the authored sound, and the readout exists to be deleted |
| **3** | **Does the lantern light its own glass, and does a relic still read from the new anchor height?** Lumen is verified live, so what remains is the lantern's own emissive and the rig. The anchor moved **17.0 cm** mostly downward and the gameplay light moved with it | Judge it in Boswell's during the visual pass, not in `LLK_LightLab` — this is where the lantern has to look right. Read the relic at the authored distance, not at a lab distance | the open half of **M1** |
| ~~**4**~~ | **ATTEMPTED 2026-11-02, blocked at step zero - and the block is the right outcome.** The positive control failed: 0 cd and 5000 cd produced identical sampling boxes, the frame showed Bay A while every camera tool reported Bay C, and after `FocusOnActors` the capture came back **byte-identical** to the previous one. The editor window was not redrawing, because it was occluded. **No rung was recorded**, which is what step zero is for. Needs the editor window frontmost and unoccluded for the duration; the lab was restored to intensity 0, cone untouched at 72/17.28. Diagnostic written into `LIGHTLAB-PROTOCOL.md` section 9 | Original: **Re-run Ladder 3 at the authored cone.** "25 cd reads as a carried lantern" was measured at **72°** outer; `data/lantern.json` authors **110°**, so the same candela is spread wider and the shipping fixture is not the one that was tested | Three rungs — 6, 25, 100 cd — at 110°, per `LIGHTLAB-PROTOCOL.md`. Or change the authored cone to match what was measured, which is Chris's call either way | **M5** |

Checks 1 and 4 are measurements and can be driven through the bridge. Checks 2
and 3 need eyes on a screen and are Chris's, since he is the only one who can
press Play.

---

## Grok's lane — RATIFIED 2026-11-05, `DECISIONS.md` row 83

Chris, 2026-11-02: *"grok is basically musing with quests, and story and
progression."*

**There is no lane for that, and the collision is real.** `docs/**` is Claude's
in the `LANES.md` table, and quests, story and progression live there —
`QUESTS.md`, `CAMPAIGN-SHAPE.md`, `PROGRESSION-GAP.md`, `REWARDS.md`,
`SPINE.md`, `THE-WORK.md`. Grok writing into those today is two writers in one
file, which is the rule the whole table exists to enforce. Grok appears nowhere
in `LANES.md`, `FLEET.md` or the README; the only trace of it in the repo is as
the author of two files under `research_notes/`.

**Chris ratified this on 2026-11-05 — *"Ratify the lane"* — and `DECISIONS.md`
row 83 is LOCKED.** `LANES.md` now carries Grok in the table, in the
responsibility list and in the handoff rules. What was proposed, and is now the
rule:

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
file is the replacement: one ledger, owners in columns. Grok's handoffs are rows
here rather than a seventh file — see **Grok** under "Live items, by owner".

**What the ratification deliberately did not do.** It granted a writer, not an
authority. Nothing already written in those six files became a decision by being
handed over, and the hard-run section of `WORLD-SYSTEMS-BOARD.md` is still
proposed. Row 83 says so in as many words, for the same reason `docs/**` being
Claude's has never made Claude the owner of a balance value.

**The one seam it left open was closed the same day.** `WORLD-SYSTEMS-BOARD.md`
was a Claude file containing Grok's research — two writers in one file, the exact
thing this table forbids, and live rather than hypothetical, since `35d73c2` had
rewritten every speed figure in that section. Ratification declined to pick a fix
because a lane change is Chris's. He picked the same day: *"Move the section"*.
The hard-run section is now `docs/THE-HARD-RUN.md`, Grok's, moved byte-identical,
and **G3** records what that costs — corrections to it are handoff rows, not
edits.

### Check Grok's board before treating it as the build — 2026-11-02

Chris asked for the research to be committed and for the other sessions to be
able to check it. The writing was on `docs/WORLD-SYSTEMS-BOARD.md`, inside **The
hard run**; since 2026-11-05 it is **`docs/THE-HARD-RUN.md`**, Grok's own file,
and the "Where" column below names sections inside it rather than on the board.
It is proposed. It is not a `DECISIONS` row, and it does not ratify the lane
above — the lane was ratified separately, by row 83.

| Block | Where | What a check is for |
| --- | --- | --- |
| Body, engine defaults, four fixture heights, the eye, the verb list | "Being there is a body", "The head, the hands, and a real counter", "The verb list" | Try to break a number against UE 5.8 source or against `data/` at `HEAD`. A paper's walking speed is not a candidate value. |
| Five feelings | "Five feelings, each turned over" | Scary and fun are one inverted-U (Andersen et al. 2020). Addictive is the thesis's "one more night," and a season, a streak, a pass, or a fifth counter is the pattern being refused. The emotional gamble is a price the crew can name. A designed near-miss is Clark's slot machine (Neuron, 2009) and is refused. |
| The build | "The build", same hard-run section | Two active cards, both Chris's: seven measured lines from one sitting, and one sound on `OnStockInterrupted`. The vault opens only if the landed counter is above the 56 cm hop. Later rows are the full scale and are inactive. |
| The instrument | "The instrument, in parts" | One lamp. Hooded / Working / Open are flashlight, lantern, beacon. The stick and setting the lamp down are the flare. Left click commits the light. Right click shutters it, and only after a machine can hear. Not the aisle's next card, and not two extra actions stuffed into the seven. |
| Grok's slice | "The slice that can be coded" | Pure `Step` for the clicks, test `LLK.Lantern.Clicks`, no actor. Blocked while the Sweeper brain files are dirty in `Source/LLK`. Not a second writer in that module. |
| The Galleria shell | `THE-MALL-IS-STILL-TRADING.md` §11, with §8–§10 behind it, the greybox in `docs/reference/galleria/`, and "The Galleria shell" on the board | The cook's order. The first level is the whole roof, blocked in, as a new level `LLK_Galleria_Shell`. The furnished night is still one anchor (row 28), still `LLK_Boswells_FirstRun`. Import the OBJ at scale 1 and read `REF_100FT` (3,048 cm) before trusting a wall. TLP-1's 100 ft bar wins a disagreement. A foot from the directory model's units fails. A York square footage used as a Johnstown wall fails. The blend name Casino is not a tenant. `FreightLift` is not an exit of this building. Real tenant names do not enter the level. Seats are below. |

Checked against `HEAD` after `bb50592` and `bd7c300`, and two of the old
misreads are retracted in the board rather than left as warnings:

1. The Andersen caveat — that the inverted U disappeared when people rated the
   *whole* attraction — was taken from the article page, not from a PDF. If the
   paper does not say it, retract that sentence and keep the repeated-measures
   result. Still open.
2. **Retracted.** `LanternAnchor` at `(42, 25, -19)` is `HEAD` as of `bb50592`.
   The fallback that adds 152 cm to the capsule center is still a comment bug
   in Claude's lane, and it is a ride-along, not a package.
3. **Retracted as a warning, adopted as the night.** `knockdownOnContact` false
   is `HEAD`. Sweeper contact is the stumble (× 0.55 for 1.2 s, no input lock).
   The third contact raises a decaying floor alert. Rows 11 and 59 still
   describe a knockdown when a later machine causes one. The Sweeper is not
   that machine.
4. The Sweeper at 2.8 m/s is retracted. `HEAD` is 14 studs/s, 392 cm/s. No new
   authored number. `data/` at `HEAD` wins any disagreement.

The active card for the other sessions to unblock is the seven lines. Until
they exist, a vault, a climb, and a footstep sensor are early.

### The Galleria shell — seats, 2026-11-04

Chris asked for the mall research turned into a plan the other two can cook
from. The plan is section 11 of `docs/THE-MALL-IS-STILL-TRADING.md`. The
prepared mesh and the marker list are `docs/reference/galleria/`. Proposed.
Not a decision. Not a release to open the editor on a new level. Content
stays Chris's until he says the level is open.

**Claude.** No `Source/LLK` change for this shell. The zone actor, the id,
and the hold already exist. Do not add an extraction id, do not place
`FreightLift` (it wants a sub-basement this building's plans do not have),
and do not put a Sweeper in the shell until `docs/MAP-READINESS.md` passes
on that level. `LLK_Boswells_FirstRun` stays the night. A check is a length:
anything sized from the leasing glTF's raw units, from `StudsToCm`, or from
York's floor area, fails. The Casino blend-name used as a tenant, or as the
way to pick the closed end anchor, fails.

**Codex.** No tool and no ini for this until Chris opens the level. When he
does, the editor job is the import and the scale check, not a new system.
The OBJ is centimeters on Unreal's axes. Scale 1. `REF_100FT` must be
3,048 cm. **File > Import Into Level** takes glTF, GLB, and FBX, so this
file comes in as a mesh. Do not point that command at the Verge3D scene.
Emergency lights are ordinary fixtures on the marker list. Exit signs may
emit. They stay out of `LLKLightField` (`docs/FIXTURE-LIGHT-CONTRACT.md`).
No candela is authored in the plan. Do not dim the whole concourse to get
there. The old mall went milky that way.

**Chris.** Name which real end is Boswell's. Until that, the extract
candidate stays on the Boscov's outer face and the other two dock faces are
marked so the fix is a move. The new level is `LLK_Galleria_Shell`. The
fourth anchor is traced from TLP-1, because the greybox does not contain
it. The fifth pad stays an unmeasured tag on the grass. One
`ALLKExtractionZone`, `ExtractionPointId` = `LoadingDock`, on the service
face he named, sized to the bay mouth the way `docs/MAP-READINESS.md`
already says. Grilles down on the other three. Row 28 is the furnished
night, and it is still the store that already exists.

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
