# The fractional studs survive. Four copies of the old scale do not.

> **Status: OPEN, 2026-11-04.** Written auditing `7426625`, which halved the
> movement scale. **The editor is closed, so the asks that needed it are not
> done:** `DA_Lantern.uasset` and `DT_Enemies.uasset` are still the old data,
> and `tests/enemies.test.mjs` is still correctly red.
>
> Everything below is a source read, which is why it could be done anyway.
> `tests/speed-scale.test.mjs` is the executable form of this file: **9/16,
> seven accurate failures.**

## First, the question that was asked

**Do the fractional studs survive, or does something coerce 11.5 to 11?**

**They survive. There is no integer anywhere in the chain.** Traced end to end:

| Step | Where | Type |
| --- | --- | --- |
| JSON parse | `LLKImportDataCommandlet.cpp:70` `GetNumber` | `double` |
| cast to the field | same, line 77 | `static_cast<float>` |
| the asset field | `LLKLanternData.h:330-336` | `float` |
| studs → cm | `LLKUnits.h:52` `SpeedFromStuds` | `float`, `constexpr` |
| the movement component | `LLKCharacter.cpp:414-416, 585` | `float` |

So `sprintSpeed: 11.5` lands as **322.0 cm/s** exactly, and `crouchSpeed: 4.5`
as **126.0**. `GetInt` exists in that file and is **not** used for any speed.
`FLLKEnemyDef::MoveSpeed` is also `float`, so a fractional enemy speed would be
safe too if one is ever wanted.

The one place a speed is narrowed is the possession log, which prints `%.0f`.
That is cosmetic, but it means **the log line cannot confirm a fractional
value** — 322 and 322.4 print identically. It is still the right confirmation
that `DA_Lantern` resolved at all, which is what it was offered for.

## Then, the thing nobody asked about

Four constants hold the **old** scale. Each is silent: nothing crashes, nothing
logs, the number is simply wrong by exactly 2×.

### 1 and 2. Two presentation denominators, and this is the one that bites

```
Source/LLK/Private/Lantern/LLKLanternFeel.cpp:135
  const float WalkSpeedCm = 448.f; // 16 studs/s, data/lantern.json movement.walkSpeed

Source/LLK/Private/Characters/LLKCharacter.cpp:222
  const float Speed01 = GetVelocity().Size2D() / 448.f;
```

Both are denominators producing a 0..1 fraction of walking pace. Walk is now
**224**, so **a walking keeper produces 0.5 where the code expects 1.0.**

In `LLKLanternFeel.cpp` that fraction drives *both* the sway phase rate and,
through `FMath::Min(1.f, SpeedFraction)`, the sway amplitude. So at a walk the
lantern now sways at **half rate and half amplitude**. In `LLKCharacter.cpp` it
drives the carry bob through `Amount`, with the same halving.

This is live code, not dormant: `LLKLanternLightComponent.cpp:141` calls
`AdvanceFeel` every tick.

**The symptom is "the lantern stopped feeling like a carried object," and
nobody debugging that would grep for `448`.** The sway comment in that file
says the sway is *"evidence of a hand"* — at half amplitude it is less
evidence. Of all the regressions a speed rescale could cause, this is the one
that hits the thing the game is named for.

Note the first one carries a comment citing `data/lantern.json movement.walkSpeed`
as its source. The citation was true when written. **A comment naming its
source is not a link to it**, and that is the whole lesson here.

### 3. The declared fallback still describes the old game

```
Source/LLK/Public/Characters/LLKCharacter.h:271-277
  FallbackWalkStuds   = 16.f    data authors 8      448 vs 224 cm/s
  FallbackSprintStuds = 23.f    data authors 11.5   644 vs 322
  FallbackCrouchStuds = 9.f     data authors 4.5    252 vs 126
```

`7426625` edited `FallbackStumbleMultiplier` in this same file and left these
behind.

The fallback is not dead code — it is the path taken whenever `DA_Lantern` does
not resolve, **which is the state the project is in right now.** Until the
assets are regenerated, a run either reads a stale asset at the old scale or
falls back to the old scale. Both roads lead to 448.

After regeneration it stays a trap: any machine without the regenerated asset
plays the whole game at double speed. And the warning it prints is confident
and wrong — it names 448/644/252 as the "DECLARED FALLBACK", which is no longer
anybody's intent. The comment beside that log states the promise this breaks:

> gameplay reads the data rather than a constant that happens to match it today

It matched yesterday. **The fallback is only honest while it matches**, and
nothing was enforcing that, which is why the guard now does.

The escape invariant is worth checking on this path too. The roster halved, so
on the fallback the player is *faster* relative to enemies, not slower — the
invariant is not violated, but it is also not the game. `movement.test.mjs`
asserts the invariant against `data/`, and the fallback is a second set of
numbers it does not see.

### 4. The importer verifies walk, and only walk

`VerifyRuleAssets` re-reads the generated `DA_Lantern` and compares it
semantically against the JSON it came from. That is exactly the right idea, and
it compares `walkSpeed`, `radiusStuds`, `intensity` and `warmColor.r`.

**It does not compare `sprintSpeed` or `crouchSpeed`** — the two values this
commit made fractional, and therefore the two most likely to be wrong. If the
import wrote them badly, the importer would still report success.

`VerifyTable`, which guards `DT_Enemies`, is thinner still: it compares **row
names and row count, and no field value at all.** Every `moveSpeed` in that
table could be zero and it would pass. That is precisely why
`tests/enemies.test.mjs` has to scan the `.uasset` bytes from outside the
engine — the engine-side check cannot be relied on for values.

This is the trap this repository keeps rediscovering, row X3's shape: **a
generator that reports success while what it generated is wrong.** Here it is
inside the verifier that exists to prevent it.

## What should happen, and by whom

The fix for 1 and 2 is **not** to edit `448.f` to `224.f`. That resets the
clock until the next rescale and leaves the same defect in place. Both should
read the resolved speed — `ALLKCharacter::GetWalkSpeed()` is already a public
accessor, and `FLLKLanternFeelInput` would need one more field carrying it,
populated where `SpeedCm` already is in `LLKLanternLightComponent.cpp`.

The fix for 3 is to make the declared fallback equal the authored data **by
construction** rather than by hand — and until that exists, the guard catches
the divergence.

The fix for 4 is three more comparisons in `VerifyRuleAssets` and at least one
field comparison in `VerifyTable`.

**None of these were made here, deliberately.** They are C++ and this session
cannot build: landing unbuilt code on a branch other writers are on is the
collision row 27 exists to prevent, and `docs/patches/README.md` says the same.
`tests/speed-scale.test.mjs` is the handoff instead — it names every file, line
and intended value, and it fails until they are fixed.

The values themselves remain Chris's. Nothing above proposes a new speed; every
target number is one already authored in `data/lantern.json`.

## Still blocked on the editor

1. **Regenerate `Content/Data/DA_Lantern.uasset` and `DT_Enemies.uasset`** from
   `import_data.py`'s payloads. `tests/enemies.test.mjs` goes green when the
   assets carry the new floats, and it should be fixed that way rather than by
   touching the test.
2. **Confirm `DA_Lantern` took** from the possession log — bearing in mind the
   `%.0f` caveat above.
3. **Play the dressed circuit at half speed.** The 28 m room should take about
   12.5 s rather than 6.2. This is what the dressing pass could not be judged
   without, and the central aisle is 573 cm — see
   `FINDINGS-BOSWELLS-DRESSING-PASS.md`.
