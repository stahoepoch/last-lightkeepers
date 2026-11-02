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

~~Tag any actor **`LLK.StockBay`** and the behaviour turns on~~ — **that was
false, and measuring the floor proved it.** Corrected 2026-11-02.

Tagging the bay is necessary and was not sufficient. The brain also gated on
`DistanceToMisplacedStuds <= Contact.ContactRange`, 5 studs, and **the closest
any patrol leg in Boswell's comes to any authored relic is 6.26 studs** — 35
centimetres outside, with three laps producing nothing. The MCP session tagged
`BSW_STOCK_BAY` at (450, 450, 50) in `46b46ee`, observed the beat end to end
in PIE only by parking a relic 3.59 studs away, and asked whether to nudge a
patrol point or a relic to close the gap.

**Neither. The gate was wrong, not the floor.** The first version used
`ContactRange` for both noticing and reaching, on the argument that reusing an
authored number invented none. That was the wrong operand dressed as a tidy
saving — contact range is the reach for *touching a keeper*. So now:

- it notices within **`aggroRadiusDark`** (12 studs, authored — what this
  machine becomes aware of nearby in the dark),
- walks to it (`ELLKSweeperMove::MisplacedItem`, which the first version left
  out on the theory that a Sweeper should never leave its route),
- and files within **`ContactRange`** (5 studs, authored), which now means
  physical reach and nothing else.

Still no invented number, and **the authored floor works unaltered** — 6.26 is
asserted in the test precisely because it is the distance the real map offers.
Diverting a stride to deal with a box is not an appetite; it is a floor
cleaner doing its job, and `attractedToDroppedLoot` is literally the word
*attracted*. What would break row 47 is diverting toward a keeper, which it
still never does.

Bounded by `losesTargetAfterSeconds`, so a relic behind glass or across a hole
in the navmesh makes it give up and log a warning rather than walk at the
thing forever.

**Tag none and the state still never runs**, unchanged: a Sweeper filing a
relic into nowhere would delete the run's objective.

### The beat had no cues at all, which the MCP session caught

`bStockStarted`, `bStockInterrupted` and `bItemShelved` each only wrote a log
line, so a player could watch a crate move and be given nothing to connect it
to. There are now `OnStockStarted`, `OnStockInterrupted` and `OnStockFiled`,
matching `OnTelegraph` so there is one pattern rather than two. `OnStockFiled`
broadcasts at the **bay**, not the Sweeper, so a cue can point at where the
crate went. The sound and the effect are Chris's.

`OnStockInterrupted` is the load-bearing one: it is the only moment the player
caused on purpose — they lit up to save their crate — and without it they
cannot tell that it worked.

**Correction, 2026-11-02: the tag is necessary and it is not sufficient.** The
sentence above was written from the tag's side and is true there, but a bay
alone does not make the state reachable. `LLKSweeperBrain.cpp:126` also
requires `DistanceToMisplacedStuds <= Contact.ContactRange`, and
`ContactRange` is **5 studs (140 cm)** - so a Sweeper files what is at its
feet, not what it can see. Measured on this floor, the closest any patrol leg
comes to any authored relic is **6.26 studs**, so the route alone never opens
the state. See the reach note in `docs/MAP-READINESS.md`.

**Resolved the same day in `e0efb06`, and the measurement is why.** The 6.26
was right and the *gate* was wrong: `ContactRange` was doing double duty as
both "can touch a keeper" and "can see a box in the lane". Noticing is now
`aggroRadiusDark` (12 studs, authored), which 6.26 clears comfortably; the
Sweeper then walks there via `ELLKSweeperMove::MisplacedItem` and files within
`ContactRange`, which from there means physical reach and nothing else. **So
the route does now open the state, and the map was never the problem.** 6.26 is
asserted in the test, with a 40-stud negative control beside it, so moving that
aisle makes the test complain rather than making the beat quietly stop.

Put it somewhere a keeper can reach but would rather not — the point is the
walk back, with a patrol between you and it.

### Verification, and what is not verified

- `LastLightkeepers` **game** target: `Result: Succeeded`. All three changed
  translation units compiled, the test TU among them.
- `npm test`: passed, every suite.
- **The editor target builds, and the test is run against the fix.** Chris
  closed the editor, so UBT stopped refusing: `Result: Succeeded`, and
  `UnrealEditor-LLK.dll` moved to **10:08:46**, later than the newest changed
  source at 09:46:59. No Live Coding patch DLLs, so it is a clean full build
  rather than a patched image.
- **The binary was checked for the new code, not just the new timestamp.** All
  four of `e0efb06`'s added assertion strings are present in the DLL. They read
  as absent on a first pass because `TEXT()` literals are **UTF-16** and the
  search was ASCII - the wrong operand, caught and redone, which is the same
  error this repo keeps naming.
- `LLK.Enemies.SweeperBrain` run headless against that binary:
  `Result={Success}`, 1 test performed, no events.
- **And the green was checked rather than believed.** The 6.26 notice was
  inverted to expect `Patrol`, rebuilt and re-run, and it failed by name and
  line:

  ```text
  Error: Test Completed. Result={Fail} Name={SweeperBrain}
  Error: ROW 47: at Boswell's real 6.26 studs it notices the crate:
         The two values are not equal. [...LLKSweeperBrainTest.cpp(417)]
  ```

  So the new block executes rather than merely compiling - which the string
  check above could not establish on its own. Reverted with `git checkout --`,
  rebuilt, re-run green, and the test file is byte-identical to `e0efb06`.
- **The run's exit code says nothing.** Both the passing and the failing run
  exited **0**. See trap 14.
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
