# Current state — Galleria crossing

Status: **active checkpoint, 2026-11-21.** Replace this file at a completed
work-package boundary. It is a present-tense pointer, not a session diary.
Historical detail belongs in Git and `HANDOFFS.md`.

## Product proof

A solo player begins in Boswell's and crosses the whole believable Galleria
Mall—overgrown, visibly scavenged, and dangerous—on the way to Sayers. They use
the lantern for one necessary read and one authored infrastructure interaction,
survive a legible machine without being pinned, learn why Sayers matters, and
reach the chapter exit wanting to continue. `GAME-THESIS.md` and `ROADMAP.md`
are the authority.

## Current boundaries

- **Playable map / binary assets:** check `LANES.md` and the current git status
  before claiming or editing any `.umap` or `.uasset`. This checkpoint does not
  transfer ownership.
- **Runtime rules:** preserve old raid/extraction code until its campaign
  replacement is specified and verified; old green tests are not chapter proof.
- **Lore and mechanics:** `WORLD-SYSTEMS-BOARD.md` is a proposed-ideas board.
  It cannot ratify a decision or override the active chapter target.

## Evidence at this checkpoint

| Claim | Evidence kind | Current status |
| --- | --- | --- |
| Current product direction | Authored decision | **Observed in project guidance:** solo first-person story-led horror RPG with stealth and environmental puzzles. |
| Whole-Galleria playable crossing | Player experience | **Unverified:** do not infer it from maps, C++ compilation, old extraction tests, or screenshots. |
| First chapter machine encounter | Player experience | **Unverified against current acceptance:** it must prove cue → hide/detour/flee → retained control, not merely legacy enemy logic. |
| Opening lantern and grille | Direct game observation, 2026-11-21 | **Functional in isolation:** in the Development game, E took Dad's lantern, F lit it, the aimed beam changed the shadowed release prompt to an E action, and E raised the south grille. The opening objective changed with each state. This was probed with `BugItGo`, not a natural route play. |
| Opening route lighting | Direct game observation, 2026-11-21 | **Partially repaired:** ten Galleria route lights had unbuilt interactions; after making them movable, the 98-object rebuild warning was absent on fresh game launch. The concourse floor and shop mouths are readable in isolated pools, but deep black gaps remain. The lantern clips the release panel nearly white. Lowering the separate glass fill from 15 to 4 cd barely changed that view; the experiment was reverted. |
| First mall shops | Saved-map traces and Development game camera, 2026-11-21 | **Furnished blockout, not finished art:** six already-built shop pockets now have 18 measured retail fixtures, matching collision bodies, department signs, and movable floor/face lights. The pocket ceilings meet their 5.5 m storefront headers instead of floating at 12 m. Fresh saved-map checks confirm open mouths and counter flank paths; the opening pickup/grille map check still passes. In the game camera, the inward lights reveal counter faces and the dimmed ceiling pools reduce glare. The furniture is still sparse and flat colored; the deep rooms remain too black for a believable department store. A debug teleport established this view, not a natural player traversal. |
| Chapter finish and lantern cost | Player experience | **Unverified:** the near concourse ends at a safety wall; Sayers and the whole-mall crossing are still unbuilt. A machine's reaction to this opening lantern use has not been played through. |

## Next risk to retire

Extend the playable crossing beyond the near-concourse safety wall toward
Sayers, then walk the opening route without debug teleportation. Dress the
first six shops with retail-quality merchandise and surfaces so they read as
departments instead of three colored meshes in each black pocket. Judge the
remaining dark gaps and overbright lantern target against the fixture contract,
and record whether the machine responds to the necessary light use while the
player still has a cover or escape choice. The current map proves a first gate
and physical shop routes, not the chapter's traversal, threat, or finish.
