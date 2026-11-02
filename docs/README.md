# docs — what is in here and which file wins

Fifty-seven design files, three agents, one human. This page exists because
that is enough files to lose a rule in, and a rule nobody can find gets
deleted by someone who never knew it was one.

## Which file wins

Authority runs top to bottom. When two files disagree, the higher one is right
and the lower one is stale.

| Tier | File | What it is |
| --- | --- | --- |
| 1 | `DECISIONS.md` | Binding calls only, numbered, `LOCKED` / `PROPOSED` / `OPEN`. Cite rows by number. Nothing is promoted into here without Chris |
| 2 | `WORLD-SYSTEMS-BOARD.md` | The living board for new lore, mechanics and encounters. **New ideas enter here first**, as a pressure test, and move only after a playtest. It carries the intake template |
| 3 | `PRODUCTION-CONTROL.md`, `../COORDINATION.md`, `LANES.md` | Who is building what, which files are owned, what is in flight |
| 4 | Everything else | Synthesis, findings, proposals, handoffs. Descriptive. None of it binds |

**Before proposing a mechanic, read tier 2.** Most good ideas about this game
have already been had, and the board is where they were written down.

## Start here

| Read | For |
| --- | --- |
| `GAME-THESIS.md` | The claim, the playable argument, what the game is not |
| `PREMISE.md` | Why the dark, and who you are |
| `THE-WORK.md` | Why any of this happens — act one |
| `SPINE.md` | One session, one week, thirty hours |
| `PITCH.md` | How to say it out loud |

## The rules of the night

`THE-NIGHT.md` — the storm and what the machines want.
`THE-LANTERN.md` — what it is, how it works, what it costs.
`COMBAT.md` — whether a keeper can fight back.
`SOUND.md` — how a threat announces itself.
`BANKING.md`, `CONTAINED-KEEPERS.md` — extraction, and rescuing each other.
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
`VERTICAL-SLICE.md`, `HUD-STATUS-SCREEN.md`.

## Lighting — the part that keeps going wrong

`../DONTFUCKUPTHELIGHTS.md` first.
Then `LIGHTING-IN-UNREAL.md` (what may be tuned, in what order),
`FIXTURE-LIGHT-CONTRACT.md`, `LIGHTLAB-PROTOCOL.md` (the lab and its
measurement protocol), `LEGIBILITY-INSTRUMENT.md`, `LANTERN-LOOKDEV.md`.
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
