# The lantern's cone is clamped to a hemisphere, and a test passes on the number it ignores

**Status: confirmed 2026-11-15 by reading the engine, not by relay.** Codex
raised this; the two checkable halves of it are both true, and the engine line
that decides it is quoted below. Nothing is changed — the fix is a visible one
and the value is Chris's.

## 1. The authored cone cannot do what its own comment says

`data/lantern.json` authors `coneAngleDegrees: 110`, and its note says the
cone is *"deliberately broad enough to read as a hooded carried source rather
than a flashlight beam, **while still leaving darkness outside the keeper's
chosen direction**."*

`LLKLanternLightComponent.cpp:285` passes that number straight through:

```cpp
SetOuterConeAngle(Level.ConeAngleDegrees);
```

**Unreal's outer cone is a half-angle, measured from the light's axis, and the
engine clamps it.** `UE_5.8/Engine/Source/Runtime/Engine/Private/Components/SpotLightComponent.cpp:39`:

```cpp
ConeAngles.Y = FMath::Clamp(InOuterConeAngle * UE_PI / 180.0f,
                            ConeAngles.X + 0.001f,
                            88.9f * UE_PI / 180.0f + 0.001f);
```

So 110 becomes **88.9°**, and a 88.9° half-angle is a **177.8° cone** — very
nearly a full forward hemisphere. There is no direction the keeper can face
that leaves darkness beside him. The authored intent and the rendered light
disagree, and the data file has been describing a lantern the engine has never
drawn.

## 2. The test passes on the number the renderer discards

`LLKLanternLightTest.cpp:90`:

```cpp
TestEqual(TEXT("the authored angle is the outer cone"), Light->OuterConeAngle, 110.f);
```

That reads the **property**, which really is 110. The clamp happens on the way
to the renderer, so the property and the image disagree and the test only ever
sees the half that agrees with it.

This is the day's recurring shape, in a new costume: a check that consults
something real and still proves nothing about what the player gets. The same
family as counting `box_elems` on a mesh with no physics body, and as
asserting a trace height against a character who had been replaced.

## 3. The second light is real too, and it was always marked temporary

`LLKCharacter.cpp:179` builds `LanternCoreGlow_Temporary`: a **15 cd point
light, 160 cm attenuation radius, shadows off**, parented to the lantern
anchor, alongside the 25 cd spot. Shadowless fill inside 160 cm is a flat wash
on everything within arm's reach, which is a good description of "puffy and
close". The component's own name says it was never meant to survive a real
lighting pass.

The spot also sets no source radius, so it is a mathematical point and its
shadows are perfectly hard. That is a shadow-softness question and **not** a
cause of the oversized wash — those are two separate findings and should not
be fixed as one.

## What the fix is, and why it is not applied here

The name `coneAngleDegrees` and its note both describe a **full** cone. The
code passes it as a **half** angle. Two ways to make them agree:

- **Halve it in code** — `SetOuterConeAngle(Level.ConeAngleDegrees / 2)` — so
  the authored 110 means a 110° cone, which is what the file says it means,
  and Chris's number does not move.
- **Re-author the value** to 55, leaving the code as a half-angle setter.

The first keeps the data honest and the second keeps the engine's convention
visible. Either way **the lantern will look materially different the moment it
lands**, which is why this is a report and not a commit: it is a visible
change to the thing Chris tunes by eye, and Codex's own recommendation is that
replacement angles be chosen in a controlled visual pass rather than inferred.

Worth adding to that pass: a check that reads the **clamped** cone rather than
the authored property, so the next angle cannot be wrong in the same silent
way.
