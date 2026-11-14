# Roadmap — the solo horror campaign

Status: **active work order, 2026-11-14.** This replaces the October raid
roadmap, preserved at `docs/archive/ROADMAP-RAID-2026-10.md`. Read
`GAME-THESIS.md` for the product and `PRODUCTION-CONTROL.md` for how work is
integrated. `DECISIONS.md` retains the decision history; its unresolved pivot
rows are not silent permission to rebuild the co-op extraction game.

## One playable target

Build the opening crossing in Unreal: **Boswell's → Galleria concourse →
Sayers threshold**. The first proof is a compelling, finishable chapter path,
not a whole Valley, a feature inventory, or a greybox whose appearance cannot
test the intended experience. Boswell's must read as a substantial two-floor
department store, with real-scale paths, departments, vertical circulation,
cover, emergency lighting, and selective use of the assets already licensed.

The chapter should let a new player:

1. Enter and orient without a minimap or debug text doing all the work.
2. Move between both floors through a working route and choose paths around
   obstacles and machines. No fall-through or inescapable corner.
3. Hear and see a machine's intention, hide or flee, and keep movement control
   through contact and recovery.
4. Use the carried lantern for a necessary read or authored interaction, while
   ambient light remains sufficient to navigate.
5. Find a story clue, change part of the building, and reach Sayers with a
   clear reason to continue.

## Work slices and exit evidence

| Slice | Work | Exit evidence |
| --- | --- | --- |
| **0. Reconcile the product** | Current thesis, roadmap, production order and decision conflicts agree on solo, campaign, stealth and puzzle. Retire obsolete work cards rather than deleting history. | An agent opening the repo reaches this roadmap; no current task requires two players, a raid payout, or Roblox. Remaining runtime migration is named. |
| **1. Make the store traversable** | Correct keeper scale, two floors, stairs or escalator, wide connected aisles, counters and alternate routes; bake and verify navigation. | One person can walk the intended Boswell's path, get upstairs and back, use cover, escape a Sweeper and never leave the map accidentally. |
| **2. Make the instrument matter** | Dress emergency orientation lights, give the lantern a readable physical presence, author one information read and one infrastructure use, and expose its notice cost through machine behavior. | The chapter remains navigable with the lantern off, yet a player must choose when and where to use it to advance. They can explain what using it cost. |
| **3. Make a threat worth answering** | Give machines distinct jobs, directional sound and visible route commitments. Tune pursuit, contact, recovery and disengage around escape paths. | A fresh player can identify a cue and choose hide, detour or sprint. No pin, long input lock, unexplained hit or endless harassment is accepted. |
| **4. Finish the chapter path** | Connect the clue, environmental puzzle and route change to the concourse and Sayers threshold; persist the meaningful result. | A build can be started, played to the chapter end and resumed without editor commands or a verbal walkthrough. |
| **5. Integrate the look and playtest** | Replace temporary shapes with selected licensed assets, authored materials and animation; mix sound; measure performance and run fresh-player sessions. This work starts in slice 1 and continues through every slice. | Players recognize a department store, use its geometry, read its lighting and want to continue. Failures found in play become the next work card. |

Slices are an order for proving the game, not a ban on art or sound until the
last row. Each slice ends in a playable integration pass. A feature is not
credited because its C++ compiles or an asset exists on disk.

## Work explicitly outside the current gate

- Two-client and listen-server acceptance, multiplayer UI, co-op crew balance
  and private matchmaking. Existing replication can stay until there is a
  reason to remove it; no new work is scheduled to prove a feature Chris cut.
- Timed raid exits, extraction payout and haul-loss balancing. Their code and
  tests remain historical or migration material until a current chapter rule
  replaces them. Do not mistake a green extraction suite for product progress.
- Roblox implementation. `src/` and `default.project.json` are a frozen
  prototype, not the game target.
- A large mall footprint before one dense, convincing crossing works. The
  route to Sayers matters; empty floor area does not satisfy it.

## Existing code and data to reconcile

The previous loop remains in the checkout. Removing tests before replacing
their behavior would hide regressions; letting them define the new product
would preserve the wrong goal. Migrate each seam with its replacement and a
playable proof:

| Seam | Current location | Next action |
| --- | --- | --- |
| Raid/extraction state and UI | `Source/LLK/Public/Run/`, `Source/LLK/Private/Run/`, `LLKExtractionReadout` | Audit what can drive chapter transitions or safe places; repurpose only after the leave/return and camp-failure rules are decided. Keep existing code compiling meanwhile. |
| Raid clock and payout data | `data/zone-pressure.json` (`earlyExitSeconds`, `hardCeilingSeconds`, `extractionCall`), `data/loot.json` | Chris decides the replacement values and loss rule. No raid timer should silently end the story chapter. |
| Old contract tests | `tests/extraction.test.mjs`, raid assertions in `tests/balance.test.mjs`, `tests/curriculum.test.mjs`, and related suites | Label current coverage as legacy behavior. Replace assertions alongside runtime changes; a green suite today is regression evidence for old code, not chapter acceptance. |
| Playable map | `Content/LLK_Boswells_Anchor.umap` and `Content/LLK_Boswells_FirstRun.umap` | Assign one map writer, inspect what each contains, and choose the chapter map before editing a binary level. Do not infer that either is already a complete crossing. |
| Narrative spine | `docs/SPINE.md`, `docs/QUESTS.md`, `docs/THE-WORK.md` and related Grok-owned files | Reconcile the old solo-or-friends departure and return-with-loot chapters with row 92. Keep the useful practical-job and changing-world ideas. Do not treat proposed story beats as already authored level objectives. |

This is a migration register, not a request to delete working code or to
silently pick answers from proposed decision rows.

## Review gate

Run a fresh-player chapter test in a packaged or otherwise reproducible Unreal
build. Watch whether the player can orient, move, hide, use the lantern for a
reason, understand a machine's cue, escape, solve the route and reach the end.
Record where they were confused, bored, trapped or unable to see. Fix the
repeated failures before expanding the campaign. The obsolete question about
banking loot and arguing for one more raid does not judge this game.
