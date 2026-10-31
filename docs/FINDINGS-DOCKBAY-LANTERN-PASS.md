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

---

## 5. Review of the occlusion fix (`370aa52`), 2026-10-31

Codex landed the fix the same day, and asked for it to be verified by standing
behind a gondola in the dock bay with a relic on the far side. **That test
cannot be performed as described, and the reason is worth knowing.** The editor
was closed when this review ran (`ECONNREFUSED` on the MCP port), so this is a
source review plus a placement audit, not a PIE observation.

### The fix is correct, and it answers section 3's channel question better than section 3 did

`LLKRelicIllumination::BrightnessAtVisible` filters sources through
`ILLKVisibilitySource` *before* summing, so a blocked source contributes zero
rather than a reduced amount. That is the right shape: partial occlusion of a
radial field has no honest meaning, and the alternative (attenuating by hit
distance) would invent a physics the rules do not have.

`Unknown` is treated as dark, matching `LLKPerception::Notice`. Correct and
consistent: an unanswerable line must not read as clear.

**Section 3 above recommended declaring a custom light-blocking channel. That
recommendation is withdrawn.** `FLLKWorldVisibility` traces `ECC_Visibility`,
which is a built-in channel that is already authored *per primitive* — a glass
case can be set to block `WorldStatic` and not `Visibility`, which is exactly
the per-piece control section 3 asked for. No `DefaultEngine.ini` edit is
needed, and the coupling it buys is a feature rather than an accident: a shelf
that hides a keeper from a Sweeper now also stops that keeper reading through
it. One channel, one meaning.

**What does not change: the greybox still validates nothing.** Section 2's
measurement stands — every obstruction in the bay blocks at every height from
152 to 800 cm. The fix will look perfect here and the half-height gondolas and
wire racks that art will land are precisely the cases `ECC_Visibility` has to
be authored for, piece by piece. That is now an art-time checklist item rather
than a code gap.

### The strobe risk from section 3 is now live, and the fix did not address it

Section 3 established that `bLegible` latches but **`bCurrentlyLit` does not**,
and that the prompt and the outline stencil hang off `bCurrentlyLit`. The trace
now sits inside that 10 Hz sample, which is the exact placement section 3
warned about:

> *legibility is free, the affordance still needs hysteresis.*

Nothing in `370aa52` adds hysteresis. `RefreshInteractable` still debounces only
the broadcast, so a trace that flips on alternate samples flips the prompt and
the stencil with it. At a shelf edge — hood up at arm's length, which is the
loop row 53 is built around — the affordance will chatter at 10 Hz. **This is
not a reason to revert anything**: the fix is right and the hysteresis is a
separate, small change on the occlusion result. It is a reason not to record
the occlusion work as finished.

Worth noting it is now a *correctness* issue as well as a feel one:
`ULLKInteractorComponent::ServerTake` re-checks the gate server-side, so a
player pressing take on a frame the client showed a prompt can have it refused
on a frame the server sampled blocked. Previously the two could only disagree
about distance, which is stable; they can now disagree about a trace, which is
not.

### Why the asked-for test cannot be run, and what can be run instead

**There are no relics in `LLK_DockBay`.** Zero of the 168 external actors carry
a relic, and nothing in the authored map spawns one — the only place
`ULLKRelicComponent` is ever constructed outside tests is
`LLKStoreFloorBuilder.cpp`, the harness.

And the harness runs anyway. `ALLKTestGameMode` calls
`LLKStoreFloor::BuildGroundFloor` **unconditionally**, at `HarnessOrigin`,
which it resolves to the first `APlayerStart` it finds. The `bHasExistingGround`
trace suppresses only the floor cube. So PIE in the dock bay gives the harness
store floor — counters, cases, sightline shelving and relics — spawned on top
of the authored bay, centred on the bay's own PlayerStart.

**So the fix is testable, on harness shelving rather than on dock bay gondolas:**

- Harness `ShelfHeightStuds` is 6 = **168 cm**, above the 152 cm gameplay
  anchor, so a shelf blocks the lantern-to-relic line. Section 4 of the builder
  is called *"the sightline breaks"* and exists for this.
- Harness relics sit at `CounterHeightStuds` 3.2 = 89.6 cm plus 14 cm =
  **103.6 cm**, on top of a counter spanning 0–89.6 cm. The line from a lantern
  at 152 cm descends to 103.6 cm and clears the counter's own top edge, so a
  relic does not occlude itself. Relics are also spawned `NoCollision`, so
  Codex's `AddIgnoredActor` is belt-and-braces rather than load-bearing.

The honest statement of what was tested is therefore: **the rule is covered by
automation** (four new assertions in `LLK.Loot.Relic`, including `Unknown`
treated as dark and one-blocked-one-clear summing correctly), **and the
behaviour has not been seen by anyone.** Whoever opens the editor next should
walk behind a harness shelf, not a bay gondola.

### One consequence of the build repair

Codex also fixed a unity-build test-name collision that stopped the editor
target compiling. That means the editor binary used for section 1's A/B
measurements predates it. Those measurements were taken with placed
`APointLight`/`ASpotLight` actors and `trace_world`, neither of which touches
the lantern component, so they stand — but any earlier observation that relied
on `ULLKLanternLightComponent` behaviour was made against a stale binary.

---

## 6. Resolved: the lantern stays a spot light, 2026-10-31

Section 1 left the spot-to-point swap open and called it *"a decision for Chris
with the importer landing in front of it."* **It is now decided, and the
decision is to keep the spot.**

Codex placed a temporary **spot light at 6 candelas** in the bay under the
exposure clamp and reports it warm, readable, not clipping, and still
directional while the rest of the room stays dark. Temporary actor deleted,
DockBay reloaded without saving, `Content/` clean.

That settles section 1's open question in the direction section 1's
measurements pointed. The order the two halves came in is worth keeping,
because it is the only reason the answer is trustworthy:

1. A point source at the authored **250 cd** was *worse* than the spot — both
   clipped; **6 cd** was the first setting that read as a lantern at all. So
   the cone was never what was blowing out.
2. With the intensity right, the spot's directionality turns out to be an
   asset rather than the problem it was taken for.

Had the swap been made first, the point source would have been tuned to 6 cd,
it would have looked fine, and three authored cone angles would have become
dead data for no reason anybody could reconstruct later.

**What this un-blocks.** `FLLKLanternLightLevel::coneAngleDegrees` stays live
data, so `docs/HANDOFF-CODEX-LANTERN-LEVELS.md`'s key can land as written with
no base-class question hanging over it. `ULLKLanternLightComponent` keeps
deriving from `USpotLightComponent`.

**What is still Chris's and still unmoved.** The intensity itself. 6 cd is a
measured reading of what does not clip, not an authored value — `intensity` in
`data/lantern.json` is 250 and neither this file nor anyone in this thread has
changed it. The gap between those two numbers is the whole of what the clamp
pass found, and closing it is a one-line data edit in Chris's lane.

**The gameplay/visual mismatch section 1 described is unchanged and is now
permanent by choice.** `FLLKLightSource` still carries no direction or cone, so
the gameplay field is a sphere while the screen shows a wedge. Keeping the spot
means accepting that a keeper can be refused a relic the beam appears to be
lighting, and granted one just outside it. That is a real seam and it is now a
deliberate one — worth watching in the first playtest rather than re-arguing
here.

---

## 7. The relic curriculum and a second exposure volume (`10f45d9`, `be52f3f`)

Read-only check of what landed, 2026-10-31. Both commits are real and both
write tracked files under `Content/__ExternalActors__/LLK_DockBay` — the first
DockBay actors to be committed rather than left loose.

### The bay now has relics, and section B of the prereq doc is retired

Four `ALLKRelicActor`s, each with a `ULLKRelicComponent`. So the run JSON will
carry a real `interactAttempts` and a real `secondsToFirstRelicTaken` instead
of the `NOBODY REACHED FOR ANYTHING` readout an empty bay would have produced.

Their labels are the interesting part: **`LLK_Relic_Hooded`,
`LLK_Relic_Working`, `LLK_Relic_Open`, `LLK_Relic_Occluded`.** This is a
curriculum keyed to the three lantern levels plus one occlusion case, not four
interchangeable pickups.

**Which makes the unwired-state limitation bigger than it reads.** Three of
those four exist specifically to be *compared*, and the comparison needs a
lantern that can be hooded. It cannot: `ULLKLanternData` logs `level Working
(1 of 1)` on every run, and the replicated state plus cycle key still waits on
the importer. So three of the four relics are currently **the same test**, and
the only one measuring something distinct is `_Occluded`, which exercises the
`370aa52` sight-blocking fix and is live.

Worth saying out loud before the playtest: walking all four and finding
identical reach is the dial being absent, not the relics being broken. That is
the shape of mistake the first authored-map run is most likely to produce.

Their ids are ad-hoc — `Relic_Open` and siblings, none of the twenty in
`data/relics.json` — and `RelicName` is authored on the component rather than
resolved from `DT_Relics`, so nothing is being looked up and nothing is
missing. For a curriculum probe that is the right choice, and it happens to
suit the run instrument: `RelicAbandoned` will record the *condition* that was
abandoned rather than an authored relic's name. The only consequence is that
`tests/balance.test.mjs` does not know these four exist, which is correct —
they are fixtures, not content.

### There are now two unbounded exposure volumes doing the same job

`be52f3f` added `LLK_ExposureClamp`. `LLK_ExposureLock` was already there from
`35831f6`. Both are `PostProcessVolume`s, both carry `bUnbound`, and both
override exactly two properties and no others:
`bOverride_AutoExposureMinBrightness` and
`bOverride_AutoExposureMaxBrightness`. Neither overrides `Priority`.

Scanning both packages for the same set of candidate float values returns the
same answer for each, which is consistent with identical settings — though a
presence scan can only say a value is absent, never that two volumes agree, so
that is a likelihood and not a measurement.

**Not a brightness bug today, and specifically not the one the playtest is
looking for.** Both volumes are unbound, so there is no boundary to walk
across and no view direction that enters or leaves one. Pinning min and max
also fixes exposure regardless of `AutoExposureMethod`, which neither volume
overrides and neither needs to. If brightness does move while looking around,
these are not the cause and the lantern is the place to look — a spot light
still authored at intensity 250 against a clamp that measured readable near
6 cd.

**It is a trap worth removing while it is cheap.** Two equal-priority unbound
volumes overriding the same fields are resolved by gathering order, which for
World Partition external actors is streaming order. Identical values make that
invisible; the first time somebody retunes one and not the other, the scene's
exposure depends on which actor streamed in first, and that is a bug nobody
will find by reading either volume. One of the two should go, and the survivor
should probably be the one with the clearer name.
