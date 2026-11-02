# Fixture light contract

Status: **LOCKED**, Chris, 2026-10-31. **Amendment drafted 2026-11-02 by
Claude on Chris's instruction, awaiting his ratification** - the amendment is
the section "How the rule is met" below, plus the traceable acceptance test
that replaces the brightness judgement at the end. Nothing above those is
changed: the gameplay-light table and the rule itself are his words and stand
verbatim. This supplements
`docs/THE-STORE.md`; it does not replace its visual direction.

The store needs neon, emergency lighting, mirror ceilings and luminous retail
fixtures. It must not accidentally turn every decorative bulb into a gameplay
lure. The distinction is deliberate:

| Fixture or source | Renders light / emissive | Joins `LLKLightField` gameplay light | Reason |
| --- | --- | --- | --- |
| Keeper's issued lantern | Yes | **Yes** | It is the carried interpreter: it makes relics legible and declares the keeper to the building. |
| Lantern's optional cosmetic glow | Yes | **No** | It improves the carried-object read without adding a second invisible source. |
| Exit signs and emergency fluorescents | Yes | **No** | They provide orientation and safe-looking landmarks, not a hidden threat multiplier. |
| Neon department signs and tube fixtures | Yes | **No** | They are retained retail identity and reflected ambience, not player action. |
| Storm flashes and exterior spill | Yes | **No** | They are weather telegraphs; their visibility consequences must be authored as a separate, explicit system. |
| Glass cases, mirrored ceilings and reflective tile | Reflect / receive light | **No** | They affect what the player can visually read, but they never duplicate a gameplay source. |
| A future machine that deliberately interprets the keeper's lamp | Only if authored | Only by an explicit gameplay rule | No material, emissive mesh or map light may gain this power by accident. |

**Visual light must not masquerade as reading light.** A player may navigate by
an exit sign, recognise a gondola silhouette under a weak fluorescent island,
or see a relic's shape through a dead display case. They must not plausibly
expect to read a price tag, engraved detail or relic label under that fixture.
Those things remain the issued lantern's job. An illuminated-looking jewellery
vitrine containing a relic would teach the player that the game's `NeedsLight`
prompt is lying, so ground-floor cases are unpowered: dark glass, reflected
signage and a tempting silhouette, never a working interior lamp.

## How the rule is met — geometry, not intensity

**Drafted 2026-11-02, awaiting ratification.** The rule above is unchanged and
is the right rule. What follows is the mechanism, because measurement showed
the obvious reading of it — "pick a dim enough fixture" — cannot work.

Measured in `LLK_LightLab` under the Lumen baseline; full tables and conditions
in `docs/FINDINGS-FIXTURE-LIGHT-BAND.md`.

| | |
| --- | --- |
| Highest candela at which a price card is still **not** readable at 2 studs | **below 0.5 cd** |
| Lowest candela at which a 40-stud aisle run stays legible | **~2 cd** troffer, **~6 cd** neon |

**There is no value between them.** The orientation floor sits about 4x above
the inspection ceiling, so any fixture broad enough to light the aisle lights a
price card directly — and *directly* is the operative word. Three levers were
tried against the inspection half and all three failed:

| lever | effect on the card | effect on orientation |
| --- | --- | --- |
| lower the candela | still reads at 0.5 cd, the bottom of the ladder | the run goes dark first |
| barn doors 15° | **−11%** | fine, and side walls improve to zero |
| barn doors 2° | card dark | run dark too, and the ratio is *worse* than plain |
| floor albedo 0.18 → 0.03 | **−2.5%** | floor read **−63%** |

Darkening the floor sixfold moved the card by 0.7%. Every lever that acts on
*indirect* light misses a surface that is lit *directly*.

### So the mechanism is geometry

**A readable face is kept dark by being shaded from its fixture's direct ray,
not by dimming the fixture.** Measured with an ambient troffer at 2 cd
throughout:

| rig | card mean | text contrast | bay wall | readable? |
| --- | --- | --- | --- | --- |
| no lip | 92.29 | 102.7 | 4.92 | yes |
| shallow lip, 10 cm | 71.36 | **104.1** | 4.70 | **yes — unchanged** |
| **deep lip, 30 cm** | 10.33 | **14.8** | **4.64** | **no** |

Text contrast falls sevenfold and **the room does not move** — the bay wall
goes 4.92 to 4.64. Every earlier lever bought card darkness by taking light out
of the aisle; this one takes none.

**The depth is a ray, not a taste call.** The lip must reach past the point
where the line from the fixture to the **bottom** of the readable face crosses
the lip's own height. For the lab's pose — fixture at (200, 0, 290), card
spanning z 75–105 — that crossing is at x ≈ 315, so the lip needs about
**23 cm**. Ten could not shade it; thirty could.

**And shading a label is not the same as shading the part you read.** The 10 cm
lip dropped the card's *mean* by shading its top edge while leaving the glyph
band fully lit, so contrast did not move at all. A lip that looks like it is
working is the failure mode here, which is why the acceptance test below is a
trace rather than a look.

### Three rules that follow

1. **Any surface carrying legible detail** — price card, shelf label, engraved
   relic face — **is recessed, under a lip, or angled off the ceiling plane**,
   so the fixtures do not see it. Legible detail is never dimmed into
   illegibility; it is placed out of the light.
2. **Landmarks are self-luminous rather than lit.** An exit sign or a neon
   department sign reads as a landmark because it emits, not because something
   shines on it — which is also what keeps it out of the inspection problem
   entirely.
3. **Neon makes the rule harder, not easier.** At matched candela a narrow
   saturated tube puts about **a third** of the troffer's luminance on
   surfaces, so it buys less orientation per candela while still lighting a
   card at close range. Neon at 6 cd lands on troffer at 2 cd. Budget it as
   identity, and let the troffers do the orienting.

### What this does not change

The gameplay-light table above, which is the load-bearing half of this page.
None of the geometry here lets a fixture join `LLKLightField`, and a recessed
label is not a gameplay rule — relic legibility still comes from the issued
lantern and from the C++ rules that name it.

## Consequences for the first art pass

1. **Re-read exposure after the first real neon fixture.** The current exposure
   clamp (priority 1) was tuned in a greybox without emissive retail lighting.
   Do not use its present value as proof that the art-passed aisle is balanced.
2. **Re-audit sightlines after placing the aisle.** This is ordinary map
   hygiene, not a reflection-specific rule: the 34-stud metric remains a
   direct geometry measurement. Mirrors change player perception, not the
   metric or enemy visibility rules.
3. **Use reflection captures for the doubled ceiling.** The visual identity
   does not depend on using mirrors to discover off-screen threats, so planar
   reflections and their additional scene render are not justified for this
   pass. Captures are the default; revisit the cost only if a later, specific
   encounter proves it needs more.

## First-pass acceptance

The first aisle is successful when a player can orient by an exit sign and a
weak neon landmark, feel their warm carried lantern as a separate source, and
still find a genuinely dark side route. No fixture may change relic legibility,
enemy detectability or storm pressure unless a C++ gameplay rule names it.

**The brightness clause is replaced by a traceable one, drafted 2026-11-02.**
"No fixture may look bright enough to read by" cannot be checked — it was
measured and no such fixture exists, because the aisle and the price card are
lit by the same direct light. What can be checked, per readable surface:

- **Trace from the fixture to the bottom of the readable face.** If that ray
  reaches it, the face is lit and no intensity choice will fix it. Shade it
  with geometry and trace again.
- **Trace from a held lantern position and from eye height.** Both must still
  reach the face, or the lip has shaded the player out of their own mechanic.
  Both were verified on the lab's lip: each first-hit the card.
- **Judge contrast across the glyph band, not mean luminance over the card.**
  The 10 cm lip looked like it was working on the mean and had changed nothing
  that matters.

A fixture passes when the aisle reads from 40 studs and every readable face in
it is shaded from direct fixture light by geometry — not when someone judges it
looks dim enough.
