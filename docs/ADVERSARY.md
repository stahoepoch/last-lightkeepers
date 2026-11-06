# Adversary procedures — how the agents check each other

Status: **proposed**, 2026-11-01. Every rule below was paid for. The source is
the lighting-measurement thread of 2026-10-29 → 11-01, in which two agents
spent roughly ten rounds correcting each other and **both** were wrong more
than once. Five instrument defects were found that way. None of them would
have been found by agreement.

This file exists so the next thread costs three rounds instead of ten.

## Why adversarial, and not just careful

An agent that drives the editor can see things this session cannot, and an
agent that cannot touch the editor has no stake in the instrument being
sound. That asymmetry is the whole value. It is also fragile: the moment one
party can hand the other its own job, the check collapses.

> **No agent certifies its own instrument.**

The MCP session built the light lab. It does not get to rule on whether the
lab's captures are valid. This session audits the captures. It does not get to
take the editor — it was offered once, and taking it would have ended the
check.

**Chris adjudicates.** Not because he arbitrates technical facts, but because
when both agents are confident and incompatible, the tie is a design question
wearing a measurement costume, and that is his.

## What counts as a claim

A measurement claim names four things. Missing any one, it is an impression:

| | | Why it was added |
| --- | --- | --- |
| **The operand** | What, exactly, is being compared to what | Both failing detectors had the right mechanism and the wrong operand |
| **The floor** | The noise level below which a difference is not a difference | A MAD of 0.131 was read as signal against a floor of 0.317 |
| **The clock** | Which clock a timestamp is on, named | A whole finding was wrong because the engine log is UTC and the machine is EST |
| **The falsifier** | What result would have changed the conclusion | Without it, a claim cannot be checked, only believed |

## Arguments that do not work

These were all made in good faith by one agent or the other, and all of them
failed. They recur, so they are named.

**"The two captures are byte-identical, so the frame is not stale."**
Identity rules out *noise*. It says nothing about staleness — a frozen
viewport produces byte-identical captures forever. Determinism and currency
are different properties.

**"The pixels hash differently, so they are not duplicates."**
A hash has no magnitude. It cannot distinguish a one-pixel crop difference
from a completely different image. Difference claims need a distance, and a
distance needs a floor.

**"The frame counter advanced, so the viewport redrew."**
Two clocks. The engine ticks whether or not the viewport redraws, and
`bThrottleCPUWhenNotForeground` decouples them further. A frame counter is
evidence about the engine, not about the image.

**"We waited, so it settled."**
Eighteen frames at three frames per second is six seconds. They waited six
seconds and the capture was stale anyway. Settle time is not a fix for a
pipeline that is not running.

**"The setting is off."**
It was written nowhere, which means it was at the engine default — on — for
every capture this project ever took. **An unwritten setting is not a known
setting.** Read the default from the engine source, or read the key from disk,
or you do not know.

## The operand pattern

Both of the instrument's failing detectors were repaired without new
mechanism:

- The staleness detector differenced each capture against *the previous
  capture*. A monotonic ladder then produces false rejections, because
  adjacent rungs are legitimately similar. Fixed by differencing against a
  **fixed baseline** and requiring the cumulative distance to increase.
- The frame detector compared raw counters across an interval of unknown
  length. Fixed by making it a **rate over the change-to-capture interval**.

> **When a detector misfires, suspect its operand before you replace it.**

A detector that is replaced rather than re-pointed loses whatever it was
genuinely catching, and the project acquires two half-trusted instruments
where it had one repairable one.

## Calibration must fail informatively

A proposed calibration patch in this thread would have passed on a broken
instrument: it checked a flat field, and a frozen viewport shows a flat field
too. It was wrong and it was caught.

The replacement works because the quantity it measures maps **monotonically**
to the thing being varied, so a wrong reading is not merely a failure — it
says which direction the rig is off and by how much.

> **A calibration that can only say pass or fail is nearly worthless. It must
> say what to change.**

## Retraction discipline

Both parties were wrong in this thread. What made it productive:

1. **A retraction is a commit, not a sentence in chat.** `e1eb630` exists
   because a finding was withdrawn. If the retraction lives only in a
   conversation, the repo still asserts the wrong thing and the next reader
   believes it.
2. **Retract the exact scope, not more and not less.** One claim here was
   half-wrong — two grounds were offered, one was a property of the scene
   rather than the instrument. The right move was to withdraw that ground and
   keep the other, not to abandon the position or defend both.
3. **Concede fast and keep working.** The valuable output of ten rounds was
   five defects found, not a tally of who was wrong.
4. **No credit laundering.** Attributing one agent's finding to another in a
   commit message corrupts the only record of who checked what. It happened;
   it was corrected.

## The refusal

An agent may refuse to certify a result. A refusal is only useful if it is
specific, so it names:

- the threshold that was not met, with the number;
- what would change its mind.

"I am not confident" is not a refusal. "The MAD is 0.131 against a floor of
0.317, so this is below noise; re-run with the viewport foregrounded and I
will take it" is.

## Reviewing design, not just measurement

The same asymmetry applies to design arguments, with one addition: **do not
hand the reviewer your conclusion.** A reviewer given "here is my idea, is it
good?" returns it confirmed. A reviewer given the constraint and the material
returns something useful.

When this session is asked to critique, the useful output is the part that
disagrees. An agreeing review of a design document is a cost with no product.

## The wrong vintage — the failure class with no reasoning error in it

Added 2026-11-02, from the MCP session, and it is a different animal from
everything above.

Every error named so far is a **reasoning** error: the wrong operand, a hash
read as a magnitude, a counter read as a redraw. You can catch those by
thinking harder about the argument.

This one has no flaw in the argument at all.

`FINDINGS-ENEMY-OPTIONALS.md` is 170 lines titled *"Every family-specific
enemy number is missing from the build"*. **On the morning of 2026-11-02 it
recorded nowhere** that Codex had closed it on 2026-10-31 in `1ae1db9`.
Reading it at face value, the MCP session assembled this:

1. `AggroRadiusDark` is `TOptional<float>`.
2. The importer drops optional fields.
3. The new stock gate is `DistanceToMisplacedStuds <= Def.AggroRadiusDark.Get(0.f)`.
4. Therefore `e0efb06` is **inert in play**, and the green test cannot see it
   because the fixture assigns that field by hand.

Coherent, alarming, actionable, and wrong. `node tests/enemies.test.mjs` took
**nine seconds** to report that every authored number reaches the table, with
a green control group. The gate is live.

**Not one link in that chain was a mistake.** The input was a document
describing a world that had stopped existing two days earlier.

It has a status line now — `12181a6` put one at its top the same day, which is
the fix working. **And the paragraph above needed correcting for the same
reason**, within hours of being written: it asserted in the present tense that
the document records no closure, and by the time anyone read that, it did. The
MCP session caught it.

That is not an amusing coincidence, it is the measurement. A section about
stale inputs went stale in under a day because it stated a fact about a file
without saying when the fact was true — which is the first rule in the defence
below, applied to the file that contains it. **Tense is the whole defect:**
"records nowhere" is a claim about now that nobody can check later, and "on
the morning of 2026-11-02 it recorded nowhere" is a claim about history that
stays true forever.

### Why it is the dominant risk here specifically

Three agents who cannot see each other, fifty-plus design documents, and a
human relaying between them. The agents read the repo rather than each other,
so **a document is the primary sense organ** — and a stale one is not a wrong
opinion, it is a wrong *observation*. Arguing with it is useless because it
is internally consistent.

It also fails asymmetrically:

| A stale doc that says | Costs you |
| --- | --- |
| "this is broken" | a cycle of alarm and investigation — expensive, self-limiting, and visible |
| "this is fine" | a defect nobody looks for — cheap today, and invisible until it ships |

The second is worse and attracts no attention, which is why the closure
protocol matters more than the finding protocol.

### The defence

> **A document is evidence about when it was written, never about now.**

- **Date every claim you act on, and every claim you write.** If the finding
  has no date, that is the finding. **Write facts about a file in the past
  tense with a date**, never in the present: "records nowhere" cannot be
  checked a week later, "on 2026-11-02 it recorded nowhere" is true forever.
  This section broke that rule about itself within hours.
- **Before acting on a document's premise, measure the premise.** Usually
  cheap: nine seconds here, against a change that would otherwise have been
  reverted as inert.
- **Run a control.** A green control group is what turned "the gate is dead"
  into "the gate is live" rather than into a second theory.
- **Closing something means writing the closure where the finding lives**, not
  only in the commit message — whoever filed it never reads that. This is now
  in `LANES.md`, and `docs/HANDOFFS.md` is the ledger: open items by owner,
  each with its evidence and how it is proved closed.

### The sub-second version: a shared working tree

The same class, at the timescale of two consecutive commands. When two
sessions share one checkout, `git status` and `git diff` describe a world the
other writer is changing underneath you, so two readings a moment apart are
observations of different worlds and their disagreement carries no
information. It produced a false document on 2026-11-02 — `FLEET.md`, *"Two
sessions in one checkout"*, has the rule.

Worse than the documentary version in one respect: a stale document at least
holds still while you read it.

### My own instance of it, which is the clearest one

`roster-sense-audit-UNBUILT.patch` had landed on 2026-10-31 in `9ddc7af`, and
the note I left beside it still said UNBUILT. The MCP session found out by
trying to apply it: two of five files failed `git apply --check`, and a 3-way
apply then succeeded with an **empty diff**, which is what "already in HEAD"
looks like through `git apply -3`. `HasReachingSense` is at
`LLKPerception.h:294`, `AuditSenses` at `LLKEnemyRosterSubsystem.h:101`.

I wrote a stale label and it cost somebody else a cycle to discover. The rule
above is not advice for other people.

## The dead branch — the sibling of the wrong vintage

"The wrong vintage" is a correct analysis of a world that stopped existing.
**This is a correct analysis of code that does not run**, and it has the same
property that makes the first one dangerous: there is no reasoning error in it
anywhere, so thinking harder does not help.

Its instance is `FINDINGS-LANTERN-ANCHOR-ORIGIN.md`, 2026-11-04, mine.
`GetGameplayLightLocation()` was read from the bottom. The function's last
statement adds a 152 cm eye height to `GetActorLocation()`, which on a
`ACharacter` is the capsule centre — a genuine double-count landing at 240 cm,
in the one position four systems judge from. Every step after that was sound,
including a deliberate elimination proof that nothing in the project could move
the capsule off UE's default 88: no Blueprint subclass, no `SetCapsuleSize`, the
only `GetCapsuleComponent()` call in `Source/` setting a collision response.

**Six lines above the statement there is an `if (GetAttachParent())` that always
takes.** `ALLKCharacter` attaches the component in its constructor, so the
analysed branch is unreachable in a running game, and the live anchor is 133 cm.

Two things made it convincing, and both are worth distrusting on sight:

- **The elimination proof.** *"I have ruled out every way this number could be
  different"* is easy to mistake for *"this number is the one in play."* The
  first is a statement about the value; reachability is a different question and
  the proof says nothing about it.
- **The explanation offered for why nobody had noticed.** The file argued the
  defect was invisible because 250 cm racking blocks both 240 and 152. A live
  defect in a path four systems read, in a project with an automation suite and a
  lighting audit, surviving on a coincidence of shelf height, is a *worse* story
  than "the path isn't reached." **"Nobody noticed because of luck" should be the
  last explanation reached for, not the first** — when it is the only thing
  holding a finding up, the finding is usually about the wrong code.

The defence costs one grep: **before reasoning about a function's body, find its
callers, and read the early returns above the line you care about.**

## Standing checks worth re-running

| Check | Why |
| --- | --- |
| Is every tracked claim's operand still the right one? | The operand pattern was found twice in one week |
| Does `git status` show untracked design work? | Eleven files and a map, 2026-11-01 |
| Does any doc assert something a `DECISIONS.md` row reverses? | `DRIFT.md` — it did, in nine files |
| Is any setting the instrument depends on unwritten on disk? | The throttle was unwritten and therefore on |
| And the complement: is a setting that **is** written actually in effect? Echo it from the running console, never read it from the `.ini` | `Config/DefaultEngine.ini` warns in its own comment that the Lumen keys "risk being enabled in config and inert in the running editor, which would look exactly like the no-GI behaviour they are meant to replace." The file is the authored **intent**; the console is the **observation**; the warning exists because they can disagree. `r.DynamicGlobalIlluminationMethod` echoed at the console is the check — `HANDOFFS.md` M1 |
| Are two copies of one contract diverging? | `FIXTURE-LIGHT-CONTRACT.md` exists twice, with different content |
| **Is the document you are acting on still true?** Date it, then measure its premise | `FINDINGS-ENEMY-OPTIONALS.md` had been closed for two days and still read as open. Nine seconds of measurement against a change nearly reverted as inert |
| Does a finding you closed say so **where the finding lives**? | A closure that exists only in a commit message is invisible to whoever filed it. `docs/HANDOFFS.md` |
| Does a test compare the container or the contents? | X2: `VerifyTable` compares `GetRowNames()` and nothing inside the rows, so the silent-drop class that `1ae1db9` fixed can land again. Handed to Codex |
| **Is the line you are reasoning about reached?** Find the callers, then read the early returns above it | The lantern anchor, 2026-11-04: a sound double-count analysis of a branch an `if (GetAttachParent())` six lines up makes unreachable. "The dead branch", above |
| Is "nobody noticed because of luck" doing load-bearing work in your argument? | Same finding. It was the only thing holding it up, and it was holding up the wrong code |

## The correction that borrows the last bug's explanation

A number gets corrected, and the reason given for the correction is *the
mechanism of a bug that was already found and retracted nearby*. It is the most
persuasive shape a wrong correction can take, because the explanation is already
familiar, already believed, and already written down.

**The case, 2026-11-04.** The gameplay light anchor's look-pitch travel was
corrected from 88 cm to "about 84 cm", on the grounds that the 88 was *"the
capsule half-height carried over from the retracted 240 cm finding."* That is a
real failure mode and it had really happened on that exact number that morning.
It was also not what happened here. World Z is `152 + 42·sin(p) − 19·cos(p)`,
which is **not monotonic in pitch**: 84.00 is precisely `z(+89.9) − z(−89.9)`,
the two endpoints, and it misses the interior minimum at `p = −atan(42/19) =
−65.66°`. The travel is 88.06. The capsule's 88 and the travel's 88.06 are a
coincidence — the file being corrected said so in those words — and the
coincidence is what made the wrong diagnosis available.

**Two tells, both cheap.** The claim's own label disagreed with its number: it
said "unclamped," and genuinely unclamped is `2·√(42² + 19²)` = 92.20, not 84.
And nothing in `Source/` or `Config/` authors a pitch clamp at all, so there was
no reading of the game that could produce 84 — it could only come from arithmetic.

**Standing checks worth re-running**

| Check | Why |
| --- | --- |
| Does this correction's reason belong to a *different* bug? | A familiar mechanism is a hypothesis, not evidence. Re-derive on this number. |
| Does the label match the number? | "Unclamped" and 84 cannot both be true. A mismatch between a value and its own description is the cheapest contradiction there is. |
| Is the extremum at an endpoint? | For anything of the form `a·sin + b·cos`, it usually is not. Differentiate before taking the ends. |

## The measurement you already had and did not use

A derivation whose inputs are estimated while the file it lives in **already
contains the measured value**, two sections up, in a table the same author
wrote. Not a missing source — an unused one.

**The case, 2026-11-04.** `FINDINGS-LANTERN-PHYSICS.md` derived a carried
lantern's natural frequency by pivoting it at the bail *ears*, estimating
ear-to-centre-of-mass at 0.15–0.22 m, and got 1.05 Hz. But a keeper carries the
thing by the bail, so the pivot is the hand and the bail's own height belongs in
the lever arm — and the finding's own §1 table carried the museum record *"8¾″
body, 14″ with the bail up"*, which measures that height at 13.3 cm. Using it
gives 0.87 Hz. The headline survived (still about 1 Hz) but the walk's drive at
0.95 Hz moved from *below* the natural frequency to *above* it, which inverts
the phase — the one consequence that depended on which side it fell.

**Why it is worth its own entry.** This is not the "correct analysis of the
wrong branch" failure and not the "endpoints of a non-monotonic function" one.
Both of those were wrong *reasoning* over right inputs. This was right
reasoning over an input chosen by estimate while the measurement sat in the same
document. The estimate looked like diligence, which is what made it invisible.

**Standing checks worth re-running**

| Check | Why |
| --- | --- |
| Does the file already measure what the calculation is estimating? | A table of real specs two sections above a derivation is either an input to it or a thing to rule out in writing. Nothing in between. |
| Is the pivot where the hand is, or where the drawing is? | Model the object as it is *used*. A lantern pivots at the grip, not at the hinge nearest the body. |
| Which conclusions would survive the input being 30% off? | The ones that do are the findings. The ones that do not need the measurement before anyone builds on them. |

## The authored gate whose only caller is a test

Two instances found a day apart, which makes it a shape rather than a bug.

| | The gate | Its callers | What it means |
| --- | --- | --- | --- |
| `M14` | `ULLKExtractionSubsystem::SetRequirementSatisfied` | one, and it is the console command `LLK.Extraction.Satisfy` | `FreightLift` authors `requires: SubBasementBreakerRestored` and no path through the game can ever clear it |
| `COMBAT.md` | `FLLKEnemyDef::CanBeDamaged()` | two, and both are automation tests | Five enemies author health, nothing in `Source/` applies damage, and the predicate that would decide it is never asked in play |

**Why this is hard to see.** Every ordinary signal reads green. The field is
authored in `data/`, the contract test asserts its type, the predicate exists
and is spelled correctly, and a grep for the symbol finds callers — so
"is it wired?" answers yes. The question that catches it is narrower: *is it
called from anywhere a player's action can reach?* A test caller and a console
caller both prove the code compiles and behaves; neither proves the mechanic
exists.

**It is the mirror of the failure this file already records about tests.**
`M7` found `OnInteractableChanged` and `OnLegibilityChanged` bound **only**
inside `LLKRelicTest.cpp`, which a `.AddDynamic` grep reports as wired while a
player hears nothing. Same arithmetic: a symbol's callers are not evidence of a
mechanism when the callers are instruments.

### The standing check

| Ask | Not |
| --- | --- |
| Can a player's input reach this call? | Does this symbol have callers? |
| Is the only caller a test, a console command, or a commandlet? | Does it compile and pass? |
| If I deleted the gate, would any playtest notice? | Would the suite go red? |

The cheap version is one grep with the instruments excluded: search for the
symbol, then subtract `Private/Tests/`, `FAutoConsoleCommand` blocks and the
file that declares it. What is left is the mechanic. If nothing is left, the
feature is a value in a JSON file and a sentence in a design doc.

**And both instances were found the same way** — not by reading the gate, but by
asking what happens to a player who tries to use the thing it gates. Chris could
not extract, and could not damage anything. The gates were found downstream of a
symptom, which is the only direction that works here, because upstream
everything looks fine.
