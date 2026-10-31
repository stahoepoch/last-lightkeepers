# Findings — the dock bay lantern pass, 2026-10-31

Written by Claude from measurements taken in the running editor, in the real
`/Game/LLK_DockBay` with Codex's `LLK_ExposureLock` in place (`35831f6`).
**Nothing was saved.** Two temporary lights were placed to capture an A/B and
then deleted; `git status Content/` is clean and no `TEMP_` actor remains.

---

## 1. Spot versus point, and the result that contradicts the brief

The hypothesis handed to this pass was *"what remains is a hard spotlight cone,
not an exposure problem."* **Measured in the bay, that is not what is happening.**

Same position (the keeper's gameplay anchor: floor + `KeeperEyeHeightCm` 152),
same authored values — 250 candelas, 560 cm reach (20 studs), warm colour:

| | What the bay actually looks like |
| --- | --- |
| **Spot**, 72° outer / 17.3° inner, 250 cd | The gondola face it points at is **pure clipped white**. A hard elliptical pool on the floor. |
| **Point**, 250 cd | **Worse.** Floor and both gondolas saturate to white, with hard shadows thrown behind them. |
| **Point, 25 cd** | Floor finally shows a gradient, far wall falls to black. Near slab still clipping. |
| **Point, 6 cd** | **Reads as a lantern.** Grey slab with vertical falloff, floor fading to black, far wall dark. Nothing clips. |

So the cone is not the thing that is blowing out. **At a fixed 1.0 exposure,
250 candelas saturates every surface within a few metres whatever shape the
light is**, and the cone only decides where the white patch sits. The usable
intensity in this map is roughly an order of magnitude below the authored
value — somewhere near 5–10 candelas, not 250.

That number is Chris's to set and this file does not move it. What the pass
establishes is that **`intensity` is still the open variable after the clamp**,
and that tuning the cone first would have been tuning the wrong knob.

**On the spot-to-point question itself**, which was the real subject: a point
source is the better *match to the rules*, for the reason given in the brief —
`FLLKLightSource` carries position, intensity, radius and detectability and has
no direction or cone, so the gameplay field is a sphere while the screen shows
a wedge. At a sane intensity the point also looks more like a carried lamp.
**But it is not a free swap.** `ULLKLanternLightComponent` derives from
`USpotLightComponent`, and `coneAngleDegrees` is an authored key that
`FLLKLanternLightLevel` carries *per lantern level* — the key Codex is about to
land. Changing the base class makes three authored cone angles dead data on the
day they arrive. So this is a decision for Chris with the importer landing in
front of it, not a visual tweak, and it should be taken deliberately or not at
all.

---

## 2. The occlusion exploit, measured in the room

`ULLKRelicComponent::TickComponent` samples `LLKLightField::BrightnessAt` every
0.1 s with no trace anywhere in the path. Measured against the bay's own
geometry, with `trace_world`:

- A trace from `(-300, -1000, 152)` to `(-300, -600, 152)` **hits at 190 cm** —
  the `LLK_Gondola_A1` slab is squarely between those two points.
- Over that same 400 cm (14.3 studs), the field says: at **Working** (20 studs)
  `I / (1 + (400/560)²) = 0.66 I` against a bar of `0.5 I` — **legible**. At
  **Open** (30 studs), `0.82 I`. Only **Hooded** (8 studs) fails, at `0.24 I`.

So a keeper at Working reads a relic through a solid slab whose face is 1.9 m
away. The exploit is real in this room, not only in the abstract.

**The authored blocked lane will not demonstrate it as it stands.**
`LLK.TestLane.Blocked` runs `(-300,-1200)` to `(-300,-300)`: 900 cm, blocked at
390 cm, which is correct for the sightline check it was built for. But 900 cm is
**32.1 studs**, past Open's 30, and the field returns `0.465 I` there — just
under the bar. The lane proves geometry blocks sight; it cannot prove light
ignores geometry. A demonstration needs a shorter pair.

### How much shelving in the bay is tall enough to matter

**All of it, which is the problem.** The gondola blocks a horizontal trace at
every height probed — 152, 250, 320, 400, 500 and 800 cm. These are not
half-height gondolas you can see over; they are full-height greybox slabs. Any
occlusion fix validated against them will look perfect and will have been tested
against nothing.

---

## 3. The three code questions

### Does `bLegible` latch, or re-evaluate?

**It latches — but it is not the field that would strobe.**

`EvaluateLegibility` returns early on `if (bLegible)`, documented *"Sticky on
purpose"*, and it is server-decided and replicated. A trace added to the
legibility decision cannot flicker an outline: once read, read.

The field that drives the prompt and the outline at grazing angles is
**`bCurrentlyLit`** — recomputed from the live field every 0.1 s, not
replicated, feeding `CanBeTaken()` → `RefreshInteractable()` →
`ApplyOutlineState()` (stencil 3 versus 2) and `OnInteractableChanged`. That one
**does not latch and must not**, because the affordance is supposed to drop when
the light leaves. So the concern is right and lands one field over from where it
was aimed: *legibility is free, the affordance still needs hysteresis.*

Note that `RefreshInteractable` already debounces the **broadcast** (it returns
early when the value is unchanged). That is not the same as debouncing the
underlying bool: a trace that flips every other sample flips the broadcast with
it. The hysteresis has to sit on the occlusion result, not on the delegate.

### Is the trace origin the agreed anchor?

**The lantern end, yes. There is no mount anchor, and the relic end is the
actor origin.**

- `ULLKZonePressureSubsystem::GetLitLightSources` takes
  `Light->GetGameplayLightLocation()`, which is
  `LLKLanternFeel::GameplayLightPosition(Owner->GetActorLocation(),
  KeeperEyeHeightCm)` — floor plus 152 cm. That is the gameplay anchor, not the
  swaying component location. A trace from the same call agrees with the radius
  by construction.
- **No mount anchor exists anywhere in the source** — no socket, no mount point.
  Mounting is step 5 of Chris's order and unbuilt, so the anchor to trace from
  today is eye height.
- The relic end is `Owner->GetActorLocation()` — the relic actor's **origin**.
  The brightness sample already uses it, so a trace to the same point stays
  consistent with the radius, which matters more than being geometrically ideal.
  But if relic actors are placed with their origin at floor level, that trace
  runs into the shelf the relic is sitting on. Check placement before trusting it.
- Worst at Hooded, as predicted: at 8 studs the whole budget is 224 cm, and the
  lamp sits 152 cm above the floor, so the vertical offset to a relic origin is
  most of the reach.

### The light-blocking collision channel

**There is none, and nothing is reserved.** `Config/DefaultEngine.ini` declares
no custom channels at all — zero `DefaultChannelResponses` entries.

Declaring one now is right, and the greybox makes the case harder than the brief
did: every obstruction in this bay blocks at every height, so world-static will
validate a fix that a half-height gondola would break. A dedicated channel lets
wire racks, gondolas and glass cases be authored see-through or solid per piece.

**Not done here.** `Config/DefaultEngine.ini` is currently dirty from another
writer, and staging it would commit their in-flight edit under someone else's
message (row 26). Whoever finds that file clean should add the channel.

---

## 4. One thing that must not be recorded as passed

Codex's Play default change to Standalone / 1 client is correct for a lantern
pass, and it reverts the two-clients-on-a-listen-server setup that was running
earlier. **The M1 gate — two keepers, one lights a lantern, both see the storm
charge move — was on screen and nobody confirmed the meter actually moved.**
That observation has not been made. It is not blocked by anything; it just
needs the two-client setup back and somebody watching.
