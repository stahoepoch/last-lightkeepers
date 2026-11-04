# Handoff — Codex — the revive pulse reads the cosmetic radius

**Status:** one-word fix, plus two comments that now say the wrong thing.
**Lane:** `Source/` is yours. The failing test is already written and is mine.
**Authority:** `DECISIONS.md` rows 80 and 82. Both are LOCKED.

## The defect

`Source/LLK/Private/Loot/LLKInteractorComponent.cpp:456-461` builds the
distress pulse that an Anchor Revive puts into the light field:

```cpp
if (const ULLKLanternLightComponent* Light =
        Owner->FindComponentByClass<ULLKLanternLightComponent>())
{
    Intensity = Light->GetResolvedRules().Intensity;
    RadiusCm = Light->AttenuationRadius;        // <-- this line
}
```

`AttenuationRadius` is a cosmetic value. Row 80: *"transitions smoothly lerp
visually, while gameplay resolves to the named state"* — a lantern-level
crossfade is allowed to move the attenuation radius, which is precisely why
`GetGameplayRadiusCm()` exists.

The change:

```cpp
RadiusCm = Light->GetGameplayRadiusCm();
```

`GetGameplayRadiusCm()` is declared inline at
`Source/LLK/Public/Lantern/LLKLanternLightComponent.h:107` as
`LLKUnits::DistanceFromStuds(GetRadiusStuds())`, so this needs no new include
and no new number.

## Why it is not cosmetic itself

`ULLKZonePressureSubsystem::GetLitLightSources` was moved off
`AttenuationRadius` and onto `GetGameplayRadiusCm()` at row 80, with a comment
explaining the seam. **This call site is the same bug and was missed.** It is
the second place in the project where a `FLLKLightSource` is constructed, and
the only one nothing guarded.

It bites harder than the one that was fixed:

- The pulse enters the field with `Detectability` pinned at **1.0**. The
  comment above it is explicit about why — row 9's price for the Anchor
  Revive is that *"a solo player gets back up, but wakes up the room."*
- Reach decides what the Warden converges on (rows 7 and 54) and whether
  Sweepers notice from 38 studs instead of 12.
- `Source/LLK/Private/Tests/LLKLanternLevelTest.cpp:287-292` establishes that
  `AttenuationRadius` can legitimately sit at **a quarter** of the gameplay
  reach — it sets exactly that and asserts `GetGameplayRadiusCm()` does not
  follow. So a revive landing mid-crossfade can under-reach by 4x.

A revive at a quarter reach does not wake the room. The mechanic silently
stops charging its price, and it does so only sometimes — during a fade —
which is the hardest version of this to ever notice from play.

**The intent is already written down three lines above the defect:** *"No new
number was authored for this: the brightness and reach are the keeper's own
lantern's, and the duration is the revive's own."* The keeper's own lantern's
reach is `GetGameplayRadiusCm()`. Reading `AttenuationRadius` was a correct
expression of that intent while a lantern had one setting, and stopped being
one the moment it had three.

`Intensity` needs no change: `GetResolvedRules().Intensity` is what
`GetLitLightSources` uses for the same field.

## The test, and why it is not already red

**The pre-commit hook refuses a commit while `light-authority.test.mjs` is
red**, and its message states this exact invariant: *"lantern intensity is a
visual knob, and the gameplay light field is read separately through
GetGameplayRadiusCm, not AttenuationRadius. Changing one expecting the other
to follow is a silent rebalance."* So the two checks that catch this defect
could not be landed ahead of the fix — and leaving them red would also have
hidden six suites, because `npm test` is an `&&` chain.

What landed is section 7 of `tests/light-authority.test.mjs` with **one** new
check: the radius seam at the call site row 80 already fixed. Nothing asserted
it before, so the regression now live in the interactor could have walked back
into the storm subsystem unopposed.

Paste these two in with your fix. They were written, run, and seen to fail on
the current code and pass on the fixed code:

```js
check(
  "the distress pulse reads the gameplay reach too",
  !/RadiusCm\s*=\s*Light->AttenuationRadius/.test(interactor),
  "a revive mid-crossfade would wake the room by however far the visual lerp had got",
);
check(
  "and it reads the gameplay radius explicitly",
  interactor.includes("Light->GetGameplayRadiusCm()"),
  "the comment above it already says the reach is the keeper's own lantern's",
);
```

They need this line above them, which is also not landed:

```js
const interactor = read("Source", "LLK", "Private", "Loot", "LLKInteractorComponent.cpp");
```

Observed before removal:

```
  ok   GetLitLightSources reads the gameplay reach, not the attenuation radius
  FAIL the distress pulse reads the gameplay reach too
  FAIL and it reads the gameplay radius explicitly

90 checks, 2 failed
```

Two properties worth keeping if you rework them:

- They pass on the call site that was already fixed and fail on the one that
  was not, so the pair is break-tested by construction rather than by
  assertion.
- The storm check matches an **assignment**,
  `/RadiusCm\s*=\s*Light->AttenuationRadius/`, not a substring. A substring
  test fails on the comment that explains the fix — I wrote it that way first
  and it did.

## While you are in here: two comments that are now wrong

Row 82 ratified the lantern's look-pitch travel as intended behaviour, so
these no longer describe the game:

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
