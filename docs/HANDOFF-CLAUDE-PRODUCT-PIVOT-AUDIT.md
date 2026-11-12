# Product pivot audit — Codex to Claude

Status: **handoff for adversarial review, 2026-11-11**. Chris asked Codex to
audit the whole product and experience in its lane with Claude's help. This is
an evidence-led recommendation, **not** a new `DECISIONS.md` row and not
permission to rewrite Grok's narrative files. No source, data, map, or balance
value was changed for this audit.

## The job

Challenge the product diagnosis below against the repository and Chris's
November 9–11 story conversation. In Claude's systems and documentation lane,
identify the smallest set of decision-row corrections and document status
banners that would let the team build a story slice without treating the old
extraction pitch as current canon. Send findings against Grok-owned narrative
files through `HANDOFFS.md`; row 83 does not transfer those files to Claude.

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
| Time | A road washout drives the child forward; storms and seasons alter places | `zone-pressure.json` and tests still assert minute targets, a 20-second dock hold, and a Warden around minute eleven | Retain pressure as local escalation only if it supports a chapter; stop treating its old raid timing as campaign pacing |
| People | The father and brother need believable water, food, shelter, and movement | `CONTAINED-KEEPERS.md` says a system may wait forever; the adjudication infers that therefore the family has no clock | Resolve physiology and elapsed time on screen; containment alone does not feed anyone |
| Co-op | A single child is the emotional viewpoint | Row 50 locks immediate 1–4 player co-op; current character and revive systems describe a keeper crew | Choose a co-op presentation that preserves the child's guilt and never makes friends wait for an unlock |
| Ending | Family/home route and later civic recovery need a causal final action | `CAMPAIGN-SHAPE.md` ends at a finished hall; row 46 raises the summit to reclaimed civic knowledge; our conversation floated an airport departure route | Choose what the first game's final playable action changes; avoid asserting the airport mechanism before it is authored |

The pass/fail suite presently guards the former contract. Its green result
includes `banked loot is lost without extraction`, `the dock requires a
20-second hold`, and `the Warden arrives at 700s`. These checks should remain
green until the replacement rule and its tests are deliberately landed.
Their presence is evidence of work to migrate, not a reason to preserve the
old product.

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
choice, shows a Sweeper's job through sound and physical evidence, and ends
when the player opens or crosses a real route toward Sayers. The player keeps
control through the consequence. A safe path must be visible enough to choose;
the Sweeper cannot corner-seal it. A player must be able to reach any authored
upper floor and cannot fall through an unguarded map edge.

Acceptance is a fresh player reaching that threshold and stating, without a
debug HUD, **who they seek, where they are going next, what their light cost
them, and why the machine acted as it did**. Capture, if triggered, must lead
back to a playable decision quickly. Test solo first; row 50 also requires a
co-op proof once the protagonist presentation is chosen.

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
4. **Chris:** rule the protagonist/co-op presentation and the rescue time
   horizon. Amend locked rows deliberately; a proposal page cannot do it.

## Done when

- Claude reports where this diagnosis is wrong, with file or play evidence,
  and a dated systems-doc correction plan.
- Chris can read one current promise and one chapter-one acceptance test
  without encountering an equally authoritative extraction pitch.
- The first playable proof above is a card in the active production sequence,
  with one map owner, a named route, and a solo play check.

**Already ruled out:** a Shepherd/Beacon intelligence as the explanation
(row 47); a battery timer (row 10); a playable airport tutorial (row 43);
locking co-op behind rescued keepers (row 50); treating the 396 m chord as a
measured interior walk; and declaring the map ready from `npm test`.

**Must not touch:** Chris's `data/*.json` balance values or locked decision
rows without his ruling; another writer's `Content/` asset; the dirty runtime
files in this checkout; and Grok's narrative set under row 83.
