# Platform — Last Lightkeepers is a Steam game

Decided by Chris, 2026-10-27. `docs/DECISIONS.md` row 1, LOCKED.

> "Choose Steam. PC-first. No parallel Roblox version."

This reverses the handoff's plan — "Roblox first; a desktop/Steam edition only
if the loop proves out" — rather than advancing it. It is not a sequencing
change. There is no Roblox product any more.

## Why

The design asks for things Roblox makes expensive and PC makes ordinary: a
dense, furnished department store; serious lighting with real shadow; authored
sound; animation that reads as weight; a private expedition for one to four
people. Roblox's own guidance is the clearest evidence against it — shadow
quality degrades at lower graphics levels, and dense lights and shadows are the
headline performance constraint. Those are exactly the areas that have fought
the mood of this game, and they fight it because the platform has to run on a
phone. A game whose entire mechanic is *light* cannot afford a platform that
treats light as the thing to cut first.

The second reason is scope honesty. The handoff's "rebuild rather than an
export" was already an admission that nothing travels. Keeping both would have
meant maintaining a product we had decided not to finish, to protect an option
we had decided not to take.

## The decisions this platform change carries

| Row | Decision |
| --- | --- |
| 1 | Steam, PC-first. No parallel Roblox version |
| 33 | The Roblox project is a design reference and prototype archive — not maintained, not shipped |
| 34 | The first PC build is one department store, vertical slice. Not the whole Galleria |
| 35 | Engine: **Unreal Engine 5** — LOCKED |
| 36 | Solo, or private co-op for 1–4 friends. No public matchmaking |
| 37 | Wider testing uses a Steam Playtest child app, not main-game beta keys |

The full level list is unchanged as the eventual game, and row 38 locks it:
"We will have all the levels, including the whole mall. It starts off small,
progressesion. Danger." Row 34 is a build order - where the shovel goes first -
and says nothing about how big the game is. The whole Galleria, the solar farm,
the rail yard, the works, the hospital wing and the city are all still the
game, reached by progression, with each expansion arriving as more danger
rather than more floor. `docs/ZONES.md` holds the shape.

The engine was locked the same day, in his words — "We use unreal." It is the
floor everything else now stands on: the lighting, audio, animation and art
decisions all assume it, and reversing it later would discard more work than
the Roblox freeze did. Nothing in this project should be chosen as
engine-agnostic insurance from here. The two layers that stay portable —
`docs/` and `data/` — stay portable because they are design and values, not
because we are keeping an option open.

## What survives the move, and what does not

**Survives — and it is most of the actual work.** The design is
platform-neutral and was written that way:

- `docs/` — 3,075 lines. `PREMISE.md`, `PITCH.md`, `THE-STORE.md`,
  `QUESTS.md`, `ZONES.md`, `COMBAT.md`, `LANTERNHOLD.md`,
  `FIRST-SIXTY-SECONDS.md`, `HOW-THE-PLAYER-LEARNS.md`. None of it names an
  engine. The premise, the ladder of recovery jobs, the one sealed store, the
  lantern rule, the Warden, the six enemies, the extraction climax — all of it
  moves intact.
- `data/` — eight JSON files. **Values, not code.** Rates, curves, thresholds,
  tiers, room and rung costs. A UE5 build reads the same numbers from the same
  files, and `tests/balance.test.mjs` keeps checking them with no engine
  present, because it never needed one.
- `docs/DECISIONS.md` — the reason the move is cheap at all. Every guardrail
  that would otherwise have been re-litigated from scratch is written down with
  who set it and why.
- The open questions. O1's run-length confusion and the untuned baseline Storm
  Charge rate are design problems, not Roblox problems. They follow us.

**Does not survive.** 15,080 lines of Luau across 36 files. Every service,
every controller, the `CollectionService` tag contracts, the procedural rigs,
the ProximityPrompt interactions, the DataStore profiles, the streaming radii,
`default.project.json`, and the Rojo workflow around all of it. Not portable,
not partially portable. It is the prototype, and its value from here is as a
reference for *what the systems do* — the services are small and readable and
say what each rule means in practice.

`tests/` (1,430 lines) splits: the balance assertions move with the data, and
`wiring.test.mjs`, `lighting.test.mjs` and `map-contract.test.mjs` are
Roblox-shaped and retire with the Luau.

## The cost nobody should paper over

**The loop has never been played.** Row 3's playtest question — do testers kill
their lanterns, bank, and argue for one more room? — is still open, and the
Roblox prototype's one job was to answer it cheaply. It never did; Studio has
not been run against this repo at any point. So the move is being made on
design conviction rather than on evidence, and the evidence now costs
UE5-shaped money instead of Roblox-shaped money.

That is a real risk and it has one mitigation, which is non-negotiable: **the
vertical slice's first job is still that question, not fidelity.** A grey-box
department store in UE5 that answers it beats a beautiful one that does not.
Row 34 exists for this reason.

The other costs, stated plainly:

- No free discovery and no free players. Roblox would have supplied both.
- No free hosting or matchmaking. Private 1–4 co-op keeps this small, but it is
  not zero.
- An art and audio bar set by the genre's own proof points, not by us.
- UE5 is harder. Chris's framing: "the right kind of hard, because we would be
  building the actual game once."

What the move buys back, immediately: a price at the door. Row 23 forfeited
real revenue to keep power out of a shop; a premium game keeps the guardrail
and gets paid anyway. See **O7**.

## The Steam setup

Reserved by Chris on 2026-10-27, outside this repo. Recorded here because an
unrecorded store page is how the next reader mistakes a reservation for a
mandate — or worse, builds for a port that no longer exists.

| What | ID | Role |
| --- | --- | --- |
| Package | `1903488` | Developer Comp — us |
| Package | `1903489` | Beta Testing — a few trusted players |
| Package | `1903490` | The main game |
| Store item | `1409731` | With a store package |

Auto-grant to publisher Christopher Kantz.

For anything wider than a handful of trusted testers, row 37: a **Steam
Playtest child app**, which is Steam's own recommendation and can be opened or
closed without affecting the main game's store page or review state. Main-game
beta keys cannot be withdrawn as cleanly.

**Nothing has been published, uploaded or released.** A reserved app has no
expiry and no obligation.

### One thing still unanswered

The Steam packages are titled **"The Last Lightkeeper"** — singular. This
project is **Last Lightkeepers** — plural. That was a curiosity while the store
page was hypothetical; now it is the name on the product, and the plural is
load-bearing in the fiction (an order, not a job — row 21; a crew, not a hero —
row 36). Chris's call: fix the store entries, or change the project's name.
Nobody should guess, and nobody should silently use one form in new work.

## For whoever picks this up next

1. The engine is **Unreal Engine 5**, locked. Row 35. Do not re-open it, and
   do not hedge against it.
2. Do not advance anything in `src/`. Row 33. It is frozen, not deleted.
3. `docs/WORKLIST.md` describes the Roblox prototype. Its lanes, owners and tag
   contracts are archive. Do not pick up a slice from it as if it were work.
4. Read `docs/DECISIONS.md` before proposing a rule. Most of what looks like an
   open question has an owner and a date.
