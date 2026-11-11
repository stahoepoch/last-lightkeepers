# The pivot is a change of container, not of machine — and it has no rows

**Status: audit, 2026-11-11, by Claude.** Written on Chris's instruction —
*"you'll need to audit the whole thing, on your lane, with the help of codex,
and get the product and experience on the right path."*

Scope: the whole written design against the pivot Chris began on 2026-11-09,
and both against what the build can actually do. Read against `DECISIONS.md`
rows 1–83, `LANES.md`, the 104 files in `docs/`, the 29,277 lines in `Source/`,
the five maps in `Content/`, and the live ledger in `HANDOFFS.md`.

**Nothing here is a decision.** Eight rows are proposed, in `DECISIONS.md`, at
`PROPOSED` — the status that file defines as *"written down, not yet
ratified."* Ratifying is Chris's and only Chris's.

Method is `ADVERSARY.md`'s. The register pattern — a dated finding with a named
owner per fix, struck through rather than deleted when it closes — is
`DRIFT.md`'s, and this file is its second run with a different subject.

---

## The headline, in three sentences

The repo contains a complete, careful, mutually-consistent design for a
**1–4 player co-op extraction loop**, and 29,277 lines that implement it.

Chris's pivot describes a **single forward-moving story campaign** — a
twelve-year-old crossing a dead mall end to end to find his family, no
extraction, no return to the hub.

**Neither of those is written in `DECISIONS.md`, because the pivot has not
produced one row in three days, and the 83 rows that exist describe the first
one.** So the authority says Game A, the build implements Game A, and all four
parties are now designing Game B in prose that binds nobody.

---

## Part 1 — The finding that matters most

### 1.1 The pivot breaks 16 rows and leaves 67 standing

I expected the pivot to invalidate the corpus. It does not. Counted row by
row against the pivot as Chris and Codex have stated it:

| | Rows | What happens |
| --- | --- | --- |
| **Untouched** | **67 of 83** | Every lantern row (5, 10, 12, 80, 81, 82), light authority, the telegraph rule (16), the enemy roster (13–15), the machines' premise (47), the Valley (57), Zone Pressure (72, 73), units (61), lanes and process (25–27, 39–42), monetisation (23, 24), relics (53, 62), traversal (63), escalation (64) |
| **Amended** | 6 | 28, 43, 45, 48, 32, 64 |
| **Superseded** | 6 | 8, 9, 59, 74, 75, 76 — all six are extraction |
| **Retired** | 2 | 30, 31 — combat, and both only `PROPOSED` |
| **Strained, unresolved** | 2 | 36, 51 — co-op's crew size. See 1.4 |

**The expensive half of this project is the half the pivot does not touch.**
Rows 80, 81 and 82 — named lantern levels, physical honesty, the
cosmetic/gameplay seam — took weeks, cost a measured argument each, and are
load-bearing in `LLKPerception`, `LLKLanternComponent`, `LLKLightField` and
seven test files. Chris's own pivot message protects them explicitly:
*"Lamp is still like a type of flashlight multi tool, still has same premise I
guess."* It does.

### 1.2 Extraction is 10.8% of the module, and it is a rename rather than a deletion

| | |
| --- | --- |
| Extraction code, 8 files | **3,169 lines** |
| `Source/LLK` total | 29,277 lines |
| Share | **10.8%**, plus 11 files that touch it lightly |

The part worth the audit: **the state machine already is the mechanism the
pivot needs.** `ELLKExtractionPhase` is `Idle → Announcing → Holding →
Secured / Failed`, with a hold that pauses rather than rewinds when the route
empties, an occupancy input, seven named refusal reasons and a `requires`
gate. Read it as **making camp** and every state keeps its meaning:

| Phase today | Under the crossing |
| --- | --- |
| `Idle` — *"the crew is still deciding, which is most of the run"* | Crossing. Still most of the night |
| `Announcing` — siren, convergence, nothing holdable yet | You commit to a shop: the light goes up, and everything hears it |
| `Holding` — the hold runs and wants somebody on the route | You hold the room while the camp establishes |
| `Secured` — *"the haul is out"* | The camp holds. **The frontier moves** |
| `Failed` | Driven off. The frontier does not move |
| `requires`, e.g. `SubBasementBreakerRestored` | You brought what this camp needs |
| `AlreadyExtracted` | This camp is already made |

One state needs real design rather than a rename, and it is the one with a
locked row in its comment. `Failed` says *"Row 8: the night keeps
everything."* With no extraction there is no haul to lose, so **what losing a
camp costs is a genuinely open question** — and `GAME-THESIS.md` already
red-teams the wrong answer: *"Full-loss runs feel arbitrary → friends stop
after one bad wipe."* My recommendation is in 2.3.

**So the pivot's largest apparent deletion is a repoint of 3,169 existing,
tested lines to a different zone, and the zone it moves to is the one actor in
the project whose `BoxExtent` was never tuned** —
`FINDINGS-EXTRACTION-NEVER-COMPLETES.md` measured it at the constructor
default to the centimetre. That work was owed anyway.

### 1.3 The pivot deletes the only thing the corpus named as the reason to play again — and that is a gift

Four documents independently identify the same single hole, and they are
emphatic about it:

- `VERTICAL-SLICE.md` element 5: *"the first session's entire payoff, the
  pitch's biggest hook… and the reason anyone plays run two."* **Half built,
  and the half that was built was built on Roblox** — `947a84b` deleted it.
- `FIRST-SIXTY-SECONDS.md`: *"That last beat is the reason there is a run two,
  and it is the one piece of this document that is not built."*
- `SPINE.md`: *"The session's job: the hall is visibly different… Not a stat
  went up — a door opened."*
- `PROGRESSION-GAP.md` / `HANDOFFS.md` **G1**: a finished run pays out and
  there is nothing to spend it on. **Open since 2026-10-26** — sixteen days.

The pivot removes the hub. So it removes the project's largest unbuilt thing
from the critical path, at the cost of needing a different answer to the same
question. That answer is Part 2.

### 1.4 Co-op is the one thing genuinely broken, and nobody has addressed it

This is the gap in the three documents written about the pivot so far, mine
included. A twelve-year-old alone looking for his family is a single-player
story. Against that:

- Rows **36**, **50**, **51**, **52** are LOCKED and all four are co-op.
- `GAME-THESIS.md`'s claim sentence — the first line of the thesis — is
  *"a first-person, one-to-four-player co-op recovery game."*
- `LANES.md` commits to **replication from the first commit**, server
  authoritative, *"even while there is only one player to test with"* — and
  that is an architecture, not a setting.
- The entire balance curve is tuned for one to four lanterns: the
  `diminishing` switch (row 12), the 2.5× anchor, and the four-crew-size
  pacing table in `VERTICAL-SLICE.md`. `tests/balance.test.mjs` asserts it.

Three readings, and this one is Chris's:

| | Reading | Cost |
| --- | --- | --- |
| **a** | The kid is the campaign; co-op is a separate mode in the same world | Keeps every row, but then the co-op loop still needs the hub payoff it never had — **you would be building both games** |
| **b** | **The kid is not alone.** The crew are the other apprentices; the family are the ones taken | **One sentence of fiction. Nothing in the build moves.** Rows 36/50/51/52 all survive |
| **c** | Drop co-op | Reverses four locked rows, the replication mandate, row 12's curve and most of the balance suite |

**I recommend (b).** It also dissolves a mistake of mine from 2026-11-09: I
proposed rescued keepers unlocking party slots, which is exactly what row 50
forbids. Under (b) the brotherhood is who you *start* with, not what you earn,
and row 50 needs no defending.

### 1.5 The pivot resolves a contradiction the project has carried since the handoff

Chris, in his own words to Copilot: *"you never attack robots, if u did, that
would be fatal."* Against that, row **30** (`PROPOSED`) says keepers can fight
back from rung 2 — *"you win the fights you start"* — `COMBAT.md` is 8 KB of
it, row **31** gives combat a hand slot, and `QUESTS.md` rung 2 pays out *"a
simple defensive tool or noisemaker."*

Row 30 existed to justify something the handoff shipped: five of six enemies
carry health values that nothing could ever reduce. **Chris's sentence
resolves it in the direction of less**, and nothing of value is lost: rung 2's
reward becomes the noisemaker alone — a decoy, which `QUESTS.md`'s later-rung
table already wanted as *"the bell: a decoy you aim"* — the health values go
back to meaning nothing, and row **7**, the Warden's unkillability, stops being
the exception and becomes the rule.

### 1.6 Row 28 is the only row blocking the next map pass

Codex named row 28's retirement as the gate, and he is right that something
has to move, but the gate is narrower than it looks:

| Row | Text | Against the crossing |
| --- | --- | --- |
| **34** | *"The **first thing built** is one department store… A build order, **not** a scope cut"* | **Compatible.** Says where the shovel goes |
| **38** | *"**The whole world is the game.** All the levels, the whole mall included, reached by progression"* | **Compatible, and it anticipates this** |
| **28** | *"**Version one is** one sealed department store"* | **The blocker.** "Version one is" is a scope claim, not a build order |

So rows 34 and 38 already permit the crossing and row 28 alone is in the way.
The cheapest resolution is to amend row 28 into what row 34 already says —
a build order — rather than retire it, which also preserves the thing it was
protecting: that density beats footprint. `GAME-THESIS.md` red-teams the
alternative by name: *"The department store becomes a giant empty map."*

### 1.7 Eighteen proposed rows are numbered 1–18 in a log numbered 1–83

A defect in my own work, found while consolidating. `PROPOSAL-THE-VALLEY.md`
proposes six rows numbered 1–6 and `ADJUDICATION-THE-STORY-DRAFT.md` proposes
twelve numbered 7–18. **Every one of those numbers is already taken.**
Ratifying "row 8" from the adjudication would collide with the locked row 8 on
extraction, and ratifying "row 16" would collide with the telegraph rule, which
is the most carefully defended row in the file.

Consolidated and renumbered in Part 3. The eight that bind **mechanism or
scope** are now `PROPOSED` rows **84–91** in `DECISIONS.md`. The ten that are
**narrative content** stay where they live, because row 83 gave that set a
writer and `HANDOFFS.md` **G2** is explicit that a lane grants a writer and not
an authority.

### 1.8 `DRIFT.md` section A is still open, ten days on

Rows **6** and **73** are both LOCKED and contradict each other — row 6 says
darkness slows the clock, row 73 says *"nothing a player does moves it"* — and
row 6 still carries no banner, in the sixth row of the file, under "The core
rules." `DRIFT.md` flagged it on 2026-11-01 as *"the serious one"* and named
the fix as one line from Chris.

**It is quoted here as evidence rather than as a new finding.** The reason this
audit pushes so hard on rows is that this project already has a proven case of
a contradiction between two locked rules sitting open for ten days because the
fix was one line nobody was blocked on. Unratified prose does not resolve
itself here. Eighteen pages of it will not.

---

## Part 2 — The right path

### 2.1 The one structural question, and its answer

The old loop's engine was **"you come home and a light comes on."** The pivot
deletes home. So:

> **What accumulates, when you can never go back?**

The answer is in the shape of the thing itself. In a forward-only crossing,
the only territory you own is **the ground you have already crossed.**

### 2.2 The frontier, and the camp that moves

Not a hub you return to — a camp you carry forward. Each night you push the
lit edge further up the concourse and bed down in a shop you have made hold.
The camp is the hub; there is one of it; and it moves.

Every constraint Chris set is satisfied without inventing anything:

| Chris's own words | What the frontier does with it |
| --- | --- |
| *"still might have to have a safe area we make our own, that's to build towards, but that safe area isn't the real purpose"* | The camp exactly: necessary every night, never the point |
| *"no extraction"* | Correct — you never leave. The **state machine** survives as making camp (1.2) |
| *"some forward progression through......???"* | The frontier. This is the answer to the question marks |
| *"we can make it across in less than a week"* | Four to six camps, one per night |
| *"so we are starting in fall-winter time? time of year and progression make a lot of reality logic"* | A crossing that takes days makes the season move **inside** the chapter |
| *"i really like the idea of it changing though"* | It changes behind you because you lit it, and ahead of you because the storm and the season moved |

And three locked rows get stronger rather than weaker:

- **Row 48** — *"one raid is one night"* — survives **verbatim**. Only
  "one deployment, one extraction-or-wipe outcome" changes.
- **Row 5** — light reveals value and attracts danger — gets the long-term
  version it never had. Lighting the mall behind you **feeds the machines**,
  which is `PREMISE.md`'s premise and `QUESTS.md`'s own line: *"every wire you
  reconnect is a wire they are on too."* **The frontier you build is the
  danger you build.** That is the game's single decision, finally compounding
  over a chapter instead of resetting every night.
- **Row 29** — the ladder of practical recovery jobs — needs no change at all.
  A rung now opens the next leg instead of a room at home.

### 2.3 What losing costs

The question `Failed` leaves open, recommended rather than decided:

**The frontier recedes to the last camp, and what you dropped stays where it
fell.** Not deleted — recoverable, by going back into ground that has had a
night to get worse. This answers `GAME-THESIS.md`'s red-team row (*"full-loss
runs feel arbitrary"*) without softening anything, because the cost is a
night and a walk rather than an erased evening, and it makes the receding
frontier the thing you are actually afraid of.

It also retires row 8 cleanly instead of leaving it dangling: there is no
securing and no losing of a haul, because there is nowhere to secure it.

### 2.4 The order of work, and what it is blocked on

| | Work | Owner | Blocked on |
| --- | --- | --- | --- |
| **1** | **Ratify or refuse rows 84–91.** Nothing below can be judged without them | **Chris** | Nothing. This is the whole gate |
| **2** | Repoint extraction to making camp. ~3,169 lines, mostly reused, **no new system** | Claude, `Source/LLK` | Row 85 |
| **3** | The concourse width — TLP-1 scaled at its own 100 ft bar | Chris, in the editor | Nothing. One measurement |
| **4** | **C2**, the Sweeper's four cues. Row 16 is LOCKED and the only threat in the chapter is currently silent | Chris (audio) | Nothing |
| **5** | **C14**, the Sweeper cannot reach the crate and has been failing in a loop | Codex / Claude | See Part 4 |
| **6** | The first camp, playable, in `LLK_Boswells_Anchor` | Claude + Chris | 1, 2, 3 |
| **—** | Art, assets, bundles, the store page | | All of the above |

### 2.5 What to stop

**Stop writing about the pivot.** I have produced three documents about it in
three days, Codex has relayed four replies, and the marginal value of a fifth
is zero. Eight rows are worth all of it.

And the honest version of the risk, said once: **this is the fourth re-scope
in 22 days.** Roblox → Steam on 10-27, whole mall → one store on 10-26, loop →
campaign now. Row **3** is LOCKED: *"Do not expand scope before the v1 loop
passes its playtest question."* **That gate has never been opened** — the loop
has never been playtested, because extraction has never been reached in one
(`M9`, and the extraction findings file). So the project has re-scoped three
times without ever answering the question the gate asks.

The pivot is not the problem. Re-scoping instead of playing is. And the pivot
makes playing *cheaper*, because it takes the unbuilt hub payoff off the
critical path — which is the strongest argument in its favour and is not one
anybody has made for it yet.

---

## Part 3 — The eighteen proposals, consolidated

Renumbered out of the 1–18 collision (1.7). **Eight are now `PROPOSED` rows
84–91 in `DECISIONS.md`**; ten are narrative and stay in their own files under
row 83's lane.

| New | Was | The call | Touches |
| --- | --- | --- | --- |
| **84** | Valley 2 | Version one is a forward-moving campaign; the night loop lives inside it | `GAME-THESIS.md`, rows 45, 48 |
| **85** | Valley 2, Adj 13 | No extraction. The unit is a night and a camp; the frontier is the progression | Supersedes **8**, **59**, **74**, **75**, **76**; amends **9**, **48** |
| **86** | Adj 9 | Attacking a machine is fatal and alerts every machine | Retires **30**, **31**; closes `COMBAT.md` |
| **87** | Adj 8 | Chapter one is the crossing, Boswell's to Sayers | Amends **28** into a build order (1.6) |
| **88** | Adj 13, 14 | Motion is one-way within a chapter. Returns happen between chapters | Reconciles the pivot with `CAMPAIGN-SHAPE.md` §2 |
| **89** | — | The crew are the other apprentices. Co-op is the fiction's own, not an unlock | Preserves **36**, **50**, **51**, **52** (1.4) |
| **90** | Valley 1 | The player is 12 or 13, and eye height is **measured** from a built capsule | New measurement task |
| **91** | Adj 16 | The season is the clock on home. The family is contained and has none | Amends **45**; bounds **73** |
| 92–101 | Adj 7, 10, 11, 12, 15, 17, 18, Valley 4, 5, 6 | Narrative and lamp-fiction | **Not written as rows.** Row 83's lane, `HANDOFFS.md` **G2** |

**Three things stay Chris's and are not proposed here**, because they have been
open across three documents and asking again in a fourth would be the thing
2.5 says to stop:

1. **Lamplighters or Lightkeepers.** Fourth day, three documents, two
   spellings. Codex proposed Lightkeepers as the group and "lamplighter" as an
   old local job word; I agree; neither of us can decide it.
2. **Row 47 against the Shepherd Project.** Codex withdrew his hedge on
   2026-11-10 and now agrees row 47 holds. That is all three of us on one
   locked row, and it still needs his sentence.
3. **Row 6's strikethrough**, while he is in the file (1.8).

---

## Part 4 — What this asks of Codex

Codex owns `Source/LLKEditor`, `Tools/**`, `Config/*.ini`, the build files and
CI (`LANES.md`). Filed as rows in `HANDOFFS.md` rather than described here,
per the rule that a finding is handed over as symptom, evidence, file and line,
and the owner decides.

| | What | Why it is his |
| --- | --- | --- |
| **P1** | `data/ledger.json`'s four rooms and `DT_LedgerRooms` / `DT_LedgerRungs` describe a hub the pivot removes. The import must not silently generate tables for a system that no longer exists | `Tools/import_data.py`, the importer, row 56 |
| **P2** | The crossing needs TLP-1 scaled at its 100 ft bar before any large map pass, and the concourse width is *"the first number the shell owes"* | Editor tooling; Chris presses Play |
| **P3** | **C14** — the Sweeper files misplaced stock it cannot reach, in a loop, and has been doing it the whole time. The crossing's only threat is this brain | `WL_` orphans are a greybox-generation artefact (**C12**) |
| **P4** | **X7** is still open: two lane tables disagree about who owns the runtime module and the maps. A pivot that repoints 3,169 lines should not start with that unsettled | Lane tables, and it is a two-writer risk |

**And the standing one, which this audit does not resolve:** Codex's four
relayed replies have been good, and three of them concede points. But every
one of them arrived as pasted text, so none of it is in the repo and none of it
binds. Four of his seven "things to settle" are now rows 84–91. The other
three are narrative and belong to row 83's lane.

---

## Turned over

**Operand.** The whole written design against the pivot, and both against the
build — as opposed to auditing the pivot documents against each other, which
is what the last three days produced.

**Floor.** `DECISIONS.md`'s 83 rows are the floor, and the row-by-row count in
1.1 is the measurement. The 29,277 lines and the 3,169 are counted, not
estimated. The state-machine mapping in 1.2 is read off
`LLKExtraction.h:53-100`, not inferred. `DRIFT.md` section A is the floor
under 2.5's argument, because it is this project's own proven case of an open
contradiction outliving its one-line fix.

**Clock.** The season Chris named — fall into winter — is now a design clock
and not only fiction. The real one: four re-scopes in 22 days and no
playtest.

**Falsifier.** The whole of Part 2 falls if Chris's answer to 1.4 is reading
(a) or (c) — a single-player campaign with a separate co-op mode means the
frontier is only half the design, and dropping co-op invalidates row 12's
curve and most of `tests/balance.test.mjs`. The frontier itself falls if a
camp turns out to be a save point with extra steps: the test is whether a
player ever chooses **not** to make camp and pushes one more shop in the dark.
If that choice is not real, the camp is a menu.

**Upside down.** The three documents arguing about this pivot, this one
included, total more words than the eight rows that would settle it. The
project's own file says why that is the wrong ratio: *"an unfindable rule gets
deleted by someone who never knew it was one."* The corpus is not too small.
It is 1.4 MB of prose with an 83-row spine, and the pivot has spent three days
not touching the spine.
