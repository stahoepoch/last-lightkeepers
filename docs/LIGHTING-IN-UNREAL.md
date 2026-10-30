# Lighting in Unreal — what may be tuned, and in what order

Written 2026-10-30 by Claude, after the dock bay's first lit PIE session read
as *"a harsh white flashlight cone"*.

`DONTFUCKUPTHELIGHTS.md` at the repo root is the ancestor of this file and is
still right about its subject. It is also **written for Roblox**: its two rules
govern `Ambient` and `EnvironmentDiffuseScale` in `default.project.json`, and
neither setting exists in Unreal. So the one document in this project that says
*"read this before you touch lighting"* currently offers no Unreal guidance at
all, which is how the cone got tuned by guessing. This file is the Unreal half.
The old file's thesis carries over unchanged and is worth repeating first:

> The lighting has been broken and re-broken several times, each time by
> someone changing one number in good faith.

---

## The invariant: lantern intensity is a visual knob, not a gameplay one

This is the important thing on the page, because the obvious reading of the code
is the opposite and acting on it would cause a silent rebalance.

`ULLKZonePressureSubsystem::GetLitLightSources` sets
`Source.Intensity = Light->GetResolvedRules().Intensity` — the **same** authored
250 that `ULLKLanternLightComponent` hands to `SetIntensity()`. One number, read
by the renderer and by the gameplay light field both. That looks exactly like
the coupling that should be split before anyone touches the look.

**It is not, and it must not be split.** Both consumers of brightness derive
their threshold from the same intensity, deliberately and with the derivation
written down in two places:

- `ULLKRelicComponent`: `IdentifyThreshold = LanternIntensity * 0.5f`
- `LLKPerception::LitThresholdFrom`: `LanternIntensity * 0.5f`, commented
  *"the same derivation ULLKRelicComponent uses, on purpose"*

Work it through. `LLKLightField::BrightnessAt` is `Σ I / (1 + d²/r²)`. For one
lantern, a point is lit when

```
I / (1 + d²/r²)  >  I / 2
        1 + d²/r²  <  2
                d  <  r
```

**`I` cancels.** "Lit" means "inside the attenuation radius", and it means that
for any intensity whatsoever. Relic legibility resolves identically, because it
is the same inequality. `FindBrightestPosition` is a comparison between sources,
so a uniform scale cannot reorder it. Every light in the game — keeper lanterns,
the anchor-revive pulse, the extraction rig — draws its intensity from the same
lantern figure (rows 72 and 75), so the scale really is uniform.

Three consequences, and they are the whole point of this section:

1. **`light.intensity` in `data/lantern.json` can be changed freely for looks.**
   Nothing about perception range, relic legibility or Warden convergence moves.
   It is a render value that gameplay happens to normalise against. The JSON
   already half-says this: *"the one key here with no portable meaning."*
2. **`light.radiusStuds` is the gameplay number.** It is what `d < r` compares
   against, and it is the real "reach". Changing it moves the distance a keeper
   is noticed from, the distance a relic can be read at, and where the Warden
   goes. Treat it as a balance change and expect `tests/` to argue.
3. **Do not introduce a separate visual intensity.** It is tempting, it looks
   tidier, and it would break the cancellation above: the moment the renderer
   and the field read different numbers, the thresholds stop being derivable
   from what the player can see, and "enough light to read by" becomes a magic
   constant somebody has to maintain. The split people reach for already exists
   — it is intensity versus radius, not visual versus gameplay.

If the look ever genuinely needs the renderer and the field to disagree, the
honest change is to make the *thresholds* read the field's intensity explicitly
rather than quietly inheriting it, and to say so here. Nobody has needed that.

### Row 80 strengthened this rather than breaking it

**Added 2026-10-30.** Chris's three named lantern levels looked like they would
cost this section: a dial over brightness makes brightness a gameplay input,
and then every candela figure on this page is a balance change.

His architecture avoided it, and the avoidance is the interesting part. *"Each
level needs a separate authored 'detectability' value, not just
intensity/radius."* So the number enemies read is authored per level
(`FLLKLanternLightLevel::Detectability`, 0 to 1) and **nothing derives it from
intensity**. The consequence is that `intensity` is now freer than when this
file was written:

- **Relic legibility** cancels it, by the derivation above. Unchanged.
- **Perception does not read it at all.** `LLKPerception::LitThresholdFrom` is
  deleted; `LLKLightField::DetectabilityAt` consults radius and detectability
  and never intensity. `LLK.Lantern.Levels` asserts this directly by moving
  intensity from 10 to 400 candelas and requiring every enemy's reach to stay
  put.

The trap it was closing is worth recording, because two reasonable builds of a
brightness dial both do nothing. A dial over `intensity` cancels. A dial over
`radiusStuds` fails for a subtler reason: `ALLKSweeper` asks for light at the
keeper's *own* position, where their own lantern sits at distance zero, so
brightness there is `I` against a bar of `I/2` — cleared by a factor of two at
any radius at all. **A keeper holding a lit lantern was lit, unconditionally.**
Hooding would have cost sight and bought no concealment.

**One thing did change, and it is a design consequence rather than a code
one.** The old threshold made "lit" mean exactly `d < r`, so a teammate exposed
you out to their authored radius and *not one stud further* — a cliff, and a
very teachable one, pinned on purpose in `LLK.Enemies.PerceptionFailures`.
Detectability has no cliff. `LLKLightField`'s own header forbids one, and
Chris's *"even the slightest light charges an enemy and they move"* asks for the
opposite of one. So the teachable rule is now **"distance from their light
always helps"** instead of "stay outside their light". Softer, continuous, no
boundary to game. Whether it teaches as well as the cliff did is a playtest
question (row 79), and the test now pins monotonic decay rather than a
crossover.

---

## Why it renders white, and the order to fix it in

The cone reads as a white flashlight while `warmColor` is authored
`1.0, 0.82, 0.56` — a distinctly warm value. The colour is not the problem.

**Auto-exposure is.** Unreal's eye adaptation adapts to the scene it is given,
and the scene is now *correctly* very dark — "lantern-off is genuinely dark" was
the goal and it was reached. So exposure climbs until something is bright, and
the only bright thing is a 250-candela spot. All three channels clip to 1.0, and
three clipped channels are white regardless of the tint that produced them. The
warmth survives only at the dim edge of the falloff, which is exactly what a
blown-out centre with warm fringes looks like.

This is why **tuning intensity first does not work.** Lower it and eye
adaptation re-adapts to the new dark and re-clips the cone; the picture ends up
in the same place and the number that moved was the wrong one. Fix in this
order:

1. **Clamp exposure in the map.** A `PostProcessVolume` with min and max EV100
   pinned — equal, or nearly, so adaptation stops hunting. This belongs to
   whoever owns the `.umap` (row 41, ratified 2026-10-30), not to `Source/`. It
   is the only step that can be done wrong in a way the others cannot fix.
2. **Then tune `light.intensity` in `data/lantern.json`** until the centre
   holds its colour. Values are Chris's (`docs/LANES.md`); the invariant above
   is why this is safe, and the import commandlet is how it reaches the asset.
3. **Then soften the cone edge in code.** `InnerConeFraction` is `0.47f` in
   `ULLKLanternLightComponent.h` and is already documented there as code-side
   because *"it softens the falloff and changes nothing about reach"* — which is
   true and is the licence to change it. Lower it to widen the band between
   inner and outer cone; the hard rim comes from inner and outer being close.
4. **Last, a warm local glow if it is still wanted.** A small visual-only point
   light at the lantern, low radius, no shadows, never added to
   `GetLitLightSources()`. A glow the light field can see is a second light
   source the Warden will walk toward.

Steps 2 and 3 are cheap and reversible. Step 1 is the one that decides whether
either of them appears to do anything.

---

## The rules, in the old file's format

**1. Exposure is clamped in the map, not compensated for in the lantern.**
An unclamped scene makes every brightness value temporary.

**2. `light.radiusStuds` is gameplay. `light.intensity` is not.**
See the derivation above before trusting either half of that sentence. Since
row 80 there is a third: `detectability` is gameplay too, it is authored per
level, and nothing may ever compute it from a brightness.

**3. A cosmetic transition may move the light. It may not move the reach.**
Chris's rule for levels: *"transitions smoothly lerp visually, while gameplay
resolves to the named state."* `SetAttenuationRadius` is a cosmetic knob now
that a fade can touch, so gameplay reads
`ULLKLanternLightComponent::GetGameplayRadiusCm()` instead - the same seam
`GetGameplayLightLocation()` is, in the other axis. A fade that reaches relic
legibility or a Sweeper's reach is this project's oldest bug shape wearing a
new coat.

**4. Nothing cosmetic joins the light field.**
`GetLitLightSources()` is what the Warden and every Sweeper read. A glow, a
muzzle flash, a UI bloom or an emissive sign that ends up in it is a lure
nobody authored. `tests/light-authority.test.mjs` guards the related rule that
the cosmetic transform never becomes the gameplay anchor.

**5. The lantern's unit stays pinned in code.**
`IntensityUnits = ELightUnits::Candelas` is set in the constructor with its
reasoning attached: Unreal's default is Unitless with a normal value of 5000, so
an authored 250 means nothing until the unit is fixed. Do not move it to
`data/`.
