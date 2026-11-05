# Why extraction never completes — the chain is correct, so the fault is in the room

**Status: diagnosed to two candidates, 2026-11-05. Not fixed, because the next
step is one line of output from a PIE session and I cannot press Play.**

Chris, remote, 2026-11-05: *"I played through some things I still can't extract.
I can't figure that out."* That is the second report of the same thing — `M9`
already says *"the route plays but extraction has never been reached in a
playtest."* This file is the audit that was missing behind that sentence.

**The headline: every link in the chain verifies correct on disk.** I expected to
find a broken one and did not. So the fault is not in the data, the asset, the
subsystem, the zone class, the tick or the HUD class, and the remaining
candidates are both about the room rather than the code.

## The chain, link by link, as verified

| Link | Checked | Result |
| --- | --- | --- |
| The exit is authored | `data/loot.json` | `LoadingDock`, `arrivalSeconds` **20**, and **no `requires`** — so the requirement gate is not the blocker. `FreightLift` does require `SubBasementBreakerRestored` |
| 20 s reaches the hold | `LLKExtractionSubsystem.cpp:122` | `OutPoint.HoldSeconds = Point.ArrivalSeconds`. A zero here would refuse as `NoHoldAuthored`; it is 20 |
| The asset was built | `Content/Data/DA_LootRules.uasset`, 2026-11-04 15:32 | Byte-scan finds `LoadingDock`, `FreightLift`, `ArrivalSeconds` and `SubBasementBreakerRestored`. The gitignored-data trap (`X3`, `M12`) is **not** firing here |
| The subsystem loads it | `:41` | `LoadObject<ULLKLootRulesData>("/Game/Data/DA_LootRules.DA_LootRules")`, with a named warning when it is absent |
| A zone is in the map | `Content/LLK_Boswells_FirstRun.umap` | `LLKExtractionZone` present, `ExtractionPointId` serialized once, value `LoadingDock`. The id is **not** a typo, which is the one mistake placing the actor can make and which the zone already logs at `BeginPlay` |
| The zone registers | `LLKExtractionZone.cpp:107` | `Extraction->RegisterZone(this)` in `BeginPlay`, with `UnregisterZone` in `EndPlay` |
| Overlap can fire | `:84-88` | `QueryOnly`, object type `WorldStatic`, `ECR_Overlap` to `ECC_Pawn`, overlap events on. UE resolves a response pair to the weaker of the two, so Pawn-blocks-WorldStatic against WorldStatic-overlaps-Pawn gives an overlap and no block |
| The hold advances | `:359` → `:387` → `:390` | Ticks, and `Input.KeepersOnBoardingRoute = ObserveBoardingRoute()` → `Zone->CountKeepersStanding()` |
| A missing zone is loud | `:424-432` | *"EXTRACTION CANNOT COMPLETE: no ALLKExtractionZone registered"* — already written, already a warning |
| The readout exists | `LLKExtractionReadout.cpp:58` | Offers at the zone, reports a carried count, and reports `bHoldStalled` |
| The HUD is active | `LLKTestGameMode.cpp:117`, `Config/DefaultEngine.ini:8` | `HUDClass = ALLKDebugHUD`, and `GlobalDefaultGameMode` is `LLKTestGameMode`. The map has **no** GameMode override — `GameMode` appears zero times in the package — so the HUD that draws extraction is the one running |

## The two candidates that are left

**1. The zone's box is a guess, and the dock was built afterwards.** The extents
come from the constructor — 9 x 6 x 4 studs of half-extent, so a 504 x 336 x 224
cm volume — and `BoxExtent` appears **zero** times in the map package, meaning
nothing overrode it when the dock was greyboxed. The class comment predicted
exactly this: *"It is a default on an unplaced actor rather than a tuned value —
the real extents come with the greybox's dock."* They never came. If the box does
not cover the floor a player naturally stands on at the dock, there is no prompt
and no hold, and nothing is logged, because an un-entered zone is not an error.

**2. The 20-second hold has no authored feedback.** The beat is: stand in the
zone, hold 1 s (`CallHoldSeconds`), extraction is called, then **20 more seconds
of standing there**. `C9b` already records that `OnExtractionFailed` is broadcast
and bound by nothing, and `C2` that none of the Sweeper cues have a sound. So the
whole 21 seconds is carried by debug-HUD text. Walking out pauses the hold —
*"paused, not reset"* — and reports `STALLED` to the readout and nowhere else.
A player who calls it and steps away to deal with the Sweeper has not failed
anything; he has stalled something he cannot hear.

## The one line that separates them

`LLK.Extraction.Status` already exists and prints phase, point, siren, hold
progress and `STALLED`. **No new code is needed.** In PIE, walk to the dock, try
to extract, then run it:

| Output | What it means | Where the fix is |
| --- | --- | --- |
| `Idle at nowhere` | The call never happened. The zone never saw you | Candidate 1. Resize or move the trigger to the greyboxed dock |
| `Announcing`, siren counting down | It is working. Keep standing there | Nothing is broken; candidate 2 is the whole problem |
| `Holding ... STALLED` | You called it and you are not in the box | Candidate 1, with the box close enough to enter once |
| `Holding`, climbing | Working, and slower than it feels | Candidate 2 |
| `Secured` | **It already worked and said so only in debug text** | Candidate 2, and it is the more urgent half |
| The `EXTRACTION CANNOT COMPLETE` warning | The zone is not registered at all | Not candidate 1 — the actor or its `BeginPlay` is the problem |

`LLK.CheckMap` is the other half, since it reports map readiness and already
exists for this class of confusion.

## What this is not

**It is not the gitignored-data trap.** `Content/Data/` is gitignored and `X3`
and `M12` are both about exactly this failure, so it was the first suspect. The
asset is present and contains the exits. Worth recording as a negative result,
because the next person will suspect it too.

**It is not the requirement gate.** `SetRequirementSatisfied` has exactly one
caller in the whole project and it is a console command, so **nothing in
gameplay can ever satisfy a requirement.** That is a real latent defect and it is
written up as its own row — but `LoadingDock` authors no requirement, so it
cannot be what Chris is hitting. It is what he *would* hit at the freight lift.

**And it is not a knockdown.** Chris: *"I get knocked over by the sweeper."* With
`knockdownOnContact` **false**, three contacts inside the decay window raise a
floor alert, not a collapse — `LLKSweeperBrain.cpp:359-368`. What he is feeling
is the stumble from `X4`: speed x 0.55 for 1.2 s, on all three gears. Being
downed *would* drop an extraction hold, so this mattered to rule out; it does
not. **But he read a speed multiplier as being knocked over**, which is worth
more than the ruling-out: the stumble has no cue, so the only evidence of it is
that you suddenly move wrong. `X4` says it must cost *"distance and tempo, never
input"*, and a cost the player misidentifies as a knockdown is one they cannot
steer through on purpose.
