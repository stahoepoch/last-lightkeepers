# Why it doesn't feel like a lantern

Chris: *"It definitely doesn't feel like a lantern."*

I read `data/lantern.json` before recommending anything. The authored light
block is:

```
radiusStuds        30
coneAngleDegrees   72        <- outer cone
intensity          250       (candelas, unit pinned in code)
warmColor          1.0, 0.82, 0.56   (linear RGB tint)
castsShadows       true
```

**The problem is the cone, not the intensity.**

A lantern emits in every direction. A 72° cone is a wide *torch beam*. The
authored note already reasons correctly — *"Wide because the keeper holds a
lantern, not a torch: narrow enough to aim is a flashlight game"* — it just
doesn't go nearly far enough. 72° is wide for a flashlight. It is nowhere near
a lantern, which is 360°.

No intensity value fixes a cone. At 250 cd it is a bright torch; at 25 cd it
will be a dim torch.

---

## Fixes, ordered by how much they will change the feeling

### 1. Outer cone 72° → 150–179°

The single biggest change, and it costs one number.

A `USpotLightComponent` with an outer cone near 180° approximates an
omnidirectional source, so the existing class, the existing component and the
per-level authored angle architecture all survive. No point-light migration,
no orphaned data.

**And it gives the Hooded / Working / Open dial a physical meaning it has been
missing.** The cone angle *is the hood aperture.* That is what a lantern hood
does — it occludes part of a 360° source:

| Level | Outer cone | What it is |
|---|---|---|
| Hooded | ~40–60° | The hood nearly closed, light pushed downward |
| Working | ~90–110° | Normal carry |
| Open | ~160–179° | Hood open, the lamp lighting the room |

Three angles that already have to exist, now meaning something a player can
see and understand. Radius changes with them, per the existing rule that the
hood changes reach rather than brightness.

### 2. The measured 25 cd transfers across cone changes — corrected

An earlier draft of this page said to re-measure intensity after widening the
cone, because the two interact. **That was wrong, and Codex caught it.**

Candela is luminous intensity **per direction**, and on a Spot Light it is
independent of cone angle. Widening the cone does not change how brightly any
lit surface is lit — it changes *how many surfaces are lit*. So a card at 2
studs inside the beam reads the same at 25 cd whether the cone is 72° or 170°.

The practical consequence is better than what I wrote: **the lab's ~25 cd is a
stable number**, and cone and intensity can be tuned independently rather than
chased around each other. The room does get brighter overall when the cone
widens, because more of it is in the beam — but that is a coverage change, not
a brightness change, and it is the change we want.

### 3. Note where 250 came from

`_intensityWas6` records that 6 was invisible because the component was
unitless and defaulting to 5000, and that 250 replaced it by eye. **That
tuning happened before the exposure clamp existed.** The clamp is now pinned
at `min = max = 1.0`, which is a very sensitive, never-adapting camera — so
every pre-clamp brightness judgement is roughly an order of magnitude too hot.
That is exactly the 10× the lab found, and it means 250 was never wrong when
it was chosen, only wrong now.

### 4. Use colour **temperature**, not an RGB tint

`warmColor 1.0/0.82/0.56` is a reasonable warm multiplier but it is a tint.
Unreal's `Use Temperature` + `Temperature` in Kelvin produces physically
correct warm light that behaves properly under exposure. Tutorials put
candlelight around 2700K against 5500K for noon sun; a flame source sits
lower, around 1800–2200K.

Which number depends on a fiction question — see §7.

### 5. Source radius: a flame is not a point

`SourceRadius` / `SoftSourceRadius` are absent from the authored block. At
zero, a light behaves like a tiny point source, producing sharper highlights
and harder shadow edges. Guides suggest roughly 5–20 for realism.

Hard-edged shadows are an unmistakable "video game flashlight" tell. A mantle
or a flame is a small *area*, and the soft-edged shadow it throws is a large
part of why real lantern light looks like lantern light.

### 6. Let the lantern's own geometry shape the light — this is the big one visually

The most recognisable thing about lantern light is not its colour or its
brightness. It is **the pattern the lamp's own body throws**: the cage bars,
the hoop, the handle, and the dark disc directly beneath the fuel reservoir
where the lamp shades itself.

That comes from the mesh casting shadows with the light inside it, not from
any light setting. A forum thread on exactly this problem found an
emissive-only magical lantern failed to light dynamic objects at all, and that
a point light placed inside the mesh worked properly.

If one change makes a screenshot read instantly as a lantern, it is this one.

### 7. IES profiles — the "real application" answer

This is the thing Chris was asking for and it exists, free, in bulk.

An **IES profile** is the global standard photometric file that encodes the
real intensity and spatial distribution of an actual light fixture. Unreal
takes them directly. Crucially, an IES only *shapes* how an existing light
emits — it adds no geometry and no extra lights, so the performance cost is
essentially the same as a normal light.

Where to get them:

- **ieslibrary.com** — over 90,000 free files from real manufacturers
  including GE, Osram and Philips, free to download and licensed for
  commercial use. No bulk download; you pull them per manufacturer page.
- **3DTexel** — 130 ready-to-use `.ies` files under **CC0**: commercial use,
  no attribution.
- **LeoMoon Studios** — 32 profiles, free, listed as GPL.
- CGTricks and All3DFree host larger collections but did not state licences —
  check before shipping.

For this job, look for a **bare-bulb or omnidirectional/globe** profile rather
than an architectural downlight. That gives the characteristic
lantern falloff with no hand-tuning, and it is a drop-in test: swap the
profile, look, swap it back.

### 8. Flicker — only if the fiction says flame

A light function material is the usual approach: a material driving brightness
over time, applied as the light's Light Function. Keep it subtle and
non-periodic; a sine wave reads as a pulsing magic item.

**But do not add flicker reflexively.** It is wrong for an electric source.

---

## The fiction question that settles §4 and §8

The lantern is an issued service instrument and an *interpreter*. Is it a
**flame** or **electric**?

- **Flame** — 1800–2200K, visible flicker, soft moving shadows, and the
  colour temperature wanders slightly. Warmest, most human, oldest.
- **Electric / filament** — ~2700K, steady with at most a faint wobble, hard
  consistent shadow pattern from the housing. Reads as *equipment* rather
  than *comfort*, which fits "sealed service instrument" better.

They look genuinely different and they want opposite settings. This is a
one-line decision that determines four others, and it is Chris's.

---

## It is a rig, not a light — Codex's correction, adopted

One cone with a better number is still one cone. Codex's version is right: a
lantern is **three things working together**, and they do different jobs.

1. **A visible lantern object.** Mesh in hand, below and slightly forward of
   the camera, with a warm emissive glass chamber that moves with the hand.
   This is the source the eye believes in, and nothing else can substitute for
   it.
2. **A small soft near-field glow.** Very low intensity, warm, around the
   lantern itself — hands, the floor underfoot, the near shelf edge. This is
   what produces *"I am carrying a luminous object"* rather than *"a light is
   attached to my head."*
3. **A broad weak forward wash.** The spot, but as a hood rather than a beam:
   wide outer cone, small hot core, low intensity, larger source radius so the
   shadows soften.

**Performance guardrail, since this is now three lights per keeper and there
are up to four keepers:** only the wash should cast dynamic shadows. The
near-field glow must have shadow casting **off** — it is fill, it has no
shadow work to do, and twelve shadow-casting dynamic lights in a store full
of gondolas is the kind of thing that is discovered late and expensively.

**And the separation Codex asks for in his point 5 already exists in code.**
`GetLitLightSources` reads `GetGameplayRadiusCm()`, not the component's
`AttenuationRadius` — landed in `d065469`, with a test that moves the
attenuation radius behind the game's back and requires the gameplay answer not
to follow. So the visual rig can sway, bloom, soften and flicker freely
without touching whether a relic is readable or a keeper is detectable. That
firewall is built and guarded; the lighting pass cannot silently rewrite
balance even if someone tries.

---

## What I would actually do next, in order

1. Set outer cone to ~170° and look at it. Nothing else changed.
2. If it now reads as a lantern at the wrong brightness, re-run Bay C for
   intensity at the new cone.
3. Enable Temperature at 2200K or 2700K per the fiction answer.
4. Set SourceRadius ~10.
5. Make the lantern mesh cast shadows with the light inside it.
6. Try a bare-bulb IES as a drop-in and compare.

Steps 1 and 2 are data changes in `data/lantern.json`. Steps 3–5 are the
component and the Content mesh. Step 6 is an asset download.

---

## Sources

- [UE5 lighting types guide — Rambod](https://rambod.net/tutorial/ue5-lighting-types-guide)
- [UE5 light types and settings — Ultidigi](https://ultidigi.com/tutorials/ue5-light-types-and-settings)
- [Need some help solving a lantern lighting problem — Epic Developer Community](https://forums.unrealengine.com/t/need-some-help-solving-a-lantern-lighting-problem/505090)
- [How to make a flickering light effect in UE5 — Rick Bueno](https://www.linkedin.com/pulse/unreal-engine-5-how-make-flickering-light-effect-rick-bueno)
- [How to create a candle flame in UE5 — Whitewash Studio](https://whitewashstudio.medium.com/how-to-create-a-candle-flame-in-ue5-695b1c398ec0)
- [Download over 90,000 free IES lighting files — CG Channel](https://www.cgchannel.com/2020/06/download-over-90000-free-ies-lighting-files-from-ies-library/)
- [Free IES profiles, 130+ CC0 files — 3DTexel](https://3dtexel.com/light-ies/)
- [Free IES light profiles for Blender, Unreal & 3ds Max — 3DTexel](https://3dtexel.com/free-ies-light-profiles-for-blender-unreal-3ds-max/)
- [IES lights pack — LeoMoon Studios](https://leomoon.com/store/shaders/ies-lights-pack/)
- [Candela — Wikipedia](https://en.wikipedia.org/wiki/Candela)
- [Candlepower — Wikipedia](https://en.wikipedia.org/wiki/Candlepower)
- [Technical and economic performance analysis of kerosene lamps (PDF) — Stanford](http://large.stanford.edu/courses/2011/ph240/machala1/docs/offgrid-lighting.pdf)
