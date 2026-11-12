# Product pivot audit — Codex to Claude

Status: **handoff reviewed by Claude, 2026-11-11**. Chris asked Codex to
audit the whole product and experience in its lane with Claude's help. This is
an evidence-led recommendation, **not** a new `DECISIONS.md` row and not
permission to rewrite Grok's narrative files. Claude's review and corrections
are in `ADJUDICATION-CODEX-PRODUCT-PIVOT-2026-11.md`. No source, data, map, or
balance value was changed for this audit.

## The job and review

Claude challenged this diagnosis against the repository and Chris's November
9–11 story conversation in `ADJUDICATION-CODEX-PRODUCT-PIVOT-2026-11.md`.
It corrected the no-co-op call and two test claims below, and drafted document
supersession text. Findings against Grok-owned narrative files still go through
`HANDOFFS.md`; row 83 does not transfer those files to Claude. Two technical
points in that review are challenged at the end of this handoff.

## What was inspected

- `DECISIONS.md`, especially rows 3, 8–10, 16, 28, 34, 43, 46–50, 73, 79,
  and 83; `FLEET.md`, `LANES.md`, `PRODUCTION-CONTROL.md`, `ROADMAP.md`.
- `PITCH.md`, `GAME-THESIS.md`, `CAMPAIGN-SHAPE.md`, `PROPOSAL-THE-VALLEY.md`,
  `ADJUDICATION-THE-STORY-DRAFT.md`, `THE-VALLEY.md`, `QUESTS.md`, and the
  latest Galleria end finding. The narrative set is proposal unless Chris
  promotes its content into a decision.
- `Content/` map names, `DefaultEngine.ini`, the runtime module's major
  systems, `package.json`, and the portable test suite. `npm test` exited 0
  on 2026-11-11. The editor was not opened and no map was played in this pass.
- Chris's actual play feedback: the lantern felt bulky and redundant; the
  Sweeper trapped and pestered him; the Anchor map's upstairs route failed;
  he fell off the map. These are experience failures even when source tests
  pass. They need a fresh in-game check after any fix.

The working tree already contained unfinished `Source/LLK` changes and
`Content/Audio/`; this audit did not touch them. Do not read a green text
test as a compiled build or a playable map.

## The current game promise is split

| Area | What the player is now asking for | What the repository still promises or runs | Product action |
| --- | --- | --- | --- |
| Goal | A child finds father, brother, and a way home after the road is cut | `PITCH.md`, `GAME-THESIS.md`, `ROADMAP.md` promise short salvage raids and a banked haul | Write one current one-sentence promise and put a dated supersession notice on obsolete pitch text |
| First chapter | Boswell's → concourse → Sayers auto centre, with real thresholds to cross | Row 28 locks one sealed store; row 43 teaches seven lessons across the older four-floor ruin gradient | Chris must explicitly amend row 28; rebuild the curriculum around a two-floor anchor and the crossing |
| Failure | Being processed/relocated should preserve fear and player agency | Down/revive/wipe and extraction-only payout are the implemented loop; rows 8–9 still price a run | Define a bounded, playable capture consequence before replacing runtime rules |
| Time | A road washout drives the child forward; storms and seasons alter places | `zone-pressure.json` and tests retain raid timing and a Warden around minute eleven; `loot.json` authors a 20-second dock hold | Retain pressure as local escalation only if it supports a chapter; stop treating its old raid timing as campaign pacing |
| People | The father and brother need believable water, food, shelter, and movement | `CONTAINED-KEEPERS.md` says a system may wait forever; the adjudication infers that therefore the family has no clock | Resolve physiology and elapsed time on screen; containment alone does not feed anyone |
| Players | Chris said “no co-op” on 2026-11-11; one child is the viewpoint | Locked rows 36, 50, 51, 52 and current character/revive systems still describe a keeper crew | Proposed row 89 records Chris's call; strike the conflicting locked rows by his ruling, then remove the two-client product gate |
| Ending | Family/home route and later civic recovery need a causal final action | `CAMPAIGN-SHAPE.md` ends at a finished hall; row 46 raises the summit to reclaimed civic knowledge; our conversation floated an airport departure route | Choose what the first game's final playable action changes; avoid asserting the airport mechanism before it is authored |

The pass/fail suite presently guards the former contract. A real assertion
says `banked loot is lost without extraction`, and another requires a hold to
complete before securing. The dock's **20 seconds is a data note, not a test**;
the Warden's **700 is a current row 73 rule**, not legacy timing. Claude caught
both distinctions in the adjudication. Existing tests remain useful until
replacement rules and tests deliberately land. Their presence is evidence of
work to migrate, not a reason to preserve the old product.

## Runtime migration inventory

This is a source inventory, not a request to delete working extraction code.

| Seam | Current implementation | Needed for the proposed story |
| --- | --- | --- |
| World lifecycle | `ULLKRunStateSubsystem` is a `UWorldSubsystem`; it ends a world on secured extraction or crew wipe, and clears carried items on wipe | Keep this local encounter outcome until a chapter failure and recovery rule is chosen; do not make loss of a haul mean loss of family knowledge |
| Cross-chapter memory | No `USaveGame`, save slot, checkpoint, chapter, mission, or family-state type appears in `Source/LLK` | A small persistent record of reached routes, learned clues, rescued people, and elapsed story time is required before the first meaningful return |
| Player verbs | `ULLKInteractorComponent` resolves a relic, downed teammate, or extraction zone; carrying is a replicated count. No general prop, placement, throw, distraction, or lantern `Present` interaction exists in the runtime | Prove one authored physical manipulation in the first chapter: a child uses an object or service interface to change a machine's route. Define its input and feedback before adding a generic inventory |
| Exit | `ULLKExtractionSubsystem` announces a rig, holds, and secures a haul; `ALLKGameState` replicates its snapshot | A route transition must say *where the child can go and why*; the existing hold can be reused for a gate or lift after that rule is authored |
| Escalation | Zone Pressure and the Warden are tuned to a roughly eleven-minute raid; `pacing.mjs` and balance tests defend the timing | Local machine alarm and storm consequences can survive, but the raid clock must not silently become the family-search clock |
| Presentation | `LLKExtractionReadout` and the debug HUD explain the dock and its hold | The first chapter needs scene-level direction: person sought, route ahead, and consequence of light, without adding a permanent objective paragraph |
| Launch | `DefaultEngine.ini` opens `LLK_DockBay` with `ALLKTestGameMode`; only five `.umap` files exist, none a verified story chapter | Keep DockBay as harness until Boswell's route plays; then give the chapter its own intentional entry and failure path |
| Player scale | `ALLKCharacter` keeps the default 88 cm capsule half-height and puts its camera 64 cm above centre, yielding the 152 cm eye assumed by `MAP-READINESS.md` | A child body, eye, hand, and lantern pose need one in-editor measurement pass; then re-evaluate shelf sightlines, passages, reach, and the stair route together |

**Order:** prove traversal and the family-trail beat in one map first; introduce
the minimum persistent story record before a second chapter or revisited
season; only then migrate old extraction tests to new outcome tests. Preserve
the current tests while their old rules remain live. A wholesale rename of
`Run/` would obscure which functions are still useful for a called lift,
powered door, or local return.

## Proposed experience spine to challenge

**Promise:** a young Lightkeeper survives a failed mission, follows the
family's trail through active buildings, and earns a route home. The player
discovers what happened by handling real objects, watching machines work,
and choosing when the lantern makes them visible. Terror comes from being
small and unrecognized inside systems that continue without them.

**Repeatable scene, not a raid timer:** arrive with a concrete need; study a
place and its machine's last job; use the lantern to read, authenticate, or
open a route; take a useful object or learn something about a person; avoid,
interrupt, or outmaneuver the machine; then reach a refuge or continue toward
the next place. Taking supplies supports travel and home. Discoveries and
people advance the mission. The player can return later, when weather,
stock, and access have changed for physical reasons.

**Ending target to write backward from:** the family is found or their fate
is learned, and the child performs an earned action that opens a viable way
home. Lanternhold's restored rooms and any later civic recovery make the
return matter, but cannot replace the family resolution. The precise ending
mechanism, any tragedy, and what the rescued family can physically survive
are open story decisions, not assertions about the existing build.

**Experience tests:** a child-sized viewpoint must change cover and sightlines
without making every shelf opaque; a lit lantern must supply a useful action
and a visible cost; a Sweeper must be heard and seen doing a legible job before
it becomes a chase; and every threat encounter must leave a player-controlled
route or counterplay. Chris's failed upstairs route, corner traps, and map
fall are direct failures of these tests. Do not spend a pass on a prettier
lantern or a larger store while those failures remain in the playable path.

**Time scale:** the first crossing can take a day or a few days. Seasonal
departments need a later return and a supply-chain cause; a Christmas change
inside a two-day rescue would make the living mall less credible. A storm
changes access and danger without acting as an invisible extraction timer.

## Keep, test, and retire

The decision log needs a **named migration**, not a blanket unlock:

| Row | Keep or seek Chris's amendment | Reason |
| --- | --- | --- |
| 3, 79 | Amend the playtest question and its metrics; retain the rule that telemetry never drives gameplay | “Kill lanterns, bank, argue for one more room” measures a raid, not whether a new player follows a family trail and chooses to manipulate the world |
| 28, 43 | Amend the sealed-store boundary and old seven-lesson contents; retain the store as a played tutorial | A Boswell's-to-Sayers crossing and a two-selling-floor Boswell's cannot both satisfy the old wording |
| 8, 48, 59, 74 | Decide which extraction and crate-payout rules remain as *local operations* and which cease to define every chapter | The child can seek a way home without a recovery truck ending each place; solo salvage and camp loss still need an authored rule |
| 49, 75 | Keep containment and the tested hold/pause mechanism, but amend the “after first slice” staging and universal extraction premise if the family is chapter one's subject | A machine can process someone without making them a raid payout or an escort NPC |
| 10, 47, 64 | Preserve the issued light, last-job machines, and agency during escalation | These still distinguish the game and constrain any new story rule |
| 36, 50, 51, 52 | Strike after Chris's no-co-op ruling is recorded against proposed row 89 | These describe a product Chris has now rejected |

Claude should check this row list against the whole log before proposing text.
In particular, row 10 says the cost of unlimited light is “the meter,” while
row 73 now makes the weather clock independent of player action. The first
chapter must show the actual cost the runtime applies—machine notice and
local procedure—rather than teach an obsolete causal link.

**Keep as the game's identity:** one issued lantern whose use reveals value
and attracts attention (rows 5 and 10); place-specific machines finishing
old jobs rather than a central villain (row 47); audible and visible threat
cues (row 16); relics about people; physical routes and earned access; and a
world that moves when the player returns. Preserve the playable Sweeper and
inspection systems as test equipment while the story slice is made.

**Test as proposals:** the washed-out road and surviving driver; the
night/day/night opening; the child at 12–13; a service route across the
Galleria; set-down lantern; daylight as an impassable concourse; and the
airport or regional transit ending. A proposed fact does not become a locked
row merely because several documents quote it.

**Retire by named decision, never by file deletion:** the sealed-store limit
in row 28 if chapter one crosses to Sayers; the raid-only M3/M4 success test;
the final hall as the sole ending; and the assumption that every place must
end with a called extraction and a wipe. The implementation can be reused
for a hold, gate, lift, or local return after its semantics are decided.

## A correction to our own story answer

Codex previously answered the starvation objection by inventing an automated
network of kitchens, ration depots, water systems, and regional transfers
lasting months. **None of that has been authored or observed.** Claude's
adjudication makes the opposite unsupported leap: a machine willing to wait
forever implies its captive can live forever. Both arguments skip the human
body.

Recommendation for Chris to judge: rescue or establish reliable contact with
the father and brother within days or a few weeks, then let seasonal returns
and civic recovery unfold after that. If a long captivity is chosen instead,
the game must show its actual food, water, sanitation, medical care, staffing,
and failure modes before a seasonal campaign can use it. No invisible family
countdown should be installed merely to patch the gap.

## The first product proof to build backward from

One **controllable Boswell's-to-concourse segment**, around ten minutes, is
the first story slice. It is a subset of the full 396 m crossing, not a tiny
claim that the entire mall exists. It starts with a legible practical goal,
lets the player see the crew's capture in play, demands one useful lantern
choice, lets them manipulate one physical object or service interface, shows a
Sweeper's job through sound and physical evidence, and ends when the player
opens or crosses a real route toward Sayers. The player keeps
control through the consequence. A safe path must be visible enough to choose;
the Sweeper cannot corner-seal it. A player must be able to reach any authored
upper floor and cannot fall through an unguarded map edge.

Acceptance is a fresh player reaching that threshold and stating, without a
debug HUD, **who they seek, where they are going next, what their light cost
them, and why the machine acted as it did**. Capture, if triggered, must lead
back to a playable decision quickly. Test the single-player experience.

The current `LLK_Boswells_FirstRun.umap` remains a mechanics lab. The larger
`LLK_Boswells_Anchor.umap` is not a proven chapter: Chris could not reach its
upper floor. `FINDINGS-GALLERIA-ANCHOR-ENDS.md` gives a **396 m straight-line
estimate** and infers Sayers' far end from a leasing plan; it does not give a
measured walked route or an authored concourse width. The chapter map should
not silently convert that estimate into collision geometry.

## Concrete work split

1. **Claude, systems/doc lane:** challenge this audit. Propose exact
   supersession text for `PITCH.md`, `GAME-THESIS.md`, and the M3/M4 product
   gates; identify which locked rows need Chris's ruling. Do not implement
   new balance values or edit Grok's seven-file narrative set.
2. **Grok, narrative lane via a handoff:** turn the approved first chapter and
   ending into a causal outline with elapsed days, food/water logistics,
   family trail, and an earned reason to cross each place. Distinguish a
   first-game ending from later seasonal returns.
3. **Codex, editor/build lane:** after the product call, validate the first
   story route, floor access, map boundaries, actual light/readability, and a
   packaged build. Replace DockBay as the launch experience only after that
   route plays. Keep UE 5.9 migration a separate proof from story changes;
   `.uproject` still associates with 5.8 today.
4. **Chris:** the no-co-op answer is already given. Rule the rescue time
   horizon, the cost of losing a camp, and the first game's final action.
   Amend or strike locked rows deliberately; a proposal page cannot do it.

## Done when

- Claude's source-backed review is recorded in its adjudication; the two
  challenged points below need resolution before implementation orders.
- Chris can read one current promise and one chapter-one acceptance test
  without encountering an equally authoritative extraction pitch.
- The first playable proof above is a card in the active production sequence,
  with one map owner, a named route, and a solo play check.

**Already ruled out:** a Shepherd/Beacon intelligence as the explanation
(row 47); a battery timer (row 10); a playable airport tutorial (row 43);
keeping co-op as a default after Chris's no-co-op call; treating the 396 m
chord as a measured interior walk; and declaring the map ready from `npm test`.

**Must not touch:** Chris's `data/*.json` balance values or locked decision
rows without his ruling; another writer's `Content/` asset; the dirty runtime
files in this checkout; and Grok's narrative set under row 83.

## Post-review source checks for Claude

Two points in `ADJUDICATION-CODEX-PRODUCT-PIVOT-2026-11.md` need a source-level
correction before they become work orders:

1. **Light's cost is not the weather meter.** Rows 72–73 make its rate flat
   and independent of the lantern. `data/enemies.json:17-18` authors the
   Sweeper's dark/lit notice range as 12/38 studs; `LLKPerception.cpp` lerps
   reach by detectability, and `LLKInteractorComponent.cpp:528-531` makes a
   lit relic hold faster. `LLKDebugHUD.cpp:1081-1110` already prints the two
   notice ranges. The player *felt* the Sweeper's cost as pestering and
   trapping. A visible pressure meter cannot, by itself, make that local
   light trade legible; its number does not respond to switching the lantern.
   Test the lamp's useful read and its machine consequence in the scene, with
   the debug HUD off. Row 10's “cost is the meter” phrase needs a named
   correction even if its unlimited-charge rule stays.
2. **A camp can reuse the hold but cannot be a rename-only migration.** The
   extraction `Secured` branch calls `Run->MarkRunSecured()`
   (`LLKExtractionSubsystem.cpp:448-456`), and `IsRunOver()` remains true
   thereafter (`LLKRunStateSubsystem.h:97`). No save/checkpoint/chapter type
   exists in `Source/LLK`. A camp that advances a frontier across nights must
   preserve learned routes, people and elapsed time, then start a new local
   encounter without the prior success terminus. That campaign shell is new
   work even if `Idle → Announcing → Holding` and its pause behavior are reused.
   Prove the first camp in one map before claiming the cross-night loop is
   implemented.

**Do not treat “lock 84–91” as one safe shorthand yet.** The proposed rows
combine Chris's explicit direction with unplayed mechanisms:

- Row 85 joins *no mandatory extraction* to a required camp, a frontier,
  “never go home,” and no secured/lost items. The first clause is Chris's;
  the camp model and loss economy still need a played proof and a separate
  ruling.
- Row 86 joins *no combat* to automatic fatality and a global machine alert
  for an attack. Chris described fighting as likely fatal, which does not
  establish those exact rules.
- Row 88's one-way travel within a chapter could prohibit backtracking inside
  the mall. A washed-out road prevents the trip to Lanternhold; it need not
  turn every store threshold into a one-way door.
- Row 90 narrows Chris's 9–13 range to 12–13 on sightline grounds; measure
  before ruling the age. Row 91 still says captive family have “no clock,”
  although Claude's own §5 correction accepts the physiological limit.

Ratify the campaign and solo direction as far as Chris has stated them, then
separate the camp, failure, age and family-timeline calls. This lets the
single-map slice test an actual choice rather than spend weeks implementing a
bundle whose speculative clauses became locked by proximity.
