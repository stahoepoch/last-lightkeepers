# Finding — what a carried lantern actually does, and five places the model disagrees

> **Status: OPEN, 2026-11-04. Nothing was changed** — the feel model is gameplay
> code and fenced off from this writer. Written on Chris's ruling, now
> `DECISIONS.md` row 81: *"We're going for realism so if a lantern moves and
> it's supposed to move then it moves. We ought to find out what the real
> physics is of these things."*
>
> So this file finds out. The headline is that **the project modelled the hand
> correctly and the lantern not at all**, and that the error is not small: the
> sway rate is 1.7× too fast, the figure-of-eight is five times too tall, and
> sprint and walk are the wrong way round.

---

## 1. The object: what one of these weighs and measures

A keeper's lantern is a kerosene hand lantern on a wire bail — the Dietz
pattern, which is the thing this game's instrument is drawn from. Real
examples, from listings and museum records:

| Model | Height | Mass |
| --- | --- | --- |
| Dietz #76 "Original" cold blast (current production) | 10½″ / 26.7 cm | 1.25 lb / **0.57 kg** |
| Dietz "Little Wizard" | 12″ / 30.5 cm | — |
| Antique Dietz railroad lantern | 14½″ / 36.8 cm | 3.05 lb / **1.38 kg** |
| Dietz #999 (museum record) | 8¾″ body, 14″ with the bail up | — |

**Call it 0.6–1.4 kg and 27–37 cm tall**, fount at the base, bail ears at the
top. Sources disagree and they are per-model, so these are a range rather than
a spec; the mass barely matters for what follows, and the geometry decides
everything.

## 2. The physics: a lantern on a bail is a physical pendulum, ~1 Hz

Gripped by the bail, the lantern hangs from the ears and swings about them. That
is a compound pendulum, not a point mass on a string:

```
T = 2π · √( (d² + k²) / (g·d) )
```

where `d` is bail-to-centre-of-mass and `k` the radius of gyration about the
centre of mass. The fount is the heavy part and it sits at the base, so for a
27–37 cm body the mass centre is low — `d ≈ 0.15–0.22 m`:

| d (m) | k (m) | period | frequency |
| --- | --- | --- | --- |
| 0.12 | 0.07 | 0.805 s | 1.24 Hz |
| 0.15 | 0.08 | 0.881 s | 1.14 Hz |
| **0.18** | **0.09** | **0.952 s** | **1.05 Hz** |
| 0.22 | 0.10 | 1.034 s | 0.97 Hz |
| 0.25 | 0.11 | 1.096 s | 0.91 Hz |

**A carried lantern's natural frequency is about 1 Hz, and the whole plausible
range is 0.9–1.25 Hz.** (**Superseded by §2b**: this table pivots at the bail
ears and omits the bail’s own 13 cm, so the figures below run high. The working
number is **0.87 Hz**, not 1.05.) It is a flat result: the period goes as the square root,
so even a 2× error in `d` moves the frequency by 40%. Take **1.05 Hz** as the
working number and treat it as solid.

## 2b. Correction, 2026-11-04 — the pivot is the hand, not the ears

**§2 above pivots the pendulum at the bail ears, and that is wrong whenever the
keeper carries the lantern by the bail rather than by the ears.** A bail is a
wire hoop rising above the body; the hand grips it at the top of the arc, so the
hand is the pivot and the bail's own height is part of `d`. The published `d` of
0.15–0.22 m is ear-to-centre-of-mass and omits it.

**The omitted length was already in §1's own table.** The museum record for a
Dietz #999 reads *"8¾″ body, 14″ with the bail up"* — so **the bail adds 5¼″,
13.3 cm, above the body.** That number was sitting two sections above the
derivation that needed it and went unused, which is the whole of this mistake.

So `d` runs **0.25–0.35 m** rather than 0.15–0.22:

| d (m) | k (m) | frequency |
| --- | --- | --- |
| 0.25 | 0.09 | 0.94 Hz |
| **0.28** | **0.09** | **0.90 Hz** |
| 0.30 | 0.10 | 0.86 Hz |
| 0.35 | 0.10 | 0.81 Hz |

**Take 0.87 Hz as the working figure, not 1.05.** The headline is unharmed — a
lantern on a bail still swings at about 1 Hz — but it moves the one thing that
mattered, because the walk's lateral drive is 0.95 Hz and the lantern has just
moved from *below* it to *above* it.

| drive | r at 1.05 Hz | r at 0.87 Hz | Q=10 before | Q=10 after | phase |
| --- | --- | --- | --- | --- | --- |
| **walk, lateral (0.95 Hz)** | 0.90 | **1.09** | 4.9× | **4.5×** | **in phase → inverted** |
| walk, vertical (1.90 Hz) | 1.81 | 2.18 | 0.44× | **0.26×** | inverted |
| **sprint, lateral (1.45 Hz)** | 1.38 | 1.67 | 1.09× | **0.56×** | inverted |
| sprint, vertical (2.90 Hz) | 2.76 | 3.33 | 0.15× | **0.10×** | inverted |

**Two of §4's three conclusions get stronger and one changes character.**

- **(b) and (c) strengthen.** Vertical suppression goes from 0.44× to 0.26×, so
  the arc is flatter still — about **1/17** as tall as it is wide rather than
  1/10. And a sprint now transmits **0.56×**, so it does not merely fail to beat
  the walk: the lantern is actively *quieter* at a run than the hand driving it.
  §5's verdict that `SprintSwayCm` 5.2 against `WalkSwayCm` 2.4 is backwards
  becomes less arguable, not more.
- **(a) changes character, and this is the interesting part.** The walk is not
  comfortably below resonance at r 0.90; it is **essentially at resonance**,
  within about 10% on either side. The amplification hardly moves (4.9× → 4.5×),
  so "a walking keeper's lantern is the largest motion in the scene" survives
  intact. What flips is the **phase**: above resonance the lantern swings
  *against* the body rather than with it.

**Which side of resonance you are on is set by the grip, and that is a real
physical detail rather than a hedge.** Choke up and hold the bail near the ears
and `d` shrinks toward 0.2, putting f₀ back above the drive — in phase. Hold the
bail at the top of its arc and `d` is 0.3 and you are below the drive —
anti-phase. A keeper's hand is also not a frictionless pivot, and a loose fist
lets the bail rotate within it, which is strictly a two-link system whose lower
mode is what is modelled here. **So the honest spec for `f₀` is 0.85–0.90 Hz
with the phase left to fall out of whatever `f₀` is authored**, which the
second-order model in §5 does automatically — it is one more reason to let the
model produce the relationship instead of authoring the relationship directly.

**What this does to §5's table.** `SwayHertz` 1.6 against a 0.95 Hz drive is
unchanged (that comparison never used `f₀`), but if `SwayHertz` is to become the
lantern's natural frequency, the target is **~0.87, not 1.05 — so 1.6 is 1.84×
too fast rather than 1.68×.** Ring-down lengthens a little: `τ = Q/(π·f₀)` gives
**1.8 s at Q=5, 3.7 s at Q=10, 7.3 s at Q=20**, so "visibly swinging for two to
three seconds" becomes two to four.

**How this was got wrong, and it is the morning's shape again.** The derivation
was correct, the arithmetic was correct, and the input was a measurement of the
wrong length — taken from the right object. The check that would have caught it
is the one `ADVERSARY.md` now carries in a different form: a figure quoted two
sections above the calculation that needs it should be *used* by that
calculation or explicitly ruled out, and "14″ with the bail up" was neither.

## 3. The driver: what a walking body does to the hand

From the gait literature, and these are the two numbers that matter:

| | walking | note |
| --- | --- | --- |
| step frequency | **1.8–2.0 Hz** | 1.82 Hz at 4 km/h, 1.92 at 5, 2.03 at 6 |
| stride frequency (a pair of steps) | **0.9–1.0 Hz** | half the above |
| vertical CoM excursion | **2–5 cm** | newer measurements 1.7–2.6 cm; the classic textbook figure is 5 cm |
| lateral CoM excursion | **~6 cm** | textbook |

And the structural fact the project already has right: **the vertical
oscillation runs at twice the stride frequency**, because the body rises once per
step while it shifts sideways once per pair of steps. `LLKLanternFeel.cpp`'s sway
comment says exactly this and it is correct.

So the hand is driven **laterally at ~0.95 Hz** and **vertically at ~1.9 Hz**.

## 4. Put the two together, and this is the finding

The lantern is a filter, not a follower. A base-driven pendulum's response is

```
amplitude ratio = 1 / √( (1 − r²)² + (r/Q)² )  ,  r = f_drive / f_natural
```

Against `f_natural = 1.05 Hz` — **corrected to 0.87 Hz in §2b, which flips the
walk’s phase and strengthens (b) and (c); the table below is kept because it is
what the three conclusions were first drawn from**:

| drive | r | Q=5 | Q=10 | Q=20 | phase |
| --- | --- | --- | --- | --- | --- |
| **walk, lateral (0.95 Hz)** | 0.90 | **3.9×** | **4.9×** | **5.4×** | +27°, nearly in phase |
| walk, vertical (1.90 Hz) | 1.81 | 0.43× | 0.44× | 0.44× | 175°, inverted |
| **sprint, lateral (1.45 Hz)** | 1.38 | 1.05× | 1.09× | 1.10× | 171°, inverted |
| sprint, vertical (2.90 Hz) | 2.76 | 0.15× | 0.15× | 0.15× | inverted |

Three consequences, and all three hold across the whole damping range, so none
of them depends on guessing `Q`:

**(a) A walk is near resonance.** The drive sits at 0.90 of natural, so the
lantern amplifies the hand's lateral motion by **four to five times** and does it
roughly in phase. This is why a hand-carried lantern swings so much at walking
pace, and it is the same coupling the load-carrying literature is about: compliant
carrying poles are economical precisely when their natural frequency sits below
step frequency, and experienced pole carriers tune their cadence to the pole.
**A walking keeper's lantern is the single largest motion in the scene.**

**(b) Vertical barely moves.** Every gait drives the vertical axis well above
resonance, so the lantern transmits 0.44× at a walk and 0.15× at a sprint, both
inverted. Against the 4.9× lateral, the real figure-of-eight has a vertical
component about **one tenth** of its horizontal one. It is nearly a flat
side-to-side arc, not a figure of eight.

**(c) Sprinting swings the lantern *less*, not more.** At a sprint the drive
crosses above resonance, the ratio collapses to ~1.1× and the phase inverts. The
arm does drive harder when running, so the drive amplitude rises — but it would
have to rise more than **4.5×** to beat the walk's amplification, and it does
not. A sprinting keeper's lantern is held comparatively still and moves against
the body rather than with it.

**(d) A knock rings.** An impulse to a pendulum is not an offset, it is a
decaying oscillation at the natural frequency, `τ = Q/(π·f₀)`:

| Q | τ | visible swings before it is down to a tenth |
| --- | --- | --- |
| 5 | 1.52 s | ~3.7 |
| 10 | 3.03 s | ~7.3 |
| 20 | 6.06 s | ~14.7 |

So Chris's *"if you get knocked down a little bit your hand moves your lantern
moves"* has a number: **roughly 1 Hz, visibly swinging for two to three
seconds.** `Q` is the one parameter here that wants measuring rather than
deriving — film a real lantern, count the swings — and it is also the parameter
a grip changes most.

## 5. Where the current model disagrees

`FLLKLanternFeelRules` (`LLKLanternFeel.h`) against the above:

| Rule | Now | Physics says | Verdict |
| --- | --- | --- | --- |
| `SwayHertz` | **1.6** | lateral drive is ~0.95 Hz; as a natural frequency, **0.87** (§2b) | **1.68× too fast** as a drive rate, **1.84×** as an `f₀`. 1.6 strides/s is 3.2 steps/s, a jog cadence, applied at a walk |
| vertical : horizontal | **0.5** (hardcoded in the sway vector) | **~0.058** (§2b; was ~0.09 at the old `f₀`) | the arc is about **nine times too tall** |
| `WalkSwayCm` | 2.4 cm | hand drives ~3 cm, lantern amplifies ~4.5× (§2b) → **~10–14 cm** at the lantern | far too small, *and* `MaxOffsetCm` is 12, so the real amplitude does not fit in the rail — a pure comfort call since O10, with no gameplay consequence either way |
| `SprintSwayCm` | **5.2 cm** | **0.56×** of its own drive and inverted (§2b) — quieter than the hand, far quieter than a walk | **backwards** |
| landing kick | downward offset, `KickRecoveryPerSecond` 0.94 → τ ≈ 0.35 s, no oscillation | **0.87 Hz** ring-down over **1.8–3.7 s** (§2b) | wrong shape *and* 5–10× too fast |

**None of this is sloppiness and the file should be read generously.** It is a
kinematic model of a *hand* — and as a model of the hand it is right, including
the 2× vertical relationship, which it derives correctly from gait. What it has
no representation of is the lantern: there is no mass, no natural frequency and
no phase lag anywhere in `AdvanceFeel`, so the lantern is rigidly welded to the
hand's motion. That is physically the behaviour of a keeper with a white-knuckle
grip on the lantern's frame, which is a real way to carry one — just not the way
a bail is for, and not what row 67's "raised-lit" mode implies.

The minimal honest change is **one second-order state per axis** — angle and
angular velocity, driven by the hand's acceleration, with `f₀` and `Q` as the two
authored numbers — in place of the direct sine. It is about fifteen lines, it
subsumes the landing kick (an impulse becomes an initial velocity and rings down
for free), and it makes the sprint/walk relationship fall out instead of being
tuned. `SwayHertz` then stops being a sway rate and becomes the lantern's natural
frequency, which is a property of the object rather than of the animation.

## 6. The thing worth deciding — **CLOSED 2026-11-04: it stays cosmetic**

Row 81 says that if the lantern moves, what it lights moves. Today the opposite
split is in force:

| motion | magnitude | reaches gameplay? |
| --- | --- | --- |
| look pitch, through `LanternAnchor`'s 42 cm lever arm | **88 cm vertical** | **yes** |
| the sway — the actual pendulum, 4.9× amplified at a walk | ~10–15 cm lateral | **no** |

The sway is sealed out structurally: feel writes `LanternLight`'s *relative*
offset and gameplay reads the parent, so the attachment order is the guarantee.
The pitch travel reaches gameplay only because it arrives through the parent
transform instead of the feel struct. The one motion that is a real pendulum is
excluded; the one that is a rigid lever on a camera is included.

**Chris answered this twice on 2026-11-04 and the answers point opposite ways.**
Row 82 is the considered one, with its reasoning attached — *"Keep pendulum sway
cosmetic… A player shouldn't become detectable, or lose a valid 'Present'
interaction, because the bail happened to be at one end of a swing"* — and
`FINDINGS-COSMETIC-GAMEPLAY-SPLIT.md` has already audited the code against it
and fixed one violation. Asked the bare question in another session later the
same day, he answered **"Sway moves gameplay."** Row 82 stands; a two-word reply
does not overturn a ruling that arrived with an argument and a test behind it.

**Then the question got measured, and it is smaller than either answer assumes.**
`GetGameplayLightLocation()` has exactly one caller outside its own file:
`LLKZonePressureSubsystem.cpp:345`, which sets `FLLKLightSource::Position`.
Nothing downstream reads the lantern; everything samples the *field*. The
Sweeper takes `DetectabilityAt(sources, Senses.KeeperPosition)`
(`LLKSweeper.cpp:326`) and a relic takes `BrightnessAtVisible(sources, relic
position)`. The falloff is **soft** — `1/(1 + d²/R²)`, not inverse square
(`LLKLightField.cpp:19-20`) — and `radiusStuds` 30 at 28 cm a stud makes
R **840 cm**, while the anchor is only 66.4 cm from the keeper's own sample
point. So a physically true 15 cm lateral sway, moved into gameplay, buys:

| quantity | with the sway in gameplay |
| --- | --- |
| detectability at the keeper's own position | 0.99452 → 0.99242 — a **0.21%** swing |
| the Sweeper's lerped aggro radius, 12→38 studs | **1.5 cm** at ~1 Hz, on a 10.6 m radius |
| brightness at a relic 1 m from the lantern | **0.83%** swing |

Row 82's feared mechanism is real in principle and about a centimetre and a half
in magnitude. Nobody becomes detectable because the bail was at one end of a
swing; they become detectable 1.5 cm sooner. The relic case cannot lose an
interaction at all: legibility is **sticky** (`LLKRelicComponent.cpp:240-244` —
*"Identity is knowledge the player already paid for"*) and `bCurrentlyLit` runs
a two-equal-sample debounce, so a wobble can grant a read a fraction of a second
early and can never revoke one.

**The reconciliation was offered and declined, and the reason is better than the
proposal was.** The rule on offer was one of outcome shape rather than motion —
*the sway may move anything gameplay resolves continuously, and may not move
anything gameplay resolves to a yes or no*, which is row 80's principle applied
to position instead of reach. Chris, 2026-11-04:

> Row 82 stands as written. The hybrid rule is clever, but it buys 1.5 cm of
> invisible behavior while creating two meanings for "lantern position." That's
> not worth another seam or exception.
>
> - Visual pendulum: full physical motion.
> - Gameplay light origin, aim, reads, and notice: stable mount.
>
> No code change. Put the pendulum work into making the lantern feel carried,
> not into a behavior difference nobody can perceive.

**So the measurement argued against the thing it was gathered to support**, which
is the part of this section worth keeping. 1.5 cm is not a reason to act; it is a
reason not to. The cost that decided it is not in the one changed line — it is
that "where the lantern is" would have had two answers from then on, and every
future reader of that position would have had to know which one it wanted. That
is a permanent tax bought with an imperceptible effect.

**Row 81 survives intact, and so does everything in §1–§5.** Physical honesty
governs what the player sees, in full; it was never a mandate to add a seam. The
derivation above is therefore the spec for the *cosmetic* model and nothing else,
which makes it simpler rather than smaller: a second-order pendulum below the
anchor has no gameplay consequence to fence, so `MaxOffsetCm` becomes a pure
first-person comfort call with no balance question attached to it.

**And the pitch lever stays in gameplay**, ratified by row 81, unchanged by this. Its
magnitude is **88.06 cm**, re-derived 2026-11-04: `152 + 42·sin(p) − 19·cos(p)` is not
monotonic, so the minimum is interior at `p = −65.66°` (105.90) rather than at the −89.9°
endpoint (109.97) — taking the endpoints is what produced a passing figure of 84.
The asymmetry in the table above is now deliberate and stated: the rigid arm the
player aims is a gameplay input, and the pendulum they merely watch is not.

## 7. What this does not claim

- **No numbers were changed.** Every figure above is derived or cited, and the
  two that cannot be derived — `Q`, and the hand's actual drive amplitude while
  sprinting — are named as needing measurement.
- **`MaxOffsetCm` is 12 and a real walk sway is 10–15 cm**, so the rail would
  clip the physical amplitude. Whether the rail should rise or the amplitude
  should be scaled for legibility is a feel decision, not a physics one: a
  lantern that swings its true 15 cm may simply be unpleasant to look at in first
  person, and that is a legitimate reason to keep a number that physics does not
  pick. Row 81 says derive first, not ship the derivation blind.
- **Nothing here touches the light field's own rules**, the gameplay radius, or
  `data/lantern.json`.

## Sources

- [Dietz #76 Original Cold Blast Lantern](https://lanternnet.com/?p=492) and [the galvanized "Original" listing](https://lanternnet.com/?p=138709) — dimensions and fount capacity
- [Antique Dietz Railroad Lantern](https://www.replacements.com/art-antiques-decorative-accents-antique-dietz-railroad-lantern/p/134294911) — 14½″, 3.05 lb
- [Dietz #2500 Jupiter](https://lanternnet.com/product/dietz-2500-jupiter-cold-blast-lantern/) — the larger non-railroad pattern, for scale
- ["Hy-Lo" Dietz lantern, museum record](https://ctcollections.org/index.php/Detail/objects/38402) and [Little Wizard record](https://hub.catalogit.app/historic-charlton-park/folder/entry/lantern-14)
- [Efficiency of gait, clinical gait analysis teach-in](https://clinicalgaitanalysis.com/teach-in/efficiency) — the classic 5 cm vertical and 6 cm lateral CoM figures
- [Natural step frequency on a treadmill](https://actabio.pwr.edu.pl/fcp/mGBUKOQtTKlQhbx08SlkTUARAUWRuHQwFDBoIVURNWHpSFVZpCFghUHcKVj0GQUw/302/public/publikacje/v12-3-2010/42.pdf) — 1.82/1.92/2.03 Hz at 4/5/6 km/h
- [Vertical CoM displacement measurements](https://pmc.ncbi.nlm.nih.gov/articles/PMC2816028) — the newer 1.7–2.6 cm range
- [Load carrying with flexible bamboo poles: optimization of a coupled oscillator system](https://ro.ecu.edu.au/ecuworkspost2013/7329) — carriers tuning cadence to the pole
- [Coupled dynamics of legged locomotion with suspended loads](https://docs.lib.purdue.edu/dissertations/AAI10151592) — suspension tuned below half the locomotion frequency; resonance with multiple gait frequencies
- [Modeling of legged locomotion with a suspended load in the sagittal plane](https://docs.lib.purdue.edu/dissertations/AAI1553581) and [the compliant-pole running study](https://core.ac.uk/download/77940015.pdf) — the walk/run asymmetry
