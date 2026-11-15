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
| **Cover masters** — `SM_BSW_FoldedGoodsTable`, `SM_BSW_ServiceCounter`, `SM_BSW_WallMerchBay` | **No** — they emit nothing | Added 2026-11-14. Their job is to block a sight trace, and a fixture that lights the keeper hiding behind it is working against itself. See below. |

## Cover and light must not be placed on top of each other

The three cover masters in `FIXTURES-COVER-TWO-MASTERS.md` emit no light, which
is the easy half. The hard half is what gets placed **beside** them.

`aggroRadiusDark: 12` against `aggroRadiusLit: 38`. A keeper crouched behind
the folded-goods table is hidden from a sight trace and, if that table sits
inside a fluorescent island's radius, **noticed from three times further the
moment the trace clears**. Geometry hides you; light tells the machine roughly
where you went. Both are true at once and they are authored by different
people in different passes.

So a lit pool next to cover is not a mistake to forbid — it is the most
interesting square metre on the floor, and it has to be deliberate:

- **A glass case beside a counter** is good: the thing worth reading is in the
  one spot the room knows is lit, and you must choose to stand in it.
- **A fluorescent island over a folded-goods table** is bad by accident: the
  fixture that exists to teach crouching sits in the one place crouching does
  not help.
- **The merchandise bay** is the exception that wants a pool near it rather
  than on it. It blocks above the eye, so a keeper behind it is blind; a lit
  aisle at one end is the only way to learn which end to leave by.

Nothing here is a rule yet. It is the thing to check at placement, and
placement is the map's, not this page's.

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
