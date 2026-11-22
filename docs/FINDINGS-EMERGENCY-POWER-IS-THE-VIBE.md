# Emergency power is the vibe. Turning it on is the mechanic.

**2026-11-22.** Chris, after seeing Boswell's lit for the first time:

> *"What would it look like with only emergency lights on. That's the vibe we
> want. Only later can we turn the power on to see everything. So we need
> both... I envision being able to turn on the power at some point that we
> will need to. Even if it is a revisit after later progress."*

That note turned a lighting fix into a mechanic, and it is `ROADMAP.md`
chapter goal 5 in its most literal form: *"find a story clue, **change part of
the building**, and reach Sayers."* `HANDOFFS.md` M25.

---

## What was wrong with the first fix

`FINDINGS-THE-STORE-IS-UNLIT-NOT-EMPTY.md` added one fill layer tuned to make
the store readable. It worked, and it was the wrong brief: a readable store
looks like a shop with the power on, which spends the chapter's best moment
on frame one. The single layer is gone, replaced by two.

## The two states

| | `BSW_EMERGENCY_FILL_*` | `BSW_MAINS_*` |
| --- | --- | --- |
| tag | `LLK_Emergency` | `LLK_Mains` |
| at boot | **on** | **off** |
| intensity | 1.0–3.0 cd | 26 cd |
| colour | cold blue | neutral white |
| count | 20 | 28 |
| falls off | with distance from the mall entrance glass | no — mains lighting is designed to be even |

Emergency spots already in the map average ~16 cd, so the emergency fill is a
fifth of one of them. **Its only job is to lift the departments off absolute
black**: shapes, not contents. You can tell something is hanging there; you
cannot tell what colour it is. That keeps `ROADMAP.md` slice 2 — *"the chapter
remains navigable with the lantern off"* — true without showing the player
anything.

The mains are what a working department store actually runs, and the contrast
is the reward for crossing the store to find the breaker.

**Found by tag, not by name or a hand-kept list.** A renamed light stops being
a mains light, which is correct; a new one joins by being tagged, which is
also correct. A manager actor holding an array would be a second place for the
map to be wrong.

## The switch, and what it costs

`ALLKBreakerPanel` is **the second user of the `ILLKInteractable` seam**,
which is the whole point of having built it — a pushable and a breaker share
nothing except that a keeper presses E on them, and neither needed a line of
change in `ULLKInteractorComponent` or in the HUD.

It is **one way**. A breaker you can switch back off lets a player undo the
price and keep the light, and there is no stealth value in darkness you can
restore at will.

The price is noise: **70 studs**, against a pushed fixture's 16 and a sprint's
12.5, and past the Sweeper's 38-stud lit sight so there is nowhere on the
sales floor to throw it from unheard. A whole floor's lighting striking at
once is the building waking up, not a local event. The noise lasts six seconds
rather than one frame, because a sound a listener has to sample on exactly the
right tick works by luck.

**The handle sits at 4.5 studs — 126 cm — which is a child's eye height.** A
breaker panel is mounted for an adult, and that number is authored so a level
author can raise it and make the player push something over and climb it
first. That is the push and climb verbs becoming a puzzle instead of two
demonstrations.

`LLK.Power 0|1` flips it from the console. It is a developer instrument and
**deliberately makes no noise**, which the help text says out loud, so nobody
tests with it and concludes there is no cost.

## Verified, in a running game

```
LogLLK: Display: Boswell's is on emergency power: 20 emergency light(s) lit,
        28 mains light(s) dark and waiting for someone to throw the breaker.
        Switching them on is heard 70 studs away.
LogLLK: The mains are ON - Boswell's is lit. 28 light(s) moved.
LogTemp: Display: Map readiness: ready.
```

Both states photographed from the same camera. Emergency: amber pools on the
racetrack, the escalator catching light, `LADIES` and `SHOES` barely legible,
masses of racking you can sense but not read. Mains: a working department
store — colour in the clothes, carpet under the racks, ceiling troffers lit.

41 automation successes, 0 failures. Seventeen JS suites green,
`power.test.mjs` 13/13.

## The hole this opened, named rather than hidden

`tests/cues.test.mjs` caught `OnPowerChanged` as a delegate nothing binds, and
the honest reason is a gap in the economy:

**The obvious subscriber is the light field, and it cannot be one yet.**
Enemies notice at `aggroRadiusLit` 38 against `aggroRadiusDark` 12 — more than
three times the reach — and `LLKLightField` only knows about lanterns. So a
building with every light burning registers as *dark* to every machine in it.

Today, restoring power costs noise once and nothing afterwards. **It should
also cost permanent visibility**, and that is the larger half of the price.
Until the light field knows about the building's own lighting, the lit store
is a reward with no ongoing tax, which makes the decision easier than it
should be.

## What this does not claim

- **Nobody has thrown the breaker in a real game**, because no level contains
  one. The subsystem is proven at boot — it reports what it found and moved 28
  lights on command — but the actor has never been placed. Same gap as the
  pushable, and one placement pass closes both.
- **The brightness of both states is tuned from stills.** Nobody has seen
  either in motion. Both are one idempotent tool run to retune.
- **The mains do not flicker, surge or fail**, and a building that has been
  dead for years almost certainly should do all three when it comes back.
- **It is one circuit.** A real store has many, and "power to *this* part of
  the building" is a better puzzle than one global switch. The tag system
  takes a second tag the day that is wanted.
- **The floor is still 76% one material**, which remains the largest untouched
  realism gap and is what Chris asked about twice.
