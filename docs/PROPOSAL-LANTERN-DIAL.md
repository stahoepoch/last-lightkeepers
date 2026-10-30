# Proposal — an adjustable lantern, and the two different ideas inside it

Written 2026-10-30 by Claude, from Chris's question: *"i almost want to see an
adjustable lantern light, like we could lower it, heighten it... i think even the
slightest light charges an enemy and they move, but the brighter the more
powerful the faster the worse?"*

**Status: PROPOSED. Nothing below is built.** It is written down because one
half of it is the natural next step and the other half reverses a row Chris
locked yesterday, and those two facts should not be discovered during
implementation.

---

## The trap: a brightness dial that moves `intensity` does nothing

The obvious build is a slider over `light.intensity` in `data/lantern.json`.
**It would have zero gameplay effect**, and it would appear to work.

`docs/LIGHTING-IN-UNREAL.md` has the derivation. `LLKLightField::BrightnessAt`
is `Σ I / (1 + d²/r²)`, and every threshold in the game is `I * 0.5f`
(`ULLKRelicComponent::IdentifyThreshold`, `LLKPerception::LitThresholdFrom`), so
"lit" reduces to `d < r`. **`I` cancels.** Turn the lantern down to a candle and
relics stay exactly as readable, Sweepers notice from exactly as far.

So the dial has to move **`radiusStuds`**, which is the real reach. That part is
easy — one authored number, already in data, already the thing `d < r` compares
against.

## The catch that makes the easy version not work either

A dial over radius alone still does not do what Chris is describing, because of
a detail that is invisible until you look for it.

`ALLKSweeper` asks for brightness **at the keeper's own position**
(`LLKSweeper.cpp:187`). The keeper's own lantern is at distance zero, so
brightness there is `I / (1 + 0) = I`, and the threshold is `I / 2`.

> **A keeper whose lantern is on is lit, always, at any radius.**

Shrink the radius to a hooded eight studs and you see less, read less, and are
still noticed by every Sweeper from the full lit 38. All cost, no benefit. The
hood the question is reaching for — *"stay dim, read the relic at arm's length,
don't announce yourself"* — cannot exist while the gate at the keeper is binary.

**So Chris's two ideas are one change.** "Lower it and heighten it" and "the
slightest light charges them, brighter is worse" are not a feature and a
refinement; the second is what makes the first mean anything.

---

## The half that is already decided against: *faster*

Row 72, LOCKED 2026-10-29: *"Light never changes its rate; light creates only
local consequences."* Row 73, LOCKED the same day: *"Nothing a player does moves
it."*

Brightness driving the clock is `LanternMultiplier`, and that function existed —
Chris had it **deleted** as part of row 72, along with `WardenMultiplierFrom`,
`GetCurrentMultiplier` and the whole spike machinery. `Advance()` is now
`Pressure += BaselineRatePerSecond * DeltaSeconds` and nothing else.

His reason is still good, and it is better than the thing it replaced: a clock a
player can accelerate makes the Warden a punishment for playing, and the price
of light is better paid where the player can *see* it being charged. Reopening
this is Chris's call, but it should be reopened on purpose, not arrived at by
building a dial.

**Recommendation: leave the clock alone.** Nothing in the dial needs it.

## The half that fits, and is the real proposal: *worse*

Row 72 endorses exactly the other half — *"light creates only local
consequences"* — and the local consequence is already authored:

| | studs |
| --- | --- |
| `aggroRadiusDark` | 12 |
| `lantern radiusStuds` | 30 |
| `aggroRadiusLit` | 38 |

That asymmetry is row 5 made physical, and `_radiusNote` says so: lit, you are
noticed from eight studs **beyond** the edge of your own light. Danger arrives
out of the dark you just announced yourself to.

The change is to make those two radii the ends of a range rather than two
states:

```
Radius = Lerp(AggroRadiusDark, AggroRadiusLit, BrightnessFraction)
```

At a hooded setting a keeper sits at ~20 — noticed from further than a dark
crew, nearer than a bright one. **That is Chris's sentence, exactly: the
slightest light moves them, brighter is worse.** No clock, no new enemy, no new
data file; `data/enemies.json` already authors both ends.

---

## Three costs, stated plainly

**1. `intensity` stops being a free visual knob.** This is the real price.
`docs/LIGHTING-IN-UNREAL.md` rule 2 — *"`radiusStuds` is gameplay, `intensity`
is not"* — is what makes the cone-softening and exposure work safe to do by eye
right now. The moment brightness is a continuous gameplay input, every value in
that block is a balance change and the lighting pass needs tests instead of
taste. **Do the look first, then this.**

**2. A slider is less teachable than a switch.** Row 43 makes the store the
tutorial, and a step function teaches: *lantern on means they come from much
further.* On a continuous slider a player cannot tell whether 40% was the
mistake. **Recommendation: 2–3 named positions, not a slider** — hooded /
normal / open, each an authored radius. Discrete states can be named, learned,
and argued about over voice chat, and the lerp above still does the work between
them.

**3. We would not know if anyone used it.** `ELLKRunEvent` has `LanternOn` and
`LanternOff` and no notion of a level, so M4 would measure a dial it cannot see.
One event (`LightLevelChanged`, with the level in `Value`) and `LitFraction`
becomes a weighted average instead of a lie. Small, and it belongs in the same
change rather than after it.

---

## What I would do

1. Nothing yet. Finish the exposure clamp and the cone, while `intensity` is
   still free to move.
2. Then build it as **three positions, lerped aggro, clock untouched**, behind
   the telemetry event, and play it.
3. Only then write a row — because this one should be locked after Chris has
   held the dial, not before.

The question it has to answer is the one it is for: does a hooded setting make
*"risk one more pickup"* a better decision, or does it just give players a way
to make the game dimmer and slower? That is a behaviour question, so it gets
measured (row 79), not debated.
