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
| 30 | Keepers **can** fight back, earned at ladder rung 2 and bounded by one rule: you win the fights you start. Nothing carried kills an enemy that has already seen you | PROPOSED | Chris + Claude, 2026-10-26 | "We might need to fight them." `docs/COMBAT.md`. Settles a contradiction the handoff shipped: five enemies have health, the code writes it, nothing could ever reduce it |
| 31 | The lantern and a tool compete for the same hand. Never both | PROPOSED | Claude, 2026-10-26 | `docs/COMBAT.md`. Makes combat cost light instead of costing a new resource, and gives a crew two jobs |
| 32 | The hall's four rooms cost **1 / 3 / 6 / 10** crates - a first-session door, then a widening gap | LOCKED | Chris, 2026-10-27 | "Keep the 1/3/6/10 room costs for now." Claude's proposal of 2026-10-26, read and kept rather than ratified by default. "For now" is his: a playtest may move them, and only he moves them. `data/ledger.json` |

## Content

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 13 | Six enemy types: Sweepers, Stockers, Glimmers, Window Walkers, Ficus, Warden | LOCKED | Chris, 2026-10-20 | Was four. Window Walker approved ("Keep the window walker, I want it"), Ficus approved ("Codex did it, I like the ficus"). `data/enemies.json` |
| 14 | Glimmers: 2-second scan, 20-second tag, all robots learn the position. No Storm Charge tax, no permanent Warden waypoint | LOCKED | Chris, 2026-10-19 | Handoff, "Resolved design decisions" |
| 15 | Enemies are derived from the job the place did. Staples everywhere, locals per zone | PROPOSED | Claude, 2026-10-21 | `docs/ZONES.md`. Already true of the roster; makes future enemies derived rather than invented |
| 16 | Every enemy telegraphs with sound before sight, distinguishably | LOCKED | Chris, handoff | An accessibility requirement, not polish |
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

### O1. Run length: target versus ceiling

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
| Target run | 6-10 min | What a normal run should feel like |
| Hard ceiling | 20 min | Nothing may exceed it. A safety valve, not a goal |

So `targetMaxSeconds` is misnamed — it is a ceiling. Renaming it would end the
argument permanently. **That rename is a gameplay-lane change, not mine.**

**The consequence nobody has faced.** My pacing simulation treats
`targetMaxSeconds` as "a long run", which is why its verdict flipped when the
value went to 20. At a 6-10 minute *target*, the original problem returns: a
fully dark crew meets its first threshold at 8:00 and the Warden at 15:59, so
within a typical run **darkness is still consequence-free**. Raising the
ceiling did not fix that; it hid it.

**Still open, and it is Chris's:** the baseline Storm Charge rate. That is the
single most consequential untuned value in the project.

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
