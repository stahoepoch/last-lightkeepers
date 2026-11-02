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

## Taken up 2026-11-01 — the indifference beat

The slice as handed over has the Sweeper as an obstacle you avoid. That is
stealth, not horror. The missing beat is row 47's: **the first Sweeper should
not threaten the player at all — it should finish its route, ignore them
completely, and take something they wanted.** Being ignored by something that
costs you is what makes a player decide to grab their haul and leave.

Landed in the brain and the pawn, not the map:

- `ELLKSweeperState::Stock`. On patrol, unnoticed, with stock in the wrong
  place within the authored `ContactRange`, a Sweeper files it.
- It wires **`Sweepers.attractedToDroppedLoot: true`** — Chris's authored key,
  imported since M0.5, asserted for presence by `tests/balance.test.mjs`, and
  consumed by nothing until now. One of the gaps
  `FINDINGS-ENEMY-OPTIONALS.md` recorded.
- **Being noticed interrupts it.** The only way to stop a Sweeper taking your
  crate is to let it see you — light is already a rate on the held pickup, and
  here it is the price of interrupting the building's work. Asserted both ways
  round, including on the frame the job would otherwise have completed.
- **It relocates, never deletes.** `TakeByServer` is the player's verb; the
  Sweeper moves the relic to an authored bay and the cost is the route and the
  time to go get it. The board forbids arbitrary bag deletion outright.

### One thing this needs from the map, and it is one actor

Tag any actor **`LLK.StockBay`** and the behaviour turns on; the nearest one
is where filed stock goes. **Tag none and the state never runs**, which is the
deliberate gate: a Sweeper filing a relic into nowhere would delete the run's
objective. Same discipline as `LLK.Patrol`.

Put it somewhere a keeper can reach but would rather not — the point is the
walk back, with a patrol between you and it.

### Verification, and what is not verified

- `LastLightkeepers` **game** target: `Result: Succeeded`. All three changed
  translation units compiled, the test TU among them.
- `npm test`: passed, every suite.
- **The editor target did not build** — Live Coding was active, so UBT refused:
  *"Unable to build while Live Coding is active."* The editor was Chris's and
  open, so it was left alone.
- **The automation test has therefore not been run**, only compiled. Run
  `LLK.Enemies.SweeperBrain` once the editor is free.
- **Nothing has been seen in PIE.** The beat is unobserved until a bay is
  tagged and someone walks it.

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
