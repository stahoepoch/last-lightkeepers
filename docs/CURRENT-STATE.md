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
| Young keeper travel scale | Authored data, build, and game log, 2026-11-21 | **Provisional feel trial:** walk 140 cm/s and crouch 84 cm/s; sprint remains 350 cm/s. The previous 224 cm/s walk and its 1.5625 sprint/walk ratio are historical, despite their still appearing in old retune notes and code comments. The generated `DA_Lantern` was rebuilt and the Development game logged `walk 140, sprint 350, crouch 84`. Movement tests prove the compounded stumble and burden escape margin, not that this speed feels right in a played route. The 50 m concourse now takes 35.7 s to walk straight through versus 22.3 s before, with no pauses or detours. |
| Atrium escalators | Saved-map traces and Development game camera, 2026-11-21 | **More legible, still stationary:** both smooth 30-degree collision ramps have 39 non-colliding tread strips each, three flat comb bars at each end, and three aimed 8 cd emergency pools per run. A fresh reload found all 96 unique details and traced the original ramp and landing collision; the game camera shows a readable ascent and lower landing. This is an inactive escalator presentation, not a moving escalator, and no player has walked its whole ascent at the new speed. |
| Boswell's-to-mall upper-floor seam | Authored geometry comparison, 2026-11-21 | **Unresolved 180.5 cm mismatch:** Boswell's upper plate and escalator head are at 662.5 cm. The separate `GalleriaShell_Greybox.obj` reference puts its upper plate at 482 cm. Neither number was measured from TLP-1; the mall reference explicitly calls its figure a stand-in. A future upper walk must meet the store at a credible elevation, with the transition shown rather than hidden in an arbitrary stair. This blocks a truthful two-floor connection, not lower-floor work. |
| Opening capture story | Saved map, compiled runtime and Development game camera, 2026-11-21 | **Playable blockout, not a finished human scene:** four skinned adult silhouettes with desynchronised walk loops cross the south grille with a low haul rover; the bars lift and close, storm light pulses, and a synthetic one-word machine call says “Sayers.” The floor lantern appears when the call happens; before that it is hidden and cannot be picked up. An emergency pool made the people visible from the child's counter after an initial black-on-black attempt failed visual review. The game log recorded sequence completion and a fresh saved-map read verified the cast and no-collision route. The actors still read as mannequins, the rover is blockout art, its wheels do not turn, and no human reaction/struggle is authored. The player has not reported whether the event lands emotionally or can be bypassed by sprinting. |
| Chapter finish and lantern cost | Player experience | **Unverified:** the near concourse ends at a safety wall; Sayers and the whole-mall crossing are still unbuilt. A machine's reaction to this opening lantern use has not been played through. |

## Next risk to retire

Replace the capture mannequins with clothed people and distinct reactions,
then play the full opening from the child behind the counter. Check whether
a sprinting player can cross the temporarily raised grille before it falls;
the scene does not remove movement input. Measure the
new gait through Boswell's and both escalator transitions in control, without
debug teleportation. The game camera confirms that emergency light shows the
stairs, but the treads are still visual strips on a ramp, and their three pools
form bright islands separated by dark gaps. The route beyond the near-concourse
safety wall then needs the scaled Galleria plan before it can reach Sayers;
`HANDOFFS.md` P2 still owes the width from TLP-1's own 100 ft scale bar, and
the upper-floor elevation mismatch must be resolved at the same seam. Dress
the existing shop pockets with real merchandise and surfaces and test the
machine's response to necessary lantern use. This map is a playable first gate,
not a story chapter.

## Measurements that conditioned this pass

- Boswell's current shell spans about 87.4 by 54.4 m (4,755 m² per full
  rectangle). Two such gross plates total about 102,400 sq ft, close to the
  101,300 sq ft anchor noted in the Galleria reference. That arithmetic does
  not prove the layout or usable floor area: atrium voids, blocked service
  space, sparse stock, oversized heights and the missing rest of the mall
  still change how large it feels. It does rule out solving the complaint by
  blindly scaling every wall outward.
- A study of children aged 6–10 measured median self-selected walking at
  1.26 m/s and reviewed other 6–13-year-old cohorts around 1.13–1.37 m/s.
  The 1.40 m/s trial is near the fast end, not an age lock or a player-feel
  verdict: <https://pmc.ncbi.nlm.nih.gov/articles/PMC5135775/>.
- Schindler's escalator planning guide puts common travel at about 0.5 m/s
  and discusses 30-degree commercial rises and landing transition geometry:
  <https://au.schindler.com/content/dam/website/au/docs/escalators-moving-walks/schindler-escalator-moving-walk-planning-guide-au.pdf/_jcr_content/renditions/original./schindler-escalator-moving-walk-planning-guide-au.pdf>.
  The game escalators are powerless and walkable; the guide is a proportion
  and presentation reference, not proof that the current ramp meets a standard.
