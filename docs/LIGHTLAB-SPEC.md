# LLK_LightLab — measuring the orientation/inspection band

`FIXTURE-LIGHT-CONTRACT.md` (`f8b66ab`) says ambient fixtures must be bright
enough to navigate by and never bright enough to read a relic under. That is
two thresholds and a gap, and none of the three has a number. This map
produces them.

**Owner: Claude, via the `unreal-mcp` session. Not `LLK_DockBay`.**

## The one methodological rule

**The lab carries an identical copy of DockBay's exposure setup** — the single
`PostProcessVolume` at priority 1 with the same min/max EV100 clamp. Auto
exposure is the whole reason the original readings were untrustworthy. A
ladder measured under a different clamp transfers nothing, and would look
exactly like a successful measurement.

Verify this by reading the clamp values off DockBay's volume and writing the
same numbers in, not by assuming defaults match.

## Layout

One sealed box, no landscape, no World Partition. Three bays off a central
spine:

- **Bay A — inspection.** A relic-sized prop with legible text on its face
  (a price card is the natural fixture). Camera positions at 2, 4 and 8 studs,
  which is the range a player actually stands at to read something.
- **Bay B — orientation.** A 40-stud straight run with two gondola-height
  blockers offset from each other. The question is whether a player can tell
  the run's direction and spot the blockers from the far end.
- **Bay C — lantern.** The untested matrix cell: the lantern as a **spot**
  (which it is — `ULLKLanternLightComponent` derives from
  `USpotLightComponent`) at stepped intensity. The 6cd reading that looked
  right was taken on a point light and does not transfer.

## The ladders

Each run is one fixture type at one intensity, screenshotted from every camera
position, with the intensity written into the shot.

| Ladder | Fixture | Steps (candela) |
|---|---|---|
| 1 | Fluorescent troffer, Bay A and B | 2, 6, 15, 40, 100 |
| 2 | Neon tube, Bay A and B | 2, 6, 15, 40, 100 |
| 3 | Lantern spot, Bay C | 6, 25, 100, 250 |

Record for each shot: fixture, candela, attenuation radius, camera distance.
A shot without its parameters recorded is not data.

## What comes out

Three numbers, as a contact sheet Chris and Codex judge by eye — this is a
perception question, not a code one, since fixtures no longer enter the light
field at all:

1. **Inspection ceiling** — the highest candela at which the Bay A card is
   still *not* readable at 2 studs. Fixtures must stay below this.
2. **Orientation floor** — the lowest candela at which Bay B's run and its
   blockers are legible from 40 studs. Fixtures must stay above this.
3. **The band**, or the finding that there isn't one.

If floor > ceiling, the contract as written cannot be satisfied with uniform
fixtures and the resolution is directional ones — downlights that wash floor
without lighting a vertical card face. Better to learn that here than in a
dressed aisle.

Ladder 3 answers separately: the candela at which a spot reads as a carried
lantern rather than a floodlight.

## Out of scope

No gameplay code, no `.umap` but this one, no changes to `LLK_DockBay`,
nothing committed to `Content/` beyond the lab map itself.
