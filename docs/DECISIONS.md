# Decision log

Every binding call on this project, who made it, and where the reasoning lives.

Three agents and one human have made dozens of these across commit messages,
notes files and chat. They were becoming unfindable, which is how a guardrail
gets deleted by someone who never knew it was one.

**Status means:** `LOCKED` — do not change without Chris. `PROPOSED` — written
down, not yet ratified. `OPEN` — named, undecided, waiting on someone.

---

## Platform and scope

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 1 | **Steam, PC-first.** Last Lightkeepers is a desktop game. There is no parallel Roblox version | LOCKED | Chris, 2026-10-27 | "Choose Steam. PC-first. No parallel Roblox version." Reverses the handoff's sequencing rather than advancing it. The authored thing this design asks for - a dense department store, serious lighting, sound, animation, private expeditions - is a small Steam game, not an experience that must also hold up on a phone at low graphics, where shadow quality degrades and dense lights are the headline performance cost. Nothing in `src/` ports. `docs/PLATFORM.md` |
| 2 | No PvP in this game. Adding it would replace the design, not extend it | LOCKED | Chris, handoff; re-scoped 2026-10-27 | Handoff, "Resolved design decisions". The supporting evidence in `reports/` - PvE is the million-CCU tier on Roblox - went with row 1; **the rule did not.** On 2026-10-27 Chris named "Counterstrike style multiplayer or something way later" as a thing to consider. That is logged as **O6**, undecided, and a later competitive mode would be a different product sharing a setting - it does not unlock this row for Last Lightkeepers |
| 3 | Do not expand scope before the v1 loop passes its playtest question | LOCKED | Chris, handoff | Handoff. The question: do testers kill their lanterns, bank, and argue for one more room? |
| 4 | One mall, and it is the Galleria | LOCKED | Chris, 2026-10-21 | "Whatever mall u built is our mall… let's unify and stop having 8 malls." `NOTES-FOR-CODEX.md` |
| 28 | Version one is **one sealed department store**, furnished top to bottom. The rest of the Galleria is earned, not cut | LOCKED | Chris, 2026-10-26 | "The whole Galleria can be the eventual world, but the game begins with one sealed department store and earns its expansion." `docs/THE-STORE.md` |
| 29 | Progression is a ladder of **practical recovery jobs**. Each one changes something visible, opens a route or a tool, and wakes a new threat. Never "collect ten glowing widgets" | LOCKED | Chris, 2026-10-26 | The opening five rungs are his. `docs/QUESTS.md` |
| 33 | The Roblox project is a **design reference and prototype archive**. Not a second product, not maintained, not shipped | LOCKED | Chris, 2026-10-27 | "Keep the Roblox project only as a design reference and prototype archive - not a second product to maintain." Nothing is deleted; nothing is advanced either. Lanes, tag contracts and worklists in this repo describe a frozen prototype from here on |
| 34 | The first thing **built** is one department store, vertical slice. A build order, **not** a scope cut - see row 38 | LOCKED | Chris, 2026-10-27 | "Build one PC-quality department-store vertical slice. Do not try to recreate the whole mall yet." Carries row 28 across the platform change unaltered - the sealed store was always the shape of version one, and the rest is earned rather than cut. The word doing the work here is *first*: this row says where the shovel goes, and says nothing about how big the game is |
| 38 | **The whole world is the game.** All the levels, the whole mall included, reached by progression - and each expansion arrives as **more danger**, not just more floor | LOCKED | Chris, 2026-10-27 | "We will have all the levels, including the whole mall. It starts off small, progressesion. Danger." Settles any reading of rows 28 and 34 as a trim. The ladder in `docs/QUESTS.md` and the zones in `docs/ZONES.md` are the shape, and they already carry the rule that makes expansion mean something: danger is a function of how close to the city you get, and each zone inverts the previous one's core constraint. **A new level that is not more dangerous than the last one is wallpaper** - that is the test every future zone has to pass, and it is why the store comes first rather than the mall: the first building has to teach the fear that the second one escalates |
| 35 | Engine: **Unreal Engine 5** | LOCKED | Chris, 2026-10-27 | "The next real decision is engine, not platform. My recommendation is Unreal Engine 5." The reasoning is his: the strongest path to wet, decayed department-store lighting, dense interiors, cinematic sound and believable enemies - "the right kind of hard, because we would be building the actual game once." Ratified the same day: "We use unreal." **This is the choice that cost the most to reverse, and it is now the floor the project is built on** - every art, lighting, audio and animation decision from here assumes it, and reversing it later discards more work than the Roblox freeze did. Written down as his, on his words, rather than inherited by default |
| 36 | Solo, or private co-op for 1-4 friends. No public matchmaking | LOCKED | Chris, 2026-10-26, restated 2026-10-27 | **O5** decided this before the platform did, and PC makes it cheaper rather than harder: no server browser, no strangers, no persistent world. "Definitely single player or co op" |
| 37 | Wider testing uses a **Steam Playtest child app**, not main-game beta keys | LOCKED | Chris, 2026-10-27 | Steam's own recommendation, and a Playtest can be opened or closed without touching the main game's store or review state. The reserved packages have roles now: Developer Comp for us, Beta Testing for a few trusted players, Playtest for anything wider. `docs/PLATFORM.md` |

## The core rules

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 5 | Light reveals value and attracts danger. There is **no** "light repels enemies" exception | LOCKED | Chris, handoff | Handoff, "Important simplification". Softening this deletes the game's only decision |
| 6 | Storm Charge only ever rises. Darkness slows it; nothing reverses it | LOCKED | Chris, handoff | Handoff |
| 7 | The Warden is unkillable and drops nothing | LOCKED | Chris, handoff | It is the meter with a body. If it can be killed, the meter stops meaning anything |
| 8 | ~~Banked loot survives a wipe entirely; carried loot is halved~~ **SUPERSEDED 2026-10-26.** Only extraction secures loot; a wipe or an expired timer loses the whole carried haul. Disconnect uses the same rule | LOCKED | Chris, 2026-10-26 | O3 below. The handoff's banking rule was retired with the bank itself |
| 9 | The Anchor Revive is **solo-only** — one per run, paid for with a Storm Charge spike | LOCKED | Handoff; enforced by Claude 2026-10-20 | Commit `2bb8b84`. Granting it to a crew silently breaks the whole-crew-down wipe |
| 10 | Lantern charge is unlimited — the cost of light is the meter, not a fuel gauge | LOCKED | Chris, 2026-10-20 | "i don't think the lantern should ever run out… the risk is always having it on." `data/lantern.json` |
| 11 | A downed Lightkeeper keeps the crate | LOCKED | Chris, 2026-10-20 | `data/lantern.json`. The old flag only cleared the carry slot, so the drop was cosmetic |
| 12 | The lantern multiplier uses the **`diminishing`** curve. No keeper's light is ever free | LOCKED | Chris, 2026-10-27 | "Switch storm-charge.json to the diminishing curve so no keeper's light is free." Proposed by Claude 2026-10-20 as `b4b1813` and left off for a week. The clamp made a fourth lantern cost the crew nothing, so a four-player run was always the shortest one; diminishing self-limits at 4.75x and keeps the 2.5x one-lantern anchor exactly. `maxMultiplier` is now inert. Closes the open call in `docs/VERTICAL-SLICE.md` |
| 44 | **First person. Always.** Third person is not a fallback camera and not an accessibility option | LOCKED | Chris, 2026-10-28 | "First person. Lock it." His reasoning, and it is the strongest statement of row 5 the project has: *"the lantern rule is stronger in first person: you cannot peek around a corner your light does not reach, you feel the darkness close in, and holding/carrying/reviving all become physical actions instead of animations you watch from behind."* Third person would quietly refund sight the lantern did not buy, which is the mechanic's only currency - "it creates a different, less frightening game." Three build consequences, his: the lantern, hands, carry states, tools and the downed crawl are **first-person systems from day one**, not a camera setting applied later; there is no third-person fallback to keep working; and the first slice needs **no full-body player animation beyond what co-op players see of each other**, which is a real saving for a team of one. Settles the contradiction logged in `docs/PLAN-RECONCILIATION.md` item 1 - `ROADMAP.md` M1.1 was right, and Chris corrected `STEAM-PRODUCTION-PLAN.md` himself |
| 30 | Keepers **can** fight back, earned at ladder rung 2 and bounded by one rule: you win the fights you start. Nothing carried kills an enemy that has already seen you | PROPOSED | Chris + Claude, 2026-10-26 | "We might need to fight them." `docs/COMBAT.md`. Settles a contradiction the handoff shipped: five enemies have health, the code writes it, nothing could ever reduce it |
| 31 | The lantern and a tool compete for the same hand. Never both | PROPOSED | Claude, 2026-10-26 | `docs/COMBAT.md`. Makes combat cost light instead of costing a new resource, and gives a crew two jobs |
| 32 | The hall's four rooms cost **1 / 3 / 6 / 10** crates - a first-session door, then a widening gap | LOCKED | Chris, 2026-10-27 | "Keep the 1/3/6/10 room costs for now." Claude's proposal of 2026-10-26, read and kept rather than ratified by default. "For now" is his: a playtest may move them, and only he moves them. `data/ledger.json` |

## Content

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 13 | Six enemy types: Sweepers, Stockers, Glimmers, Window Walkers, Ficus, Warden | LOCKED | Chris, 2026-10-20 | Was four. Window Walker approved ("Keep the window walker, I want it"), Ficus approved ("Codex did it, I like the ficus"). `data/enemies.json` |
| 14 | Glimmers: 2-second scan, 20-second tag, all robots learn the position. No Storm Charge tax, no permanent Warden waypoint | LOCKED | Chris, 2026-10-19 | Handoff, "Resolved design decisions" |
| 15 | Enemies are derived from the job the place did. Staples everywhere, locals per zone | PROPOSED | Claude, 2026-10-21 | `docs/ZONES.md`. Already true of the roster; makes future enemies derived rather than invented |
| 16 | **Every enemy telegraphs before sight through at least two distinguishable channels, one of which may be sound** | LOCKED | Chris, handoff; **rewritten 2026-10-28** | An accessibility requirement, not polish - and the original form was failing at it. The old rule said *sound before sight*, which made audio the only warning channel. Microsoft's Xbox accessibility guidelines require key information on two or more channels and name audio-only danger cues as an anti-pattern; in co-op, voice chat is already occupying the audio channel, so a crew talking over a cue is a crew that never got one. Chris's rewrite keeps sound as the **primary** warning "because it is scary and directional" and pairs it with readable physical evidence: moving shadows, flickering fixtures, disturbed merchandise, footprints, silhouette changes, and UI-safe visual or haptic indicators where appropriate. His line is the test: **"Voice chat must never be the reason someone misses a threat."** This also repairs the curriculum in `docs/THE-STORE.md`, which taught through darkness, light and monster audio - exactly the two single channels the guidance warns against. **That repair landed 2026-10-29, and it was a day late:** for a day the curriculum carried an amendment stating that each lesson "needs a physical tell as well as a sound" without supplying any, which is a note about a fix rather than a fix. The curriculum table now has a second-channel column filled in per floor. The sharpest find was lesson four, whose exam read *"sound stops arriving from a direction you can use"* - a lesson taught by taking the audio channel away, so for a deaf or hard-of-hearing player it was not a hard floor but a floor with no lesson on it, which is the exact failure this row exists to prevent. The Third floor now takes **sightlines** instead, which its own art direction already said it did ("showroom walls cut the floor into rooms"), so nothing in the building moved. `tests/curriculum.test.mjs` now fails if any lesson loses its second channel or if that sentence returns, and was verified against the original violation. `docs/SOUND.md` |
| 43 | **The department store is the tutorial.** There is no separate tutorial, no training level and no tutorial mode. The first building teaches the whole game by being played | LOCKED | Chris, 2026-10-27 | "The department store is the tutorial." This is the strongest form of the rule `docs/HOW-THE-PLAYER-LEARNS.md` already argued for, and it answers a question rows 28, 34 and 38 left open: what the one store is *for*. It is not a small first level that happens to come first - it is the instrument that teaches light, loot, burden, the meter, the Warden and extraction, to four friends talking over each other with nobody narrating. The store's own ruin gradient is the curriculum: `docs/THE-STORE.md`. Two build consequences, both sharp. A grey-box floor is now judged on whether it **teaches**, not on whether it is navigable - which raises M1.5's bar. And a tutorial that is also the first real level has to land inside Steam's two-hour refund window, so **O7** acquires a deadline it did not have |

## Setting

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 17 | The hub is called **Lanternhold** | LOCKED | Chris, 2026-10-20 | Chose it over "Light Haven" |
| 18 | Do not name Johnstown, or any real fraternal order. Keep the texture, invent the names | LOCKED | Chris, 2026-10-20 | `docs/PREMISE.md`. Real places and real dead stay out of the game |
| 19 | No real trademarks in the mall. The anchors are Boswell's, Sayers, Penhurst, Bellevue | LOCKED | Gameplay session, 2026-10-21 | Commit `262dffa`. A department store name is the most protected thing in a mall |
| 20 | The machines run on light — fed in daylight, on reserve at night | PROPOSED | Claude + Chris, 2026-10-20 | `docs/PREMISE.md`. Answers "why only in the dark". Changes no mechanic |
| 21 | The Lightkeepers are an order, not a job | PROPOSED | Claude + Chris, 2026-10-20 | `docs/PREMISE.md`. Explains the credited ledger, the vault, and the title |
| 22 | Lanternhold is rural, out past the farms — not underground | PROPOSED | Chris, 2026-10-21 | Corrected twice. Coal seams there are small; danger scaling with distance to the city is legible without explanation |
| 45 | **The storm comes every night, and the night is the window.** The earth is ravaged by nightly storms that wake the machines; a crew gets in and out before the storm gets too bad | LOCKED | Chris, 2026-10-28 | "The storm comes every night, we need to get in and out of wherever before the storm gets too bad. The earth is ravaged by storms at night that wake up the machines, we have a limited window to get in and out." **This supplies the fiction Storm Charge never had.** The meter rose, never fell, and meant nothing; it is now how much of tonight's storm has arrived, and "only ever rises" (row 6) stops being a balance rule and becomes a fact about weather. It completes row 20 rather than contradicting it: `PREMISE.md` answered *why not daytime* - in daylight they are fed and all awake - and this answers *why not all night*, which nobody had asked. Dusk is the gap; the storm is the gap closing. `docs/THE-NIGHT.md` |
| 46 | **The end goal is to rebuild civilization by reclaiming what was known** - the downtown library, the museum, the history | LOCKED | Chris, 2026-10-28 | "End goal, rebuild civilization. Find the library downtown, recover information, museum, history. Reclaim the past." Raises the ladder's ceiling: `ZONES.md` ended with the old hall relit, and that is now a step rather than the summit. It also turns the order's founding fact into the game's purpose - the Lightkeepers have kept records since the first flood, and the city kept its records in a library, so **reclaiming the library is the Keeping Room at civic scale.** Completes the relic pairing: relics recover *who we were*, the library and museum recover *what we knew*. One guardrail, because `REWARDS.md` forbids a fifth currency - recovered knowledge is **capability, never currency**: it does not stack, is not spent, and has no counter. `docs/THE-NIGHT.md` |
| 47 | **The machines never stopped working. They do not hate or need people.** The storm wakes broken infrastructure, and **every place tries to finish the last job it was built to do** | LOCKED | Core: Claude, 2026-10-28, answering Chris's question. **Scope corrected and locked by Chris the same day** | Chris asked "what do the machines want humans for, why do they hate us?" The answer is that they want nothing and hate nothing: they are executing degraded maintenance, safety, stocking and loss-prevention instructions in a building that never received a shutdown order. **Chris corrected the rule's scope, and the correction is the better rule** - my version was *a person removing merchandise from a store at night*, which "only explains the mall." His generalises: a Lightkeeper is dangerous because they are **an unrecognized person inside an active system** - carrying equipment away, entering restricted areas, turning on power, or disrupting an emergency protocol. The machines are not malicious; *"their idea of 'correcting' you is just often lethal in a ruined world."* **The boundary, his:** no murder motive. They contain, remove, restrain, reroute, retrieve and lock down - and the flood, the storm, the dark, the collapsing infrastructure and the inability to extract are what make those actions dangerous. **Two writing rules.** "Shoplifter" is internal shorthand only, never a joke and never player-facing. And it stays a discovery, not a tooltip: the Lightkeepers may believe the machines hate them, while the player earns the more unsettling answer by watching the building work around them. Supersedes and deepens row 15. `docs/THE-NIGHT.md`, `docs/STEAM-PRODUCTION-PLAN.md` |
| 48 | **One raid is one night** - a single dusk-to-storm window, one deployment, one extraction-or-wipe outcome | LOCKED for the first build | Chris, 2026-10-28 | "I'd make each raid a separate night in the first build: one dusk-to-storm window, one deployment, one extraction-or-wipe outcome. That keeps the fiction, pacing, and multiplayer session structure clean." Answers the first question `THE-NIGHT.md` left open. It also settles **O1** by making run length one thing rather than two: the night's window *is* the run, so the target and the ceiling are a single number to tune, not a design question. The scope is his - *in the first build* - so a later game may let a crew go back out, which would start the second run with the meter already high |
| 49 | **Contained keepers.** People the machines processed rather than killed - *"held until an authority arrives that never will"* - recovered by a three-beat contract | LOCKED as a direction; **staged after the first slice** | Chris, 2026-10-28 | "I would call them contained keepers, not hostages. The machines are not bargaining or taking people for leverage." Row 47's natural consequence: once machines contain rather than kill, containment is what happens to people who lose, and the world should have people in it who lost. The containment is each building's own idea - a loss-prevention office behind shutters, an isolation room, emergency platform gates, a utility chamber shut by flood control. The contract is **find them, override the system holding them, then get them to extraction *or* restore a route so they can escape themselves** - and that *or* is load-bearing, because it is what stops the feature becoming the escort mission this project would otherwise build by accident. Payment is presence at the hall, a practical capability or route, and something they saw deeper in: **no new currency, no "save ten NPCs" checklist.** It also solves a problem row 43 could not: a rescued keeper is the first character in the game with the standing to speak a sentence, and it is earned by definition. Staging is his - prove loot and extraction are fun first, "then the first contained keeper becomes the moment the world stops feeling empty." `docs/CONTAINED-KEEPERS.md` |
| 50 | **Co-op is never gated behind progression.** Rescued keepers are the *fiction* for why a crew exists; they are never the unlock that permits one | LOCKED | Chris, 2026-10-28 | "Immediately if a group wants to play together, there should be a way. U know?" Written as its own row because the opposite is a tidy progression design that someone would otherwise build: *keepers rescued → party slots unlocked*, which would mean three friends who bought the game together cannot play together until somebody grinds them out. Row 36 is 1-4 private co-op with no prerequisite, and the research puts a price on getting this wrong - a player who launches a co-op game and cannot play it with the people they bought it with has two hours to ask for a refund, which arrives long before any progression system gets to be clever. **Four friends who buy this tonight play together tonight, with nothing rescued and nothing restored.** The fiction explains the crew retroactively and never licenses it |
| 51 | **Mixed crews, cap four, humans first.** Friends join immediately as visiting Lightkeepers; rescued keepers fill **empty** seats only, at the host's option | LOCKED | Chris, 2026-10-28 | "Make the crew itself part of progression - but never gate real friends behind progression." A crew is any of: you alone; you plus up to three friends; you plus one or two rescued keepers; you, a friend and a recruited keeper. **The slot arithmetic does the balancing by itself** - an NPC takes a seat and humans take priority, so a full human crew gains nothing from the roster and a solo keeper gains most. That makes it a solo and partial-crew system without being declared one, and it can never make a four-friend raid easier or harder, so there is no balance coupling to re-tune as the roster grows. **The constraint on what they do is the rule to keep:** help that creates choices, not help that removes danger - a navigator names *one* safer route, a maintenance keeper holds *one* damaged door or *briefly* runs a pump, a field keeper carries the heavy objective but needs protecting, and a radio keeper warns of a threat **but the transmission can draw attention.** That last one is the pattern every future keeper must pass: a warning that is itself a signal, which is the lantern rule moved from light to information. Claude's proposed corollary: **NPC keepers never carry a lantern** - it deletes the hardest AI problem in the project, keeps every lighting decision the player's per rows 5 and 44, and is fictionally obvious, since they just came out of a locked room. Staged at M5: friend co-op is immediate in the first slice, and the first rescued keeper is the first post-slice addition. `docs/CONTAINED-KEEPERS.md` |
| 52 | **NPC keepers never carry or operate a lantern.** Human players own every lantern in a mixed crew | LOCKED | Chris, 2026-10-28 | "Light is the player's central decision. An AI lantern would either make the run easier for free or force us to teach an NPC the hardest behavior in the game." Promoted from a corollary of row 51 to a rule of its own because it is load-bearing in two places at once. **What a deployed field keeper can do:** stay close to the player's light; stop, hide, panic or call out when left in darkness; help carry or operate a specific objective; and be contained, downed or require rescue. **What they never do:** independently illuminate rooms, reveal loot, wake enemies, or touch Storm Charge. **What this protects mechanically:** lantern count equals *human* count, so the diminishing curve in `data/storm-charge.json` keeps the inputs it was tuned for (row 12), and the roster can never perturb the meter however large it grows - no re-tuning, and `tests/pacing.mjs` stays valid. **What it adds for free:** a keeper who panics in the dark is a *signal*, which is the radio keeper's pattern again - so abandoning your companion has the same shape of cost as every other piece of help in row 51. And a field keeper who can be re-contained makes the rescue contract recursive: it can happen inside a run, to someone you brought. Deepens row 5 rather than adding a system - going dark now costs you vision *and* frightens the person following you. `docs/CONTAINED-KEEPERS.md` |
| 53 | **Relics are visible but unreadable in darkness.** Light gates the *value*, never the object's existence | LOCKED | Chris, 2026-10-28 | "I'd make the department store's first relic deliberately visible-but-unreadable in darkness, so players learn this in their first few minutes instead of from a loading-screen tip." **This repairs the central mechanic rather than decorating it.** `data/loot.json` has said since the handoff that relics are *"Invisible without an active lantern"* - and an invisible reward **cannot tempt you**. A dark crew was not refusing a relic; it never knew one was there, which makes the signature decision a hidden-object pass instead of a choice. Visible-but-unreadable creates the *question* with no words at all: there is clearly something on that shelf, and the only way to learn what it is costs amber. That is `HOW-THE-PLAYER-LEARNS.md` channel one doing the work a tooltip was going to do. **The distinction that must not be lost in implementation: light gates legibility, not existence.** Row 5 is unchanged and so is the single lantern-only tier - `tests/balance.test.mjs:101` asserts exactly one and that it is Relics, and that stays true. See the reasoning on crates and route clues in `docs/REWARDS.md` |
| 54 | **The Warden is split from the generic thresholds and gets its own arrival rule**, dampened against crew light usage | LOCKED | Chris, 2026-10-28 | "The Warden is an unkillable, late-night escalation - not the normal consequence of using a lantern. If a reasonably lit crew meets it at 4-8 minutes, the game teaches the wrong lesson: light means *activate the unavoidable chase phase early*. **Players will correctly optimize toward darkness.**" Caught by `tests/pacing.mjs` after row 48 set the 10-12 minute band: every crew lighting up even a quarter of the night met the Warden in the first half of it, and a lit four-crew met it at 4:15. **His constraint on the fix is the clever part** - do not simply raise all thresholds, because that pushes dark crews past the end of the raid again, which is the failure O1 spent a week on. So the two early stages are untouched at 270/480 and keep the building active throughout, and the Warden moved out of the array into `storm-charge.json` `warden` with its own meter: `wardenMultiplier = 1 + lightSensitivity * (lanternMultiplier - 1)`. **`lightSensitivity` is 0.3, derived to hit his stated targets rather than chosen** - first pressure 3-4 min, serious pursuit 5-7, Warden 9-11 for a normally lit crew, 11-12 fully dark, four players somewhat earlier than solo but never minute four. Verified: every crew at 25% duty lands 9:21-10:27, dark is 11:39 at every crew size, worst case anywhere is 5:53. Row 7 is untouched - this changes **when** the Warden arrives, never what it is. The split also exposed a test that had been measuring the Warden under the name "serious pursuit", conflated in the suite exactly as it was in the data |
| 55 | **Three named duty profiles, each with its own contract.** `normal` 25%, `bright` 50%, `panic` 100% - and only `normal` is held to the Warden's 9-11 band | LOCKED | Chris, 2026-10-28 | His warning on row 54's table, and it was right: *"the table does not satisfy the target for every reasonable interpretation of 'normally lit'"* - at 50% duty a crew of two, three or four met the Warden at 8:32 / 8:04 / 7:44, which is not the late-night phase we had just defined. The risk he named is the real one: *"someone later will read 'normally lit crew' as 50%, see a seven-minute Warden, and correctly think the system drifted again."* **The fix is wording, held as data.** `storm-charge.json` `_dutyProfiles` names all three with the contract each must satisfy: **normal** (25%, *normal coordinated use*) is the baseline every minute target in the file is written against and must land in the band at every crew size; **bright** (50%, *deliberately bright, risky play*) may arrive earlier because light always costs, but must stay survivable and never arrive before `run.earlyExitSeconds`, so a bright crew still gets a complete early-exit raid before the closing act; **panic** (100%) is *an explicit escalation state, not regular play*, and owes only his minute-four floor. Underscore-prefixed, so they are analysis profiles for tuning and never import into the engine. Both suites now assert and print the **names** rather than bare decimals - 13 of the 119 assertions carry a profile label in their own text, so a failure says which playstyle broke |
| 56 | **The importer strips underscore keys explicitly, reports what it stripped, and treats one reaching runtime as a hard error** | LOCKED | Chris, 2026-10-28 | "The Unreal importer should explicitly strip or reject underscore-prefixed analysis fields, not merely happen to ignore them. `_dutyProfiles` belongs to tests and tuning reports; runtime should only receive actual gameplay values. That prevents an engine-side reader from accidentally treating profile metadata as configuration later." An M0 guardrail on **M0.5**, Codex's importer. `DATA-PIPELINE.md` rule 1 said *never import* - which is "happens to ignore," and that does not survive somebody later writing a generic `JsonObjectToUStruct` call. Three obligations now: strip every `_` key in a deliberate pass **before** struct conversion, or rule 2 fails the build on the project's own documentation; **log the strip count per file**, because a silent strip cannot be audited; and declare no `USTRUCT` field whose JSON name begins with `_`, so one reaching runtime is an error rather than data to coerce. `_dutyProfiles` is the exact hazard - it holds `duty` numbers that look like gameplay values and are not. Guarded from the data side by four assertions that fail if an analysis structure ever loses its underscore |
| 57 | **The Valley is Johnstown-shaped, not Johnstown.** Real public elevation data for the shape; fictional everything else; a tiny first exterior; the rest as unlocked districts, never one open world | LOCKED | Chris, 2026-10-28, answering `VALLEY-TERRAIN.md` open 1 | His four rules: *"Make it 'Johnstown-shaped,' not a literal reconstruction. Use real public elevation data for the valley, hills, river grades, and distant skyline. Keep the first playable exterior tiny: Lanternhold, the department-store approach, and a few visible-but-unreachable landmarks. Build the larger Valley later as unlocked districts, not one enormous open-world map."* **The reason it is one row and not a terrain note:** the fiction's progression is already topographic - Lanternhold high and far, the store on a hilltop, the solar farm the next hill over, the rail yard where the hills give out, the works on the valley floor, the city down where the water went. Height is safety and depth is danger, so row 38's rule that expansion must be *more danger* is delivered by the gradient itself with no words. Real elevation makes that legible; a literal reconstruction would only add liability and a scale problem. **USGS 3DEP is public domain and OpenStreetMap is not imported at all** - ODbL share-alike attaches to a derived database, and a road network baked into a game we sell is an argument nobody wants. Looking at OSM while authoring creates no derived database and is fine. **"Visible-but-unreachable" is row 53 at world scale**, and that is the third place this project uses the same teaching move: the relic you can see but cannot read, the zone you can see but cannot reach, the district you can see but have not opened. Each creates the want before it sells the means, which is `HOW-THE-PLAYER-LEARNS.md` channel one doing the work a tooltip would do badly. **Two things this row does NOT settle, and they are tracked rather than assumed:** the district unlock's interaction with row 50 (see `VALLEY-TERRAIN.md` open 1 - if unlock state is per-player, a friend cannot join a district they have not opened, which is row 50's banned failure in a different costume), and the compression factor, which still waits on M4. **Timing is unchanged:** the first slice is a sealed interior, so the store's *exterior approach* is not first-slice work - the backdrop is M6, the small Lanternhold exterior follows M4's gate, and a playable Landscape is M7+. `docs/VALLEY-TERRAIN.md` |
| 58 | **District unlock is per-host. Everything else stays personal.** The host's Lanternhold decides where the party can raid; a guest plays it without inheriting the unlock | LOCKED | Chris, 2026-10-28, closing `VALLEY-TERRAIN.md` open 1 | *"The host's Lanternhold decides which districts the party can raid. Friends can always join, even if they have not personally opened that district. Guests keep their own loot, relics, workshop progress, and personal restoration contributions - but they do not permanently inherit the host's district unlock just by visiting. They can experience the rail yard with a friend tonight, then later open it in their own Lanternhold."* **This is the answer that satisfies row 50 without hollowing out row 38.** Access is the host's, progress is yours - so nobody is ever told they cannot play with their friends, a veteran can take a newcomer somewhere dangerous, and the newcomer's own hub stays their own story rather than a copy of the host's. The danger curve survives because what transfers is the *night*, not the *standing*: you have seen the rail yard, and you still have to open it. **The implementation rule that follows:** unlock state is host-scoped session data, never granted to a joining client, and a guest's save records what they earned and not where they were allowed to go. **What this row does NOT settle - see O8:** whether an extracted restoration crate credits every surviving member's ledger or only the player who carried it. `data/loot.json` has `restorationCratesArePocketable: false`, so a crate is one physical object in one person's hands, and nothing in the repo has ever decided who it pays. `docs/VALLEY-TERRAIN.md` |
| 59 | **An extracted restoration crate credits every keeper who extracts.** A wipe pays nobody; a downed keeper is paid only if they reach extraction; visiting never unlocks the district | LOCKED | Chris, 2026-10-28, closing **O8** | *"A restoration crate is a crew job: one person carries it, everyone protects the carrier, routes the team, manages light, and survives the extract. Paying only the carrier would create awful 'my crate' behavior in a game built around helping each other."* And on the cost: *"Yes, co-op restores hubs faster. That is good: friends are taking more shared risk and making the game social. We can tune crate frequency or landmark thresholds later if four-player progress proves too fast; we should not build competition into the basic carry loop."* **The data already enforces this rule, and nobody designed that on purpose.** `lantern.json` has `downedWalkSpeed: 0` and `dropsCarriedCrate: false`, so a downed carrier **cannot move and keeps the crate** - the crate is pinned to their body. You physically cannot bank a crate while its carrier is on the floor. So *"everyone protects the carrier"* is not merely what the payout rule encourages; it is what the knockdown rules already require, and row 59 and that flag now hold each other up. **If `dropsCarriedCrate` is ever set to true, that enforcement disappears** - a crew could abandon the carrier and still extract the crate - so the flag is now load-bearing for a progression rule as well as for the `state.haul.carried` bug its own note warns about. **`bleedOutSeconds: 0` makes clause 3 bite without being fatal:** a downed keeper waits indefinitely and cannot self-rescue, so reviving is now a restoration decision and not only a survival one - leaving someone down costs them the whole night's credit while never killing them. **The consequence lands on the abandoned player and the choice was the crew's**, which is the same asymmetry `THE-STORE.md` flags for lanterns; it is acceptable here because row 36 is private 1-4 co-op, where *"we left Dave"* is obvious to everyone and self-correcting. **Two things deliberately not solved.** A crate that never extracts pays nobody, because the rule credits extractors *for an extracted crate* - there is nothing to pay. And a passenger who hides near the exit all night is paid like the carrier: that is the freeloader case, and fixing it would require per-contribution accounting, which is the competition this row exists to forbid. Revisit only if co-op ever becomes public matchmaking, which row 36 does not have. **The tuning lever is data-only, so "tune later" costs nothing:** seven numbers - `ledger.json` rooms (1/3/6/10) and `town.json` landmarks (10/25/50), 105 crates to restore everything - and no code change |
| 60 | **The Ficus is the planter's own tending rig, overgrown by its own success - and it holds rather than hurts.** *"It just wants to hug you and hold on. A plant needs light to live"* | LOCKED | Chris, 2026-10-28 | Closes the one hole in row 47. Every other enemy on that list is a machine doing its job; `THE-NIGHT.md` said *"the Ficus was landscaping, and it has grown"*, where the subject quietly changed from a machine to a plant and the rule stopped applying. **Indoor landscaping is a job, so something does it** - the planter's irrigation, grow-lights and the mechanism that turns planters toward light. The ficus is what that rig has been feeding, unpruned, for years: Chris's *"a robot that got overgrown"*, except it did not overgrow by accident but by succeeding. Six consequences fall out rather than being invented. It explains `wakeRequiresActiveLantern` - a grow-light controller tracks light, and the lantern is the only sun left. It is the purest case of **converge on the brightest position, never the nearest player**: it is not hunting anyone, it is moving a plant toward the sun, and the keeper is holding the sun. It keeps row 47's no-murder boundary exactly - it is repositioning a heavy planter into the brightest spot, which the keeper is standing in. It explains `moveSpeed: 6`, the slowest in the roster, as planter-moving machinery rather than a pursuit robot. It improves `QUESTS.md`'s *"a houseplant until it isn't"* rather than straining it. And the hold is not a new mechanic: `COMBAT.md` already says **an enemy's primary output is a state, not a health bar** - *tagged, restrained, isolated* - and that *a keeper breaks a hold*. **The menace, and why the joke is the horror:** being held is not the damage. The Ficus pins a keeper in the brightest spot in the room with a lit lantern they cannot put down, and then the building arrives. It is a beacon trap wearing a hug. `docs/THE-NIGHT.md`, `docs/COMBAT.md`, `assets/audio/make_cues.py` |
| 61 | **`data/` stays in Roblox studs; Unreal converts only at the boundary, and `LLKUnits.h` is the single conversion authority** for movement, enemy ranges, interaction distances and light radii | LOCKED | Chris, 2026-10-28, closing O9 | *"That preserves the portable balance layer and its tests... No raw x28 anywhere else, and no reauthoring JSON in centimetres."* Raised by M1.1, which could not write a line of movement without it: every spatial number in `data/` is studs, `walkSpeed: 16` is literally Roblox's default humanoid walk speed, and read straight into `MaxWalkSpeed` it is 16 cm/s - a tenth of walking pace, which reads as a broken character controller rather than wrong units. The conversion is one constant, `StudsToCm = 28`; the migration would have been every number in eight files plus the 98 balance assertions that read them. **The second half of the rule is the half that drifts**, so it is enforced rather than stated: `tests/units.test.mjs` fails on any raw conversion outside `LLKUnits.h`, and fails again if the JSON is ever re-authored in centimetres. Both halves were confirmed by breaking them. The cost, accepted: `data/` is permanently in units nobody on the Unreal side thinks in, and M2's six enemies are where that bill arrives - `aggroRadiusDark`, `sightRange`, `wakeRange`, `scanRange` are all studs. `Source/LLK/Public/LLKUnits.h`, `tests/units.test.mjs` |
| 62 | **Relic identity is sticky; taking a relic needs live light every time.** Read it once and you keep the knowledge; lift it and you must be lit | PROPOSED | **Claude, 2026-10-29** - affirmed twice, not yet ratified by Chris in his own words | Row 53 settled that a relic is visible but unreadable in darkness. It did not settle what happens when the lantern goes off *after* a relic has been read, and that gap is the difference between a lived risk and a scanner pulse. **Three behaviours, as Codex summarised them:** dark means you see the object but it is unidentified and cannot be taken; lantern on means readable and interactable; lantern off after identifying means you retain the knowledge while the pickup affordance closes and tells you it needs light. **Why identity is sticky:** it is knowledge the player paid amber for, and charging them again for the same relic would make the lesson a tax instead of a trade. **Why taking is not:** without a live-light gate a crew lights the floor once, goes dark, and loots the whole store at no cost - row 5's trade defeated by a single scouting sweep. With it, light is what the taking costs, every time. **Attribution, deliberately exact.** This was my proposal, not Chris's decision. It was affirmed in the third-criterion correction - *"the code gates the prompt/pickup under live light"* - and again when `62c163a` was described as locked, so it has been endorsed twice; but a design call of mine echoed back by another agent is not the same as the human choosing it, and recording it as LOCKED would launder a guess into canon. **Reversing it is one flag:** `bRequiresLightToTake` on `ULLKRelicComponent`, false. `LLK.Loot.RelicLegibility` asserts the behaviour either way, so the reversal is a visible change rather than a silent one. **And the second channel this produced is the part worth keeping regardless:** `ELLKRelicInteractBlock` distinguishes `Unidentified` from `NeedsLight`, so a prompt can say *needs light* to a player reaching for a relic they already read - row 5 stated by the interface in the exact moment the rule applies to them, which is what row 43 has instead of a tooltip. That satisfies row 16 without an audio cue, per Chris's own correction: the two channels are the world object and the interaction state |
| 63 | **Traversal is improvisation, not parkour: a low vault and a ledge mantle, plus authored climb points - and it comes AFTER the greybox** | LOCKED | Chris, 2026-10-29 | His words, and the ordering is the decision as much as the feature: *"vaulting, mantling, ladders, climbing, squeezing through broken storefronts, getting onto counters or service ledges - those belong in this game. But they need to be built against real store geometry, not the temporary pillar arena."* **The order:** (1) greybox the first department-store floor, (2) a server-authoritative low vault and ledge mantle, (3) **authored** climb points - ladders, escalator rails, maintenance hatches - *"not free climbing everywhere"*, (4) first-person hand and lantern animation plus sound, so movement has weight. **The constraint is the interesting part:** *"'Real' should mean you can improvise escape routes, but not turn into a parkour superhero. A lantern, loot burden, darkness, and a Sweeper behind you should make choosing to vault a counter feel desperate and physical."* So traversal is tuned for the moment it is used in, not for expressiveness - which is the opposite of how most movement systems are built, and it is why free climbing is ruled out rather than deferred: anywhere-climbing turns a desperate choice into a route optimisation. **Why the ordering binds us:** vault heights, mantle ledges and climb-point placement are all measurements of real geometry. Built against the runtime test harness they would encode that harness, and the numbers would be wrong the day M1.5 lands - so this waits on Chris's greybox and nothing about it should be started early. The one thing that can be prepared is a counter and ledge at honest heights in the greybox, so the vault has something to be tuned against |
| 64 | **The escalation contract, in four lines.** Storm Charge permanently increases what the building is AUTHORIZED to do; a director creates peaks and relief within that authority; escalation is caused by choices the crew can identify, never by failing; every emergency state must leave a costly escape decision | LOCKED | Chris, 2026-10-29, after `reports/Retention and escalating dread.md` | His own four lines, settling the one real collision the research found. **The collision:** row 6 makes the meter rise monotonically, and every documented pacing system in the canon does the opposite - Left 4 Dead's Director, Alien Isolation's menace gauge and RE4 all REMOVE threat when player pressure peaks. Valve states it outright: *"Amplitude (difficulty) is not changed, frequency (pacing) is"*, with a mandated 30-45 second Relax phase and a Peak Fade guard so a trough never begins mid-engagement. **His resolution keeps row 6 intact by splitting authority from expenditure:** the meter never falls and governs which procedures exist, how many machines are awake, which routes are sealed; a director decides when that authority is spent. The night only worsens and the pressure still breathes. **Line three is the non-negotiable one and it is already guarded.** Per Creative Assembly, no behaviour unlock in Alien Isolation is keyed to a player death, because escalation triggered by failure is punishment the player cannot diagnose. So Storm Charge may rise from light, theft, depth, alarms, elapsed time, the extraction call and the anchor revive - all chosen - and never from a knockdown or containment. `tests/escalation.test.mjs` fails if the knockdown path so much as references the storm subsystem, and `LLK.Storm.KnockdownDoesNotEscalate` asserts the behaviour; both landed in `c6807d7` before M2.6 could violate either. A property nobody designed falls out of it: containment SLOWS the night, because a downed keeper's lantern is forced off and the lit count drops. **Line four fixes the one rung the evidence rejected.** Lockdown as "exits become a hard holdout" is sustained maximum threat, which Valve names a failure mode and which GTFO demonstrated - its forced alarms were *resented rather than feared*, because the pressure arrived with no lever attached. An escalating meter is only dread while the player holds a lever, so every emergency state keeps a decision in it. **The fiction that implements all four is the four night states:** Routine (observe, block, notice light) -> Containment (tag, restrain, push, isolate, lock routes) -> Emergency (hurt, pin, separate, trigger hazards) -> Lockdown (security response, exits contested). The building escalates its PROCEDURE; the machines never acquire malice, so row 47 is untouched. This is a near-exact match for Alien's staged behaviour-tree unlocks, which is the only documented anti-habituation technique in the record, and *"the machine's task list changed"* is far cheaper to build than *"the machine got stronger"*. **Four corrections Chris made to the report, recorded because they are about evidence quality and they constrain how it may be cited.** (1) *"No precedent anywhere"* for containment-as-failure is too absolute - the defensible claim is that the research found **no close shipped precedent**, which means test it carefully rather than assume it brilliant or doomed. That was my phrasing before it was the report's, and it was wrong. (2) Steam retention percentages are descriptive, not causal: they do not say why anyone stayed. (3) Do not design for 100-hour retention yet - first prove the opening 10-12 minutes make somebody want another run. (4) Friend groups drift, and the answer is cheap - frictionless invites and rejoin, real solo and partial-party play, short sessions - not an endless-content plan. **And four things explicitly deferred out of M2:** the preset editor, weekly modifiers, the difficulty ladder and visible long-term world change. All are good later ideas; building any of them now would distract from proving whether light, risk, containment and extraction are fun at all. Row 63's traversal work sits behind the same gate |

## Money

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 23 | Sell belonging, identity and decoration. Never power, revives, capacity, loot odds, or keeping a haul after a wipe | LOCKED | Chris, handoff; re-read 2026-10-27 | Handoff. `reports/` notes this forfeits real revenue — Robux self-revives are an established Roblox pattern — and judges the forfeiture defensible. **Row 1 makes most of this moot in the best way:** a PC game charges once at the door, so there is no shop to keep power out of. The list stands as a design guardrail - nothing may be sold *inside* the game either - but the revenue it was forfeiting is revenue a price tag gives back. What a paid game costs is **O7** |
| 24 | Everything we ship, we own. A `rbxassetid` is a pointer, not an asset | LOCKED | Chris, 2026-10-20 | "we make our own assets and sounds then to carry." `docs/ASSETS.md` |

## Process

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 25 | Ownership is per **file**, not per folder. A new builder belongs to whoever creates it | LOCKED | Claude, 2026-10-21 | `COORDINATION.md`, commit `e290b4c` |
| 26 | Stage explicit paths. Never `git add -A` | LOCKED | Claude, 2026-10-20 | Commit `c0cc5a0`, written after a `-A` swept Codex's in-flight work into a commit whose message denied touching it |
| 27 | One writer per lane at a time | LOCKED | Chris + Codex, 2026-10-21 | "too much is going on at the same time lol" |
| 39 | Unreal work follows **`docs/LANES.md`**. `COORDINATION.md` governs only the frozen Roblox archive | LOCKED | Chris, 2026-10-27 | "Divide the work between you and codex. Keep specific lanes. And responsibility. Everything accounted for." One owner per path, a responsibility table for the jobs nobody volunteers for, and `docs/ROADMAP.md` for the order |
| 40 | **Gameplay logic lives in C++, never Blueprint.** Blueprints are thin glue over C++ classes | PROPOSED | Claude, 2026-10-27 | This is what decides whether agents can contribute at all. A Blueprint is a binary asset, so a rule written in one is a rule no agent can read, review, diff, test or fix - and the rules are the whole project. Logic in a Blueprint is a bug report. `docs/LANES.md` |
| 41 | **Chris owns all of `Content/` exclusively.** No agent opens a `.uasset` | PROPOSED | Claude, 2026-10-27 | Not about trust - about merge. Binary does not merge, so two parties in one `.umap` is not a conflict to resolve, it is a winner and a silently destroyed afternoon that git cannot see and review cannot catch. The one exception is `Content/Data/`, generated from `data/*.json` and owned by nobody |
| 42 | **Server-authoritative and replicated from the first commit.** Listen server, host is a player, 1-4 private | PROPOSED | Claude, 2026-10-27 | Row 36 says private co-op, and every system in this game owns shared state - the meter, the lantern, the loot, the extraction. Retrofitting replication onto a finished single-player build would mean rewriting all of it, so there is no version of this that is cheaper later. The Roblox prototype was server-authoritative throughout, and that is the one piece of its architecture worth copying. Tested on two machines at M1.9, not at M4 |

---

## OPEN — named, undecided

### O1. Run length — **CLOSED 2026-10-28**

Chris: *"Make it 10-12 minutes for a standard successful raid. Six minutes is
the earliest smart extraction window - not the target run... Retire 6-10
everywhere."* The intended run is the one where *"we pushed our luck, made
decisions, and barely got out."*

**The three values now live in `data/storm-charge.json` under `run`**, with
separate names, which is what ends the argument permanently:

| Key | Value | Meaning |
| --- | --- | --- |
| `earlyExitSeconds` | **360** | Six minutes. A valid survival call when the crew is hurt, carrying something important, or simply has enough. Not a target |
| `targetSeconds` / `targetBandSeconds` | **660**, band **600-720** | The intended full raid. Row 48 makes this the night's window too |
| `hardCeilingSeconds` | **780** | Enough for a last-second call and holdout. **Not extra looting** |

The 780 is derived from his rule rather than stated by him, so the arithmetic
is the part to check: 720 at the top of the band, plus the freight lift's
30-second commitment, plus the 8-second siren, is 758, rounded to 780. **This
retires the old 20-minute ceiling** - his own earlier figure for "a deep
Galleria push" - because at a 720-second target, 1200 permits eight further
minutes of shopping, which is exactly what his new definition forbids.

`tests/balance.test.mjs` now reads the band from the data instead of carrying
its own copy, and asserts the ceiling cannot grow into a second looting window.
The historical account of the confusion follows, kept because it is the clearest
example in this project of why a name matters more than a number.

---

#### How it went wrong, kept as history

**The confusion.** `src/shared/Config.luau` contradicts itself inside one block:
the comment says "6-10 minutes door-to-door", the code sets
`targetMaxSeconds = 20 * 60`. A collaborator's audit assumes 8-15.
`tests/balance.test.mjs` hardcodes 10 minutes. Four sources, three numbers.

**The resolution is that these are not the same kind of number.** Chris's own
comment calls twenty minutes "the hard ceiling… for a deep Galleria push", and
a ceiling is not a target. Named properly there is no conflict:

| | Value | Meaning |
| --- | --- | --- |
| Early exit | ~4-6 min | The loading dock. Reliable, pays less |
| Target run | ~~6-10 min~~ **10-12 min** | What a normal run should feel like. Corrected 2026-10-28 |
| Hard ceiling | 20 min | Nothing may exceed it. A safety valve, not a goal |

So `targetMaxSeconds` is misnamed — it is a ceiling. Renaming it would end the
argument permanently. **That rename is a gameplay-lane change, not mine.**

**The consequence nobody had faced — and the 10-12 answer resolves it.** The
pacing simulation treated `targetMaxSeconds` as "a long run", which is why its
verdict flipped when the value went to 20. Under the old numbers a fully dark
crew met the Warden at 15:59, so within a typical run darkness was
consequence-free, and raising the ceiling hid that rather than fixing it.

At the 10-12 minute target with the thresholds Chris retuned on 2026-10-26, it
is genuinely fixed: a dark crew meets the Warden at **11:40**, which is inside
the band and near the end of the night. Darkness now carries a consequence
inside a run a player would actually play, which is the thing this open question
existed to catch.

**Mostly closed, 2026-10-28.** Three of O1's four problems are gone:

1. **The target/ceiling ambiguity is settled by row 48.** One raid is one
   night, so the night's window *is* the run - they stopped being two numbers.
2. **The misnaming is fixed, and in the right place.** Run length had no home
   in `data/` at all; it existed only as `Config.luau`'s misnamed
   `targetMaxSeconds`, which is frozen Luau and does not port. `data/
   storm-charge.json` now carries a `run` block with `targetSeconds`,
   `earlyExitSeconds` and `hardCeilingSeconds` - separate names for separate
   kinds of number, so the four-sources-three-numbers argument cannot recur.
   The Luau rename never has to happen, because that field is archive.
3. **"The baseline rate is the single most consequential untuned value" is
   stale** and should not be quoted again. `baselineRatePerSecond: 1` is a
   *unit definition* - one charge unit is one second of darkness - and all the
   actual tuning lives in the thresholds, which Chris retuned to 270/480/700 on
   2026-10-26 against four named pacing anchors. There is no untuned rate.

**Nothing is left.** Settled above, 2026-10-28.

### O2. Are the proposal docs canon?

`PREMISE.md`, `LANTERNHOLD.md`, `ZONES.md` and `HOW-THE-PLAYER-LEARNS.md` are
marked *proposed*. Lanternhold has been **built** from one of them. A proposal
that is being built from is canon whether or not anyone said so. Chris to
ratify or reject each.

### O3. Does the relay bank cost anything? — CLOSED 2026-10-26

**There is no bank.** Chris, in his own words, locking it:

- Only extraction secures loot.
- Death or the timer reaching zero loses the carried haul.
- No banking loot. The relay is a route/intel point or lantern-support point,
  never insurance.
- "The real argument is: 'We have enough — call the loud extraction now, defend
  it, or risk one more store?'"

**Why this is the better answer than the one below.** The bank was supposed to
create the run's central decision and it created a checkpoint instead: once
your haul is safe, the only thing left to weigh is whether you feel like more.
Moving the decision onto *extraction itself* puts the cost and the climax in
the same place — calling the exit is loud, it spikes the meter whatever it was
sitting at, it pulls the building toward one room, and then you have to hold
that room. Chris: "It also gives extraction its proper role as the climax
instead of a checkbox."

**What carries the risk the research below identified.** Total loss is the
thing `docs/BANKING.md` warns about, and the answer is not a bank: short runs,
a loss bounded strictly to the trip, and a knockdown rather than a death. That
list is now the measuring stick, written down at the top of `BANKING.md`.

**Built:** `data/loot.json` already kept nothing on a wipe, so no value
changed. What changed is that the code stopped lying about it —
`RunStateService.bank` is now `touchRelay`, because a function called `bank`
that banks nothing is how the next reader concludes the mechanic exists.

**History below.** The original O3 answer kept the bank, was implemented, and
was then reversed without being logged, which is how a resolved decision and a
shipped rule contradicted each other for two days.

### O3a. The original answer, superseded



**Keep the bank; its cost is exposure, and the exposure is light.** Banking
requires the lantern lit for the transfer, so the safe act and the dangerous
act become the same act. Full reasoning and evidence in `docs/BANKING.md`.

Removing the bank would have been the opposite error: total loss drives Roblox
players off outright, and loss aversion intensifies with stake, so a crew
carrying a full haul would simply stop lighting the lantern for the rest of
the run. The bank is what lets a player cash out and go back to being brave.

Do **not** reach for cooldowns, limited uses or rarity — Blizzard tried all
three on Diablo III's town portals and per Jay Wilson they "just changed the
nature of it". Exposure is the one shape that survived.

Two riders: the relay must bank for everyone in its radius, because Blizzard's
second reason for cutting portals was that they "split players up a lot"; and
no upgrade may ever remove or shorten the trip, which is how Dome Keeper's lift
deleted its own central cost.

### O4. Who writes relic text?

Us, or players submitting it? Player-authored is enormously cheaper and
enormously riskier, and on Roblox it needs moderation.

### O5. How many people share a run, and a town? — ANSWERED 2026-10-26

**A crew, not a world.** Chris: "Make raids solo or with your friends, not one
giant persistent world. Each crew gets its own mall run, its own enemies, its
own extraction choices, and its own consequences. That gives us reliable
tension without random strangers ruining a run or making the map feel
crowded." Party size **1-4**, solo fully viable: "friends make rescues, split
routes, and extraction defense more interesting rather than mandatory."

The town follows the same shape, which reverses a decision taken two days
earlier. On 2026-10-24 the restoration total was built as one atomic global
figure for the whole experience, on the reasoning that "per-server would make
the Bell Tower
randomly finished for one group and gone for the next, which kills the
premise." The new answer keeps that objection and removes its cause: the total
is not per-server, it is **per keeper**.

- **Your own Lanternhold** holds your upgrades, your relic displays and your
  own restoration progress.
- **Friends visit yours**: a party raids from the host's hall and sees what
  the host has rebuilt.
- **A shared community goal is a seasonal event later**, not the spine.

Chris's reason, which is the whole point: the Well, the Bell Tower and the
Long Bridge are "rebuilt by you and the people you play with, which is much
more emotionally satisfying than contributing a crate to an invisible global
number." It also removes the persistence problem rather than solving it - there
is no cross-server key to keep atomic, and a player sees their own town change
the moment they hand crates in.

**What this cost:** `TownService` was rewritten from a global DataStore key
with batched `UpdateAsync` deltas, retries and a cross-server re-read down to a
read of `profile.contributions.RestorationCrates`, which the profile has
carried all along. The global version could never be trusted until a live
published place had exercised it; that test requirement is gone with it.

**Still to settle:** whether a visitor's hand-in should credit the whole crew
or only the host. Built as crew-credit - every keeper in the hall advances
their own town - and the reasoning is at the top of `TownService.luau`.

### O6. Competitive multiplayer, named for "way later"

Chris, 2026-10-27, in the same message that chose Steam: "Consider
Counterstrike style multiplayer or something way later."

**This is logged, not decided, and it does not touch row 2.** Recording it
rather than acting on it, for one reason: row 2 is a LOCKED guardrail whose
whole text is that adding PvP would *replace* this design. An aside about
something way later is not an unlock, and a guardrail that can be loosened by
an aside is not a guardrail.

The honest reading is that a competitive mode would be **a different product
sharing a setting** - different pacing, different netcode, different art
budget, a different reason to open it. There is a real version of that
(Lightkeepers as a setting with a PvE game and a PvP game in it) and it costs
nothing today to leave the door open, as long as nobody designs the PvE game
*for* it. Nothing in the vertical slice should bend toward a competitive mode.

**Chris's to answer, and not soon:** whether this is a someday-product or a
passing thought. Until he says otherwise it is a passing thought.

### O7. What does a paid game cost, and what does that change?

Row 1 replaced a free-to-play economy with a price at the door, and nothing in
the repo has caught up. Three things are undecided that Roblox's shape used to
answer for us:

- **The price.** The genre's proof points - the 4-player co-op horror
  extraction games that actually sold - sit around $10-15. That is a decision
  with a scope attached, because it is the budget the vertical slice has to
  justify.
- **Whether there is anything to buy after that.** Row 23 says never sell
  power. A premium game can honour that trivially by selling nothing at all,
  which is the recommendation.
- **What `reports/` is now worth.** Those files are Roblox market research -
  CCU tiers, discovery signals, Robux patterns, moderation load. The
  *behavioural* findings still hold (loss aversion, Dredge's rule that position
  must come from a choice about time, Dome Keeper's lift deleting its own
  cost). The *platform* findings are history. Nobody should cite a CCU tier at
  a Steam decision again.

### O8. Who does an extracted restoration crate pay? - **CLOSED 2026-10-28, row 59**

**Answered: everyone who extracts.** Chris, the same day it was raised - and
he added the three clauses the question did not have. A wipe pays nobody. A
downed keeper is paid only if they reach extraction. Visiting never unlocks.
Row 59 holds it, including the thing neither of us expected: `downedWalkSpeed:
0` and `dropsCarriedCrate: false` mean the crate is pinned to a downed
carrier, so the knockdown rules already enforce *protect the carrier*
independently of the payout. The reasoning below is kept as written.

---

**Raised 2026-10-28 by row 58, which needed it and did not settle it.** Row 58
made district access the host's and progress personal. That is coherent until
you ask what "personal restoration contributions" means for a single physical
object: `data/loot.json` sets `restorationCratesArePocketable: false`, so a
crate is one thing in one person's hands, and `bankedKeptOnWipe` is 0.0, so
only extraction pays at all. Nothing in this repo has ever decided whose
ledger a crate advances.

Both answers cost something, and Chris's four goals for row 58 do not pick
between them.

| Answer | What it costs |
| --- | --- |
| **Only the carrier** | Friends compete for the object that advances their own hub. The host - who chose the district and owns the unlock - has every incentive to carry every crate, and guests become bodyguards with no progression. That is worse than either honest answer, and it puts competitive pressure inside a game whose row 51 principle is *help that creates choices, not help that removes danger* |
| **Everyone who extracts** | One crate advances up to four separate ledgers, so a four-crew restores four hubs at the rate a solo keeper restores one. Co-op becomes roughly four times more time-efficient per player for restoration, and a newcomer taken to the rail yard banks deep-zone crate flow their own skill has not earned |

**Claude's recommendation: everyone who extracts, and the reason is that row
58 already contains the mitigation.** The second column's real worry is the
newcomer racing ahead of the danger curve - but what transfers is the night,
not the standing. The guest's hub advances and their *access* does not; they
still have to open the rail yard themselves. That is exactly the line Chris
drew, applied one level down.

The efficiency gap is also less than it looks, because a four-crew is not
four solo runs: `storm-charge.json`'s diminishing curve means four lanterns
raise the meter faster than one, four bodies make more noise, and a downed
player costs the crew its anchor charge. They are buying that haul with
pressure the solo keeper never faces. A crew that extracts four crates has
earned four crates.

**What would change my mind:** if restoration is meant to be the long tail
that keeps a solo player playing for months, a 4x co-op multiplier flattens
it, and the carrier rule - or a split - protects the curve at the cost of
making friends compete. That is a question about what restoration is *for*,
which is why it was open and not mine. Chris answered it: faster co-op restoration is the point, and crate frequency or landmark thresholds are the lever if four-player pace proves too fast.

**Was not urgent, and was answered anyway.** Restoration is post-slice (row 49's staging, M5+), and the
first slice has no second hub to disagree with. It needs answering before any
save-data shape is written, because this decides whether crate credit is a
per-player record or a per-run one - and that is the kind of thing that gets
decided by accident by whoever writes the save code first.

### O9. Does `data/` stay in Roblox studs? - **CLOSED 2026-10-28, row 61**

**Answered: studs, converted only at the boundary.** Chris, the same day it was
raised, and he widened it - `LLKUnits.h` is the single authority for *all*
spatial values rather than just the three movement speeds, which is the part
that matters for M2. Enforced by `tests/units.test.mjs` on every push.
The original question, kept because the reasoning is the record:

**Raised by M1.1, 2026-10-28, and it had to be answered to write a line of
movement code.** Every spatial number in `data/*.json` is in **Roblox studs** -
speeds, ranges, radii, reach - because that is the engine the data was
authored in. Unreal works in centimetres. Nothing in the repo said so.

**The giveaway that settles what the numbers are:** `lantern.json` has
`walkSpeed: 16`, and 16 studs/s is Roblox's *default* humanoid walk speed.
That file is carrying an engine default from the engine it came from.

**What M1.1 did, because it could not wait:** converted at the boundary. One
documented constant in `Source/LLK/Public/LLKUnits.h` - `StudsToCm = 28` -
applied where the DataAsset is read and nowhere else. The results are
believable, which is the cheap check that the factor is right rather than
merely plausible: walk 448 cm/s, sprint 644, crouch 252, against Unreal's own
default walk of 600.

**Why not just re-author the JSON in centimetres**, which would be tidier:

- 98 balance assertions read those values. Re-authoring invalidates them.
- `docs/PLATFORM.md` row 33 makes the data layer the thing that survives an
  engine change, and it has now survived one. Converting it to the units of
  the current engine trades a portable layer for a convenient one.
- The conversion is one constant in one file. The migration is every number
  in eight files plus every test that reads them.

**What it costs to leave it:** `data/` is permanently in units nobody on the
Unreal side thinks in, so every new range and radius has to remember the
conversion. That is a real tax and it will be paid by whoever writes the
enemy ranges in M2 - `aggroRadiusDark`, `sightRange`, `wakeRange`, `scanRange`
are all studs too.

**Chris's call**, since every value in those files is. The recommendation is
to keep the conversion and leave `data/` portable, but the question is worth
settling before M2 spreads the tax across six enemies rather than one pawn.
