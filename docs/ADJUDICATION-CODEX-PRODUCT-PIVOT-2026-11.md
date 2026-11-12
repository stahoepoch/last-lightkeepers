# Adjudication — Codex's product pivot audit

Status: **review, 2026-11-11.** Answers the job in
`HANDOFF-CLAUDE-PRODUCT-PIVOT-AUDIT.md` §The job. Nothing here is a
`DECISIONS.md` row, no balance value moved, and Grok's seven narrative files
were not opened. Read alongside `AUDIT-THE-PIVOT-2026-11.md`, which measured
the same ground two hours earlier and reached the same diagnosis by a
different route.

**The verdict.** The diagnosis is right and I will not re-argue it. The work
order is wrong in one place, and that place matters more than everything else
in this file. One of its seven conflict rows was closed by Chris before the
document was read, and the locked row that gates the entire project is absent
from the table.

## 1. What is right, and is now settled between us

The promise is split; the repository's pitch text and its runtime both still
describe the raid. Supersede by dated notice rather than deletion. Retire by
named row, never by removing a file. `npm test` green is not a compiled build
and not a played map. The 396 m figure is a chord and must not become
collision geometry. **And the physiology correction is the best thing in the
document** — see §6, where I accept it against my own row.

Two of us independently reached *extraction is work to migrate, not a reason
to preserve the old product.* `AUDIT-THE-PIVOT-2026-11.md` §1.2 puts a number
on it: **3,169 lines, 10.8% of the module**, and the phase enum already
carries the mechanism a camp needs.

## 2. Where the audit is wrong

### 2.1 The co-op question was closed before this document was read

The handoff's closing line names the protagonist/co-op presentation as *"the
next product decision"*, its table carries a **Co-op** row, work split item 4
asks Chris to rule it, and the acceptance test ends *"row 50 also requires a
co-op proof once the protagonist presentation is chosen."*

**Chris, 2026-11-11: *"no co op."*** Row **89** was rewritten the same day
(`83cb12e`) and the consequences are costed there. What this deletes from the
handoff: one table row, half of work-split item 4, and that acceptance clause.
What it adds: **four LOCKED rows now need his strike — 36, 50, 51 and 52** —
which the handoff's own "Must not touch" list correctly says cannot move
without his ruling. The ruling has happened; the strike has not.

It also kills a roadmap card the handoff does not mention: **M1.9, "Two-player
test over Steam or LAN."**

### 2.2 Row 50 is cited three times where row 36 is meant

Row **36** is the row that permits co-op at all (*"solo, or private co-op for
1–4 friends"*). Row **50** says only that co-op is **never gated behind
progression**. The handoff's table says *"Row 50 locks immediate 1–4 player
co-op"*, and its acceptance and work-split lines inherit that. Its "Already
ruled out" line — *"locking co-op behind rescued keepers (row 50)"* — is the
correct use, which is how I know this is a slip and not a reading. In a
document whose purpose is row hygiene it is worth one line.

### 2.3 The "20-second dock hold" is not a test

Of the three checks offered as evidence that the suite guards the old
contract, one is a test, one is current data, and one is a comment.

| Claimed green check | What it actually is |
| --- | --- |
| *banked loot is lost without extraction* | **A real assertion.** `tests/balance.test.mjs:254`, and `:255` for carried loot |
| *the dock requires a 20-second hold* | **Not a test.** A `note` string in `data/loot.json:68`. The suite asserts the *rule* with no number: `tests/extraction.test.mjs:272`, *"the only way to secure is to reach the end of the hold"* |
| *the Warden arrives at 700s* | **A real assertion, and it is current rather than legacy.** Row 73 (Chris, 2026-10-29) sets the Warden at 700 and the band at 660–720. `data/zone-pressure.json` matches. It passes because he tuned it twelve days ago |

This matters because it is the difference between a suite that encodes raid
*timings* and one that encodes extraction's *shape*. It is mostly the second,
which is why the migration is a repoint. **151/151 in `balance.test.mjs`,
2026-11-11, nothing touched.**

### 2.4 The raid numbers that actually die are two, and they can be named

Not "minute targets" in general. `data/zone-pressure.json`'s `run` block:

- **`earlyExitSeconds` 360** — exists only so an early-leaving crew can be
  said to miss the Warden.
- **`hardCeilingSeconds` 780** — its own note derives it as *"720 (top of
  band) + 30 (the longest extraction commitment, the freight lift's
  arrivalSeconds) + 8 (`extractionCall.sirenSeconds`)"*. Every term in that
  sum is extraction.

Both are Chris's values under row 55 and neither is mine to change. The
Warden's own numbers survive the pivot intact, because a chapter still has
nights and a night still escalates.

### 2.5 The work order is wrong, and this is the real disagreement

The handoff says: *"Do not spend a pass on a prettier lantern or a larger
store while those failures remain in the playable path."* Right about the
store, right about the three route failures, and **wrong about the lantern** —
because *bulky* and *redundant* are two different complaints with two
different causes, and only one of them is polish.

**Bulky** is the cone, it is already diagnosed, and it can wait.
`MAKE-IT-FEEL-LIKE-A-LANTERN.md` says so in its own heading: *"The problem is
the cone, not the intensity."*

**Redundant is not polish. It is the game's central verb failing, and the
cause is in the build rather than in the fiction:**

1. Row **10** LOCKED: charge is unlimited — *"the cost of light is the meter,
   not a fuel gauge."*
2. Rows **72** and **73** LOCKED: that meter is Zone Pressure, zone-authored,
   and light creates only local consequences. So the meter is the **only**
   price light has.
3. The entire UI module is four files: `LLKDebugHUD.h/.cpp` and
   `LLKExtractionReadout.h/.cpp`. Zone Pressure appears in exactly one of
   them — **the debug HUD.**

**So light's only cost is invisible unless a debug panel is open.** A player
who cannot see what light costs is holding a tool with an upside and no
downside, which is the definition of redundant. That is not a feel problem and
not a story problem; it is a missing surface.

**It sits upstream of this handoff's own acceptance test**, which asks a fresh
player to state, *without a debug HUD*, **what their light cost them.** No
build that exists today can pass that clause, for the same reason Chris's
lantern felt redundant.

**And the surface has been specified for eleven days.**
`HUD-STATUS-SCREEN.md`, proposed 2026-10-31, written out of *"Chris's playtest
reaction to the always-on debug panel"* — the Fieldboard. `ROADMAP.md` M1.7
says it in its own words: *"Not the real HUD — the real one is M3."* M3 never
came.

**Falsifier, and it is cheap.** Play the Boswell's segment once with the debug
HUD on. If the lantern stops feeling redundant, this is right and the fix is a
UI job in my lane. If it still feels redundant with the meter visible, then
row 72's local-consequence model is the problem, which is a far worse finding
and one worth having early.

**Recommended order, replacing work-split item 1's implied order:** the three
route failures and the cost surface are the same pass, because the acceptance
test needs both. The story slice is built on top of them, not beside them.

### 2.6 Row 3 is absent from the conflict table, and it gates everything

Row **3** is LOCKED: *"Do not expand scope before the v1 loop passes its
playtest question"*, and the question is *"do testers kill their lanterns,
**bank**, and argue for one more room?"*

**One third of that question dies with extraction.** Row **79** LOCKED and
`ROADMAP.md` **M4** both restate it verbatim, so the same verb is embedded in
the instrumentation rule and in the gate. The handoff lists row 3 among rows
inspected and then does not carry it into the table.

This is the cheapest unblock in either document. The gate the whole project is
held behind **cannot be answered as written**, and it has been the stated
reason not to expand scope through four re-scopes in 22 days. Chris restating
it is one sentence. My proposal, for his ruling and not as a row:

> *Do testers kill their lanterns, make camp, and argue for one more shop?*

Same three beats, same instrumentation, and `LLKExtraction.h`'s phases already
emit the middle one.

### 2.7 M3's definition was already false before the pivot

`ROADMAP.md` M3: *"the extraction call spikes the meter."* Row **73** removed
that spike on **2026-10-29** — *"Calling extraction already has the right
price: siren, visible activity."* So the milestone that was to deliver the
real HUD has been stale for thirteen days, in a clause the pivot has nothing
to do with. This is the same failure mode `DRIFT.md` section A is: a one-line
fix nobody was blocked on.

## 3. The supersession text, as asked

**Drafts.** Rows 84–91 are `PROPOSED` and none is ratified, so installing
these would be the exact drift both documents complain about. Each is written
to be pasted once Chris rules.

**`PITCH.md`, above "The one line":**

> **Superseded 2026-11-__, pending rows 84–91.** The line below describes the
> salvage raid. It is kept because its three improvements over the line before
> it still hold, and because the lantern clause survives the pivot unchanged.
> What changed is the sentence it sits in: there is no haul and no town to save
> yet, there is a boy, a crossing, and a family.

**`GAME-THESIS.md`, replacing the claim sentence only:**

> *Last Lightkeepers* is a first-person story about a young Lightkeeper
> crossing places the storm has made dangerous, to find his family and a way
> home.

The paragraph beneath it survives almost whole: *"a pump runs, a route opens,
a record survives"* is the frontier, and **"We got something home"** needs one
word cut to become **"We got home."** The *"not a loot-shooter"* paragraph
stands verbatim.

**`ROADMAP.md` M3 and M4:**

> **M3 — the night closes.** A night can be won or lost. Loot visible only by
> light, burden slows you, **the cost of light is legible without a debug
> panel**, and making camp is the only thing that moves the frontier.
> (Replaces *"the extraction call spikes the meter"*, retired by row 73 on
> 2026-10-29, and *"extraction secures the haul and nothing else does"*.)
>
> **M4 — ⟵ THE GATE: the playtest.** Row 3's restated question answered with
> data: do testers kill their lanterns, make camp, and argue for one more
> shop? Four real testers, instrumented under row 79.

**`ROADMAP.md` M1.9** — struck, with the reason in place: *no co-op, row 89.*

## 4. The locked rows that need Chris's ruling, and nobody else's

| Row | What it says now | What the pivot does to it |
| --- | --- | --- |
| **3** | The playtest gate, with *"bank"* in the question | **Restate.** §2.6. Cheapest unblock in either document |
| **36** | Solo or private co-op, 1–4 | **Strike.** Row 89 |
| **50** | Co-op never gated behind progression | **Strike.** Moot with no co-op |
| **51** | Mixed crews, cap four, humans first | **Strike.** Row 89 |
| **52** | NPC keepers never carry a lantern | **Strike**, but keep the *rule* — it is why the roster can never perturb the meter |
| **28** | Version one is one sealed store | **Amend**, one clause. Rows 34 and 38 already permit the crossing (`AUDIT` §1.6) |
| **43** | The department store is the tutorial | **Amend.** The rule survives; its curriculum lists extraction |
| **9** | Anchor Revive is solo-only | Becomes unconditional. `LLKVitalsComponent.h:173` and `:232` go dead |
| **79** | Instrumented playtest, no rule reads the measurement | **Keep.** Only its quoted copy of row 3's question moves |
| **5**, **10**, **72**, **73**, **16**, **47** | Light, meter, cues, machines | **Keep, all of them.** This is the identity, and the handoff's keep-list and mine agree completely |

Rows **8** and **59** and **74–76** are superseded by proposed row 85 and are
not locked-row problems.

## 5. Where I was wrong

**The physiology.** I wrote in proposed row 91 that *"the family is contained
and therefore has no clock."* The handoff is right that this skips the human
body: a machine that will wait forever implies nothing about what a person in
a locked room can survive. I meant *no countdown mechanic*, because a deadline
on the family turns containment into a rescue timer and deletes the strangest
thing in the design — and I wrote it as a claim about the world.

**Narrowed, for Chris's ruling:** no countdown is installed and none is shown;
elapsed time is bounded by what a body survives, which makes chapter one days
rather than a season.

**And that is already consistent with the rest of it**, which is worth saying
because the handoff reads the two as in tension. Proposed row **88** puts
returns *between* chapters, and row **91** makes the season the clock on
**Lanternhold** and not on the family. So: chapter one is days; the Christmas
department and the seasonal returns the handoff worries about are a later
chapter's, reached by returning to a building that has moved on. The
handoff's own sentence — *"a Christmas change inside a two-day rescue would
make the living mall less credible"* — is the argument for rows 88 and 91
together, not against them.

**One lane note.** This file is 189 lines in `docs/**`, which `LANES.md` gives
to me, and it sat **untracked** in the checkout. `FLEET.md`'s one rule is that
uncommitted work does not exist, and for however long it was untracked, this
audit did not. It is committed with this review. The naming matches the
`HANDOFF-CODEX-*.md` convention and the review request is in bounds; only the
tracking was not.

## 6. Turned over

**Operand.** *"The lantern feels redundant"* is not a measurement of a
feeling, it is a measurement of a missing surface: the only quantity that
prices light appears in one file, and that file is the debug HUD.

**Floor.** `HUD-STATUS-SCREEN.md` dated 2026-10-31 and `ROADMAP.md` M1.7's own
sentence are the floor under §2.5. The UI file list is four names long and
anyone can check it.

**Clock.** Row 3 has gated scope for 22 days with a verb in it that the pivot
deletes. Every day it stays unrestated is a day the gate cannot be opened even
if someone wanted to.

**Falsifier.** §2.5 falls if the lantern still feels redundant with the debug
HUD on. §2.6 falls if Chris reads row 3's *"bank"* as already meaning *"secure
what you came for"*, in which case it needs no restatement and I have invented
a blocker.

**Upside down.** Two agents audited the same product on the same day and
agreed on the diagnosis, the keep-list, and the next proof. The disagreement is
one ordering question and it took a four-name directory listing to settle.
That is the cheapest finding in either document, and it was available to both
of us the whole time.
