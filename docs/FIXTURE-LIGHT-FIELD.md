# Fixture light contract — supplement to `docs/THE-STORE.md`

`THE-STORE.md` (`e6a21d0`) locks the look. This page covers only the part it
does not: **which fixtures enter the gameplay light field**, and the three
places the art pass collides with systems already built. Nothing here restates
the visual identity; that document is the authority on it.

## The question every fixture answers

`FLLKLightSource` carries position, intensity, radius, detectability. Both
consumers derive the lit threshold as `I/2`, so intensity cancels out:

> **Lit means inside the radius.** Intensity is a visual knob, not a
> mechanical one.

So each placed fixture is a binary, and it must be decided at placement time
rather than inferred later from how bright it looks.

| Fixture | In the light field? | Why |
|---|---|---|
| Weak fluorescent islands | **Yes** — low, small radius | The "occasional lit department island." Pools, not a wash. |
| Neon department signage | **No** (emissive only) | A sign reads from across the floor without lighting the floor. |
| Neon *anchor* under a threshold | **Yes**, one per department, tiny | Keeps the sign honest: it does faintly light what is directly beneath it. |
| Exit signs | **No** | Nobody expects to read by exit-sign light. Stays honest while costing nothing. |
| Glass case interiors (jewellery, optical, toy) | **Yes**, tiny radius | The case lights the relic, not the aisle — so reading it means standing in a spot you know is lit. |
| Mirrored ceiling islands | **No** | Reflections carry no light. See below. |
| Restrooms / fitting rooms / service / basement | **No fixture** | True black, per `THE-STORE.md`. |

The reason this matters more than it looks: `aggroRadiusDark: 12`,
`aggroRadiusLit: 38`. Lit means seen from three times further. Every fixture
promoted into the field widens the danger, so the store's emergency lighting
is a difficulty budget, not just dressing.

## Three collisions with work already landed

**1. The exposure clamp was tuned without emissive in the scene.**
The single clamp at priority 1 (`9673ec3`) was read against greybox. Neon is
emissive and mirrored islands multiply it. A green exposure reading today does
not survive the first neon fixture — re-read the clamp after one goes in,
before tuning anything else against it.

**2. A mirror can reopen a sightline geometry closed.**
`LLK.CheckMap` wants 34 studs and currently reports 80. Mirrored ceiling
islands over an aisle can restore a sightline the gondolas were placed to
break. Two consequences:
- Keep the segmentation. Faceted panels with visible seams fragment the image,
  so a mirror says *something moved* without saying what or where. That is
  also what keeps it inside the existing rule that a reflection may reveal only
  what is physically visible in it.
- **Re-measure the sightline after the mirrors are placed, not before.** The
  current 80 is a floor-plane measurement; mirrors add a path the metric was
  not watching.

**3. Reflection cost is a decision, not a detail.**
- Screen-space reflections cannot do the interesting case — SSR only reflects
  what is already on screen, and the whole value is seeing what is not.
- A planar reflection actor is the real thing and costs a second scene render
  per plane.
- A reflection capture gives the shimmer and the doubled ceiling but no moving
  Sweeper in it.

Suggested split: capture-based for the general ceiling, planar on the one
island above the central aisle where catching movement matters, and treat that
count as a hard budget rather than something to grow later.

## Colour as crew language

`THE-STORE.md` names the palette. The additional constraint is operational:
**no two departments on a floor share a colour**, because "meet me at the red
one" has to work as a sentence over a voice call with four people talking.
That is the cheapest wayfinding the game will ever get, and it costs only
discipline at placement.

## Open questions

- **Does detectability differ by source?** A keeper standing under an
  emergency fixture is lit by something that was already on. Arguably a
  Sweeper should not read that the same way it reads a lantern someone just
  switched on — otherwise the safest-looking corridor is a permanent aggro
  field nobody chose to enter. This is the one field on `FLLKLightSource`
  nothing currently varies.
- **Do the neon circuits degrade with Storm Charge?** If the colour language
  dies progressively, wayfinding decays exactly as the run gets dangerous.
  Attractive, and it costs the crew their shared vocabulary at the worst
  moment — which may be the point, or may be too cruel.
