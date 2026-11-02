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

**Correction, 2026-11-02: the tag is necessary and it is not sufficient.** The
sentence above was written from the tag's side and is true there, but a bay
alone does not make the state reachable. `LLKSweeperBrain.cpp:126` also
requires `DistanceToMisplacedStuds <= Contact.ContactRange`, and
`ContactRange` is **5 studs (140 cm)** - so a Sweeper files what is at its
feet, not what it can see. Measured on this floor, the closest any patrol leg
comes to any authored relic is **6.26 studs**, so the route alone never opens
the state. See the reach note in `docs/MAP-READINESS.md`.

Put it somewhere a keeper can reach but would rather not — the point is the
walk back, with a patrol between you and it.

### Verification, and what is not verified

- `LastLightkeepers` **game** target: `Result: Succeeded`. All three changed
  translation units compiled, the test TU among them.
- `npm test`: passed, every suite.
- **The editor target did not build** — Live Coding was active, so UBT refused:
  *"Unable to build while Live Coding is active."* The editor was Chris's and
  open, so it was left alone.
- `LLK.Enemies.SweeperBrain`: **run, and it passes** - 1/1, no errors, in
  **6.5 ms**.
- **That green is not evidence about the bay.** The test is 483 lines with
  **zero** references to `UWorld`, `GEditor`, `ActorHasTag` or any map; it
  builds `FLLKSweeperSenses` by hand and sets `bHasStockBay = true` itself. So
  it cannot read the tag, and it would have passed identically with the map
  untagged. The 6.5 ms is the corroborating detail: nothing that loads a level
  finishes that fast. Recorded because a green here is the obvious thing to
  mistake for a tagged map.
- **Seen in PIE, 2026-11-02.** `BSW_STOCK_BAY` placed and the beat observed
  end to end:

  ```
  LLKSweeper_0 began filing misplaced stock.                       14:38:30.506
  LLKSweeper_0 filed LLKRelicActor_1 into the stock bay.           14:38:32.439
                                   It is still there to be found.
  ```

  1.93 s apart, against the authored `FallbackStockSeconds = 2`. The relic's
  destination was then read out of the running PIE world at
  **(450, 450, 50)** - the tagged actor's own origin - so the filing went to
  the bay rather than merely happening.
- **The condition was forced, not waited for.** An idle 16-second run - about
  three full patrol laps, the circuit being 2021 cm at 392 cm/s - produced *no*
  stock line at all, because of the 5-stud reach gate above. The observation
  above was obtained by parking a relic 3.59 studs from the Sweeper, and the
  relic was then restored to its authored `(300, 250, 90)` and the restore read
  back before saving. Both halves are the finding: **the beat works, and the
  authored floor does not reach it on its own.**

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
