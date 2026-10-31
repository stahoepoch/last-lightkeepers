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

## Consequences for the first art pass

1. **Re-read exposure after the first real neon fixture.** The current exposure
   clamp (priority 1) was tuned in a greybox without emissive retail lighting.
   Do not use its present value as proof that the art-passed aisle is balanced.
2. **Re-audit player-readable sightlines after mirror islands are placed.** The
   map check's current sightline measurement is a direct floor-plane trace; it
   does not simulate reflected paths. A mirror can make an aisle feel open or
   reveal a moving light to the player without changing the direct trace. Treat
   the check as a minimum geometry test, then inspect the aisle in play.
3. **Treat reflections as an art and encounter cost.** Screen-space reflections
   cannot show off-screen threats; planar reflections can, but add a second
   scene render per plane. Reflection captures sell the doubled ceiling but do
   not show a moving Sweeper. Use captures for broad ceiling identity first;
   reserve planar reflections for a small number of authored encounter mirrors
   only after the slice proves their value.

## First-pass acceptance

The first aisle is successful when a player can orient by an exit sign and a
weak neon landmark, feel their warm carried lantern as a separate source, and
still find a genuinely dark side route. No fixture may change relic legibility,
enemy detectability or storm pressure unless a C++ gameplay rule names it.
