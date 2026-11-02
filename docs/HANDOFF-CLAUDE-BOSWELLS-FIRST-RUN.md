# Boswell's First Run — Claude handoff

## Slice now present

`/Game/LLK_Boswells_FirstRun` is the compact playable loop:

1. Start at the front aisle (`WL_PlayerStart`).
2. Search the high-value **Misrouted Toy Crate** (`BSW_WRONG_CRATE`), with two
   smaller optional relics nearby.
3. Avoid the active Sweeper on the authored three-marker `LLK.Patrol` route.
4. Return to `BSW_LOADING_DOCK_EXIT`, call recovery with `E`, and hold the
   Loading Dock through extraction.

The map deliberately has no `LLK.Harness` WorldSettings tag, so it receives no
server-only scaffold world. PIE was verified to log one registered `LoadingDock`
zone, an awake Sweeper, and the authored patrol route.

## Ownership transfer

Claude may now own **only** `Content/LLK_Boswells_FirstRun.umap` for the next
art/animation pass. No one else should save that map concurrently.

## Finish next

- Replace the temporary local hard-surface lantern rig with dedicated
  first-person arms and a held-lantern pose. Do not use the full co-op body as
  a first-person substitute.
- Keep gameplay sourced from `ALLKCharacter::LanternAnchor` / the gameplay
  light API. Cosmetic mesh sway and hand animation must not move relic
  inspection or enemy perception.
- Selectively migrate only the retail fixtures needed for this aisle from the
  asset intake: gondola/rack, toy counter, dead display case, dock door, and
  exit sign. Do not import vendor demo maps or a whole marketplace pack.
- Replace the present cream/green emergency-light placeholder with authored
  fixture housings, keeping light orienting rather than inspection-bright.
- Add the Sweeper's visual/audio telegraph and a physical recovery-door cue.

## Do not regress

- Do not add a `PostProcessVolume` until the exposure decision is made.
- Do not turn the lantern into a camera flashlight or attach gameplay position
  to animated hand bones.
- Do not remove the roof, nav bounds, patrol tags, or LoadingDock zone.
- Preserve the one-owner-per-`.umap` rule.

## Verified before handoff

- `LastLightkeepersEditor` build: `Result: Succeeded`.
- `npm test`: passed.
- PIE startup: no runtime harness; one Sweeper initialized with dark/lit ranges
  and one LoadingDock extraction zone registered.
