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

## Standing checks worth re-running

| Check | Why |
| --- | --- |
| Is every tracked claim's operand still the right one? | The operand pattern was found twice in one week |
| Does `git status` show untracked design work? | Eleven files and a map, 2026-11-01 |
| Does any doc assert something a `DECISIONS.md` row reverses? | `DRIFT.md` — it did, in nine files |
| Is any setting the instrument depends on unwritten on disk? | The throttle was unwritten and therefore on |
| Are two copies of one contract diverging? | `FIXTURE-LIGHT-CONTRACT.md` exists twice, with different content |
