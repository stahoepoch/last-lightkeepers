# Roadmap — the Unreal build

Written 2026-10-27 on Chris's instruction: a roadmap big and small, work
divided, lanes specific, responsibility clear, everything accounted for.

Read `docs/LANES.md` first for who owns what. Read `docs/PLATFORM.md` for why
we are here. `docs/DECISIONS.md` is the authority if anything below contradicts
it.

---

## The shape of it

| | Milestone | Exit criteria — the thing that proves it is done |
| --- | --- | --- |
| **M0** | **It builds** | A clean clone builds, the editor opens, the data contract gate passes in CI, and a headless automation test passes **locally** - see the two-tier CI rule below. Nobody writes gameplay before this |
| **M1** | **The keeper and the lantern** | Two players on a listen server in a grey-box store floor. One lights a lantern; both see the storm charge move. Movement reads from `data/lantern.json` |
| **M2** | **The storm has teeth** | A dark run and a lit run end measurably differently. Thresholds fire, the Warden arrives, a keeper can be knocked down |
| **M3** | **The loop closes** | A run can be won or lost. Loot visible only by light, burden slows you, the extraction call spikes the meter, extraction secures the haul and nothing else does |
| **M4** | **⟵ THE GATE: the playtest** | **Row 3's question answered with data, not opinion:** do testers kill their lanterns, bank, and argue for one more room? Four real testers, instrumented |
| **M5** | **The ladder and the hall** | Rungs 1–5 from `docs/QUESTS.md`. Lanternhold persists per keeper, restoration visible, each rung wakes its enemy |
| **M6** | **One store, furnished** | The art, audio and animation pass on the single department store. Fidelity, deliberately **after** the gate |
| **M7+** | **Expansion by danger** | Row 38. One zone at a time, in `docs/ZONES.md` order, each one passing the inversion test or not shipping |

### Two things about that order

**M4 is a gate, not a milestone.** Nothing after it is scheduled, estimated or
started. If testers do not kill their lanterns, the answer is not more content
— it is that the central trade does not work yet, and M5–M7 would be building
on sand. This is the same gate the Roblox project carried from the handoff and
never reached. Reaching it is the only goal that matters.

**M6 is after M4 on purpose, and it will feel wrong.** The whole argument for
leaving Roblox was fidelity — real shadow, real sound, weight. The temptation
will be to build M6 first because it is the reason we are here. Resist it: a
beautiful store that is not frightening to stand in has told us nothing, and it
costs ten times what the grey box costs to change. Grey-box the fear, prove it,
then make it beautiful. `DECISIONS.md` row 34 exists for this.

---

## M0 — It builds

Nothing here is gameplay. The point is that the next milestone can be judged
instead of debugged.

| # | Task | Owner | Done when |
| --- | --- | --- | --- |
| M0.1 | **Install Unreal Engine** and record the exact version | Codex | `UE_5.8.4` exists, the version is written into this file, and it is **pinned** — no mid-milestone engine upgrades, ever. An engine upgrade is its own milestone with its own regression pass |
| M0.2 | Project skeleton: `LastLightkeepers.uproject`, `Source/LLK`, `Source/LLKEditor`, both `.Target.cs`, both `.Build.cs` | Codex | `-nocompile -game` build succeeds from a clean clone with no editor open |
| M0.3 | Git hygiene: `.gitignore` for `Binaries/ Intermediate/ Saved/ DerivedDataCache/`, `.gitattributes` marking UE binaries `-text`, Git LFS for `Content/**` | Codex (seeded by Claude, see below) | A built project leaves `git status` clean, and `Content/` goes to LFS rather than into the pack file |
| M0.4 | Data pipeline spec: the JSON→DataTable contract, one row type per file, with the schema written down | Claude | `docs/DATA-PIPELINE.md` exists and Codex can implement from it without asking a question |
| M0.5 | `Tools/import_data.py` — the importer that runs the contract | Codex | Running it regenerates every `Content/Data/*.uasset` from `data/*.json` with no hand editing, and is idempotent |
| M0.6 | Headless test runner + CI | Codex | `Automation RunTests LLK` runs from the command line, exits non-zero on failure, and prints which test failed. Data checks gate every push in CI; the UE run is local until a self-hosted runner exists - see the two-tier rule below |
| M0.7 | The first automation test — a trivial one that proves the harness runs | Claude | It fails when deliberately broken. A green suite that cannot go red is worse than no suite |
| M0.8 | Open the editor, press Play, walk a cube around | Chris | He has seen it move. Nothing in this project is "done" because a test passed and nobody looked |

**M0.3 note on lanes:** `.gitignore` and `.gitattributes` are Codex's by the
lane table, but they block the first commit anyone makes, and Codex is not in
this session. Claude has seeded them today with UE-ready entries. **Ownership
is Codex's from here** — change them rather than inheriting them, and the LFS
decision in particular deserves a second opinion before anyone pushes 2GB.

**M0.1 status, 2026-10-28 — complete.** **Unreal Engine 5.8.4 is installed
and pinned.** Exact build, read off disk rather than off the launcher:
**changelist `58902052`**, branch `++UE5+Release-5.8`, compatible changelist
`55116800`, at `C:/Program Files/Epic Games/UE_5.8`. Source:
`Engine/Build/Build.version`, corroborated by the editor binary's own product
version string `++UE5+Release-5.8-CL-58902052`. The changelist is what makes
this a pin - "5.8.4" is a launcher label and a hotfix can move under it, while
a changelist cannot. The Epic Games Launcher exposes its Launch button. It is the
current stable UE5 hotfix release available through the launcher. Visual Studio
Build Tools 2022 with MSVC 14.44 and Windows SDK 10.0.26100 are present, and
740GB is free. Opening the editor remains M0.8's explicit visual proof; M0.2–
M0.7 are now unblocked, but none is verified until 5.8.4 builds it.

**M0.7 status, 2026-10-28 - compiled, not yet observed failing.**
`Source/LLK/Private/Tests/LLKHarnessTest.cpp` registers `LLK.Harness.Runs`
and `LLK.Harness.CanFail`. What has actually been verified: the file compiles
against the pinned 5.8.4, `WITH_DEV_AUTOMATION_TESTS` is live in a Development
build, and all four test names and assertion strings are present in the linked
binary - so `EAutomationTestFlags::EditorContext | ProductFilter` is valid on
this engine version. That was checked through the **Game** target, because a
running editor holds Live Coding and owns the editor module DLLs.

**What is NOT verified, and M0.7 is not done until it is:** nobody has watched
`LLK.Harness.CanFail` go red. Both tests are `EditorContext`, so running them
needs `Tools/run_automation.ps1` and a free editor. Step 3 of the procedure at
the bottom of the test file is the deliverable; compiling is setup for it.
A green suite nobody has seen fail is exactly the false confidence this
milestone exists to prevent, so this line says "compiled" and not "complete".

---

## The two-tier CI rule, decided 2026-10-28

**Every push:** GitHub runs the fast JSON contract and data checks.
**Unreal automation:** local for M0, and required in CI only once a
maintained self-hosted Windows runner with the pinned UE 5.8.4 install
exists.

Codex's call, and it is the right one. An `ubuntu-latest` job cannot prove
Unreal tests passed - it has no engine on it - and a hosted Unreal job nobody
operates would be slow, costly and brittle this early. The alternative was a
CI line that describes something nobody is building, which is worse than an
honest local-only rule.

**This amended two rows above, and the second one mattered more.** M0.6's
acceptance said `Automation RunTests LLK` must run "from the command line and
in CI". But M0's own definition - the gate for the entire milestone - said a
headless automation test "passes in CI", which under this rule M0 can never
satisfy. Left alone, M0 would have been unclosable by its own terms while
every task inside it was finished, and somebody would eventually have closed
it anyway by ignoring the line. A gate that cannot be met is a gate that gets
waived.

**What this costs, stated plainly:** between now and that runner, the Unreal
suite is only as good as somebody remembering to run it. The data gate is the
only thing CI actually enforces. That is an accepted risk and not an oversight
- but it does mean M0.7's canary procedure is the whole safety net, which is
why the evidence for it gets recorded rather than assumed.

---
## M1 — The keeper and the lantern

The first milestone with the game in it. One room, one mechanic, two players.

| # | Task | Owner | Done when |
| --- | --- | --- | --- |
| M1.1 | `ALLKCharacter` — first-person keeper, server-authoritative, speeds read from `data/lantern.json` (walk 16, sprint 23, crouch 9) | Claude | Two clients move independently on a listen server; speeds come from data, not constants. **Row 44, LOCKED:** first person is not a camera setting here - the lantern, hands, carry states, tools and downed crawl are first-person systems from day one, and no third-person fallback is kept working. The saving: no full-body player animation in the first slice beyond what co-op players see of each other |
| M1.2 | `ULLKLanternComponent` — toggle, replicated state, owns the light's on/off only | Claude | Client presses the key, server decides, both clients see the same state. No client-side authority anywhere |
| M1.3 | The lantern actor: light, mesh, the sound of it striking | Chris | It looks and sounds like a lantern. The C++ class is complete and works with a placeholder before this exists |
| M1.4 | `ULLKStormChargeSubsystem` — the meter, rate and multiplier from `data/storm-charge.json`, the **diminishing** curve (row 12) | Claude | Four lanterns cost more than one and no keeper's light is free. The existing 98 data assertions still pass |
| M1.5 | Grey-box store floor: **the ground floor**, blockout only, no art | Chris | It **teaches**. `DECISIONS.md` row 43 makes the store the tutorial, so navigable is no longer the bar: someone who has never seen it loots with the lantern off, then turns it on for the first relic, is noticed, and gets away - without being told to. Not pretty, just legible and big enough to be dark. Curriculum in `docs/THE-STORE.md` |
| M1.6 | Enhanced Input: move, look, sprint, crouch, lantern, interact | Codex | Every action is an Input Action asset, rebindable, with gamepad bindings. Nothing hardcoded to a key |
| M1.7 | Debug HUD: storm charge as a number on screen | Claude | Readable while playing. Not the real HUD — the real one is M3 |
| M1.8 | Automation tests for the storm charge rules | Claude | The curve, the multiplier and "only ever rises" (row 6) are asserted in-engine, not just in `tests/` |
| M1.9 | Two-player test over Steam or LAN | Chris + Codex | Two machines, one host, lantern state agrees. This is where networking mistakes surface, so it happens in M1 and not M4 |

**The lighting warning.** `DONTFUCKUPTHELIGHTS.md` exists at the repo root
because lighting globals were broken
repeatedly by people who did not know they were load-bearing. Unreal makes this
*easier* to break and harder to notice. M1.5's grey box gets a lighting setup
owned by Chris, and no C++ touches a global lighting value. Ever. The lantern
is a light; the world's darkness is a creative decision, not a constant.

---

## M2 and M3, in outline

Sliced properly when M1 lands, because estimating them now would be fiction.
The content is already written and should not be re-derived:

- **M2** — thresholds and the Warden. `data/storm-charge.json` holds the
  numbers; row 7 says the Warden is unkillable and drops nothing; row 5 says
  light reveals value and attracts danger with no "light repels" exception.
  **Enemies converge on the brightest position, never the nearest player** —
  `data/enemies.json` and `storm-charge.json` both state it, and the archived
  `EnemyService.luau` broke it by retargeting to nearest. **Do not inherit that
  bug.** It is not a fidelity detail: brightest-position is what puts cause and
  consequence in the same pair of hands, so in four-player co-op it is the
  difference between the building teaching the lantern rule and nobody learning
  it (`docs/THE-STORE.md`, the co-op teaching trap).
  Knockdown not death (row 11, and `docs/BANKING.md` on bounded loss). The
  Anchor Revive is solo-only (row 9).
- **M3** — loot, burden, extraction. `data/loot.json` holds tiers, pickup
  seconds and the burden penalty; row 8 says only extraction secures loot and a
  wipe or an expired timer loses the whole haul; O3 says the extraction call
  itself is the climax and the cost. `docs/FIRST-SIXTY-SECONDS.md` is the
  opening-minute spec and should be read before the HUD is built.

**O1 has to be settled inside M2**, by Chris: the baseline storm charge rate is
"the single most consequential untuned value in the project" and the run-length
numbers disagree across four sources. M2 cannot be judged without it.

---

## What we are deliberately not doing yet

Written down so nobody has to ask, and so nobody quietly starts:

- No animation work beyond placeholder. No retargeting, no mocap, no IK.
- No audio mix, no middleware, no music. Enemy telegraph sounds are the
  exception — row 16 makes them a build requirement, so they are in M2.
- No art pass, no materials beyond grey. M6.
- No save-format compatibility guarantees before M4. Say so to testers.
- No packaging for release, no store page build, no trailer, no capsule art.
- No second zone, no second store, no mall. Row 38 is the scope; row 34 is the
  order.
- No dedicated servers, no public matchmaking, no player count above four.
- No monetisation work. O7 is open and a premium game may sell nothing at all.

---

## Risks, with an owner each

| Risk | Owner | What we do about it |
| --- | --- | --- |
| Blueprint logic creeps in because it is faster | Claude | Review every handoff note. A rule in a Blueprint is a bug report, not a shortcut |
| Someone edits a `.uasset` an agent also touched | Chris | `Content/` is his alone. The DataTables are generated and belong to nobody |
| Replication retrofitted late | Claude | Server-authoritative from the first commit, and M1.9 tests two machines before the loop is even finished |
| Fidelity before fun | all three | M4 gates M6. The temptation is the whole reason the gate is written down |
| The engine version moves under us | Codex | Pinned in M0.1. An upgrade is a milestone with a regression pass, never a convenience |
| The loop is not fun | Chris | That is what M4 is for, and it is better to learn it at M4 than at M6. If the answer is no, we change the trade, not the content |
| Scope creep into the mall | Claude | Row 34 is the order and row 38 is the scope. Both are written down now precisely so this argument does not need having twice |

---

## Status, 2026-10-28

- **Current milestone: M0.** Done: M0.1 (engine pinned), M0.2 (skeleton builds
  from a clean clone), M0.3 (git hygiene), M0.4 (`DATA-PIPELINE.md`), M0.6
  (headless runner + the fast CI gate). `ProjectID` is committed as shared
  identity in `30ca0ca`.
- **M0.5 is NOT done, and the half that is missing is the half that produces
  assets.** Its acceptance is *"running it regenerates every
  `Content/Data/*.uasset` from `data/*.json`"*. What exists is the validating
  front end: `Tools/import_data.py` enforces the whole contract and writes
  canonical payloads to `Intermediate/LLKDataImport`, with red-path tests. What
  does not exist is the editor-side writer that turns those payloads into
  DataTables and DataAssets - `Content/Data/` is absent, nothing in the repo
  references `.uasset`, and `Source/LLKEditor` is still a module stub. Checked
  2026-10-28 rather than inferred from the commit message.
- **Why that matters more than bookkeeping:** M1 is held until M0 closes, and
  M1's `ULLKBalanceSubsystem` loads generated assets. Marked done, this becomes
  M1.1's problem to discover - which is the exact failure M0 exists to prevent:
  *gameplay should not be the thing that finds out the project cannot import
  data.*
- **M0.7 is a *proof* still owed, not code.** The test compiles and is
  committed; nobody has watched it fail. That needs the editor target rebuilt,
  which needs Live Coding released. M0.8 - open the editor, make a cube, press
  Play - is Chris's, and is the milestone's other deliberate "somebody looked
  at it" step.
- Engine version: **Unreal Engine 5.8.4, pinned.** No engine upgrade occurs
  inside a milestone.
- Done today: the platform decision and its paper trail (`86c5bb8`,
  `312ea8a`, `7cde823`), these lanes and this roadmap, and the UE-ready git
  hygiene that M0.3 hands to Codex.
- The Roblox prototype's 490 tests still pass and will keep passing — 98 of
  them are data assertions that survive the engine change and are the reason
  `data/` is worth carrying across at all.
