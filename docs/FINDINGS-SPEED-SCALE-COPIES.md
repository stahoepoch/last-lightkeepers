# Four copies of the scale are fixed. The fifth is the one the player feels.

> **Status: updated 2026-11-04, after `a766711`, `1751090` and `d663596`.**
> Written against `7426625` (the halving), when four source constants still
> held the old scale and the editor-side asset had never been regenerated.
> **All four are now fixed and the guard is green.** What remains open is the
> fifth copy, which is not source at all: `Content/Data/DA_Lantern.uasset`
> holds `sprintSpeed` 11.5 while `data/lantern.json` authors 12.5.
>
> `tests/speed-scale.test.mjs` is this file's executable form: **16/16.**
> `tests/movement.test.mjs` is the one that is red, on the asset, correctly.

## What changed since this file was first written

Two things moved underneath it, and both matter more than the original audit.

**The sprint moved again.** `d663596` took sprint from 11.5 to 12.5 studs —
322 to 350 cm/s — on Chris's "running could be a little faster," holding walk
deliberately so the sprint/walk separation widened 1.4375 to 1.5625. **Every
"11.5" and "322" in the first version of this file was correct when written and
is now wrong.** They are corrected below. The halving itself (`7426625`) is
still the commit that created the divergence this file is about.

**The four defects were fixed, by Codex, not here.** This file said they were
C++ and that this session could not build them. That was true; they were landed
by the writer who could. The analysis is kept because *why each copy existed*
is the part worth not relearning.

| # | The copy | Then | Now |
| --- | --- | --- | --- |
| 1 | `LLKLanternFeel.cpp` sway denominator | `448.f` literal | reads the resolved walk speed |
| 2 | `LLKCharacter.cpp` carry-bob denominator | `/ 448.f` | reads the keeper's own walk speed |
| 3 | `LLKCharacter.h` declared fallback | 16 / 23 / 9 studs | 8 / 12.5 / 4.5, matching authored |
| 4 | `VerifyRuleAssets` in the importer | walk only | walk, sprint and crouch, plus `VerifyEnemyMoveSpeeds` over every row |
| **5** | **`DA_Lantern.uasset`** | never regenerated | **regenerated, then went stale again** |

## First, the question that was asked

**Do the fractional studs survive, or does something coerce 12.5 to 12?**

**They survive. There is no integer anywhere in the chain.** Traced end to end:

| Step | Where | Type |
| --- | --- | --- |
| JSON parse | `LLKImportDataCommandlet.cpp:70` `GetNumber` | `double` |
| cast to the field | same, line 77 | `static_cast<float>` |
| the asset field | `LLKLanternData.h:330-336` | `float` |
| studs to cm | `LLKUnits.h:52` `SpeedFromStuds` | `float`, `constexpr` |
| the movement component | `LLKCharacter.cpp:414-416, 585` | `float` |

So `sprintSpeed: 12.5` lands as **350.0 cm/s** exactly, and `crouchSpeed: 4.5`
as **126.0**. `GetInt` exists in that file and is **not** used for any speed.
`FLLKEnemyDef::MoveSpeed` is also `float`, so a fractional enemy speed is safe.

This was also confirmed from the other end, which is the stronger proof: a
byte scan of the generated `DA_Lantern.uasset` finds the IEEE-754 float32 for
**4.5 at offset 2265** and **8.0 at 2207**. The fractional crouch is genuinely
in the shipped asset, not merely in the types.

The one place a speed is narrowed is the possession log, which prints `%.0f`.
That is cosmetic, but **the log line cannot confirm a fractional value** — 350
and 350.4 print identically. It is still the right confirmation that
`DA_Lantern` resolved at all.

## The fifth copy, which is the open one

The same byte scan that proves 4.5 survived also proves the sprint did not:

```
PRESENT  walk 8.0                float32 at 139, 1193, 2207
PRESENT  sprint 11.5 (old)       float32 at 2236
ABSENT   sprint 12.5 (authored)
PRESENT  crouch 4.5              float32 at 2265
```

`DA_Lantern.uasset` is timestamped 10:08:23; `data/lantern.json` is 10:42:13.
The asset predates the sprint change by 34 minutes. **So the live game sprints
at 322 cm/s — the value Chris asked to raise — while the JSON, the declared
fallback, the importer's verifier and `speed-scale.test.mjs` all agree on 350.**

Four of the five copies were brought into line and the fifth is the only one
the player's thumb can feel. Every green check in this repository was green
while that was true, until `movement.test.mjs` grew a byte scan for it in
`d663596`. `enemies.test.mjs` had scanned `DT_Enemies` for exactly this reason
since earlier; nothing had done it for the lantern, **which is where the
player's own speed comes from.**

**The fix is one command, and it needs the editor closed:**
`Tools/import_unreal_data.ps1`, which throws at line 22 if an editor is
running. That is a precondition, not a blocker — `TOOLING-TRAPS.md` trap 16.

## What the original four taught, kept because it is still the lesson

### 1 and 2. The presentation denominators, and why these bit hardest

Both were denominators producing a 0..1 fraction of walking pace against a
literal `448.f`. Walk became 224, so **a walking keeper produced 0.5 where the
code expected 1.0.** In `LLKLanternFeel.cpp` that fraction drives *both* the
sway phase rate and, through `FMath::Min(1.f, SpeedFraction)`, the sway
amplitude — so at a walk the lantern swayed at **half rate and half
amplitude**. Live code, not dormant: `LLKLanternLightComponent.cpp:141` calls
`AdvanceFeel` every tick.

**The symptom is "the lantern stopped feeling like a carried object," and
nobody debugging that would grep for `448`.** The sway comment calls the sway
*"evidence of a hand"* — at half amplitude it was less evidence. Of all the
regressions a rescale could cause, this one hit the thing the game is named for.

The first of the two carried a comment citing `data/lantern.json movement.walkSpeed`
as its source. The citation was true when written. **A comment naming its
source is not a link to it**, and that was this file's original thesis.

### 3. A declared fallback is a promise that it matches

The fallback is the path taken whenever `DA_Lantern` does not resolve, and its
warning named 448/644/252 as the "DECLARED FALLBACK" long after that stopped
being anybody's intent. The comment beside that log states the promise:

> gameplay reads the data rather than a constant that happens to match it today

It matched yesterday. **The fallback is only honest while it matches**, and
nothing enforced that, which is why the guard now does. It remains a trap in a
second way: any machine without a regenerated asset plays at whatever the
fallback says, and `Content/Data/` is gitignored, so that is every fresh
checkout.

### 4. A verifier that reports success for what it did not check

`VerifyRuleAssets` re-read the generated asset and compared it semantically
against its source JSON — exactly the right idea — for `walkSpeed`,
`radiusStuds`, `intensity` and `warmColor.r`. **It did not compare
`sprintSpeed` or `crouchSpeed`**, the two values the retune made fractional and
therefore the two most likely to be wrong. `VerifyTable` was thinner still:
row names and row count, **no field value at all**; every `moveSpeed` could
have been zero and it would have passed.

This is row X3's shape — **a generator that reports success while what it
generated is wrong** — sitting inside the verifier that exists to prevent it.
Both are now fixed, and `d663596` is the proof that fixing them was not
sufficient: the verifier only runs when the importer runs.

## The lesson that replaces the original one

The first version of this file ended by saying a comment naming its source is
not a link to it. That is still true and still the cause of copies 1 through 3.

The sprint change taught a sharper one. `speed-scale.test.mjs` derived the
superseded scale as `authored x 2`, which held for exactly as long as the only
recorded change was a halving. At sprint 12.5 it began hunting for 25 studs —
700 cm/s, a number this project has never held — while the real old value, 644,
went unwatched. **A guard that computes history from the present is a guard
with a shelf life.** `d663596` narrowed it to look for speeds in speed contexts
and verified the narrowing by planting `/ 448.f` and `644` back and watching it
still fire.

And the deepest one, which is why this file is retitled: four source copies
were corrected, the guard went green, `npm test` passed, and **the player still
ran a sprint nobody authored.** Authoring a number is not shipping it.

## Still blocked on the editor

The editor is currently **open** (Chris is playing, and is remote), which is
what blocks these — and the open editor is also why the build target refuses.

1. **Re-run `Tools/import_unreal_data.ps1`** with the editor closed, so sprint
   12.5 reaches the asset. `movement.test.mjs` goes green on its own when it
   does, and it should be fixed that way and never by touching the test.
   `d663596` also compiled HUD and extraction-readout changes that reach the
   game by the same route.
2. **Confirm `DA_Lantern` took** from the possession log — bearing in mind the
   `%.0f` caveat above cannot distinguish 350 from 350.4.
3. **Play the dressed circuit at the new speed.** The 28 m room should take
   about 12.5 s rather than 6.2. This is what the dressing pass could not be
   judged without, and the central aisle is 573 cm — see
   `FINDINGS-BOSWELLS-DRESSING-PASS.md`. Note that
   `FINDINGS-BOSWELLS-READS-AS-STOCKROOM.md` is the live objection to that
   pass, and it is about the art pack rather than the measurements.
