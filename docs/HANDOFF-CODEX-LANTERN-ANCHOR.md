# Handoff — Codex — the lantern anchor, and the state of the machine

Written 2026-11-04, end of a Claude session that ran with Chris remote. **Your
lane:** every item below is either `Source/LLK` gameplay code, which Chris's
standing instruction to this writer fences off (*"Do not change
`data/lantern.json` or any gameplay code"*), or `Tools/**`, which is Codex-only
by Chris's instruction, or the authored `umap`, which needs the editor.

Read `FINDINGS-LANTERN-ANCHOR-ORIGIN.md` alongside item 1. It is the evidence;
this is the work.

---

## 0. The state of the machine, before anything else

**The editor is CLOSED.** It had been open since the morning and Chris, remote,
authorised closing it. Its log carried **zero** `MarkPackageDirty` or
`SavePackage` lines for the whole session and four hours of nothing but hourly
`LogDerivedDataCache: Maintenance finished`, so nothing was dirty; it was closed
with `WM_CLOSE` rather than killed and it exited with **no save prompt**, which
is the second independent confirmation. **Nothing of yours was in that session**
— no dock bay package was touched.

What that buys you, all verified in the last three hours:

| | Evidence |
| --- | --- |
| `Tools/import_unreal_data.ps1` | 7 DataTables, 4 DataAssets, `Success - 0 error(s)` |
| EDITOR build target | `Result: Succeeded`, 17.9 s, compiling five files including your `LLKExtractionReadout.cpp` and `LLKDebugHUD.cpp` — which had never reached the editor binary |
| `Tools/run_automation.ps1` | 35 `Result={Success}`, 0 failures, DLL stamped 15:33:27 against a 15:34:08 log, so the suite was not testing a stale binary |
| `npm test` | exit 0, full chain, `movement.test.mjs` 24/24 |
| `Content/Data/DA_Lantern.uasset` | sprint **12.5** at offset 2236, **11.5 absent from the file** — Chris's "running could be a little faster" finally ships |

What it costs: no PIE, no scene reads, no MCP bridge. **If you reopen the editor,
the bridge also needs `ModelContextProtocol.StartServer` typed into its console
by a human** — it does not autostart, which is why holding the editor open all
morning preserved the one capability that was unreachable anyway and blocked the
four that were not. That mistake is written up as `TOOLING-TRAPS.md` trap 17.

---

## 1. The lantern anchor — the headline item, and it is two decisions

### What is live, and it is not what this project thought

`GetGameplayLightLocation()` is the single position the storm, the light field,
the Warden's convergence and relic legibility all judge from
(`LLKZonePressureSubsystem.cpp:345` feeds `Source.Position` from it). In a
running game it returns `LanternAnchor->GetComponentLocation()`, because the
function's **first line** is an early return on `GetAttachParent()` and
`ALLKCharacter` always attaches:

```
capsule 88  →  FirstPersonCamera (0,0,64) = 152  →  LanternAnchor (42,25,-19) = 133
```

So the gameplay light sits **133 cm above the floor standing level**, 19 cm
below the eye, which `LLKCharacter.cpp:67-71` chose deliberately and which
nothing here argues with.

**The thing nobody authored is the travel.** `LanternAnchor` hangs 42 cm
*forward* of a camera with `bUsePawnControlRotation = true`, so its world Z is
`152 + 42·sin(pitch) − 19·cos(pitch)`:

| look pitch | gameplay light Z |
| --- | --- |
| +89.9° | **194.0** |
| 0° | **133.0** |
| −65.7° | **105.9** |

**88.1 cm of vertical travel in the gameplay light source, with the keeper
standing perfectly still.** Enemy perception and Warden convergence therefore
depend on where the player is looking, through an input nobody fenced: the sway
*was* fenced (feel writes the child's relative offset, gameplay reads the
parent — that comment is correct and holds), but the look direction arrives
through the parent transform instead of through the feel struct, which is the
door that was watched.

**Decision 1 is made. The travel stays.** Chris, 2026-11-04, now
`DECISIONS.md` row 81 — *"if a lantern moves and it's supposed to move then it
moves… if you get knocked down a little bit your hand moves your lantern moves.
Everything moves you kind of shift around. That's the name of the game."* **Do
not fence it out, and do not move `LanternAnchor`** — that pose is approved art.
The travel is now a ratified property of the game, and row 81 makes physical
derivation the standard the rest of the feel model is judged against.

That turns this item from a bug into **item 1b**, below, which is the larger
piece of work.

### 1b. The feel model is a model of the hand with no lantern in it

`FINDINGS-LANTERN-PHYSICS.md` is the full derivation with sources; this is the
work it implies, and row 81 is the mandate for it.

**This is where the pendulum work goes, and as of 2026-11-04 it is the only place
it goes.** Chris closed O10 by re-confirming row 82 and said so directly: *"Put
the pendulum work into making the lantern feel carried, not into a behavior
difference nobody can perceive."* That makes this section easier rather than
harder. The second-order state lives below `LanternAnchor`, in the light
component's own relative offset, exactly where the sway already lives — so it
reaches no gameplay reader, needs no new fence, and carries no balance question.
Every number below is a feel number, free to be tuned by eye against the
derivation rather than defended as a gameplay value.

A kerosene hand lantern on a bail is a compound pendulum with a natural
frequency of **~1.05 Hz** (0.9–1.25 Hz across the whole plausible geometry — the
period goes as a square root, so the result is robust). A walking body drives the
hand **laterally at ~0.95 Hz** and **vertically at ~1.9 Hz**. Running that
through a base-driven pendulum's response gives three things that hold for any
damping:

- a walk sits at 0.90 of resonance, so the lantern **amplifies** the hand's
  lateral motion **4–5×, nearly in phase**;
- every gait drives the vertical axis well above resonance, so the real arc is
  **~1/10 as tall as it is wide**, not the hardcoded 1/2 — nearly flat;
- a sprint crosses above resonance: the ratio collapses to ~1.1× and inverts, so
  **sprinting swings the lantern less than walking does**, which is the opposite
  of `SprintSwayCm` 5.2 against `WalkSwayCm` 2.4.

So four of the current rules disagree with the physics: `SwayHertz` 1.6 is
**1.68× too fast** (1.6 strides/s is a 3.2 steps/s jog cadence, applied at a
walk), the vertical ratio is ~5× too tall, sprint and walk are reversed, and the
landing kick is a non-oscillating downward offset recovering in τ ≈ 0.35 s where
a struck pendulum rings at ~1 Hz for 1.5–3 s.

**The minimal honest change is one second-order state per axis** — angle and
angular velocity, driven by the hand's acceleration, with `f₀` and `Q` as the two
authored numbers — replacing the direct `FMath::Sin` in `AdvanceFeel`. About
fifteen lines. It **subsumes the landing kick** (an impulse becomes an initial
angular velocity and rings down for free, which is also exactly Chris's
knocked-down case) and it makes the sprint/walk relationship *fall out* rather
than be tuned. `SwayHertz` stops being an animation rate and becomes the
lantern's natural frequency — a property of the object.

Two cautions:

- **`Q` is the one number that cannot be derived.** Film a real lantern and count
  the swings; until then it is a range, and the three conclusions above do not
  depend on it.
- **`MaxOffsetCm` is 12 cm and a physically true walk sway is 10–15 cm**, so the
  existing rail would clip the real amplitude. Whether the rail rises or the
  amplitude is scaled for legibility is a feel call and Chris's, not a physics
  one — a lantern swinging its true 15 cm may simply be unpleasant in first
  person. Row 81 says derive first; it does not say ship the derivation blind.
  **Since O10 closed, this is comfort only**: whichever way it goes, no enemy,
  relic or light-field reader can tell the difference.

### What is latent, and this part is unambiguously a defect

`LLKLanternLightComponent.cpp:75`, the branch reached only when the component
has no attach parent:

```cpp
return LLKLanternFeel::GameplayLightPosition(
    Owner->GetActorLocation(), LLKLanternFeel::KeeperEyeHeightCm);
```

`KeeperEyeHeightCm` is 152 and its own header says it is *"derived from two
engine defaults"* — the capsule's 88 plus the camera's 64. **So 152 is a height
above the floor, and `GetActorLocation()` on a `ACharacter` is the capsule
centre, 88 above the floor.** The 88 is counted twice and that branch lands at
**240**.

Unreachable from `ALLKCharacter` today. Reachable the moment anything else owns
one: a lantern placed in the world, a dropped or docked lantern, a second body
rig, a fixture that spawns the component bare. The neighbouring
`Owner == nullptr` branch carries a comment saying that reaching it means *"the
cosmetic position has become the gameplay position and that is the bug this
method exists to prevent."* This branch deserves the same and has none.

**Decision 2, and this one is yours:** two shapes, and the first is better.

- **Rebase the call site to the capsule foot** —
  `GetActorLocation() - FVector(0,0,HalfHeight)` — keeping 152 as "above the
  floor." This makes the fallback agree with `LLKMapReadiness.cpp:299`, which is
  **already correct** and is the reference: it adds 152 to a `PlayerStart` Z, and
  this project treats a `PlayerStart` Z as the floor surface — `LLKTestGameMode.cpp:195-196`
  says so while building the harness floor cube at `HarnessOrigin - (0,0,25)`
  with thickness 50, so its top face lands exactly on that Z. Rebasing also
  stays correct when crouched, because UE shrinks the half-height and the feet
  stay planted.
- **Redefine the constant as 64**, an offset from the capsule centre. Fewer
  moving parts, but it silently stops working the moment anything authors a
  non-default capsule, and it makes the name wrong.

**Either way `LLKLanternFeelTest.cpp:92` must be rebased**, and note *why*: its
fixture is `Capsule(1200,-400,90)` and it asserts the sum is `90 + 152`. The 90
is itself a capsule-centre height, so the test **pins the double-count** and
would keep passing after a correct call-site fix. Change the fixture to a floor
Z and assert eye height above it. Guarantee 3 is sound and is not what is at
stake — it proves no *cosmetic* value reaches the anchor, which is purity, not
position, and it holds either way.

### One thing to be careful about, from how this was got wrong here

The morning version of the finding reasoned correctly about the 240 branch and
spent real effort proving its premise — confirming by elimination that no
Blueprint subclass and no `SetCapsuleSize` call could move the capsule off 88.
All true, all irrelevant, because six lines above there is an
`if (GetAttachParent())` that always takes. **Ruling out every way a number could
be different says nothing about whether that line runs.** Before changing either
branch, read the early returns above it and find the callers.

---

## 2. X1 is closed — proven today, both halves

`HANDOFFS.md` row X1: *`run_automation.ps1:101` fails a green run on any
narrowed filter.* The code is already fixed — the marker requirement is gone and
replaced by a comment that says not to reintroduce it. **The row's own proof of
closure has now been executed:**

| run | exit | log |
| --- | --- | --- |
| `-TestFilter LLK.Enemies.SweeperBrain` | **0** | exactly one `Test Completed. Result={Success} Name={SweeperBrain}` |
| `-TestFilter LLK.Enemies.NoSuchTestAtAll` | **1** | `No automation tests matched`, plus `exited with code 255`, evidence saved to `Saved/Automation/History/failed-20261104-171333` |

A narrowed green filter exits 0; a filter that matches nothing still exits 1 with
the reason named. **Strike the row.**

It is not struck in `HANDOFFS.md` itself because that file has uncommitted
changes from another writer and row 27 is one writer per lane — splitting it
would be the `8196893` mistake. **Whoever next commits `HANDOFFS.md` should
strike X1 and cite this file.**

## 3. X2 — the presence half is closed, the value half is not

Row X2 asks that *"a deliberately dropped field fails the import, not a later
test. Proven red before it is called done."* **Proven red today, and it fails at
the earliest possible gate** — `Tools/import_data.py`, before the editor is even
launched. Run against a scratch copy of `data/` (nothing authored was touched):

```
dropped  Sweepers.aggroRadiusDark   → exit 1: DATA CONTRACT ERROR:
                                      enemies.json.Sweepers: missing required key(s): aggroRadiusDark
renamed  → aggroRadiusDarkk         → exit 1: same error
```

`exact()` at `import_data.py:83` checks **both** directions — missing *and*
unknown keys against an explicit required set — so a typo'd rename fails twice
over. That is better than the row asked for.

**What is still open is the failure mode that caused the row.** The original
defect was 17 of 32 numbers reaching `DT_Enemies` as zero while the JSON was
perfect — a **JSON-to-generated-asset** drop, which the contract gate cannot
see, because the authored file is correct in that scenario. On the editor side:

- `VerifyTable` (`LLKImportDataCommandlet.cpp:326`) still compares **row names
  and row count only**. Every field in every row could be zero and all seven
  tables would verify clean.
- `VerifyEnemyMoveSpeeds` (`:349`) covers exactly one field of one table, and its
  own comment is honest about that: *"Row names prove a table exists. They do not
  prove its gameplay values survived JSON conversion."*
- `VerifyRuleAssets` (`:377`) covers walk, sprint, crouch, radius, intensity and
  `warmColor.r`.

So the class fix is still instance-shaped. **The generalisation is one function:**
`VerifyEnemyMoveSpeeds` already does the right thing — reload the generated row,
compare field by field against the source JSON object — for one field. Widen it to
every numeric key the contract declares required, and `VerifyTable`'s name-only
check stops being load-bearing. You own this file.

## 4. Needs the editor, so it waits for whoever opens it next

- **The relic prop swap.** `BSW_WRONG_CRATE` and `BSW_DISPLAY_RECEIVER` both use
  `SM_Crate_Stack_1`, a floor-standing warehouse pallet, for objects
  `data/relics.json` authors as hand-held — all 20 authored relics are
  department-store objects and none is a crate. `PROPOSAL-BOSWELLS-RETAIL-FLOOR.md`
  names four cardboard boxes already in the pack at honest hand scale, including
  `SM_Box_Cardboard_30x30x30_Open` for "arcade tokens, six hundred". A swap of
  mesh and transform on two existing actors, positions unchanged. It needs the
  editor because `LLK_Boswells_FirstRun.umap` is monolithic — unlike
  `LLK_DockBay`, there are no One-File-Per-Actor files to edit from outside.
- **Two measured map defects**, both still open: the east side aisle is closed at
  y 470 by `WL_DarkPocket_Partition`, 60 cm against a 68 cm capsule; and
  `ToyBay_0` sits 16 cm through the west wall.
- **The runtime read** that is left in the anchor finding is now *only* about
  readiness, not the lantern: whether `LLK_Boswells_FirstRun`'s authored
  `PlayerStart` honours the floor-surface convention `LLKTestGameMode` builds
  against, or sits ~92 above the floor with its own capsule resting on it — in
  which case the harness store floor is built 92 cm high. It no longer changes
  anything about the light.

## 5. Chris's, not yours

- ~~Whether the 88 cm of pitch travel stays.~~ **Answered 2026-11-04: it stays.**
  `DECISIONS.md` row 81.
- ~~Does the sway join the gameplay anchor?~~ **CLOSED 2026-11-04: it does not,
  and there is nothing to build.** Row 82 stands as written, re-confirmed by
  Chris against the measurement rather than in ignorance of it: *"it buys 1.5 cm
  of invisible behavior while creating two meanings for 'lantern position'.
  That's not worth another seam or exception… No code change. Put the pendulum
  work into making the lantern feel carried, not into a behavior difference
  nobody can perceive."* The one-line change previously specced here is
  **withdrawn — do not build it**, and `GetGameplayLightLocation()` keeps its
  single meaning. `DECISIONS.md` O10 has the measurement that closed it (0.21%
  of detectability, 1.5 cm of a Sweeper's 12→38 stud aggro radius, and a relic
  read that cannot be revoked because legibility is sticky and debounced).
- **Whether `MaxOffsetCm` rises to fit a physically true sway**, or the amplitude
  is scaled down for first-person comfort. **Purely a comfort call since O10
  closed** — the sway reaches no gameplay reader, so there is no balance
  consequence on either side of it.
- **The retail floor: central island or re-zoning.** The recommended 55 cm wall
  run plus a 110 cm double-sided island gives two aisles of 282.5 cm, but it
  doubles the routes through the room and makes avoiding a Sweeper positional —
  a design change arriving dressed as an art change. Re-zoning keeps the 573 and
  costs nothing. His call, and the proposal says so.
- **Playing the dressed circuit at the new speed.** Never done. The 28 m room
  should now take about 12.5 s rather than 6.2, and sprint is genuinely 350 cm/s
  for the first time.
- The two stumble numbers on row X4, still as recorded.

## 6. What was deliberately not touched

`LLK_DockBay` and everything under it (yours, art pass, row 41, one owner per
map). `Tools/**` was read and run but not edited. `data/` was copied to scratch
for the X2 experiment and never modified — `git status data/` is clean and the
generated `DA_Lantern.uasset` still carries its 15:32:30 timestamp and the right
sprint. `Config/DefaultGame.ini` left alone. No gameplay code changed, which is
why item 1 is a handoff rather than a commit.
