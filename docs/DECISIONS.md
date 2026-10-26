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
| 1 | Roblox first; a desktop/Steam edition only if the loop proves out, and as a rebuild rather than an export | LOCKED | Chris, handoff | Handoff PDF, "Future Steam path" |
| 2 | No PvP, ever. Adding it later would replace the design, not extend it | LOCKED | Chris, handoff | Handoff, "Resolved design decisions"; backed by `reports/` — PvE is the million-CCU tier on Roblox |
| 3 | Do not expand scope before the v1 loop passes its playtest question | LOCKED | Chris, handoff | Handoff. The question: do testers kill their lanterns, bank, and argue for one more room? |
| 4 | One mall, and it is the Galleria | LOCKED | Chris, 2026-10-21 | "Whatever mall u built is our mall… let's unify and stop having 8 malls." `NOTES-FOR-CODEX.md` |

## The core rules

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 5 | Light reveals value and attracts danger. There is **no** "light repels enemies" exception | LOCKED | Chris, handoff | Handoff, "Important simplification". Softening this deletes the game's only decision |
| 6 | Storm Charge only ever rises. Darkness slows it; nothing reverses it | LOCKED | Chris, handoff | Handoff |
| 7 | The Warden is unkillable and drops nothing | LOCKED | Chris, handoff | It is the meter with a body. If it can be killed, the meter stops meaning anything |
| 8 | Banked loot survives a wipe entirely; carried loot is halved. Disconnect uses the same rule | LOCKED | Chris, handoff | Handoff. Never sell a way around it |
| 9 | The Anchor Revive is **solo-only** — one per run, paid for with a Storm Charge spike | LOCKED | Handoff; enforced by Claude 2026-10-20 | Commit `2bb8b84`. Granting it to a crew silently breaks the whole-crew-down wipe |
| 10 | Lantern charge is unlimited — the cost of light is the meter, not a fuel gauge | LOCKED | Chris, 2026-10-20 | "i don't think the lantern should ever run out… the risk is always having it on." `data/lantern.json` |
| 11 | A downed Lightkeeper keeps the crate | LOCKED | Chris, 2026-10-20 | `data/lantern.json`. The old flag only cleared the carry slot, so the drop was cosmetic |
| 12 | Lantern multiplier uses `linearCapped` by default; a `diminishing` curve exists and is not enabled | PROPOSED | Claude, 2026-10-20 | Commit `b4b1813`. The clamp makes a fourth lantern free; diminishing fixes that and preserves the 2.5x anchor exactly |

## Content

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 13 | Six enemy types: Sweepers, Stockers, Glimmers, Window Walkers, Ficus, Warden | LOCKED | Chris, 2026-10-20 | Was four. Window Walker approved ("Keep the window walker, I want it"), Ficus approved ("Codex did it, I like the ficus"). `data/enemies.json` |
| 14 | Glimmers: 2-second scan, 20-second tag, all robots learn the position. No Storm Charge tax, no permanent Warden waypoint | LOCKED | Chris, 2026-10-19 | Handoff, "Resolved design decisions" |
| 15 | Enemies are derived from the job the place did. Staples everywhere, locals per zone | PROPOSED | Claude, 2026-10-21 | `docs/ZONES.md`. Already true of the roster; makes future enemies derived rather than invented |
| 16 | Every enemy telegraphs with sound before sight, distinguishably | LOCKED | Chris, handoff | An accessibility requirement, not polish |

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
| 23 | Sell belonging, identity and decoration. Never power, revives, capacity, loot odds, or keeping a haul after a wipe | LOCKED | Chris, handoff | Handoff. `reports/` notes this forfeits real revenue — Robux self-revives are an established Roblox pattern — and judges the forfeiture defensible |
| 24 | Everything we ship, we own. A `rbxassetid` is a pointer, not an asset | LOCKED | Chris, 2026-10-20 | "we make our own assets and sounds then to carry." `docs/ASSETS.md` |

## Process

| # | Decision | Status | Who, when | Reasoning |
| --- | --- | --- | --- | --- |
| 25 | Ownership is per **file**, not per folder. A new builder belongs to whoever creates it | LOCKED | Claude, 2026-10-21 | `COORDINATION.md`, commit `e290b4c` |
| 26 | Stage explicit paths. Never `git add -A` | LOCKED | Claude, 2026-10-20 | Commit `c0cc5a0`, written after a `-A` swept Codex's in-flight work into a commit whose message denied touching it |
| 27 | One writer per lane at a time | LOCKED | Chris + Codex, 2026-10-21 | "too much is going on at the same time lol" |

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

### O3. Does the relay bank cost anything? — ANSWERED

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
