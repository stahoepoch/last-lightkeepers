# Fixture light contract

Status: **LOCKED**, Chris, 2026-10-31. This supplements
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
No fixture may look bright enough to read by while those rules remain true.
