# Finding — the gameplay light anchor may be one capsule half-height too high

> **Status: OPEN, 2026-11-04. High confidence, not confirmed.** One editor read
> settles it and the editor is open but unreachable (no MCP bridge, Chris
> remote). **Nothing was changed.** The fix is one line and *which* line is a
> decision for whoever owns the lantern, which is why no test asserts a choice
> yet.

Found while speccing the retail-floor swap in
`FINDINGS-BOSWELLS-READS-AS-STOCKROOM.md`. It is not an art finding; it
surfaced there because the current dressing is what hides it.

## The arithmetic

`LLKLanternFeel.cpp:7` authors:

```cpp
const float KeeperEyeHeightCm = 152.f;
```

Its header says the number is **"derived from two engine defaults."** There is
exactly one pair of UE `ACharacter` defaults that produces 152:

```
capsule half-height 88  +  BaseEyeHeight 64  =  152
```

The same default set has `CapsuleRadius 34`, giving a **68 cm capsule** — which
is the width this project measured independently and used to judge the
`WL_DarkPocket_Partition` pinch at 60 cm. So the defaults are the ones in play,
and nothing in `Source/` or `Config/` calls `SetCapsuleSize` to change them.

**So 152 is eye height above the floor.** That is the correct number.

But the anchor is built as:

```cpp
// LLKLanternFeel.cpp:43
FVector GameplayLightPosition(const FVector& CapsuleLocation, float EyeHeightCm)
    return CapsuleLocation + FVector(0.f, 0.f, Finite(EyeHeightCm));
```

and the caller passes `Owner->GetActorLocation()`
(`LLKLanternLightComponent.cpp:76`). For a `ACharacter`, actor location is the
**capsule centre**, 88 above the floor. So:

| | above floor |
| --- | --- |
| where a keeper's eyes are | 152 |
| where the anchor lands | 88 + 152 = **240** |
| error | **+88, exactly one capsule half-height** |

The 88 is counted twice: once inside the authored constant, once by the origin
it is added to.

## Why this has never been noticed

**The current dressing hides it.** The placed pallet racking is 250 cm tall. At
250, an anchor at 240 and a true eye at 152 are *both* blocked, so every
sightline, every light-field read and every route probe gives the same answer
either way. The bug is unobservable in the map as it stands.

Low retail shelving is what exposes it:

| Shelf | anchor at 240 | a true eye at 152 |
| --- | --- | --- |
| gondola 48" (122 cm) | clears | clears |
| gondola 54" (137 cm) | clears | clears |
| gondola 60" (152 cm) | **clears** | blocked |
| gondola 72" (183 cm) | **clears** | blocked |
| gondola 78" (198 cm) | **clears** | blocked |
| pallet racking (250 cm) | blocked | blocked |

So on a dressed sales floor the keeper's light would sail over every shelf in
the building while the keeper could not see across any of them. **The art
change would have been blamed for it**, because that is the commit it would
first appear in.

## Why it matters more than a cosmetic offset

The header states the stakes itself:

> `GetLitLightSources` reads this, so the storm, the light field, the Warden's
> convergence and relic legibility all judge the position the keeper actually
> occupies.

Four systems judge from this point, and the header's whole argument for the
constant living in one place is that a drifting copy *"decides what enemies
hunt"* and would be noticed last. The single-authority decision was right. The
origin it is applied against is the part that was not checked.

The second caller has the same shape. `LLKMapReadiness.cpp:299` builds its
sightline eye as `KeeperSpawn.Z + EyeHeightCm`, where `KeeperSpawn` is a
`PlayerStart` location — also a capsule centre when the actor sits on the
floor. The two copies agree with each other, exactly as the header intended,
and that is why agreeing was never enough.

## The test asserts the current behaviour

`LLKLanternFeelTest.cpp:92` pins it:

```cpp
const FVector Capsule(1200.f, -400.f, 90.f);
TestEqual(TEXT("the gameplay anchor is the capsule plus the authored eye height"),
    Anchor, FVector(1200.f, -400.f, 90.f + 152.f));
```

The fixture's `Capsule.Z` of 90 is itself about a capsule centre height, so the
test encodes the same double-count and would keep passing after a fix that
changed only the call site. **Guarantee 3 is sound** — it proves no cosmetic
value reaches the anchor, and that is a different claim from the anchor being
in the right place. The guarantee is about *purity*, not *position*, and it
holds either way.

## What would settle it, in one editor read

1. `GetCapsuleComponent()->GetScaledCapsuleHalfHeight()` on a possessed keeper.
   If it is 88, the arithmetic above is confirmed.
2. `GetActorLocation().Z` on that keeper standing on a known floor, and the
   `PlayerStart`'s own Z in `LLK_Boswells_FirstRun`.
3. `GameplayLightPosition(...)` against the floor under it.

If half-height is 88, the anchor is 240 and the finding stands.

## Two fixes, and the choice is not mine

- **Change the call site.** Pass `GetActorLocation() - FVector(0,0,HalfHeight)`
  — the capsule's foot — keeping 152 as "above the floor." Correct for a
  crouched keeper too, because UE shrinks the half-height and the feet stay
  planted, so the eye drops with the crouch on its own.
- **Change the constant's meaning** to 64, an offset from the capsule centre.
  Fewer moving parts, but it silently stops working the moment anything
  authors a non-default capsule, and it makes the name wrong.

The first is better and it is still a judgement call, so it is left open rather
than taken here. Either way `LLKLanternFeelTest` needs its fixture rebased from
a capsule centre to a floor, or it will keep asserting the double-count.

## Consequence for the crouch mechanic, recorded while it is in view

`data/lantern.json` calls crouch *"a deliberate slow mode for tight shops and
careful approach"* and `movement.test.mjs` asserts *"crouch does NOT outrun a
Sweeper, so crouch is concealment and not escape."* Crouch goes through UE's
own `Crouch()`/`UnCrouch()` (`LLKCharacter.cpp:712-729`) with no second source
of truth, which is the right call — so the capsule does shrink and the anchor
does drop with it.

But against 250 cm racking a standing keeper is already fully hidden, so
**crouch currently buys no concealment it did not already have.** It costs
43.75% of walk speed and returns nothing but a smaller silhouette nothing is
testing against. Low shelving is what gives crouch its authored job for the
first time — which is an argument *for* the retail floor, and a second reason
the anchor origin should be settled before that pass runs rather than after.
