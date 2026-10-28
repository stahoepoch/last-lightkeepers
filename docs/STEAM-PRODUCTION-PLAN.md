# Last Lightkeepers — Steam Production Plan

Status: **proposed for approval**, 2026-10-27.

## The decision this plan serves

Last Lightkeepers is a **PC-first Steam game in Unreal Engine 5**, playable
solo or in a private crew of up to four friends. It is not a Roblox port and
not a whole-mall build yet.

The first product is a filmed, externally tested **10–12 minute department
store expedition**. It must make a player want one more run before the team
earns the right to build the rest of the Galleria.

## What the first playable must prove

1. A crew can create or join a private game, raid, extract or wipe, and return
   to Lanternhold together.
2. The lantern gives sight and value while creating danger.
3. A recovery contract gives a clear reason to enter a particular part of the
   store.
4. The player faces one meaningful choice: push farther, or call a loud
   extraction and defend it.
5. The store feels like a stocked, damaged department store—not a collection
   of empty game rooms.
6. A return changes something visible at Lanternhold.

## The vertical-slice content lock

### Spaces

- Entrance / cosmetics floor: readable arrival and lantern lesson.
- Home goods sales floor: dense fixtures, carts, shelves, first route choice.
- Fitting rooms and staff hall: close sightlines, hiding, mannequin encounter.
- Stockroom and loading bay: heavy objective and extraction pressure.
- One upper-floor overlook: proves verticality without building the full mall.

Everything else is sealed with plausible shutters, flood damage, powerless
doors, or collapsed escalators.

### Gameplay lock

- One contract: retrieve a restoration crate or required recovery part.
- Three carried reward classes only: **restoration crate**, **components**,
  **named relic**. Store merchandise is dressing, not inventory clutter.
- Two enemies at playable milestone: Sweeper and Mannequin/Window Walker.
- Third enemy at vertical-slice milestone only if the first two are fun:
  Glimmer.
- One active defense tool and one diversion: create time or space; do not turn
  the game into a shooter.
- Downed crawl, teammate revive, loud extraction holdout, wipe, return.
- One Lanternhold room visibly restored by a successful return.

### Explicit cuts

- No entire mall, solar farm, city, seasons, battle pass, PvP, public
  matchmaking, open-world crafting, weapon tiers, or dedicated servers before
  the vertical-slice gate.
- No five-enemy roster, procedural quest system, or generic inventory framework
  before the core run proves fun.

## Technical architecture

| Area | First implementation | Rule |
| --- | --- | --- |
| Engine | Unreal Engine 5.8 | C++ for replicated core systems; Blueprints for authored encounters and content. |
| Multiplayer | Steam private/friends lobby + UE listen server | Build server authority from day one; solo uses the same networked gameplay path. |
| Authority | Server owns loot, interactions, AI, damage, extraction, quest rewards, saves | Clients own camera, UI, local responsiveness, VFX, and audio prediction. |
| Party | 1–4 | Hard cap from the first test. |
| Persistence | Per-player profile | Lanternhold restoration, settings, relics, and validated rewards only. |
| Abilities | Gameplay Ability System | Lantern, sprint, crouch, carry, interaction, knockdown, revive, tools, extraction. |
| Input | Enhanced Input | Keyboard/mouse and controller, remappable from the beginning. |
| Data | Data Assets/Data Tables | Quest definitions, enemy tuning, loot, encounters, UI strings, sound palettes—no scattered magic numbers. |
| Maps | World Partition + One File Per Actor | Long-term mall streaming and fewer level-edit conflicts. |
| Source control | Perforce for UE assets; Git optionally for code/docs | Lock binary assets; one owner per level/system while it is in flight. |

### Hosting decision

Use a **listen server** for the first private 1–4 player Steam test. It is
appropriate for casual co-op and keeps hosting cost out of the first proof.
Dedicated servers become a later decision only if testing demonstrates host
advantage, poor host performance, unacceptable cheating, reconnect needs, or a
shared-world requirement.

Steam lobbies solve party/invite/readiness. They do not replace Unreal’s
replicated server-authoritative game code.

### Physics rule

Physics makes the store react: carts rattle, a door swings, loose debris shifts.
It does **not** decide damage, loot, quest success, or extraction. Most retail
dressing is static or locally simulated. Server-authoritative state machines
control every object that matters to a run.

## Art, lighting, and environment pipeline

### Build order

1. **Reference and route.** Make a reference board and top-down route before
   modeling. Test it in graybox at player scale.
2. **Reusable retail kit.** Floors, carpet, ceiling grid, walls, columns,
   storefronts, shutters, shelves, racks, mannequins, checkout lanes, carts,
   pallets, signage, utilities, and one broken-escalator hero kit.
3. **Damage language.** A single master material supports clean, dusty,
   water-stained, moldy, wet, chipped, and broken variants.
4. **Dressing recipes.** Author reusable clusters for cosmetics, clothes,
   checkout abandonment, stockroom pallets, employee areas, and flood damage.
   Hand-author routes, objectives, hiding places, and enemy sightlines.
5. **Hero spaces.** Spend bespoke effort on the fitting-room mirror corridor,
   loading bay, and one upper-floor overlook—not every aisle.

### Density rule

Realism comes from deliberate repetition: a real store has many of the same
fixture, product, price card, and damage pattern. Reuse a small modular kit
with color, layout, signage, and wear variation. Do not attempt thousands of
unique props.

### Lighting rule

- One night/weather condition in the slice.
- Cold exterior/moon light; sparse dead fluorescents; warm held lantern;
  extraction flare as danger color.
- Every light must have a physical reason.
- Darkness is navigable, never illegible black.
- Test in the player camera while moving, at target scalability settings.
- Use Lumen, Nanite, and Virtual Shadow Maps intentionally; profile them every
  week. The lantern must not depend on a graphics setting that changes gameplay
  readability.

## Audio and music are gameplay systems

Audio starts in the first playable, not the final polish pass.

| Sound family | Job |
| --- | --- |
| Lanternhold | Safety, warmth, and contrast with raids |
| Store ambience | HVAC, rain, fluorescent hum, structure creaks, distant water |
| Lantern | Ignition, held presence, danger/charge state |
| Player | Footsteps by surface, carry burden, downed state |
| Enemies | A distinct directional language for each family |
| Extraction | Flare, siren, approaching threats, holdout release |
| Music | Sparse exploration layer; escalation only when earned |

Use spatial audio, occlusion, sound-concurrency limits, Audio Modulation, and
MetaSounds for varied machinery, lights, ambience, and enemy loops. Every
important action and enemy state must be identifiable with eyes closed.

## UI and accessibility

The HUD serves decisions, never competes with the store.

- Lantern/storm state, current contract, carried haul, squad condition, and
  extraction state only.
- Context prompts anchored to the object; no permanent wall of instructions.
- Clear lobby: solo, host, invite friends, ready, launch.
- Controller-first focus navigation and remapping.
- Captions, volume sliders by sound class, brightness/contrast controls,
  motion/flicker reduction, and color-independent threat cues.
- Test all UI in solo, host, remote client, 16:9 and ultrawide before marking it
  complete.

## Division of labor

One person may hold multiple lanes early. One lane may never have multiple
writers on the same live asset.

| Lane | Owns | First acceptance deliverable |
| --- | --- | --- |
| Creative director / producer | Pillars, canon, scope cuts, quest acceptance | A one-page 12-minute raid brief |
| Technical lead | UE project, sessions, replication, saving, builds, CI | Two PCs can host/join/raid/return |
| Gameplay engineer | Interaction, lantern, carry, enemy interfaces, down/revive/extraction | An ugly but complete raid loop |
| Level designer / environment artist | Route, scale, modular store kit, dressing, lighting placement | A furnished playable first route |
| Technical artist | Materials, decals, Niagara, performance budget, lighting diagnostics | Store stays within frame budget |
| Character / animation artist | Player lantern carry, enemy rigs, locomotion, hit/down/revive | One polished Mannequin encounter |
| Audio designer | Sound palette, mix, ambience, enemy cues, music states | Audio-only threat readability test |
| UI/UX designer | Lobby, HUD, prompts, controls, accessibility | No-confusion first ten minutes |
| QA / playtest owner | Test scripts, captures, bugs, telemetry, external tester notes | Weekly playtest report with decisions |

### Tiny-team operating model

Until the team grows, protect only three lanes:

1. **Engineering:** networked gameplay, UI plumbing, saves, builds.
2. **World:** levels, models, lighting, materials, VFX.
3. **Design experience:** contracts, encounters, audio, pacing, playtests.

All work enters a single task board. A task has an owner, a playable acceptance
test, affected assets, and a screenshot/video when closed. `main` must always
build and launch.

## Roadmap: sixteen weeks to an external Steam test

### Weeks 1–2 — Preproduction and foundation

- Create the UE5 project and folder/naming conventions.
- Set up source control, locks, automated Windows packaging, and a test map.
- Connect Steam development app; prove private host/join/invite/readiness.
- Build first-person movement, camera, Enhanced Input, and target-PC
  performance baseline.
- Publish the raid brief, reference board, economy table, cut list, UI wireframes,
  and target specification.

**Gate:** two real PCs can create a private party and load the same empty map.

### Weeks 3–5 — Networked verbs

- Held lantern, sprint/crouch, interact holds, loot/carry, limited haul.
- Server-authoritative knockdown, crawl, revive, wipe, extraction call and
  holdout.
- One Sweeper and one Mannequin prototype with navigation and replicated state.
- First audio pass: lantern, footsteps, enemy cue, extraction alarm.

**Gate:** two people complete ten ugly 10-minute raids without desync, broken
return travel, or an unclear objective.

### Weeks 6–9 — The actual department store

- Build the playable first floor, staff route, stockroom, loading dock, and
  upper overlook from the modular kit.
- Add dense retail dressing and the authored risky pickup.
- Implement one recovery contract and one visible Lanternhold restoration.
- Add one diversion/defense tool and its counterplay.

**Gate:** a new tester reaches the objective and understands extraction without
a tutorial wall.

### Weeks 10–12 — Fear, identity, and clarity

- Finish lighting palette, soundscape, lantern response, enemy behavior
  readability, encounter pacing, and extraction convergence.
- Add the third enemy only if it improves, rather than dilutes, encounters.
- Complete lobby, HUD, settings, controller, and accessibility baseline.
- Profile and fix CPU, GPU, memory, networking, and loading budgets.

**Gate:** testers voluntarily say they wanted to risk “one more room.”

### Weeks 13–16 — Closed Steam playtest

- Package, smoke-test server/client, save validation, crash/bug pass, and
  controller pass.
- Recruit 10–20 trusted testers through the Beta Testing package.
- Run recorded playtests and a five-question post-run survey.
- Decide from evidence: revise the loop, polish the store, or begin the next
  department-store wing. Do not build the mall-wide expansion by assumption.

## Required test modes for every feature

1. Standalone solo.
2. Listen host plus one remote client.
3. Dedicated-server simulation in editor.
4. Packaged standalone build before a feature is called done.

## Metrics that decide whether expansion is earned

- Time until first meaningful action.
- Percent of new players who understand their objective unaided.
- Percent encountering the first threat.
- Average run length, extraction rate, wipe rate, and crash/abort rate.
- Whether players manage and discuss light with their crew.
- Whether players call extraction early, late, or never.
- The answer to: “Would you play one more run right now?” and why.

## Skills to learn or hire deliberately

- UE5 multiplayer replication, server authority, and Steam sessions.
- C++ gameplay architecture plus Blueprint content workflows.
- Gameplay Ability System; Enhanced Input; save/persistence validation.
- Behavior Trees, State Trees, EQS, NavMesh, Smart Objects, and networked AI.
- Modular environment art, UVs, trim sheets, decals, PBR materials, and PCG as
  an editor helper—not gameplay logic.
- Blender modeling/rigging; Animation Blueprints, IK Rig, Control Rig.
- Niagara, Lumen/Nanite/VSM profiling, Unreal Insights, GPU capture.
- Technical sound design, MetaSounds, spatial mixing, and music states.
- UI/UX, controller accessibility, QA repro writing, and telemetry.
- Steamworks depots, branches, packages, and Steam Playtest.

## Research basis

- [Unreal networking overview](https://dev.epicgames.com/documentation/unreal-engine/networking-overview-for-unreal-engine)
- [Unreal dedicated vs listen servers](https://dev.epicgames.com/documentation/unreal-engine/setting-up-dedicated-servers-in-unreal-engine)
- [Steam multiplayer and lobbies](https://partner.steamgames.com/doc/features/multiplayer?language=english)
- [Gameplay Ability System](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system)
- [Unreal Lumen lighting](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine)
- [Lumen performance guidance](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine)
- [Unreal audio and MetaSounds](https://dev.epicgames.com/documentation/unreal-engine/audio-in-unreal-engine-5)
- [Unreal source control](https://dev.epicgames.com/documentation/en-us/unreal-engine/source-control-in-unreal-engine)
- [Steam Playtest](https://partner.steamgames.com/doc/features/playtest?language=english)
- [GDC: The Vertical Slice](https://media.gdcvault.com/gdc2015/presentations/Donovan_Greg_TheVerticalSlice.pdf)
