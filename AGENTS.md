# Last Lightkeepers — current task context

Read `docs/GAME-THESIS.md`, `docs/ROADMAP.md`, and
`docs/PRODUCTION-CONTROL.md` before planning game work. `docs/DECISIONS.md`
row 92 records Chris's 2026-11-14 direction: **solo, first-person, story-led
horror RPG with stealth and environmental puzzles in Unreal**. The playable
target is the Boswell's-to-Sayers opening crossing, with a believable store,
cover, escape routes, useful lantern, legible machines, and a story reason to
continue.

The former co-op extraction raid and Roblox prototype are history. Do not use
their roadmap, run timer, payout, two-client test, or Roblox code as acceptance
criteria for current work. Old runtime code and tests still exist; keep them
intact until their replacement behavior is specified and verified. A green
legacy test is not proof that the new chapter plays well.

When an older document conflicts with the current product direction, cite the
conflict and update the active plan or handoff. Do not silently restore an old
mechanic because it has a locked row, test, data field, or existing C++ class.
Proposed details in decision rows 84–88 and 90–91 are not automatically
ratified by row 92. Chris can change the direction at any time.

Follow `docs/LANES.md` for file ownership and give each binary Unreal asset
one active writer. Do not overwrite another writer's uncommitted work. Use
explicit paths when staging, and verify changes in a runnable build where
the acceptance criterion is player experience.
