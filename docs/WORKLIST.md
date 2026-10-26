# Worklist — sliced, evidenced, ready to pick up

Written 2026-10-21 from `reports/Roblox extraction games and tooling.md`.

Each slice is roughly an hour, independent of the others in its lane, and
states what done looks like so nobody has to ask. Take them in order within a
lane; the lanes don't block each other.

**Ground rules, unchanged.** No PvP. No selling power, revives, capacity, loot
odds, or any way to keep a haul after a wipe. No naming a real town or
fraternal order. Light reveals value and attracts danger — there is no
"light repels enemies" exception. Balance values in `data/*.json` are Chris's.
Stage explicit paths, never `git add -A`.

**Two things come first, before anything else in any lane:** A1 (instrument)
and C1 (run length). Everything else in this document is a hypothesis until the
funnel exists, and three files currently disagree about how long a run is.

---

# Lane A — gameplay systems

Owner: the session that owns `src/server`, `src/client`, `src/shared`.

### A1. Instrument the loop — do this first

**Why.** The report's flattest conclusion: everything else is guesswork without
a funnel. Roblox's own ranking signals are play-through rate, first-play bounce
(segmented under 60s and 61–180s), play days and playtime per user — so the
funnel you build should mirror the signals you're judged on.

**Done when.** `AnalyticsService` funnel events fire for: joined hub → opened
the departure board → launched a raid → picked up first loot → first knockdown
→ banked at the relay → called an exit → extracted → returned next session.
Each has a run id. Verified by playing once and reading the events back.

**Files.** New `src/server/services/TelemetryService.luau`; call sites in
`RunStateService`, `LootService`, `InteractionService`.

### A2. Make the relay bank cost something

**Why.** The sharpest critique of our own design. Dredge's rule is that the
player's position must result from a choice about time; Badlanders removed that
and "nothing really means anything" — insurance became pointless and the
economy hollowed out. A free bank is auto-loot with extra steps.

**Done when.** Banking costs at least one of: a real detour (the relay is off
the efficient route), a Storm Charge contribution, or a forced lantern-on
window while the transfer runs. Pick one, implement it, write down which and
why. The cost is a value in `data/loot.json`, so Chris can tune it.

**Files.** `RunStateService.bank`, `InteractionService`, `data/loot.json`
(schema only — propose the number, don't set it).

### A3. Attribute the Storm Charge

**Why.** A shared meter that one player can spend for everyone is a griefing
vector unless it's visible who is spending it. Every well-evidenced escalating
threat — Dredge's eye, Risk of Rain's clock — is continuously readable. Ours is
readable but anonymous.

**Done when.** The HUD shows who is lit and roughly what each lantern is adding
("3 lit · +4.0x"), and a downed or dark player can see it too. Crews should be
able to *argue* about the meter, which they can't do if they can't attribute it.

**Files.** `RunStateService` snapshot, `HUDController`.

### A4. Make the wipe a punctuation mark

**Why.** The cheapest win in the whole report. ARC Raiders' downed camera used
to linger on the body and "got demoralizing"; a hard cut to black flipped
players "from dwelling on their defeat to 'oh, it was a punctuation'". Embark
almost did it by accident.

**Done when.** A wipe cuts fast — no lingering — to a Lanternhold recap that
leads with **what was banked** (the thing they kept), then what was lost. No
scoreboard, no dwell.

**Files.** `RunStateService.wipe`, `HUDController`, the RunEnded path.

### A5. Pay partial credit for a failed run

**Why.** Proven on this platform: *99 Nights* pays Diamonds for reaching Night
50 of 99. Every durable game in the genre pairs real loss with cheap re-entry.
Right now a wipe with nothing banked pays nothing at all.

**Done when.** A run that ends badly still pays something scaled to how far it
got — rooms reached, Storm Charge survived, crates carried. Rates live in
`data/loot.json` as a proposal for Chris.

**Files.** `RunStateService.resolveHaul`, `PlayerDataService`.

### A6. Difficulty-scaled departures

**Why.** Dead Rails pays 10 Bonds for Normal and 20 for Nightmare, chosen at
run selection. It converts "how brave are you" into a decision made *before*
the run, which is cheap and proven.

**Done when.** The departure board offers at least two routes with different
payout multipliers and different starting Storm Charge or patrol density. The
choice is the crew's, made together, before launch.

**Files.** `ExpeditionService`, `data/loot.json` (schema).

### A7. Harden persistence

**Why.** ProfileStore's default autosave is 300 seconds. For an extraction game
that's up to five minutes of haul at risk on a server crash — and the one thing
we promise is that banked loot is safe.

**Done when.** Either a forced save at every bank and every extraction, or an
autosave interval short enough that the promise holds. Whichever is chosen is
written down next to the code.

**Files.** `PlayerDataService`.

### A8. Commit the entangled pile

**Why.** `init.server.luau` requires `ExpeditionService`, which is untracked, so
committing it alone hands anyone a broken checkout. That pile has been sitting
uncommitted for a day and includes Chris's balance files.

**Done when.** `git status` is clean, or everything left dirty is deliberate and
written down. The hub wiring's four call sites survive.

---

### A9. Rung one - the departure generator

**Why.** Chris locked a ladder of practical recovery jobs on 2026-10-26
(`docs/QUESTS.md`, decision 29) and rung one is the smallest end-to-end proof
of it: salvage fuses and wiring from the ground floor, restore the generator,
get reliable departures and map markers - and the powered lights wake the
Window Walkers. It exercises every part a later rung needs: a ledger line, a
broken place, a cost, a thing that opens, a thing that wakes.

**Done when.** One rung is completable in a session, the thing it opens is
visible from where the player stands, the thing it wakes actually wakes, and
the trade was stated in the ledger before the player accepted it. No quest
state machine - a flag and a cost.

### A10. The fight you start

**Why.** Five of six enemies have had a health value since the handoff and
`EnemyService` writes it onto a real Humanoid in four places; nothing in the
game can reduce it. `data/loot.json` has `blocksCombatWhileCarried` describing
a system that does not exist. Settled 2026-10-26: keepers can fight what has
not seen them (`docs/COMBAT.md`, decisions 30 and 31).

**Done when.** A dormant enemy can be removed with a hold, a found tool and the
lantern compete for one hand, and nothing a keeper carries can kill an enemy
that is already hunting them. The Warden is untouched. Values in a new
`data/tools.json` are Chris's.

---

# Lane B — integration, map and build

Owner: Codex.

### B1. Raid directors — one event per run

**Why.** The best-evidenced single recommendation in the report. Deep Rock's
retention is credited by its own studio to "procedural level design" —
hand-authored components arranged by code — and to systems before content. One
event per run makes the Galleria different every time without a second mall.
This is the answer to the only real risk the PvE decision carries: *"PvE gets
boring if the difficulty remains the same every match."*

**Done when.** A director picks one event per run from a table: a blackout, a
store shutter opening, a distant alarm that lights a wing, a flooded corridor
closing a route, a cart trail. Each is map-side, tagged, and fires once.

**Files.** `GalleriaBuilder` plus a new director module in `src/map`.

### B2. Instance streaming and the light budget

**Why.** A hundred shop bays with interiors is a lot of parts, and Roblox names
large scene replication, many shadow-casting lights, transparency and duplicated
assets as the specific traps. We just turned facades into rooms.

**Done when.** Streaming is enabled and tuned, shadow-casting lights are counted
and capped, and there's a measured before/after on part count and frame time.
Numbers written down, not impressions.

### B3. Decide how many bays are real

**Why.** Scope. A hundred enterable bays is a hundred bays to fill with loot
spawns, enemy pathing and a reason to enter. Most malls have anchor stores that
matter and inline units that don't.

**Done when.** A written decision on how many bays are enterable, which, and
what the rest are. Chris approves the number.

### B4. CI

**Why.** Everything green locally means nothing once three agents write to one
tree. We've already had a commit sweep in someone else's work.

**Done when.** A workflow runs `npm test`, `stylua --check`, `selene`,
`luau-lsp analyze` and `rojo build` on every push, and fails loudly.

### B5. Toolchain

**Why.** Aftman is archived; Rokit is current. `luau-lsp`, StyLua and selene are
all actively maintained and the project has none of them wired in. Nothing in
this project has ever been typechecked.

**Done when.** `rokit.toml` installs them, they run clean or their findings are
triaged into a list, and the commands are in the README.

### B6. Clean the stale comments

**Why.** Four files still describe `FloodedMallBuilder`, which no longer exists.
The one in `LanternholdBuilder` describes how it "clears every child of Map" —
actively misleading to the next reader.

**Done when.** Comments describe what the code does now, keeping the history
only where it explains a decision.

---

# Lane C — design, docs and assets

Owner: this session.

### C1. Reconcile run length — do this first

**Why.** Three sources disagree: the handoff says 6–10 minutes, `Config.Run`
says 20, a collaborator's audit assumes 8–15. The research resolves it — 6–10
is right, and the metric to engineer is **runs per session ≥ 2**, not minutes.
Until this is one number, every pacing conclusion is unsound.

**Done when.** One number in `Config.Run`, the same number in `tests/`, the
same number in every doc, and the reasoning recorded once.

### C2. Promote the proposals, or kill them

**Why.** `PREMISE.md`, `LANTERNHOLD.md`, `ZONES.md` and
`HOW-THE-PLAYER-LEARNS.md` are all marked *proposed*. Several are now being
built from. A proposal that is being built from is canon whether or not anyone
said so.

**Done when.** Chris has said yes or no to each, and the headers say which.

### C3. Write the first twenty relic plaques

**Why.** Relics are the only lore-delivery channel, and nothing writes on the
plinths. The plinths exist and are tagged.

**Done when.** Twenty relics exist as data: name, two sentences, which tier,
where found. Two sentences, about a person not the world, never explaining a
mechanic. Three of them say *unknown* — the burnt-records gap is the point.

### C4. The first sixty seconds, specified — DONE 2026-10-26

See `docs/FIRST-SIXTY-SECONDS.md`.

**Why.** CHI 2012 across 45,000 players found tutorials are only justified in
the most complex games, and that *context-sensitive* help is the form that
works. So the first run should teach by arrangement, not by text — but nobody
has written down what the player sees, in order.

**Done when.** A beat-by-beat spec of a first arrival: what's on screen, what's
lit, what the first prompt is, when the lantern is first mentioned, what the
first relic hand-in feels like.

### C5. The five enemy cues, mixed against each other

**Why.** The telegraph rule is accessibility, not polish. The cues exist as
synthesised originals but have never been heard in sequence or against the
game's own ambience.

**Done when.** The five render, play back-to-back, and are distinguishable by
ear with eyes closed. Anything that isn't gets re-synthesised.

### C6. A decision log

**Why.** Three agents and a human have made dozens of binding calls — solo-only
anchors, no real trademarks, one mall, six enemy types, unlimited lantern
charge. They're scattered across commit messages and notes files. The next
person to join cannot find them.

**Done when.** One file lists every locked decision, what it was, who made it,
when, and where the reasoning lives.

### C7. Reconcile the exits with the ladder

**Why.** `data/loot.json` has LoadingDock as the reliable 1.0x exit and
FreightLift gated behind `SubBasementBreakerRestored`. The ladder makes rung
one's departure generator the thing that makes departures reliable, and rung
five's dock gate the way into the mall rather than a way out of it. Those are
compatible but not aligned, and two lanes would guess differently.

**Done when.** One line from Chris says whether the breaker flag is the
generator, and whether the dock gate is an exit at all. Then `loot.json` and
`docs/QUESTS.md` say the same thing.
