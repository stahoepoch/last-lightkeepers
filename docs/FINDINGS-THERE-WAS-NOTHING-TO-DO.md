# There was nothing to do, and the verbs were compiled all along

**2026-11-24.** Chris, having played the opening map himself:

> *"I was kind of burned out. I just want to have a fun game to play, and it's
> kind of, I don't know, not fun. I guess because there's nothing to do. It
> feels kind of like something I'd see in 1996 or something."*

Also, and this ends a caveat that has closed every findings file in this
directory for a week:

> *"I played it yesterday. I looked at it, walked through it, that's when I
> gave you all those feedback."*

**Somebody has played it.** Every document before this one closes with
*"Nobody has played any of it"*, and each was true on the day it was written.
It is not true any more, and the complaint that arrived with the play session
is worth more than all of the measurements that preceded it.

He also said the climb works: *"that was cool... it wasn't like an animated
feel to it, just kind of climbed up it."* Noted and not touched - a mantle
with no animation is a known, named gap, not a defect.

---

## 1. The count behind "nothing to do"

`LLK_Boswells_OpeningPass` holds **3,063 actors**. These are all of the ones
that do anything:

| actor | n |
| --- | --- |
| `ALLKRelicActor` | 4 |
| `ALLKStoreElevator` | 2 |
| `ALLKLanternPickup` | 1 |
| `ALLKOpeningGrille` | 1 |
| `ALLKSweeper` | 1 |
| `ALLKExtractionZone` | 1 *(a leftover of the co-op raid)* |
| **`ALLKBreakerPanel`** | **0** |
| **`ALLKPushableFixture`** | **0** |

Three thousand pieces of department store, ten things to touch, and **the two
verbs that would make the floor a place you act in were in no level at all.**

`ALLKBreakerPanel` is not a stub. It is a finished class with a one-way
switch, a replicated thrown state, a noise window, and a header that argues
carefully about mounting a handle at a child's reach. `ALLKPushableFixture` is
the same. Both are covered by the test suite. Neither has ever been pressed,
because neither has ever been in a map.

**This is the project's own failure shape, one layer up.**
`Tools/verify_boswells_ground_cells.py` named it a week ago: *a correct system
over an absent thing, with a clean log.* Every previous instance was a system
over absent geometry - lights over a missing ceiling, racks that were not
there, fittings wearing a circuit their light had left. This one is the same
shape pointed at the gameplay, and it is the most expensive version of it:
`docs/ROADMAP.md` goal 5 is *"find a story clue, change part of the building,
and reach Sayers"*, and restoring power is that beat in its most literal form.
The reason three days of lighting work never felt like a choice is that **there
has been no way to end the dark.**

### What changed

`Tools/place_boswells_first_verbs.py`:

- **One breaker**, on `ROUTE_SERVICE_WALL_W` in the north service corridor -
  traced out of the map rather than typed in. It sits **before** the crossing,
  not after it, which makes it a decision instead of a reward: the keeper can
  light the floor they are about to walk, and pay for it with
  `ULLKPowerSubsystem::PowerOnNoiseStuds` - **70 studs**, against a pushed
  fixture's 16. Every machine on the floor hears the building wake up.
- **Three pushable fixtures** on the ground floor, 60 to 110 cm, all under
  `LLKClimbFeel::MaxLedgeHeightCm` so their tops are ledges the climb accepts.
- **One emergency light in the service corridor**, because the first render of
  the finished breaker came back as **an entirely black frame**. The corridor
  had no light of any kind. A breaker nobody can find is a breaker nobody
  throws, which would have been this same failure shape again, inside the fix
  for it.

### What was deliberately not done

**The breaker is not gated behind the climb**, although its own header
suggests it. `ReachCm` is an eye-to-handle *sphere*: with the eye at 124 cm
and a body that can stand 40 cm from a wall, every handle below **268 cm** is
reachable standing, however high it is authored. Gating on height therefore
needs either an absurd mounting or a reach so short (~60 cm) that the prompt
vanishes when the keeper stands slightly wrong. The handle is at **196 cm**,
where a real panel is, and the gate is one number away whenever Chris wants
it. **A puzzle invented to justify a verb is not a puzzle.**

## 2. The exit signs, and why they were white

> *"Emergency fittings are cool, but I don't have that like the exit signs and
> stuff with that orange glow that you normally see with emergency lights."*

The store had four things with EXIT in the label. Two were faces on the
children's deck wearing a material called `burgundy`. None of them glowed.

`Tools/sign_boswells_exits.py` hangs a sign under every exit header that
exists - four mall entrances and the dock staff door - and repaints the two
that were already there. Each sign is **lit in both power states**, which is
the whole point of an exit sign: its light carries `LLK_Emergency` and its
body must *not* carry `LLK_MainsFixture`, or throwing the breaker would switch
off the one fixture in the building that is supposed to be reliable. The tool
asserts both.

### The emissive scale is wrong by about twenty times, and that is the find

The first sign went in at `Emissive = 4.0` and rendered as a **white**
rectangle. Halving it to 1.8 changed almost nothing. The sweep:

| authored emissive | what the render shows |
| --- | --- |
| 4.00 | white, with an amber halo |
| 1.80 | cream |
| 0.35 | amber-yellow |
| **0.15** | **orange** |
| 0.12 | orange, slightly flatter |

This floor peaks at **0.0005** measured brightness. Auto-exposure opens all
the way up for it, and anything emissive above roughly 0.2 clips to white
before a viewer ever sees its colour.

**That also answers a question I asked Chris twice and had wrong both times.**
`docs/FINDINGS-THE-UPPER-STOREY-HAD-NO-HEIGHT.md` ends by offering to dim
`MI_LLK_Fixture_Emergency` from 2.5, as though the problem were a value that
was somewhat too high. It is not. At 2.5 the fitting is roughly **seventeen
times** past the point where colour survives, which is why every emergency
tube has rendered as a white blob for three days and why nothing in this
building has had a colour at the fitting itself. The honest fix is a scale
change across the fixture materials, not a nudge.

**It is left alone anyway**, for the reason it was left alone before: that
value is shared by both storeys, and re-authoring the whole emergency layer is
a pass of its own with its own before-and-after pictures - not a side effect of
hanging five exit signs.

## 3. Two things that rendered as holes

Both new objects first went in on borrowed materials, and both came back from
the render as **pure black rectangles**: the breaker on `M_BSW_DarkChrome`,
the crate on the service carpet. A black rectangle is not a breaker and not a
crate - it is the absence of one, which is exactly what Chris objected to on
the upper storey, where seven engine cubes on a default material were the most
visible thing in every render of it.

`MI_LLK_Panel_Breaker` (grey enamel) and `MI_LLK_Crate_Stock` (cardboard) are
dull diffuse instances off `M_LLK_LightFixture` at `Emissive = 0`. The panel
is still a flat box with no handle or louvres modelled.

## 4. The exterior Chris asked for does not exist

> *"there's supposed to be like a couple exits or entrances from the outside to
> Boscov's... Definitely the entrance-exit to the mall on both top and bottom
> have that glass, but so do the exits to the outside... Plus the whole exit
> for the loading dock... if you actually looked at the whole Johnstown
> Galleria and everything, you could see how it's on a hillside and the things
> are weird because we don't even have like an exterior or anything."*

Correct, and nothing here addresses it. The level has **no exterior shell, no
exterior doors and no outside**. The sign tool reports each named exit it
looked for and could not find rather than hanging a sign over a wall. The
upper-storey mall entrance is also absent: both mall headers that exist are at
ground level.

## What is measured and deliberately not fixed

**The whole fixture emissive scale**, above. One pass, both storeys, before
and after pictures of each.

**The breaker is reachable standing**, by design for now. One number gates it
behind the climb when that is wanted.

**The four relics are 50 cm engine cubes on `WorldGridMaterial`.** They are
four of the ten things in this level that do anything, and they are untextured
grey boxes.

**`ANCHOR_LOADING_DOCK_EXIT` is still an `ALLKExtractionZone`**, from the
co-op raid `AGENTS.md` calls history. Removing it is a decision about the dock
chapter, not a side effect of placing a breaker.

**The two repainted children's-deck exit faces have no light under them.**
They glow and cast nothing.

**184 overlapping prop pairs on the deck and 261 on the ground floor**,
unchanged.

**Somebody has played it, once.** Every number here is still a measurement
against a saved map and every picture is still a render - but the thing that
set this work going was a person walking the floor, and that is the only test
that has ever found this particular defect.

---

**Corrected 2026-11-25.** Section 2's conclusion about the fixture materials
was wrong, and a sweep of the actual fitting says so.

The claim was that `MI_LLK_Fixture_Emergency` at 2.5 is "roughly seventeen
times past the point where colour survives, which is why every emergency tube
has rendered as a white blob **and why nothing in this building has had a
colour at the fitting itself**." The first half is true of the tube's own
face, where 26.5% of its pixels are fully clipped. **The second half is not
true of the room.** Cutting that value seventeenfold moves the lit ceiling by
0.04 of 255 and the frame median not at all: this store is lit by its placed
lights, and the fittings contribute nothing measurable to it.

So the "honest fix" that section called for - "a scale change across the
fixture materials" - would have changed nothing a player could see. The real
defect was narrower and the real fix was free. The emergency fitting is now at
**0.40**, where its face stops clipping and the room does not move.

See `docs/FINDINGS-THE-FITTINGS-WERE-NOT-THE-PROBLEM.md`.

**Section 4 is answered**, too: the exterior Chris described now exists. See
`docs/FINDINGS-THE-STORE-HAD-NO-OUTSIDE.md`.
