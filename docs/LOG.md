# Fleet log

Append-only. One entry per session that changed something durable. Newest at
the bottom, so the file reads forward.

`git log` says what changed. This says **who was driving, why, and what it
cost** — which is the part three agents and a human cannot reconstruct a week
later.

**Entry format.** Date, party, one line of what happened, then only what a
future reader needs: what was decided, what was retracted, what is still
open. Not a changelog. If a commit message already explains it, cite the hash
and stop.

Seeded 2026-11-01. Earlier history is in `git log` and in the `FINDINGS-*`
files, which are better records than a retrospective summary would be.

---

## 2026-11-01 — Claude (MCP): the lighting instrument thread closed

Roughly ten rounds between the MCP session and the chat session. **Five
instrument defects found**, both parties wrong more than once. The procedures
that came out of it are now `ADVERSARY.md`.

- Capture geometry drift — fixed insets against a moving window (`a109611`).
- Offscreen GI omission; the occlusion/visibility gate.
- The staleness detector's false-rejection mode: fixed by differencing against
  a **fixed baseline** rather than the previous capture (`d1a92b9`).
- The frame counter's wrong operand: fixed as a **rate over the
  change-to-capture interval** (`9063269`, `215d134`).
- `e1eb630` retracts `e55c587` — the restart claim was wrong because the
  engine log is UTC and the machine is EST. The lantern numbers reproduced.

**Result that matters:** `023f675`. The exposure sweep's card half lands on
**EV100 5** (lock 4.0), exactly as the illuminance arithmetic predicted.
Glyph separation 175.8 at 0% clipped, against 104.2 at the current lock 1.0 —
so **the current lock is marginal**, not merely suboptimal. The plateau from
EV100 5→7 is flat and one-sided; there is no argument for going higher.

**Still open:** the orientation half. EV100 3 against 5 at the Bay B pose,
with Bay A's card as in-run witness. Chris judges "navigable but uneasy"
against "nothing is there." Not a measurement question.

**Also open:** `bThrottleCPUWhenNotForeground` was written nowhere, so it was
at the engine default — on — for **every capture this project has ever
taken**. Codex has toggled it off in the editor. Persistence across a restart
is unverified and the key is still not on disk.

## 2026-11-01 — Chris: the Epic Launcher asset pipeline

Blocked: "No compatible user projects found" with *Show all projects* checked,
so nothing redeemed on Fab could be added to the project.

Cause, from the engine source: `CreatedProjectPaths` in
`EditorSettings.ini` holds **parent directories**
(`SProjectDialog.cpp:1950` takes `FPaths::GetPath` twice), and the key was
absent. The `RecentlyOpenedProjectFiles` route cannot substitute for it —
`DesktopPlatformBase.cpp:1809` parses those entries as bare filenames while
the property is a `TArray<FRecentProjectFile>` struct, so double-clicking a
`.uproject` never registers its folder.

**Fix:** `CreatedProjectPaths=C:/Users/chris/Documents`. Confirmed working.

Habits set with it: one pack per commit, with file count and LFS bytes in the
message; resist bulk imports, because LFS cannot be undone without a history
rewrite.

## 2026-11-01 — Chris: the playtest that changed the core verb

He walked the lantern walk lab and found the pickup instant. His own
conclusion: *"holding in E for a few seconds to pick something up will work.
Less time if your light is on."*

Committed as `8f940c9`. **Light becomes a rate, not a gate** — the central
tension now comes out of existing systems, because holding still in the light
is what summons a Sweeper. Constraints: accumulate, never reset; drive it from
the debounced `bCurrentlyLit`; read gameplay radius, never visual light
(`d065469`); and make the ratio a real decision — roughly 4s dark against
1.5s lit.

## 2026-11-01 — Codex: the walk lab became playable

`a3cb4ad` shell, `2a9970e` relics and Sweeper patrols, then past what the
chat session had seen: `d91774f` held lantern and progressive relic
inspection, `362688e` relic searches visible and usable in darkness,
`a060e78` the walk lab turned into an enclosed emergency-lit store,
`7010501` a physical first-person lantern rig, `2b275ab` asset intake
documented, `c18f9ef` `GameFeatureData` registered, `0bd1534` and `7d2c6c7`
the keeper mannequin and the co-op lantern mounted to the hand.

## 2026-11-01 — Claude (chat): docs made findable, then audited

`0ef5ed5` — `docs/README.md` as an index with four authority tiers, and
`WORLD-SYSTEMS-BOARD.md`, `THE-VALLEY.md`, `THE-WORK.md` committed. All three
had been written and left untracked, which is why the board's contents kept
being re-derived in conversation.

Then this commit: `FLEET.md`, `ADVERSARY.md`, `DRIFT.md`, `LOG.md`.

**`DRIFT.md` section A needs Chris and nothing else can clear it:** rows 6 and
73 of `DECISIONS.md` are both LOCKED and contradict each other on whether
darkness slows the clock. Row 73 is the live one. Row 6 has no banner and sits
sixth, under "The core rules."
