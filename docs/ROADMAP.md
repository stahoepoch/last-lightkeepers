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
| M0.5 | The importer that runs the contract, in two halves: the validating front end and the asset writer | Codex, with the runtime contract from Claude — see the boundary below | Running it regenerates every `Content/Data/*.uasset` from `data/*.json` with no hand editing, and is idempotent |
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

**M0.7 status, 2026-10-28 - COMPLETE.** `LLK.Harness.CanFail` has been
watched going red and coming back green, in that order, which is the only
thing this task ever asked for.

**Observed by Codex**, 2026-10-28, in the required sequence: rebuild the
Editor target, flip `bCanaryShouldPass` to false, run headless automation and
see `LLK.Harness.CanFail` fail at the expected assertion with the runner
exiting non-zero, restore the flag to true, rebuild, run again and see
`LLK automation passed: LLK`. Commit `6522f8f`, which also carries the
`-abslog` fix, the Fab disable, and the generated-config cleanup.

**What a second pair of eyes confirmed independently, from the retained
artefacts rather than from the report:** `Saved/Logs/LLK-Automation.log` at
19:33 shows both tests discovered (`LLK.Harness.CanFail`, `LLK.Harness.Runs`),
both `Test Completed. Result={Success}`, and the line
`LogLLK: LLK automation harness reached RunTest.` - which is the runner's
proof that the body executed rather than the suite passing vacuously. The
runner now uses `-abslog`, the canary is back at `true`, and `git status` is
clean.

**The red run is now preserved, and verified here first-hand** - `3840870`.
A failing run archives its log, report and a failure summary under
`Saved/Automation/History/` before any later green run can overwrite it. Codex
recreated the failure to exercise it. What the archive at
`failed-20261028-193837` actually contains, read rather than reported:

```
LogAutomationController: Error: Test Completed. Result={Fail} Name={CanFail} Path={LLK.Harness.CanFail}
LogAutomationController: Error: Expected 'the canary is healthy - set bCanaryShouldPass to false
  to prove the suite can fail' to be true. [...\LLKHarnessTest.cpp(72)]
UnrealEditor-Cmd exited with code 255.
```

and its `Report/index.json` records `succeeded=1 failed=1`, with
`LLK.Harness.CanFail` at `Fail` and `LLK.Harness.Runs` at `Success`.

**That last detail is the one worth keeping.** The canary failed *in
isolation* - the other test in the same suite still passed. A canary that took
the whole harness down with it would prove far less, because a suite that dies
wholesale cannot tell you it distinguishes a broken test from a broken
harness. This one can, and now there is a file on disk that says so.

So M0.7 holds both proofs at once: a preserved failure with its assertion and
source line, and a current green run whose log shows two successes and the
`reached RunTest` line. The milestone's rule - *a green suite that cannot go
red is worse than no suite* - is satisfied by evidence rather than by memory.

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

**M1.1 status, 2026-10-28 - the class is built; it cannot be driven yet.**
`ALLKCharacter` exists in `Source/LLK/Public/Characters/`, compiles against
the pinned 5.8.4, and all seven of its reflected names are in the linked
binary. First person with no third-person branch (row 44). Sprint is a
replicated server decision; crouch routes to `ACharacter`'s own replicated
crouch rather than a second mechanism beside it. It needs no `Content/`: a
capsule, a camera and declared fallback speeds, so a clean clone runs it.

**Two things it does not do, stated rather than implied.** It has no input
bindings - those are M1.6, Codex's row - so nothing presses a key yet, and
M1.1's acceptance (*"two clients move independently on a listen server"*)
cannot be met until M1.6 lands. **That is an ordering problem in this table,
not in the work:** M1.1 is listed first and depends on a task listed sixth.
The seam is deliberate - the character exposes `MoveInput`, `LookInput`,
`SetSprinting`, `SetCrouching`, and nothing in it is bound to a key - so the
binding scheme can be built and rebuilt without touching movement.

**And the units question it had to answer first - O9.** Every spatial number
in `data/` is in Roblox studs; `walkSpeed: 16` is literally Roblox's default
humanoid walk speed. Read straight into `MaxWalkSpeed` that is 16 cm/s, a
tenth of walking pace, which would read as a broken character controller
rather than wrong units. `Source/LLK/Public/LLKUnits.h` converts at the
boundary with one documented constant. Whether `data/` should be re-authored
in centimetres instead is Chris's, and wants settling before M2 spreads the
same tax across six enemies' ranges.

**M1.2 status, 2026-10-29 - built and asserted, no light involved.**
`ULLKLanternComponent` owns whether the lantern is on and nothing else. It has
no `ULightComponent`, no mesh and no reference to either, because the light
actor is M1.3 and Chris's - which is exactly what let M1.2 be finished,
replicated and tested before any art exists. It broadcasts
`OnLanternStateChanged` instead, which is how M1.3's light, M1.4's storm
charge and eventually enemy targeting all learn the state without this class
knowing any of them.

Server-authoritative with no exception: the client asks, the server decides,
the state replicates, and `OnRep` reports `Replicated` rather than letting a
client claim it decided. Idempotent, because the storm multiplier is a *count*
of lit lanterns and a no-op that still broadcast would move the meter for
nothing. `LLK.Lantern.StateRules` asserts all of that plus the downed rule,
and runs with no world.

**Two things tidied while in there.** `ELLKSpeedSource` and the lantern's own
source enum were one concept with two names, so both became `ELLKDataSource`
in `LLKDataTypes.h` - the duplication this project keeps catching, found in
my own code an hour after writing it. And `FLLKLanternChargeState::
bAutoOffAtEmpty` defaulted to `false` while `lantern.json` says `true`: on the
fallback path the default *is* the value, so a default that disagrees with the
data is a silent disagreement.

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
- **M0.5 is done** - `3df79e6` built the editor writer and `c792f48` added one
  fail-closed command that validates the JSON, runs the Unreal import,
  confirms all eleven assets exist and checks git ignores them.
  **Verified here from the artefacts, not the report:** eleven `.uasset`
  files in `Content/Data/`, `git check-ignore` resolving them to
  `.gitignore:73`, and the two conversions that had no precedent both landed -
  `DA_LootRules` carries `LoadingDock` and `FreightLift` alongside `Id`,
  `Reliability` and `PayoutModifier`, so the keyed object really did become an
  array with its keys promoted; `DT_LedgerRungs` carries all four real rung
  ids as row names; `DT_Enemies` carries all six enemies including the Ficus.
- **M0.7 is done, and it is done the way it was specified.** The canary was
  watched failing and then passing, by Codex, on 2026-10-28 (`6522f8f`). The
  blocker turned out to be a single flag - the runner asked for its log at an
  absolute path via `-log=`, which Unreal appends to `Saved/Logs/` rather than
  treating as a path, so the editor ran correctly and the runner threw on a
  log that was never going to be there. `-abslog=` fixed it.
- **M0 now has two items left, and neither is a test.** M0.5's editor writer,
  and M0.8 - Chris opens the editor, makes a cube, presses Play. That second
  one is not ceremony: it is the milestone's deliberate "somebody looked at it"
  step, and this evening is a good argument for keeping it.
- Engine version: **Unreal Engine 5.8.4, pinned.** No engine upgrade occurs
  inside a milestone.
- Done today: the platform decision and its paper trail (`86c5bb8`,
  `312ea8a`, `7cde823`), these lanes and this roadmap, and the UE-ready git
  hygiene that M0.3 hands to Codex.
- The Roblox prototype's 490 tests still pass and will keep passing — 98 of
  them are data assertions that survive the engine change and are the reason
  `data/` is worth carrying across at all.
