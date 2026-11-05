# The hard run

**Owner: Grok**, `DECISIONS.md` row 83. Moved out of `WORLD-SYSTEMS-BOARD.md` on
2026-11-05 on Chris's instruction — *"Move the section"* — which closes
`HANDOFFS.md` row **G3**. Everything below is **byte-identical to what stood on
the board**, its own status line, its falsifiers and its dated corrections
included. The only change is that the section's `##` heading became this file's
title. The move was a move, not an edit, and the commit proves it.

**Status: still proposed.** The move changed who maintains this, not what it is
worth. It is not a `DECISIONS` row, it ratifies nothing, and `FLEET.md`'s
authority tiers put it where the board put it — below the numbered rows and
below process. Its own status line below says the same thing in its own words.

## Why it moved, and what that costs

The board is a Claude file. This section is Grok's research. That is two writers
in one file, which is the thing `LANES.md` exists to forbid, and it was not
hypothetical: on 2026-11-05 Claude rewrote every speed figure in this section
against `data/` at `HEAD` (`35d73c2`), because the section's own falsifier 4
says any number here that disagrees with the data loses. Correct under the lane
table as it stood — and under `FLEET.md`'s board rule, *"append-only, anyone;
never rewrite another's entry"*, it was a rewrite of another's entry made by
substitution rather than by a dated banner. Both readings were defensible at
once, which is what a seam is.

**So the correction channel is now the cost, and it is the price Chris chose.**
The numbers in this file are Grok's to maintain. A finding against them is a row
under Grok in `HANDOFFS.md` — the symptom, the evidence, the file and the line —
and the owner decides. Claude does not edit this file to fix it, the same way
Claude does not edit `Tools/**` to fix it. That rule has a dated breach behind it
in `LANES.md` and it cost nothing only because git serialised the two writers.

**What that does not change.** Falsifier 4 still stands as this file's own rule:
a number here that disagrees with `data/` at `HEAD` loses, and the data wins.
The move does not make a stale number true. It makes fixing one a handoff.

**The standing example, already recorded.** `FINDINGS-SPEED-SCALE-COPIES.md`
tracks this section as copy 6 of the speed scale — the first copy that was prose
rather than a literal in code. Its lesson is why falsifier 4 is worth keeping
where a reader will hit it: a stale number in prose does not announce itself,
because nothing compiles it and no test reads it.

---


Status: **proposed scaffold, 2026-11-02.** Not a decision, not a milestone, and
not an ending. Endings stay candidates. The valley stays a later argument.
This is the one night a solo developer follows until a person in a chair
wants to leave and then wants to come back.

Chris, the same day: a single-player or co-op horror survival extraction
psychological thriller RPG. Slow, real, alive.

Those words are tests the night has to pass. They are not a second pitch.

| Word | The night passes if |
| --- | --- |
| Slow | Waiting is legal. The Sweeper finishes a lap whether or not the player moves. The first minute can be spent watching. |
| Real | It is a store doing store work. A lit lane, a stamped crate, a dock. No tutorial sentence. |
| Alive | Standing still does not pause the building. The crate can leave. A loop keeps sounding. Nothing waits for the player to arrive before it starts. |
| Horror | The cost is indifference. The machine takes the thing they came for. It does not hunt them first. |
| Survival | The lantern reads the stamp and makes the keeper legible. The haul can be lost. |
| Extraction | The dock call is the loudest act, and the crate is not theirs until the hold ends. |
| Psychological | The fright is a long hold and a walk back. The player caused the interrupt on purpose, and they can hear that it worked. |
| RPG | They are a keeper with a job. No level, no skill tree, and no shop stat in this night. Home changing is the next night. |
| Co-op | Solo is this same night. A second keeper is another lantern, not a different mission. |

The map is `/Game/LLK_Boswells_FirstRun`. PIE has already registered one
Loading Dock, one awake Sweeper, one authored patrol, the misrouted toy
crate, and a stock bay.

### The night, in order

1. Arrive with the lantern off. One way forward. Enough light to walk. Nothing explains itself.
2. Watch the Sweeper finish a lap and ignore you. If it comes for you first, this is the wrong game.
3. Find the misrouted crate. The stamp is readable in lantern light. The hold is long, and the meter starts moving.
4. Stay dark, and the Sweeper files the crate to the bay. You were ignored, and the cost is the walk.
5. Light up to stop it, and `OnStockInterrupted` is audible. A success that looks like nothing is not a success.
6. Walk to the bay with the patrol between you and the crate.
7. Call the dock. Hold. Leave with it, or lose it.
8. Stop. One stamp is the whole future. A window and one distant light may show it. The hall door, the next floor, and the valley are not how this night becomes finished.

### Already true, as of this writing

The stock state notices at `aggroRadiusDark` (12 studs), walks off the route,
and files within `ContactRange` (5 studs). The aisle's 6.26 studs clears the
notice, and a test asserts that distance. A bay tagged `LLK.StockBay` is
still required. The debug HUD latches the four cues for three seconds. That
readout is not a sound, so row 16 is still unmet in play.

On 2026-11-02, PIE saw filing begin and finish 1.93 seconds apart when a
relic was in reach. The headless Sweeper test builds its senses by hand and
never loads the map. Play the floor. Do not take that green as the night.

### Order of work

One package at a time. The hands that can author sound and the map go first.

1. Play the map in the dark once. Write down whether the Sweeper takes the authored crate without anyone parking it beside the machine. That note is the gate.
2. Author one sound on `OnStockInterrupted`. Then `OnStockFiled` at the bay, so the walk has a direction. `OnTelegraph` is the locked accessibility row and can share the same pass.
3. One lit checkout, or one service loop, that plays whether or not the player moves. That is curriculum step 1. It proves the building was already running. It is not, by itself, the scare.
4. The stamp on the wrong crate, legible only under the lantern.
5. Greybox the fixtures that make the aisle a store: gondola, counter, dock door, exit sign. No marketplace pack and no demo map.
6. `IMC_LLK` and its seven actions when play cannot express lantern and interact. Not before the beat can be read.
7. First-person arms after the beat reads. The mesh must not move the gameplay light.

### Enemy AI, light, and sound are not blank research projects

Chris, 2026-11-02: these three feel like the whole of game development.
They are large. They are also already researched past the point where more
reading helps this night. The open work is one proof each.

| System | Already in the repo | Still open, and it is small | Do not research yet |
| --- | --- | --- | --- |
| **Enemy AI** | Six jobs in `data/enemies.json`. Perception, visibility, and a Sweeper state machine: Patrol, Telegraph, Pursue, Search, Stock. Row 78 locks the decision layer before the body, and the body now walks. Row 47 locks the output as a state, not a kill. | Play the authored aisle and see whether Stock happens without parking the crate. | A second enemy. Behavior Trees, State Trees, or a combat AI tutorial. `docs/COMBAT.md` still describes Roblox Humanoids and a damage path nothing in Unreal has. |
| **Lighting** | Gameplay brightness and the rendered lantern are firewalled (`d065469`): changing the look cannot retune who is noticed. Lumen is on in the running editor. Fixture research closed on geometry, not a candela band: fixtures orient, shelf lips keep labels dark, signs may glow and may not carry the objective. | Whether the lantern reads as a lantern in Boswell's. The measured "25 cd" used a 72° cone; the data authors 110°. | Another lighting theory pass. Intensity is not the gameplay knob. `I` cancels in the lit test. |
| **Sound** | Row 16 is locked: two channels, and voice chat must not be the only way a threat arrives. Six distinct cues plus the extraction siren are rendered by `assets/audio/make_cues.py`. The Sweeper broadcasts `OnTelegraph`, `OnStockStarted`, `OnStockInterrupted`, `OnStockFiled`. The debug HUD listens. The storm-as-startle design is in `docs/SOUND.md` and is not this night. | Import `sweeper-telegraph-v1.wav` is the wrong first file for the crate beat. Author `OnStockInterrupted` first, then the filed cue at the bay. A bus layout can follow one sound that plays. | MetaSounds, occlusion, and a full mix before a single cue is audible in the aisle. |

### Being there is a body, and the vault waits on a counter

Chris, 2026-11-02: real physics, real movement, real traversal, try to really be there.

Row 63 already locks the answer, and the order is the decision. Greybox the floor first. Then a server-authoritative low vault and a ledge mantle. Then authored climbs only: a ladder, an escalator rail, a hatch. Not free climbing. Hands, lantern motion, and sound come after, so the move has weight. A lantern, a burden, the dark, and a Sweeper behind you are what make a counter feel desperate. Expressiveness is the failure.

What is already true of the body:

- First person is locked. You cannot peek past your own light.
- The keeper is a character capsule, not a simulated rigid body. That is the right real for this project, and the outside reading agrees with the direction while naming a real alternative. Unreal 5.8's Mover plugin can drive a character from Chaos networked physics. Epic marks Mover experimental, says its API can still change, and says the Character Movement Component stays supported. Epic's own Mover sample is a parkour course, which is the moveset row 63 refuses. The simulation literature (DeepMimic and the motion-matching physics controllers after it) trains a policy to imitate motion capture inside a research engine. A 2026 attempt to bring those policies into a game engine reports that production physics solvers were not built for that control problem, and that matching good animation is the part that fails. Shipping games have used physics for cloth, fluids, props, and ragdolls, and mostly not for the body the player steers. Physics belongs on the store: a cart, a door, a loose crate. The person stays a capsule that cannot walk through a gondola.
- Jump apex is about 56 cm over 0.60 s (`JumpStuds` 13.2, gravity 1.25). Stock Unreal clears about 90 cm, which is a checkout counter, so the stock jump already did the vault's job. The shorter hop is why a mantle has to exist.
- Crouch will not stand the capsule up under a low ceiling the player did not ask to leave.
- Relic burden slows the walk. Ordinary loot does not. Light never costs speed.

What is still the engine's body, read from the UE 5.8 constructor in `CharacterMovementComponent.cpp` on this machine. `LLKCharacter` does not set any of these:

| Default | Value | What it does to this night |
| --- | --- | --- |
| `MaxStepHeight` | 45 cm | A pallet lip at 15 cm is a step, which is what we want. So is anything up to knee height. The first obstacle that should cost a decision has to be taller than 45 cm, or the step eats the vault. |
| `AirControl` | 0.05 | Already almost no steering in the air. The jump is not a platforming verb. Leave it. |
| `MaxAcceleration` and walking brake | 2048 cm/s² | Walk speed is reached in about 0.11 s, a sprint in 0.17 s (at the speeds authored 2026-11-04; this row said 0.22 s when the walk was twice as fast). Motion-capture starts are slower and shaped like a spring, not a clamp. This is the snappy part. |
| `bEnablePhysicsInteraction` | true | The capsule already shoves simulating props, with a very large push factor. A loose cart can be physical. The crate the Sweeper files cannot be, or the beat becomes a shove. |
| Walkable floor | Z 0.71, about 45° | Steep enough for a collapsed tile. Not the first problem. |
| Capsule | radius 34, half-height 88; crouched half-height 40 | Standing body about 176 cm. Crouched body about 80 cm. A stock gondola base is not a crouch gap. A gap has to be built. |

One override is already ours: crouching may walk off a ledge. The engine default refuses. The dock edge is where that shows up.

**Retuned 2026-11-04. The live numbers are here; the paragraph after them is kept as the reasoning that caused the change, not as a reading of the game.** Walk is 8 studs a second, 224 cm/s. Sprint is 12.5 studs, 350 cm/s. Crouch is 4.5 studs, 126 cm/s (`data/lantern.json` at `HEAD`). The Sweeper is 6 studs, 168 cm/s (`data/enemies.json` at `HEAD`).

The speed was the presence question nobody had played, and then Chris played it. Walk was 16 studs a second, 448 cm/s, because that was Roblox's default humanoid and the conversion was checked against Unreal's 600, not against a person. The measured preferred human walk sits around 1.1–1.6 m/s (Ralston's cost minimum is about 1.23 m/s; a 2025 center-of-mass study finds about 1.2 m/s for young adults on level ground). 4.48 m/s is past the usual walk-to-run transition, near 2 m/s. It was a run. That is the reading Chris acted on: after the dressing pass gave real racking to judge against, every movement value halved, and then sprint was raised by hand from 11.5 to 12.5 studs because the dressed circuit read slow in that one gear. So this paragraph's closing line held exactly — the number stayed Chris's, and it got played, not pasted in from a paper. What did **not** follow is the metabolic number: 2.24 m/s is brisk, not a stroll, and the walk still beats the Sweeper by 56 cm/s — the same margin as before the retune, because the ratios were preserved at k=0.5 on purpose. The stumble no longer simply inverts that race; the pain row has what it does instead.

The greybox earns the traversal by containing four measured things and nothing else:

| Thing | About | What the body should do |
| --- | --- | --- |
| Pallet lip | 15 cm | Step. No prompt. |
| Checkout or service counter | 90–100 cm | Cannot be jumped. Vault or mantle, and a bulky carry blocks it until set down. |
| Gondola run | taller than the capsule | Go around, or crouch a gap that was built, not a gap the capsule ignores. |
| One hatch or rail | authored | A climb point. The rest of the wall is a wall. |

The four holes from the first outside pass, closed 2026-11-02. This is still not every paper.

**Engine source.** The table above is the 5.8 constructor, not a remembered default. Mover remains experimental in the 5.8 docs. Daniel Holden's January 2026 note on Smooth Walking, written while on the Unreal team, is the one that matches the acceleration row: the old walk mode is Unreal Tournament, and captured starts and stops are not linear.

**Horror locomotion.** *Alien: Isolation* shipped without a jump, and the team has said sticking the camera to the head is what made the station feel alive (Noclip, 2020). Footsteps are a sensor for the creature, not just a travel speed (Creative Assembly, documented in *Game Developer*, 2017). *Amnesia* connects the body by making a door a continuous gesture rather than a button. The pattern that transfers is a short move list, noise that matters, and no extra verb that turns the aisle into a course. It is not a measured walking speed we should copy.

**Motion matching.** Simon Clavet's GDC 2016 talk, and the Ubisoft Toronto bootcamp the same year, search a few minutes of captured motion for the frame that matches the current pose and the path you are about to take. The result can look like a person and still answer the stick. It is an animation search. It does not decide collision, step height, or whether a counter can be jumped. There is no player body to play it on yet. First person, and the co-op body, come after the beat reads.

**OpenSim and MuJoCo.** OpenSim Moco's own paper says direct-collocation muscle solves are a poor fit for interactive control and for uneven ground. An OpenSim walking policy that was not told a speed settled near 1.45 m/s, inside the preferred band, and still missed the ankle. MuJoCo humanoid benchmarks in 2024 show flat reinforcement learning struggling to walk at all, and doing worse when the hands are dexterous. A 2023 transfer onto a real HRP-5P humanoid needed the simulator to be made worse at torque tracking before the policy survived. A 2026 game-engine port of the same family of policies reports foot sliding and notes that shipped games do not run the player this way. Muscle simulation is how you study a gait. It is not a character controller.

Until the four heights exist, a vault system would be tuned against the test harness, and the numbers would be wrong the day the store is.

### The head, the hands, and a real counter

Researched 2026-11-02, after the movement pass. Still proposed. Not a decision.

**The eye is consistent. The constant's comment is not, and the fallback is the dangerous path.** The camera sits 64 cm above the capsule center. The center is 88 cm off the floor, so the picture is 152 cm off the floor. In play the gameplay light is not that constant. It is attached to `LanternAnchor`, a child of the camera at 42 cm forward, 25 cm right, and 19 cm down (that pose is `HEAD` as of `bb50592`), so the building judges a lantern in the hand, and that hand moves with the look. There is no camera dip today, so the coupling is quiet. The header in `LLKLanternFeel.h` still says 152 is both "above the floor" and an offset added to `GetActorLocation()`. A character's actor location is the capsule center, so that sum is about 240 cm, 88 cm above the eyes. `GameplayLightPosition` does exactly that add, and the tests lock it. The running component only calls it when the light has no parent. If that fallback ever runs on a live keeper, the storm, the Warden, and relic legibility jump to the ceiling. The comment should be split into a floor height and a center offset before anyone uses the constant as both. That correction is Claude's lane.

**Retail heights, so the greybox stops being round numbers.** ADA 2010 section 904.3.2 caps a checkout counter at 38 inches, 96.5 cm, and allows a 2-inch lip above that. Section 904.4 caps the accessible part of a sales counter at 36 inches, 91 cm. A standing retail counter is commonly 36 or 42 inches (91 or 107 cm). All of those are above the 56 cm jump. The mantle still has a job. An empty GMA pallet deck is about 15 cm, under the 45 cm step, so it stays a step. Convenience gondolas are often 54–60 inches (137–152 cm). Department-store runs go to 84 inches and past it, which is taller than the 176 cm capsule, so you do not look over them. Knee clearance under an accessible counter is 27 inches, 69 cm. The crouched capsule is about 80 cm, so that hole is not a hiding place unless it is built deeper than the code.

**The view.** First person already raises presence and third person lowers it, including outside games (a 2024 driving-simulator study). Row 44 stays right. The camera's field of view is unset, so it is Unreal's 90°. A 2025 review of visual cybersickness finds that oscillating the view makes people sick and that restricting the peripheral field reduces it. There is no head bob and no landing dip in the character code. Do not add one to "sell" the walk. The look is already immediate, and the lantern is allowed to trail it by at most 9°. That split is the right one: the head is honest, the object in the hand is late.

**The hands.** A relic hold is already a timed commitment (`pickupSeconds` runs from 0.7 to 3.8). The prompt is shared by everyone looking at it, which is a known limit, not a new system. Bulky carry taking both hands, and peeking a lantern past a shelf, are proposed and waiting on geometry. *Amnesia* makes a door a continuous gesture. That is a later interaction, and it is not required for the crate beat. What the beat needs is the hold you can feel and the interrupt you can hear.

**Footsteps.** They are not a sensor in this game. Notice is light. Row 16 is the enemy announcing itself, not the floor announcing you. Footsteps, when they are authored, are so a keeper can hear their own weight and a friend can hear where the other one is. Wiring them into Sweeper perception would be a second notice rule, and it does not belong in this pass.

### The verb list

Chris, 2026-11-02, the body as a pile of words: jumping, running, stopping, climbing, animation, getting down, ladders, hanging, vaulting, getting up, parkour, pain, falling, walking, limping, carrying, relativity, sounds, footsteps.

Row 63 already answered the pile. Improvisation, not a superhero. The verbs below are what each word is allowed to mean. Anything with no row is not a system yet.

| Word | What it is today | What it is allowed to become |
| --- | --- | --- |
| Walking | 8 studs/s, 224 cm/s, 2.24 m/s. Brisk, and still above a measured human walk. | Chris's number, played in the aisle. Not a paper's 1.2 m/s pasted in. |
| Running | Sprint 12.5 studs/s, 350 cm/s, 3.50 m/s. A jog, not a sprint. Same jump as a walk. No sprint-jump boost — which now costs more, because jump *reach* scales with ground speed. | A commitment you can hear. Not a faster route around the Sweeper's job. |
| Stopping | Brake is the engine's 2048 cm/s². A walk dies in about 0.11 s, a sprint in about 0.17 s. Both halved with the speeds; the stop was never tuned, it just followed. | A stop that takes a step or two, if play says the current one is a skid. Not a slide. |
| Jumping | Apex about 55.8 cm, airtime 0.603 s, air control 0.05. Clears a pallet. Does not clear a counter. `JumpStuds` 13.2 did **not** move in the retune, so both of those stand — but horizontal reach is airtime × ground speed, and only the speed halved: a running jump went from 270 to **135 cm** off a walk and from 388 to **211 cm** off a sprint. Nothing authored has to be re-cut, because the greybox and the Galleria shell are unbuilt. A gap cut between roughly 211 and 388 cm would now be uncrossable. | Stays that hop. A higher jump deletes the vault. Cut gaps against 135 and 211, not against the old numbers. |
| Falling | Gravity 1.25. No fall damage anywhere in the character. | A hop never injures. A drop through a broken floor is an authored hole, later, not a global damage curve tuned in the test room. |
| Getting down | Crouch, at 4.5 studs/s, 126 cm/s, 1.26 m/s — now the one gear that is near a measured human walk. The capsule will not stand you up under a ceiling you did not choose to leave. | A crouch. Not a prone, and not a roll. |
| Getting down, the other one | `downedWalkSpeed` is 0. `bleedOutSeconds` is 0. The lantern is forced off. You keep the crate (row 11, row 59). | A collapse. You wait, and the crew comes to you. A bleed timer is a playtest decision, not a default. |
| Getting up | A teammate's revive hold, or the one solo anchor. Not a kip-up. | Stays a hold. Standing is the other person spending attention. |
| Pain | On this night a Sweeper does not knock you down (`knockdownOnContact` false, landed in `bb50592`). Contact is a stumble: speed × 0.55 for 1.2 s (`bd7c300`), the stick stays yours, and there is no scripted shove. Walk during it is about 123 cm/s and sprint about 192.5 cm/s, against the Sweeper's 168 — so the beat **inverted** in the retune, and is better for it: a stumbled walk still loses, but a stumbled sprint escapes. The machine is faster than you unless you spend the sprint. That separation is what the Sweeper took a further stud off for (14 → 6, not → 7), so the worst legal state — stumbled and carrying three relics — clears by 21.7 cm/s instead of 2.4. The next contact is 1.5 s later, so one Sweeper cannot stack a second window inside the first. The third contact reports you: a floor alert that decays, 3 s when the Sweeper row omits `losesTargetAfterSeconds`. The downed collapse still exists in code for a later containment. It is not what this machine does. | A tempo cost you can feel and still steer through. Not a limp, not a flinch, not a bleed timer added to make the smack "more real." |
| Limping | Not built. | A third speed between walk and zero. It would be a second tax beside relic burden. Do not add it to make pain "realistic." |
| Carrying | Only relics slow you: 4% each, cap 12% at three (row 77). Crates do not. Light never does. A downed carrier cannot walk the crate to the dock. | Hands full when the carry is bulky, which blocks mantle, revive, and tools until it is set down. Proposed, waiting on geometry. |
| Vaulting | Not built. | One low vault, server-authoritative, after a real counter exists. |
| Climbing, ladders, hanging | Not built. | Authored points only: a ladder, an escalator rail, a hatch. A hang you can start on any ledge is the superhero. |
| Parkour | Refused, row 63. | The word for the moveset that makes a counter a puzzle instead of a risk. |
| Animation | A co-op mannequin exists. The owner does not see their own body. No montage for any verb above. Look is immediate. The lantern may trail the look by at most 9°. | Hands and the lantern, after the move exists, so the animation is of a thing you already did. Motion matching is a search over a body we do not have. |
| Sounds, footsteps | No footstep asset is wired. Enemy cues exist as files and as unbound-until-debug events. | One sound per verb that is already real: the interrupt, then a step that marks your own weight and tells a friend where you are. Not a sensor. |
| Relativity | Not a system. | How fast the aisle feels. Optic flow from a gondola close to the eye, the Sweeper at 1.68 m/s beside a 2.24 m/s walk, that same Sweeper faster than a stumbling walk and slower than a stumbling sprint, the lantern late relative to the head. Perceived speed is a ratio. It is not a new movement mode. |

The order under the table is the same order as row 63. Play the walk, the stop, and the hop on a floor that has a pallet and a counter. Then one vault. Then one authored climb. Then the hands and the sounds of those moves. Pain stays the collapse that is already written.

### Five feelings, each turned over

Status: **proposed, 2026-11-02.** Chris asked for a night that is real, scary, fun, addictive, and a gamble with the emotions, and for every reading to be turned over before it is trusted. These are tests for the aisle. They are not a second pitch and they are not a `DECISIONS` row.

`reports/Retention and escalating dread.md` (2026-10-29) already covers fear habituating before players notice, the thin evidence for loss-aversion as a retainer, and the median buyer of the comparable games leaving inside the novelty window. This section does not re-argue that report. It asks what each feeling word does to this aisle, and what result would kill the claim.

A claim here names an operand, a floor, a clock, and a falsifier, per `docs/ADVERSARY.md`. No number below is a new authored value.

#### 1. Real

**Claim.** The aisle feels real when the store is doing store work and the body meets store sizes. A pallet lip near 15 cm is a step. A counter near 90–100 cm is not a hop. A department-store gondola is taller than the capsule.

**Operand.** One played minute in Boswell's, the body against the four heights in the table above.

**Floor.** Anything under the engine's 45 cm step is not a traversal decision. The step eats it.

**Clock.** One PIE session on the map that is actually loaded. A headless test has no fixtures.

**Falsifier.** The counter can be hopped, or the pallet asks for a prompt. Then the heights are wrong, or the jump number moved, and a vault tuned tonight would be wrong the day the store is.

**The reading that kills "make the walk real."** Preferred human walk sits near 1.1–1.6 m/s. This walk is 2.24 m/s and the Sweeper is 1.68 m/s (both halved 2026-11-04; this paragraph said 4.48 and 3.92, and its 2.8 m/s Sweeper was retracted before that). Gait realism and threat realism are two operands, and a paper cannot pick between them — but note what playing the aisle actually did to this reading. The walk came *most* of the way toward the metabolic band without being set from it, and the thing this paragraph warned against did not happen: the ratios moved together, so the keeper still outwalks the cleaner by the same 56 cm/s. The one gear now inside the measured human band is the crouch, at 1.26 m/s, and it is the gear you choose when you are hiding. Playing the aisle picked. It did not pick 1.2.

**The reading that kills "make the floor real."** Lowering `MaxStepHeight` so every curb costs a vault turns the store into a course. That is parkour by friction, which row 63 refuses. One counter above 45 cm is the decision. A global step change is the costume of that decision.

**Footsteps, turned over.** *Alien: Isolation* lets the creature hear steps, and a store that never hears the keeper can feel like a slideshow. The split that survives the turn-over: the keeper hears their own weight, and a friend hears where the other one is. The Sweeper's notice stays light. Wiring volume into aggro is a second notice rule beside row 16. **Falsifier:** with a step playing, a dark walking keeper is still unnoticed inside the dark radius, and a lit one is still noticed out to the lit radius. If darkness stops protecting a walking keeper, the split failed in the build.

**The eye, turned over.** The pose landed. `LanternAnchor` at `(42, 25, -19)` is `HEAD` as of `bb50592`, and the earlier warning that it was only in the working tree is retracted. The dangerous path is unchanged: `GameplayLightPosition` adds 152 cm to the capsule center, the tests lock that add, and the live component uses it only when the light has no parent. Splitting the comment into a floor height and a center offset is still Claude's lane, and it is a ride-along on the next touch of that header, not its own milestone. **Falsifier:** in PIE, print the gameplay light's world Z and the actor's world Z. Attached, the difference should be a hand below the eye. A difference of about 152 cm means the fallback is what the building is judging, and the comment bug has become a play bug.

**Head bob, turned over.** The 2025 review of visual cybersickness is mostly about VR, where oscillating the view makes people sick. It does not prove a flat 90° picture needs zero camera motion. A walk with no gait cue can feel like a dolly, which fails "being there" from the other side. The move that does not spend the refused bob is the one already written: a step you can hear, and a lantern allowed to be late by at most 9°. **Falsifier:** Chris plays and the walk feels like a slider. Sound and lantern lag are the first change. A camera dip stays refused until those fail in a chair.

#### 2. Scary and fun are one curve

Splitting these two words is how a horror game gets louder until people stop playing it.

**Claim.** In a frightening experience people chose, enjoyment peaks at a middle amount of fear. Too little is flat. Too much stops being pleasant. Positive and negative feelings can be on together. For people who like the genre, the pleasant peak can sit inside the frightening stretch. People who do not like it need a frame that keeps the harm bounded.

**What was measured.**

- Andersen, Schjoedt, Price, Rosas, Scrivner, and Clasen, "Playing With Fear: A Field Study in Recreational Horror," *Psychological Science* 31, no. 12 (2020), doi:10.1177/0956797620972116. A commercial haunted house, N = 110, ages 12 to 57, heart-rate monitors, repeated self-report. Enjoyment against fear is an inverted U on the repeated measures. Large heart-rate fluctuations track fear in a straight line. Small fluctuations track enjoyment as an inverted U. The article page for that DOI also carries the authors' summary caveat: when the same people rated fear and enjoyment for the *whole* attraction, the curve was not there. That sentence is a check for the other sessions. If the PDF does not say it, this paragraph retracts in a commit.
- Andrade and Cohen, "On the Consumption of Negative Feelings," *Journal of Consumer Research* 34, no. 3 (2007): 283–300, doi:10.1086/519498. Four studies on horror-movie clips. Positive and negative affect co-activate. Among fans, pleasant ratings peaked in the most frightening stretches. Among avoiders, pleasant ratings rose when a protective frame was put in front of the clips, a reminder that the footage was performed. The term is Apter's; Andrade and Cohen use it as detachment.

**Operand.** What the crew does during the run. Row 79 already locks the instrument: measure the behaviour. A sentence after the run is the rating Andersen's caveat says can miss the curve, and it is the rating Grizzard, cited in the retention report, says will keep calling the night tense after the body has stopped responding.

**Floor.** One "that was scary" at the dock is under the floor. It cannot see the sweet spot.

**Clock.** Inside the run, at the moment the Sweeper files or the hold is called. The paper's curve was taken at the scares, not at the exit.

**Falsifier.** The lantern stays on for the whole night and nobody argues about the dock: that is under the curve, no fear and so no fun of this kind. Or somebody quits during the hold and does not load the map again: that is past the curve. "They said it was tense" moves neither result.

**Where this aisle already sits.** The first Sweeper ignores the keeper and takes the crate. That is a small deviation from the expected night. A chase in the first minute is the large, long deviation the heart-rate result associates with unpleasantness. The protective frame is already locked, and it is what lets the fear stay consumable: machines contain and file rather than hunt for a kill (row 47), the haul is lost for this trip and secured only by extraction (row 8), a downed keeper waits with the crate so the crew still has a choice (rows 11 and 59), and the dock call is a hold the crew can name (rows 75 and 76).

**The reading that kills "make it scarier."** Past the middle, more fear buys less enjoyment. A bleed timer added to raise the stakes, a hunt in the first minute, or a failure the crew cannot name are all exits off the right-hand side of the curve. The retention report already found no documented pacing system that answers peak stress by adding threat. Row 64 keeps the meter's authority rising and spends it in breaths. This slice says the same thing from the feeling literature: the breath is what keeps the fear inside the part people will pay to feel again.

**The reading that kills "they're friends, lean into the jokes."** Andrade's frame and the retention report's voice-chat finding are one mechanism. Detachment is how an avoider enjoys fear. Four people talking turn the night into comedy, which is detachment winning completely. The night cannot maximize both. Row 16 is the structural answer already locked: a threat channel the joke does not cover. A procedure you can see, and a cue that is not only someone saying "it's behind you." If the only scare is a sound the call talks over, the frame ate the fear and the curve collapsed to flat.

**`bleedOutSeconds` at 0, turned over.** An indefinite wait can be the middle of the curve. The crew still has agency, and the choice is slow enough to argue, which is the version the retention report treats as the strong one. The same zero can sit past the curve: a body on the floor, the night stalled, nobody playing. The data file cannot tell those apart. **Falsifier:** in a two-keeper session the standing keeper goes to the downed one and the run continues. If they stand there because nothing is allowed to end, the zero is past the curve, and a timer becomes a playtest question. It is not a number this section sets.

#### 3. Addictive

Chris's word. In the research the same word names a clinical pattern: control slips, the activity outranks the rest of life, it continues through harm. That pattern is the one this night must not be built to produce. The want that belongs in the thesis is the line already in `GAME-THESIS.md`: finish or lose a run and want another.

Przybylski, Rigby, and Ryan, "A Motivational Model of Video Game Engagement," *Review of General Psychology* 14, no. 2 (2010): 154–166, doi:10.1037/a0019440, put that want on three needs: competence, autonomy, relatedness. They treat disordered play as a different pattern from play that satisfies those needs, and they report that rewards bolted on from outside the activity undermine the reason the activity was worth doing. This section uses the distinction. It does not propose their questionnaire. A questionnaire would also cut against row 79.

**Operand.** The crew starts another night because the last one left a job unfinished, or because a counter outside the night told them the window was closing.

**Floor.** One replay after a novel first run is the chair test. It is not a month of retention. The retention report's median buyer of the comparable games is gone inside roughly 7 to 16 hours. A second click does not refute that.

**Clock.** The minute after extraction or wipe.

**What this aisle can offer without borrowing a live-service trick.**

- Competence. The keeper caused the interrupt and could hear that it worked. A success that looks like nothing teaches nothing.
- Autonomy. Call the dock, or go back for the crate. The meter does not pay a track for the choice.
- Relatedness. A second lantern is this same night. The long-horizon social finding in the retention report is real, and a private lobby cannot replace a group that drifts. That limit stays.

**The reading that kills "add a reason to come back."** Seasons, streaks, battle passes, and randomised boxes manufacture the return from outside the night. Zendle and Cairns, *PLOS ONE* (2018), surveyed 7,422 adults and found loot-box spending associated with problem-gambling severity at about η² = 0.054. Their 2019 replication (doi:10.1371/journal.pone.0213194) found about η² = 0.051. They state the direction of cause as unknown: the boxes may lead toward the pattern, or people with the pattern may spend more. This game does not sell a randomised box. The shape to refuse is still theirs: a reward whose contents you do not know, purchased outside the night. `REWARDS.md` already says a fifth resource has to answer a fourth question, and that the four resources do not convert into each other. A streak, a season, and a pass are that fifth resource wearing a calendar.

**The reading that kills the refusal, if the refusal is heard too wide.** "Do not be addictive" can be heard as "the game should not want you back." The thesis already wants the next night. The return that survives both readings is an unfinished store job: the crate was filed and the bay is known, or the crew extracted and the hall shows one change. That is competence and autonomy. A timer that punishes logging off is the other pattern.

**Falsifier.** After a completed or lost run, a tester loads the same map again with no prompt from us. If they do not, the night failed the chair test, and a streak will not repair a night that was not worth a second pass. If they load again only when a counter threatens to expire, the build produced the pattern the clinical word names, and that counter comes out.

#### 4. Gambling with the emotions

**Claim.** The night is a bet the crew can price before they take it. One more relic, or call the dock. The price of the call is noise, light, and a route (rows 75 and 76). A wipe loses the whole carried haul (row 8). Each relic in the hands slows the next step, capped at three (row 77), so greed has a body. The haul feels owned before it is secured, and the feeling and the object can be lost together.

**What the gambling result actually was.** Clark, Lawrence, Astley-Jones, and Gray, "Gambling Near-Misses Enhance Motivation to Gamble and Recruit Win-Related Brain Circuitry," *Neuron* 61, no. 3 (2009): 481–490, doi:10.1016/j.neuron.2008.12.031. On a slot-machine task, a near-miss felt worse than a clean miss and raised the desire to play again. That desire showed up when the person had arranged the gamble themselves. The near-miss recruited striatal and insula activity that also answered a real win, and no money was paid. The surrounding write-up of the same study makes Clark's distinction: that reinforcement is sensible in a skill, where "almost" means the aim is improving, and it is a mistake in a game of chance, where almost means nothing.

**Operand.** A choice the crew can say before the outcome, set against an outcome the building then actually delivers.

**Floor.** "We almost made it," with no choice under it, is under the floor. That sentence is the slot machine. A near-miss the designer forces — the Sweeper always finishes a second after the keeper arrives, the hold always fails on its last second — keeps the urge and deletes the lesson, because there was no skill and no price.

**Clock.** The decision at the crate, and the decision at the dock.

**The reading that kills the honest bet.** Authoring the near-miss. If the file, the siren, or the wipe is tuned so the crew almost wins on a curve they cannot see, the store is wearing Clark's reels. Personal control was the condition for the desire effect. Remove the control and the unpleasantness and the urge remain, and the crew learns nothing they can do differently.

**The reading that kills it from the loss side.** A wipe the crew cannot explain. The retention report already cuts the folklore down: tension scales with how large and visible the stake is, the evidence that haul-loss *retains* people is nearly empty, and a total loss feels like a bet only when the crew can see that they chose to stay. Row 8 removed mid-run banking. The substitute already locked is a short night and a loss bounded to the trip. If a wipe still feels random, the bet failed while the rule stayed true. **Falsifier:** after a wipe, the crew names the choice. They lit up, they stayed for another relic, or they called and left the dock empty. If the only sentence is "it got us," the price was not legible. The fix is legibility. A softer loss and a nearer miss are the two escapes this slice refuses.

**Not built from this slice.** A pity timer, a last-second save, a visible "almost" meter. Each of those is the near-miss, authored.

#### 5. Great, turned over

On the test this repo already trusts, great is the list in `GAME-THESIS.md`: they understand the job and leave, they can say why they used the light, they recognise the threat on two channels, they argue about the dock, they want another night, and home shows one change from what they recovered. Under that list sits the retention report's harder number. The median buyer of the closest games leaves inside the novelty window, and a solo premium game that plans for a steep decay is planning for that dataset. Lethal Company's small retained fraction and large sales are the case the report uses. This pass did not re-pull SteamCharts. If those medians have moved, the report is stale, and this sentence is stale with it.

**The reading that kills the ambition.** Hearing "it is going to be great" as permission to open the valley, a second enemy, a quest log, or a calendar of return-rewards before the aisle passes the chair test. The hard run already refuses the content. This slice adds the feeling-shaped version of the same refusal: a bigger scare, a stickier counter, and a crueller near-miss are scope.

**Falsifier.** The six thesis behaviours, observed, on this map. A sentence that the night will be great is not one of them.

#### What the other sessions can try to break

1. The Andersen whole-attraction caveat. It is taken from the article page at doi:10.1177/0956797620972116, not from a PDF opened in this session. If the paper does not say the summary rating lost the curve, retract that sentence in a commit and keep the repeated-measures result, which the abstract does state.
2. ~~The anchor pose is dirty.~~ **Resolved 2026-11-05.** `(42, 25, -19)` is `HEAD`, and the board may now be quoted for it. M1 records the 17.0 cm move.
3. ~~`data/lantern.json` is dirty.~~ **Resolved 2026-11-05.** It is committed, `knockdownOnContact` false is the shipped row, and the pain row above describes the stumble rather than a knockdown. The downed paragraphs still hold rows 11 and 59 for a later containment.
4. Any number in this section that disagrees with `data/` at `HEAD` loses. The data wins, and the sentence comes out. The 2.8 m/s Sweeper and the "anchor is uncommitted" warning were that kind of sentence. So was every speed on this board, rewritten 2026-11-05 — and it is worth recording *how* they were caught: not by reading the board, but by checking it against the engine and `data/` deliberately, two days after the retune. A stale number in prose does not announce itself the way a stale literal in code does, because nothing compiles it and no test reads it. `docs/FINDINGS-SPEED-SCALE-COPIES.md` tracks this as copy 6, the first copy of the speed scale that was prose.

### The build

Status: **proposed scaffold, 2026-11-02.** Chris asked to assimilate the research and implement it at full scale, as something the fleet can work from. This is that. It is not a `DECISIONS` row, and it does not open a new system.

Full scale here means the same five tests on every later floor. It does not mean building the valley, a second enemy, or a return-calendar while the aisle is unplayed. Production control allows one active package per owner. Two are active. Everything under "Later, same rules" is inactive until the active cards close.

What already exists, so the cards do not rebuild it. The Sweeper patrols, telegraphs, files, and interrupts. Extraction is a state machine with a price. Relic burden is read. The stumble is server-authoritative and replicated. `knockdownOnContact` is false. Boswell's has departments: toy-side tables, a furniture platform, a service counter, a front display case, gondola runs with raised shelf lips, and TOYS / HOME / EXIT landmarks (`bb50592`). There is still no vault, no climb, no footstep asset, and no authored sound on `OnStockInterrupted`. Nobody has measured the landed counter in centimetres. That measurement is the implementation. A vault written before it would be tuned against a guess.

#### Active — Chris — one sitting

**Owner.** Chris. He is the only one who can press Play.

**Depends on.** The map and the pawn that are already on `HEAD`. Not blocked.

**Does.** Load `/Game/LLK_Boswells_FirstRun` and write seven lines. No code.

1. Height of the service-counter top, in cm, against the 56 cm hop and the 45 cm step.
2. Height of a gondola run, against the 176 cm standing capsule.
3. Whether any lip on that floor is under 45 cm, and whether the capsule walks it with no prompt.
4. Whether the Sweeper, unwatched-into, takes the authored crate while you stay dark.
5. Whether `OnStockInterrupted` is audible. The expected answer today is no: the debug HUD has the line, and no sound is bound.
6. On contact: the stick stays, and for about 1.2 s the Sweeper is faster than both the walk and the sprint. If the shove that already lives in the movement component throws you off a ledge, say so. A scripted impulse is still his call and stays off until this line exists.
7. After a wipe or an extraction, the choice you would name out loud. "We lit up," "we stayed," "we called and left the dock," or only "it got us."

**Acceptance.** Seven lines, dated, in the handoff or in `NOTES-FOR-CHRIS.md`. A missing line is a missing measurement, not a soft pass.

**Modes.** Solo PIE is enough for this card. A second keeper is the later card.

**Evidence.** The seven lines. A capture only if a line is easier to show than to write.

**Files.** None in `Source/`, `Content/`, or `data/`. The note is the product.

**Feelings this closes, or fails.** Real, on lines 1–3. Scary and fun, on lines 4 and 6: the crate leaving while you are ignored is the small deviation; a first-minute hunt, or a stumble that takes the stick, is past the curve. The bet, on line 7. Competence, on line 5, which is expected to fail and so names the next card.

#### Active — Chris — the interrupt you caused

**Owner.** Chris. Row 41: sound is his.

**Depends on.** The delegates, which exist. Can share the sitting above. Not blocked on the counter measurement.

**Does.** One authored sound on `OnStockInterrupted`, then `OnStockFiled` at the bay so the walk has a direction. `OnTelegraph` can ride the same pass because row 16 is the accessibility lock. The debug readout is deleted only when the sound carries the beat without it.

**Acceptance.** A keeper who lights up can tell that the lighting is what saved the crate, with the voice channel muted. That is the curve and the competence need in one event. A success that looks like nothing is not a success.

**Modes.** Solo first. Then one listen-host check that the other keeper hears the same interrupt.

**Files.** The sound asset, under Content, his. If the wav cannot be heard without a subscribe on the existing delegate, that subscribe is one binding and it is Claude's, on the delegate that already exists. It is not a bus, not occlusion, and not MetaSounds.

**Does not.** A footstep sensor. A mix. A second enemy cue set.

#### Ride-along — Claude — the eye comment

Not a package and not a milestone. The next time `LLKLanternFeel.h` is open, split the comment so 152 cm is a floor height and the center offset is a different number. Do not change the add the tests lock. Do not change the attached path, which is the hand. The PIE print on the falsifier above is what says whether this is a comment or a play bug.

#### Opens only after the seven lines — Claude — one vault, or no vault

**Owner.** Claude, runtime. Chris keeps the heights.

**Depends on.** Lines 1–3 of the sitting. Blocked until they exist.

**The branch.** If the counter top is above 56 cm, build one server-authoritative low vault over that counter, and a ledge mantle of the same family. Bulky carry blocks it once bulky carry exists; until then the block is a commented seam, not a fake crate. If the counter top is under 56 cm, do not build the vault. The greybox is short, and the hop is already doing the vault's job. Raising `JumpStuds` to clear it is the same mistake.

**Acceptance.** On the real counter, a jump fails and the vault succeeds, and the server is the one that says so. A client cannot vault a counter the host rejects.

**Modes.** Solo, then listen host plus client.

**Files.** `Source/LLK` traversal only, plus a test that fails when the vault succeeds with no counter in range. Not `IMC_LLK`. Not a free-climb trace.

**Does not.** `MaxStepHeight` changes. A hang on an arbitrary ledge. Motion matching. Mover.

#### Later, same rules — inactive

These are the full scale. Each one inherits the card above it. None of them is a reason to skip the sitting.

| When | What gets built | Owner | The test it has to pass | Refused while building it |
| --- | --- | --- | --- | --- |
| The interrupt is audible | One footstep. The keeper hears their weight. A friend hears where they are. | Chris for the asset. Claude only if a subscribe is required, and the subscribe reads no enemy sense. | With the step playing, dark aggro is still 12 studs and lit aggro is still 38. A walking dark keeper is not noticed. | Volume as a Sweeper input. Head bob. A landing dip. |
| The vault has been played on the real counter | One authored climb: a ladder, an escalator rail, or a hatch. | Chris places the point. Claude makes it server-authoritative. | The rest of the wall is a wall. | Free climbing. A hang on any ledge. |
| That climb exists | Hands and the lantern, animating a move the player already did. | Chris for the mesh, Claude so the gameplay light does not follow the cosmetic sway. | The building still judges the hand anchor, not the posed mesh. | Motion matching. A body the owner can see, before row 44 moves. |
| The seven lines include a named wipe or a named extract | Nothing new. Extraction already has the price. | Chris plays it. | The crew can say which choice cost the haul. | A pity timer, a last-second save, an "almost" meter, a nearer miss. |
| The aisle is fun in a chair, per the six thesis behaviours | One visible change at home from a crate that was secured. A job becomes real by gaining a `restoration` block, which is the existing rule. | Chris for what the hall shows. Claude only on the block the rules already name. | They load the same night again with no prompt and no expiring counter. | A season, a streak, a pass, a fifth resource, a shop stat on night one. |
| That replay happened | The next floor repeats the four heights. It does not repeat a new moveset. | Chris, content. | A pallet is still a step and a counter is still not a hop. | A second zone opened to make the game feel larger. |
| The first slice is played | A later machine may contain. Rows 11 and 59 describe that collapse. Row 49 stages it after the slice. The Sweeper is not that machine. | Claude for the pawn, when a pawn is actually next. | The crew can still choose. `bleedOutSeconds` stays 0 until a session shows the wait is past the curve. | Giving the Sweeper its knockdown back in order to have a failure state. The stumble and the alert are the failure state. |

#### Not in any card

A metabolic walk pasted over the authored walk. A lower step height. A global fall-damage curve. A limp. A bleed timer as a default. A designed near-miss. A streak. A questionnaire about whether it felt scary. A second enemy. A quest log. The valley. Any of those would be the turned-over reading, built by mistake.

### Refused until this night is fun in a chair

A second enemy. Lanternhold's opening door. A shop. A quest log. A valley
map. Any answer about who is in control. Each can be true later. None of
them makes this aisle alive.

### What the other games are for

Checked 2026-11-02. Used as tests, not costumes.

**Amnesia: The Bunker.** One small place, a generator that is a job, a
stalker that is a system. The making-of states the rule in the team's own
words: a stalker scripted to step in when you round a corner is less
frightening, because you would know he was coming. The Sweeper's version is
a lap that might take the crate and might not.

**Still Wakes the Deep.** The rig hums, and other people are still trying to
keep it up. The version that fits this night is a service loop and a lit
lane, with nobody to talk to. People, when they arrive, are the filed, and
they are not this run.

**Alien: Isolation.** Someone is left who can service a machine, and the
machine's attention is the price of the work. The lantern is that job.

**The trap those games already fell into.** Bunker players who found most of
the fuel early have described the rest of the dark as comfortable. This
curve already moves the needle when the lantern comes on and does not fire a
threshold inside the first minute. Keep that. A resource that makes darkness
feel provided for is the wrong scarcity. Scarcity, if this night grows one,
is a distraction you can spend once.
