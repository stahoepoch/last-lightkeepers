# Handoff to Codex — `light.levels` needs four lines in the importer

Written 2026-10-30 by Claude, after landing the detectability rule layer in
`d065469`. One new key, one validator block, one struct field to copy. The C++
that reads it is already in and already tested; the data cannot arrive without
you because `Tools/import_data.py` is your lane (`docs/LANES.md`) and its
`exact()` check would reject the key on sight.

**A terminology hazard, introduced by this file's own name.** In Unreal a
*level* is a map. Here a *level* is one of three lantern settings, because that
is the word the code uses (`FLLKLanternLightLevel`, `light.levels`). The two
collide, and they have already caused one real confusion: "no level is authored
yet" was written about `light.levels` and read as *"no map exists"*, which was
wrong - `LLK_DockBay` has been committed since `9fe73fa`. Prose in this project
should say **lantern level** or **named state** when it means the lantern, and
plain *map* when it means the `.umap`. Chris's own wording was "three named
states", which has no collision at all.

---

## What Chris decided

An adjustable lantern, as three named states rather than a slider. His words,
2026-10-30:

> Build it after the exposure pass as three named states: **Hooded** — close
> reading, minimal spill, enemies only react nearby. **Working** — current
> practical lantern. **Open** — broad, bright work light; strongest local enemy
> response. Keep the zone clock unchanged... The important implementation rule:
> each level needs a separate authored "detectability" value, not just
> intensity/radius.

That last sentence is the architecture and it is load-bearing. The two obvious
builds both silently do nothing:

- **A dial over `intensity`** cancels out. Every light threshold in the game is
  `I * 0.5f`, so "lit" reduces to `d < r` — the derivation is in
  `docs/LIGHTING-IN-UNREAL.md`.
- **A dial over `radiusStuds`** does nothing either, less obviously.
  `ALLKSweeper` asks the field for light at the keeper's *own* position, where
  their own lantern is at distance zero. Brightness there is `I` against a bar
  of `I/2`, cleared by a factor of two at any radius whatsoever.

So detectability is authored and nothing derives it. `LLK.Lantern.Levels`
asserts both halves: that detectability moves a Sweeper's reach, and that
moving intensity by a factor of forty moves nothing at all.

---

## The key, as it should appear in `data/lantern.json`

Inside the existing `light` block. **These are Chris's own seeded values**,
given 2026-10-30 and explicitly *"seed - not lock"* - the reaches and
detectabilities are his, the cone angles are not and still need him:

```json
"levels": [
  { "name": "Hooded",  "radiusStuds":  8, "coneAngleDegrees": 44,  "detectability": 0.20 },
  { "name": "Working", "radiusStuds": 20, "coneAngleDegrees": 72,  "detectability": 0.55 },
  { "name": "Open",    "radiusStuds": 30, "coneAngleDegrees": 100, "detectability": 1.00 }
],
"defaultLevel": "Working"
```

They give a Sweeper notice radius of 17.2, 26.3 and 38.0 studs respectively,
by `Lerp(aggroRadiusDark 12, aggroRadiusLit 38, detectability)`. All three were
run against `tests/balance.test.mjs` with the levels seeded in temporarily:
14 new checks, all passing, 164/164.

**Note what this does to the number every doc in the project quotes.** Today's
lantern reaches 30 studs; under these seeds 30 is the *Open* setting and the
default drops to 20. So today's lantern becomes the loud choice, which reads as
deliberate - *"Open remains the 'I can see and everyone knows it' choice"* - but
it means `radiusStuds: 30` stops being "the lantern's reach" and becomes "one
of three", and the `_radiusNote` in `data/lantern.json` is written about the
single-setting world.

`intensity`, `warmColor` and `castsShadows` stay where they are, shared by
every level. That is deliberate and it is the thing to protect: a per-level
intensity would make every candela figure a balance change and end the lighting
pass being safe to do by eye.

### Validation worth having

`validate_lantern` already refuses degenerate lights, which is the right
instinct here too. The checks that matter:

- `levels` is a non-empty array; each row is exactly
  `{name, radiusStuds, coneAngleDegrees, detectability}`.
- `radiusStuds > 0` and `0 < coneAngleDegrees <= 180` per row — the same two
  guards the block already applies, for the same reason.
- `0 <= detectability <= 1`. Out of range is not tuning: the C++ clamps it, so
  an authored 2.0 would read as a level that works and silently behaves as 1.0.
- `defaultLevel` names a row that exists. The C++ falls back to the first level
  rather than failing, so a typo here is invisible at runtime.
- **`detectability` strictly increasing down the list**, and `radiusStuds` too.
  Chris authored these as an ordered dim-to-bright progression; a list where
  Hooded is more detectable than Open is a data bug no runtime check can
  distinguish from a design choice.
**`tests/balance.test.mjs` already checks all of the above** as of this
handoff's companion commit, in a block conditional on `light.levels` existing -
so the moment you land the key the suite either passes or names the number that
moved. It also settles one rule that needed deciding, recorded here because it
is a design call and not a test detail:

> **The "shows you something" rule belongs to the default level alone.**

That check - reach must exceed `aggroRadiusDark`, or *"lighting the lantern
would never show you anything you had not already been detected by, so it would
be pure cost and no player would use it"* - was written when the lantern had one
setting doing every job. Applied per level it fails Hooded, whose reach of 8
sits below the dark radius of 12.

**Hooded is not pure cost, though, and the reason is row 53.** Relics are
visible but unreadable in darkness, so the loop is: navigate dark by
silhouette, hood up at arm's length to read one, take it. And taking it is
gated by light rather than by a separate range -
`ULLKRelicComponent::GetInteractBlock` returns `NeedsLight` until the relic is
inside the lantern's radius. So Hooded buys exactly that reading, for five
studs of extra notice radius over being dark, which makes it the cheapest way
to read a relic rather than a bad way to see. The rule applies to the setting a
lantern switches on to, because that is the one still doing both jobs.

### The struct to fill

`FLLKLanternLightRules` in `Source/LLK/Public/Data/LLKLanternData.h` already
carries them (`d065469`):

```cpp
TArray<FLLKLanternLightLevel> Levels;   // name, radiusStuds, coneAngleDegrees, detectability
FName DefaultLevelName = TEXT("Working");
```

`FLLKLanternLightLevel::DescribesARealLevel()` is there for the same job
`DescribesARealLight()` does one struct down — a stale asset that resolves and
hands back zeros is more dangerous than a missing one.

---

## Nothing is blocked on you, and nothing breaks if this waits

**Behaviour today is unchanged.** `Levels` is empty on every asset in
existence, and `ResolveLevel` synthesises a single level carrying the block's
own `radiusStuds` and `coneAngleDegrees` at detectability 1 — which is
bit-for-bit the one setting the game shipped with. The lerp's endpoints are the
two states perception already had.

So the order is yours to choose. The one thing worth knowing: a current asset
logs `level Working (1 of 3)` only once you land this, and `(1 of 1)` until
then. That line is in `ResolveAndApplyLight` specifically so the next person to
read a log can tell "the importer has not been taught the key" from "the dial
is broken".

## What is still missing after this, and whose it is

The rule layer is in. The body is not:

| | Whose |
| --- | --- |
| `light.levels` in the JSON, and the importer taught the key | **Codex** (this file) |
| The three numbers | **Chris** |
| Replicated level state on `ULLKLanternComponent`, and a key to cycle it | **Claude** |
| The visual crossfade between levels | **Claude**, in `TickFeel` beside the sway |
| `PostProcessVolume` exposure clamp | **Codex** — and still first, per `docs/LIGHTING-IN-UNREAL.md` |
| **Row 80 itself, in `docs/DECISIONS.md`** | **Claude**, written 2026-11-04 |

**Row 80 was referenced by this change and did not exist for weeks; it was written on 2026-11-04, reconstructed from the twelve `Source/` citations that had been relying on it. The realism ruling briefly occupied 80 and moved to 81.** The code
comments, `LIGHTING-IN-UNREAL.md` and this file all cite it as the row that
ratifies named lantern levels and authored detectability; 79 is the highest row
in the log. It is unwritten for a lane reason rather than an oversight:
`docs/DECISIONS.md` has an uncommitted line in it from another writer, and
staging the file would commit their in-flight edit under someone else's message
(row 26). 80 is the correct number and nobody else has claimed it. Whoever
finds the file clean should write it, from `d065469`'s commit message.

**The crossfade has a trap already closed, so do not reopen it.**
`GetLitLightSources` read `Light->AttenuationRadius` until `d065469`. That was
correct while a lantern had one setting and became wrong the moment it had
three: a fade moves the attenuation radius every frame, so relic legibility and
every Sweeper's reach would have wobbled for the length of a transition. It now
reads `GetGameplayRadiusCm()`, which resolves to the named state. `SetAttenuationRadius`
is free for cosmetics; anything gameplay reads must not come from it.
`LLK.Lantern.Levels` section 5 asserts this by moving the attenuation radius
behind the game's back and requiring the gameplay answer not to follow.
