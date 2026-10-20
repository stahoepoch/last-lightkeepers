# Flooded Mall graybox

This is the first playable space: **Galleria bones, Rain Garden heart**.

`FloodedMall.rbxmx` imports as a single model in Studio and is also picked up
by the Rojo project under `Workspace.Map`. It deliberately uses fictional
landmarks and has no real store names, logos, or copied signage.

## First player route

1. Spawn at the Loading Dock, on the west side of the Main Concourse.
2. Cross the dark concourse or take the upper gallery toward the amber Relay.
3. Descend into the Rain Garden via either ramp; the Stormfall Water Feature is
   the central visual marker.
4. Bank at `RelayBank_RainGarden` and choose to return to the Loading Dock or
   commit to the lower Freight Lift.

## Studio tagging manifest

The game services use CollectionService tags. Apply the following once the
model is in Studio; the names make each target unmistakable.

| Part name | Tag | Required attributes |
| --- | --- | --- |
| `RelayBank_RainGarden` | `RelayBank` | none |
| `Extraction_LoadingDock` | `Extraction` | `ExtractionId = "LoadingDock"` |
| `Extraction_FreightLift` | `Extraction` | `ExtractionId = "FreightLift"` |

Add `Loot` and `SweeperSpawn` marker parts in each named district after the
first walk-through. They are intentionally not baked into the graybox: their
exact count and placement should follow playtest sightlines rather than a map
diagram.

## Art direction that affects play

- Teal structural beams / glazed roof / two-level sightlines: orientation.
- Amber only marks player-controlled light and bank safety.
- Electric blue marks storm water, hostile machinery, and long-distance threat.
- Keep the Rain Garden shallowly flooded, not fully submerged; movement must
  remain readable and mobile-friendly.
