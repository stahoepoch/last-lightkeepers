# Handoff — Codex — the revive pulse read the cosmetic radius (LANDED), and three comments that are now wrong

**Status:** the defect is **fixed and guarded**. What is left for you is three
comments, which are wrong for an unrelated reason (row 82).
**Lane:** `Source/LLK/Private/Loot/LLKInteractorComponent.cpp` was edited by
**Claude**, not you. Chris ruled it into my lane explicitly — see below — so if
you are reconciling `Source/` against `FLEET.md`, this is the authorized
exception and not a lane breach to undo.
**Authority:** `DECISIONS.md` rows 80 and 82. Both are LOCKED.

## What changed, and why it did not wait for you

`Source/LLK/Private/Loot/LLKInteractorComponent.cpp` built the distress pulse
that an Anchor Revive puts into the light field, and read the **cosmetic**
radius to do it:

```cpp
Intensity = Light->GetResolvedRules().Intensity;
RadiusCm = Light->AttenuationRadius;        // <-- cosmetic
```

It now reads:

```cpp
RadiusCm = Light->GetGameplayRadiusCm();
```

declared inline at `Source/LLK/Public/Lantern/LLKLanternLightComponent.h:107`
as `LLKUnits::DistanceFromStuds(GetRadiusStuds())` — no new include, no new
number. A seven-line comment above it now records the seam, in the same shape
as the one at `LLKZonePressureSubsystem.cpp:352-359`.

This was written up for you first. Chris's instruction was: *"Make the one-line
GetGameplayRadiusCm() fix in Claude's source lane and land the two regression
checks with it as one atomic follow-up commit… the live revive-pulse seam is
still wrong; ship the explanation and the actual protection together once the
suite is green."* So the alternative — docs describing a correct split while
the seam stayed broken for a release — was the thing he ruled against, and the
lane moved rather than the timing.

## The defect, kept as the record

`AttenuationRadius` is the value a lantern-level crossfade is allowed to lerp.
Row 80: *"transitions smoothly lerp visually, while gameplay resolves to the
named state"* — which is precisely why `GetGameplayRadiusCm()` exists.

`ULLKZonePressureSubsystem::GetLitLightSources` was moved off
`AttenuationRadius` and onto `GetGameplayRadiusCm()` at row 80, with a comment
explaining the seam. **This call site was the same bug and was missed.** It was
the second place in the project where a `FLLKLightSource` is constructed, and
the only one nothing guarded.

It bit harder than the one that was fixed:

- The pulse enters the field with `Detectability` pinned at **1.0**. The
  comment above it is explicit about why — row 9's price for the Anchor
  Revive is that *"a solo player gets back up, but wakes up the room."*
- Reach decides what the Warden converges on (rows 7 and 54) and whether
  Sweepers notice from 38 studs instead of 12.
- `Source/LLK/Private/Tests/LLKLanternLevelTest.cpp:287-292` establishes that
  `AttenuationRadius` can legitimately sit at **a quarter** of the gameplay
  reach — it sets exactly that and asserts `GetGameplayRadiusCm()` does not
  follow. So a revive landing mid-crossfade could under-reach by 4x.

A revive at a quarter reach does not wake the room. The mechanic silently
stopped charging its price, and only sometimes — during a fade — which is the
hardest version of this to ever notice from play.

**The intent was already written down three lines above the defect:** *"No new
number was authored for this: the brightness and reach are the keeper's own
lantern's, and the duration is the revive's own."* The keeper's own lantern's
reach is `GetGameplayRadiusCm()`. Reading `AttenuationRadius` was a correct
expression of that intent while a lantern had one setting, and stopped being
one the moment it had three.

`Intensity` needed no change: `GetResolvedRules().Intensity` is what
`GetLitLightSources` uses for the same field.

## The guard, now landed

Section 7 of `tests/light-authority.test.mjs` holds three checks: the storm
call site row 80 had already fixed, which nothing had asserted before, and the
two that were red on this one.

```js
check(
  "the distress pulse reads the gameplay reach too",
  !/RadiusCm\s*=\s*Light->AttenuationRadius/.test(interactor),
  ...
);
check(
  "and it reads the gameplay radius explicitly",
  interactor.includes("Light->GetGameplayRadiusCm()"),
  ...
);
```

Two properties worth keeping if you ever rework them:

- They passed on the call site row 80 had fixed and failed on the one it
  missed, so the set is break-tested by construction rather than by assertion.
  Observed red before the fix: `90 checks, 2 failed`. Green after: `90 checks,
  0 failed`.
- Every one matches an **assignment**,
  `/RadiusCm\s*=\s*Light->AttenuationRadius/`, never a substring. A substring
  test fails on the comment that explains the fix — the storm check was written
  that way first and did exactly that, which is the same false-positive class
  this file exists to catch in the game.

The pre-commit hook refuses a commit while this file is red, and its message
states this invariant in so many words. That is why the checks and the fix are
one commit and could not have been two.

## What is actually yours: three comments that are now wrong

Row 82 ratified the lantern's look-pitch travel as intended behaviour, so these
no longer describe the game:

- `Source/LLK/Private/Characters/LLKCharacter.cpp:33` — *"Gameplay still
  samples LanternAnchor, which remains stable beneath the camera."*
- `Source/LLK/Private/Characters/LLKCharacter.cpp:75` — *"the latter is the
  stable gameplay source"*
- `Source/LLK/Private/Lantern/LLKLanternLightComponent.cpp:59` — *"The parent
  is the keeper's stable hand-space anchor."*

`LanternAnchor` is attached to `FirstPersonCamera`, which has
`bUsePawnControlRotation = true`, so the anchor carries full look pitch and
its authored `(42, 25, -19)` offset travels about **84 cm** vertically across
the pitch range. That is now correct by row 82 — the mount is what gameplay
reads and the *pendulum* is what must stay out. What the comments should say
is that the anchor is stable against **cosmetic sway**, which it is, and by
attachment order rather than by convention: sway is written to the light
component's relative offset, one level below the anchor.

The word "stable" is doing load-bearing work in three places and means
something narrower than it says. Someone will eventually read it as "does not
move" and build on that.

I left these alone deliberately. Chris's ruling named the one-line fix and the
two checks; three comment rewrites in two more files is not that, and comments
are where a paraphrase softens a locked decision, which is the failure mode
`FLEET.md` cites for the lane rule in the first place.

## Out of scope — do not do these here

- **Do not change the pendulum or any feel number.** `FINDINGS-LANTERN-PHYSICS.md`
  finds five of them disagreeing with the real physics. That is row 81 work,
  it needs Chris, and it is cosmetic either way.
- **Do not add gameplay aim.** `FLLKLightSource` carries no direction and the
  gameplay light is a sphere. Row 82's aim clause governs a mechanism that
  does not exist yet; adding one is a design change, not a fix.
- **Do not touch `GetGameplayLightLocation`'s unreachable branch.** The
  240 cm arithmetic in it is real and is in a branch a running game never
  enters, because the component always has an attach parent. See
  `FINDINGS-LANTERN-ANCHOR-ORIGIN.md`, which retracts the original claim.
