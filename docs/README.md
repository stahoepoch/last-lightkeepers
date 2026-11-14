# docs — current game and decision history

**Start with the current game:** `GAME-THESIS.md`, then `ROADMAP.md`, then
`PRODUCTION-CONTROL.md`. On 2026-11-14 Chris restated the product as a **solo
horror RPG with stealth and environmental puzzles**, built in Unreal. It is
not a two-player extraction raid or a Roblox release. `DECISIONS.md` row 92
records that direction. The superseded raid thesis, pitch, roadmap and
production schedule live under `docs/archive/` so they remain readable without
assigning current work.

Older feature documents and tests still describe extraction and co-op code
that exists in the project. Treat those as implementation history or migration
material. They do not override the current product direction. The specific
mechanics in proposed pivot rows 84–88 and 90–91 remain open until decided
individually; row 89 is locked as solo.

## Older index and research map

This folder contains current product direction alongside dated research and
prototype decisions. This index exists so an older document is not mistaken
for today's work order.

## Which file wins

Authority runs top to bottom. When two files disagree, the higher one is right
and the lower one is stale.

| Tier | File | What it is |
| --- | --- | --- |
| 1 | `DECISIONS.md` | Binding calls, numbered and dated. Row 92 records Chris's current product direction; `SUPERSEDED` rows are historical. Cite rows by number. |
| 1a | `GAME-THESIS.md`, `ROADMAP.md` | Current product interpretation and playable gate. A proposal elsewhere cannot turn the old raid into an active work order. |
| 2 | `WORLD-SYSTEMS-BOARD.md` | The living board for new lore, mechanics and encounters. **New ideas enter here first**, as a pressure test, and move only after a playtest. It carries the intake template |
| 3 | `FLEET.md`, `PRODUCTION-CONTROL.md`, `LANES.md` | Who is building what, which files are owned, what is in flight. `../COORDINATION.md` is the superseded Roblox-era agreement, kept for its tag contracts |
| 4 | Everything else | Synthesis, findings, proposals, handoffs. Descriptive. None of it binds |

## How the fleet works

| Read | For |
| --- | --- |
| `FLEET.md` | The four parties, lanes, handoff format, and the one rule: **uncommitted work does not exist** |
| `ADVERSARY.md` | How the agents check each other. What counts as a claim, which arguments have already failed, retraction discipline |
| `DRIFT.md` | The stale-concept register. **Section A is a contradiction between two LOCKED rows and only Chris can clear it** |
| `AUDIT-THE-PIVOT-2026-11.md` | Dated audit of the old design and build. Row 89 is now locked; the other pivot rows remain proposed. Its mechanics are evidence to consider, not current work orders. |
| `RULINGS-OPEN.md` | **Every open ruling waiting on Chris, on one page.** A ballot, not an argument: twenty-three questions with their options and what each answer unblocks. Start here |
| `HANDOFF-CLAUDE-PRODUCT-PIVOT-AUDIT.md` | Codex's product audit: the promise is split, and one Boswell's-to-concourse segment is the first proof |
| `ADJUDICATION-CODEX-PRODUCT-PIVOT-2026-11.md` | The review of it. Agrees on the diagnosis; finds the co-op row already answered, row 3's gate unanswerable as written, and **the lantern redundant because its only cost appears on the debug HUD alone** |
| `LOG.md` | Append-only log: who was driving, why, what it cost |

**Before proposing a mechanic, read tier 2.** Most good ideas about this game
have already been had, and the board is where they were written down.

## Narrative research to reconcile

The files below contain useful world and quest ideas, but several still open
with a co-op raid, a return-to-Lanternhold run, or a sealed-store-only scope.
They are not the current product brief. Grok owns the assigned narrative set;
reconcile it against `GAME-THESIS.md` and row 92 before turning a paragraph
into a chapter task.

| Read | For |
| --- | --- |
| `GAME-THESIS.md` | The current solo horror / stealth / puzzle claim and its playable proof |
| `PREMISE.md` | Why the dark, and who you are |
| `THE-WORK.md` | Why any of this happens — act one |
| `SPINE.md` | One session, one week, thirty hours |
| `THE-HARD-RUN.md` | The one night followed end to end: the body, the verb list, the five feelings, the build cards. Moved off `WORLD-SYSTEMS-BOARD.md` 2026-11-05 |
| `PITCH.md` | How to describe the current game; the raid pitch is archived |

## The rules of the night

`THE-NIGHT.md` — the storm and what the machines want.
`THE-LANTERN.md` — what it is, how it works, what it costs.
`COMBAT.md` — whether a keeper can fight back.
`SOUND.md` — how a threat announces itself.
`BANKING.md`, `CONTAINED-KEEPERS.md` — raid-era mechanics and rescue research;
reconcile with the current chapter before assigning work from them.
`QUESTS.md`, `REWARDS.md`, `PROGRESSION-GAP.md` — the ladder, what it pays,
and the sinks that are still not wired.

## Places

`THE-STORE.md` — Boswell's, the whole first game.
`LANTERNHOLD.md`, `LANTERNHOLD-PLAN.md` — the hub, and the real building.
`ZONES.md` — the ladder in toward the city.
`THE-VALLEY.md`, `VALLEY-TERRAIN.md` — the long story, and what the geography
does to play.
`KNOWLEDGE-OF-PLACE.md` — how a location teaches time and space.

## Teaching the player

`FIRST-SIXTY-SECONDS.md`, `HOW-THE-PLAYER-LEARNS.md`,
`VERTICAL-SLICE.md` (historical raid test), `HUD-STATUS-SCREEN.md`.
`FOUR-ANCHORS.md` — the four ways of knowing where you are.
`KNOWLEDGE-SPACE-TIME.md` — the research behind it, and what it buys.
`FIELDBOARD-OBSERVATIONS.md`, `UI-NOVELTY.md` — the Fieldboard records
observations rather than state, and where it could go.

## The shape of the whole thing

`CAMPAIGN-SHAPE.md` — the campaign, and the two things it accidentally solves.
`THE-STORE-STILL-RUNS.md` — the store is still running, and you are inventory.
`THE-MALL-IS-STILL-TRADING.md` — the same reading, one building out.
`PERMISSIVE-HOSTILE.md` — permissive about shopping, hostile to exceptions.

## Lighting — the part that keeps going wrong

`../DONTFUCKUPTHELIGHTS.md` first.
Then `LIGHTING-IN-UNREAL.md` (what may be tuned, in what order),
`FIXTURE-LIGHT-CONTRACT.md`, `LIGHTLAB-PROTOCOL.md` (the lab and its
measurement protocol), `LEGIBILITY-INSTRUMENT.md`, `LANTERN-LOOKDEV.md`,
`LIGHTLAB-SPEC.md` (measuring the orientation/inspection band).
`FIXTURE-LIGHT-FIELD.md` — **which** fixtures enter the gameplay light field,
fixture by fixture. Distinct from the contract above; it was called
`FIXTURE-LIGHT-CONTRACT.md` at the repo root until 2026-11-01, which was a
filename collision rather than a duplicate.
`MAKE-IT-FEEL-LIKE-A-LANTERN.md` — why it doesn't feel like one yet.
Open proposals: `PROPOSAL-LANTERN-DIAL.md`,
`PROPOSAL-LANTERN-DIAL-NEXT.md`.
Negative results worth not repeating: the four `FINDINGS-*.md` files.

## Build, pipeline, intake

`PLATFORM.md`, `ROADMAP.md`, `WORKLIST.md`, `STEAM-PRODUCTION-PLAN.md`,
`PLAN-RECONCILIATION.md`.
`ASSETS.md`, `ASSET-INTAKE.md`, `FIELD-KIT.md` — what we own, how outside work
comes in, what to refuse.
`DATA-PIPELINE.md` — `data/*.json` into Unreal.
`MAP-READINESS.md`, `PREREQS-AUTHORED-MAP-SEAM.md` — what a map needs before
an enemy goes in it.
`TOOLING-TRAPS.md` — signals that do not mean what they say. Read it before
trusting a measurement.
`HANDOFF-CODEX-*.md` — scoped tickets, each one a single job.

## House rules for writing in here

1. A new idea is a pressure test on the board, not a new file. Doc fifty-eight
   is how this project forgets things.
2. A binding call goes in `DECISIONS.md` as a numbered row with who and when,
   and only Chris puts it there.
3. **Commit it.** An uncommitted design file does not exist: the other agents
   cannot read it, and nothing is holding onto it.
4. When an idea is cut, leave the one-line reason in its pressure-test row.
   Otherwise the same dead end comes back next month as new scope.
