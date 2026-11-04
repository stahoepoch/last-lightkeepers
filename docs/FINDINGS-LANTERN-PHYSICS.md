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
range is 0.9–1.25 Hz.** It is a flat result: the period goes as the square root,
so even a 2× error in `d` moves the frequency by 40%. Take **1.05 Hz** as the
working number and treat it as solid.

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

Against `f_natural = 1.05 Hz`:

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
| `SwayHertz` | **1.6** | lateral drive is ~0.95 Hz | **1.68× too fast.** 1.6 strides/s is 3.2 steps/s, a jog cadence, applied at a walk |
| vertical : horizontal | **0.5** (hardcoded in the sway vector) | **~0.09** | the arc is about **five times too tall** |
| `WalkSwayCm` | 2.4 cm | hand drives ~3 cm, lantern amplifies ~4.9× → **~10–15 cm** at the lantern | far too small, *and* see §6 — `MaxOffsetCm` is 12, so the real amplitude does not fit in the rail |
| `SprintSwayCm` | **5.2 cm** | less than walk, and inverted in phase | **backwards** |
| landing kick | downward offset, `KickRecoveryPerSecond` 0.94 → τ ≈ 0.35 s, no oscillation | ~1 Hz ring-down over 1.5–3 s | wrong shape *and* ~5× too fast |

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

## 6. The thing worth deciding, and it follows from row 81 rather than from physics

Row 81 says that if the lantern moves, what it lights moves. **Today the
opposite split is in force, and it is the reverse of what the physics would
choose:**

| motion | magnitude | reaches gameplay? |
| --- | --- | --- |
| look pitch, through `LanternAnchor`'s 42 cm lever arm | **88 cm vertical** | **yes** |
| the sway — the actual pendulum, 4.9× amplified at a walk | ~10–15 cm lateral | **no** |

The sway is deliberately sealed out: feel writes `LanternLight`'s *relative*
offset and gameplay reads the parent, which is a correct and well-commented
guarantee. The pitch travel reaches gameplay only because it arrives through the
parent transform instead of the feel struct. So the one motion that is a real
pendulum is excluded, and the one that is a rigid lever on a camera is included.

**Both are now ratified as "it moves", which makes this a real question rather
than a defect:** does the sway join the gameplay anchor, or does the pitch travel
leave it? There is a third answer and it may be the best one — let gameplay read
the lantern's *physical* position, sway and all, and keep the fence where it
belongs, around the purely decorative part (the trail, the cone lag). That
honours row 81 exactly: the physics moves gameplay, the animation does not.

**Chris's call.** The cost of the first is that enemy perception starts moving
10–15 cm laterally at 1 Hz while walking, which is small next to the 88 cm the
pitch already contributes and would make a walking keeper's legibility pulse. The
cost of the second is losing the thing he just ratified.

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
