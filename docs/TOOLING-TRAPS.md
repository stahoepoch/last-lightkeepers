# Signals that do not mean what they say

A standing caution, not an incident log. Every entry here is a success signal
this project has already trusted and been wrong about, and together they are
the reason an enemy roster with no senses at all survived six playtests on
2026-10-31 while every guard reported green.

The shape is always the same: **something answered a question nearby to the one
being asked**, and the answer was true.

---

## 1. `Build.bat` exits 0 when the build failed

Refused build, 2026-10-31. The script printed:

```
Unable to build while Live Coding is active. Exit the editor and game, or
press Ctrl+Alt+F11 if iterating on code in the editor or game

Result: Failed (OtherCompilationError)
```

and exited **0**.

So any script, hook, CI step or agent that branches on `$?` reads a failed
build as a success and then tests the binary that was already on disk.

**Read Unreal's own `Result:` line.** It has been the only build signal that
has held up. `Result: Succeeded` also covers staleness: a build that is already
up to date says so, and one that could not run says `Failed`.

### Correction, 2026-11-02: the exit code did not reproduce

Re-measured, because this entry is one people act on. `Tools/build_editor.ps1`
was run against an open editor, producing the identical refusal:

```
Unable to build while Live Coding is active...
Result: Failed (OtherCompilationError)
```

and it exited **6**, not 0.

Same wrapper, same `Build.bat`, same Live Coding refusal, a different exit
code from the one recorded above. **One of the two observations is
conditional on something neither of them wrote down** — most likely the
invocation path, since the 2026-10-31 run is not recorded as having gone
through `build_editor.ps1`.

**What does not change:** read the `Result:` line. It was correct on both
occasions and it is the signal this project relies on.

**What does change:** do not invert the lesson. A reader who takes "exits 0 on
failure" as a rule might treat a non-zero exit as noise, or build a check that
expects zero to mean failure. **The exit code is unreliable in an unknown
direction, which is weaker and more honest than unreliable in a known one.**
Trust `Result:`, and treat a non-zero exit as a reason to look rather than as
information in itself.

This entry is kept rather than rewritten because a trap that stopped
reproducing is itself worth recording — the failure it describes was real when
it was written, and the gap between the two runs is a fact about this
toolchain.

## 2. `Tools/import_unreal_data.ps1` exits non-zero when the import succeeded

Found by Codex the same day, and the exact mirror of the above: the MCP and
GameFeature plugins log two unrelated commandlet errors, the script propagates
them, and a complete and correct import of 7 DataTables and 4 DataAssets
reports failure.

Both directions of one disease. An exit code that is wrong in the optimistic
direction hides breakage; one that is wrong in the pessimistic direction trains
people to ignore the signal, which hides the next breakage. The second is worse.

## 3. A passing test suite does not mean your code was tested

A test run taken immediately after the refused build in (1) reported **33
tests, all successful** — using the DLL already on disk. It did not run the
assertions that had just been written, and nothing in its output said so.

**The reliable check is the red proof, not a count.** Mutate the thing under
test, run, and require the expected failure. If it does not appear, the binary
is not yours. In that run `LLK.Enemies.PerceptionFailures` reported `Success`
while `HasReachingSense` had been mutated to `return true;`, which is
impossible for a current binary — the receipt was sitting in the output and
went unread.

**Retracted, explicitly, because it was briefly trusted:** a claim that the
suite counts 44 tests and that seeing 33 proves a stale build. The suite is
**33**. `Source/LLK/Private/Tests/` holds exactly 33
`IMPLEMENT_SIMPLE_AUTOMATION_TEST` macros across 31 files, at every commit of
2026-10-31 and at `HEAD`; every full run in every log in `Saved/Logs` reports
33 completed, and no log anywhere reports 44. The 44 was wrong rather than
stale — it most likely came from conflating assertion counts with test-case
counts, the node suites having their own 33 among them, but that reconstruction
is guesswork and the receipts are not. **A count heuristic would have waved a
genuinely stale build straight through**, which is worse than having no
heuristic, because a tripwire people trust is load-bearing.

## 4. A listening port does not mean the right world is open

The unreal-mcp bridge answered on its port while the editor sat on
`/Temp/Untitled_1` rather than `LLK_DockBay`. A live connection would have read
the wrong world and answered confidently about it. See
`docs/PREREQS-AUTHORED-MAP-SEAM.md` — the bridge is also started by hand with
`ModelContextProtocol.StartServer`, so "it worked yesterday" says nothing about
today.

Check what is loaded, not whether something responded.

## 5. `WorldAlreadyHas` asks before the answer exists

On a World Partition map, a GameMode's "does this world already have one?"
check at init runs before the authored external actors have streamed in. On
2026-10-31 the harness extraction zone therefore won registration and the
authored `LLK_LoadingDock` was rejected as the duplicate — the exact reverse of
the guard's intent, with a warning that read as if the guard had worked.

## 6. An empty answer is not an empty editor

`SceneTools.get_current_level` returned `""` on 2026-10-31 while
`LLK_DockBay` was open and fully loaded. The reason is in the log one line
later:

```
LogUtils: Error: The Editor is currently in a play mode.
LevelEditorSubsystem: Error: GetCurrentLevel. Can't Get the current level
because there is no world.
```

Chris was in PIE. The editor world was exactly what it should have been; the
*editor* world simply is not what the tool can see from inside a play session.

This one is worse than it looks, because the standing rule for the day was
"confirm which world is open, and if it is anything other than what you expect,
stop and report." An empty string reads as **no map loaded**, which would have
produced a confident report of the wrong state — and the plausible second guess,
that World Partition hides the persistent level's path, is also wrong. Neither
error is visible from the return value alone.

**Enumerate the world, don't ask it its name.** `find_actors` answered
correctly through the same bridge seconds later, because actor paths carry the
package name (`/Game/LLK_DockBay.LLK_DockBay:PersistentLevel.…`). Where a
one-field query can be empty for several unrelated reasons, prefer the call
whose answer contains its own evidence — and check `IsPIERunning` before
believing any editor-state reading.

## 7. A screenshot is not a screenshot of now

Driving `LLK_LightLab` on 2026-10-31, every capture came back **byte-identical**
— md5 `c983014fe6d3`, 437,313 bytes — across camera moves, across
`CaptureViewport` and `CaptureEditorImage`, and across separate MCP calls. The
editor was not rendering: `Responding: True`, **0.4 seconds of CPU over six**,
no shader workers. A capture tool on a stalled editor hands back **the last
frame that happened to be drawn**, with no indication that it is old.

Two distinct shapes of the same trap:

- **Batched captures never update at all.** A script that moves the camera and
  captures, repeatedly, runs entirely on the game thread, so no frame is drawn
  between the steps. Two poses in one script returned the identical image. A
  forty-eight shot matrix gathered this way would have been forty-eight copies
  of one frame, each filed under a different candela — numbers with pictures
  attached, and every picture the same.
- **Unbatched captures are worse, because they mostly work.** Separate calls
  succeed often enough to look reliable, then silently serve a stale frame when
  the editor goes idle.

**The check is a negative control: move the camera, capture, and require the
image to differ.** Hash it. If two poses hash the same, nothing you capture
afterwards is evidence. This is the red proof from entry 3 applied to pixels —
and it is the only reason the lab's ladders were reported as *not taken* rather
than reported as data.

## 8. A reading is not a reading until the zero reads zero

`LLK_LightLab` is a sealed box with no sky, no directional light, no skylight
and no fog — the only light in it is the fixture under test. On 2026-10-31 its
price card was readable at 2 candela, which looked like a finding: an
inspection threshold below the bottom of the ladder.

Then the negative control was run. **With every light set to 0 cd the room is
still plainly visible** — walls, floor and both gondola blockers
(`docs/lightlab/control-00cd-all-lights-off.png`). The card reads with the
lights off. The "2 cd" result was a measurement of an ambient floor nobody had
looked for, wearing a candela value as a label.

**Zero your instrument before you trust it.** Every measuring rig needs a
reading where the thing being measured is absent, and that reading has to come
out as nothing. This costs one capture and it is the difference between a
threshold and a number with a unit stapled to it.

**And the follow-up was worse than the leak.** Holding the camera still and
taking the same shot at 0 cd and at **5000 cd** produced images that were
**pixel-identical — mean absolute difference 0.0, maximum difference 0**. There
was no ambient floor drowning the fixtures; the fixtures were contributing
nothing at all, because the captured viewport was rendering albedo without
lighting. Every "candela" in that series was a label on an unlit picture.

The general form: **a control that only tests for darkness cannot tell you the
instrument is disconnected.** Zeroing proves the floor; it takes a *maximum* —
an absurd value the rig could not possibly ignore — to prove the input is
wired to the output at all. Run both ends. A rig that answers the same at 0 and
at 2500× the top of your ladder is not measuring your variable.

The sharpest part: the contaminated ladder would have been *monotonic and
plausible*. Brighter candela did give brighter frames. Nothing about the series
looked wrong — it would simply have had a floor baked into every rung, and the
thresholds derived from it would have been confidently, consistently false.

## 9. A capture succeeded, of the wrong window

Driving `LLK_LightLab` on 2026-11-01, `CaptureEditorImage` returned a clean,
well-formed, correctly-sized image **of the Material Editor**. Chris had left
`M_LLK_LanternCore` open, and its tab — not the level viewport — was frontmost.
The capture did exactly what it says: an image of the editor as the user sees it.

The trap is what the next step would have been. The task was a `CoreEmissive`
ladder, and the Material Editor shows **a preview sphere of the same material**.
Every rung would have brightened, monotonically and plausibly, and the ladder
would have measured the preview thumbnail while claiming to measure bounce in a
room. Camera moves would have had no effect and nothing would have said so.

**Prefer the capture that names what it captured.** `CaptureViewport` returns
`cameraLocation`, `cameraRotation` and `cameraFOV` **with the image**, so each
frame carries proof of the pose it was taken from. That killed the long-standing
"`SetCameraTransform` succeeds and `GetCameraTransform` agrees while the capture
keeps the old pose" problem outright — not by fixing it, but by making the answer
contain its own evidence. Same lesson as entry 6.

## 10. A black frame is not a dark room

With every light at 0 cd and the lantern core's emissive at an absurd 500, the
lab rendered a blazing sphere surrounded by black. Read as "the emissive lights
nothing", which was in fact the right answer — but **not yet an earned one**, as
a camera pointing into empty space, out of the sealed box, or into the back of a
wall produces the identical picture.

A negative control proves the floor is dark (entry 8). A positive control proves
the instrument is wired (entry 8's follow-up). Neither proves **there was
anything in front of the lens**.

`GetVisibleActors` answered it: 16 actors in the frustum, ten of them static
meshes. `ScreenCoordsToWorld` answered it better, because it answers per
measurement region — every sampling box used in the ladder was traced and
returned a solid hit at 116–316 cm, naming the wall, ceiling and floor it landed
on. One box believed to be "far background" turned out to be a wall **116 cm
from the camera**, which made its flat zero far stronger evidence than assumed.

**Trace what your measurement region is actually pointed at.** A region named in
screen coordinates is a guess about geometry until something resolves it to a
surface.

## 11. Bloom looks exactly like bounce

The same 500-emissive frame shows a broad warm glow filling much of the screen.
It reads, instantly and convincingly, as a lantern lighting a room. It is
post-process bloom, and no surface in the image receives a single photon.

What separates them is not brightness but **what the brightness is a function
of**. Two patches of one flat wall, 86 cm and 261 cm from the source, read 153
and 2.2 — a 70x difference where inverse-square predicts about 9x. The value
tracks distance from the source **on screen**, not in the world. The other tell
is negative: a lit room has edges, corners and a cast shadow, and the frame had
none — just a smooth radial gradient.

This one is dangerous because it fails in the flattering direction. A lighting
result that looks warm and plausible invites you to stop measuring, and the
author who set the value is the least likely person to doubt it.

## 12. The offscreen capture renders a lit room with no global illumination

On 2026-11-01, with Lumen confirmed live, `CaptureViewport` was used to
photograph `LLK_LightLab`. It returned a correct, well-exposed, convincingly
lit image: the price card legible, the cool-white troffer colour right, a soft
contact shadow under the card. Nothing about it looked wrong.

It has no GI in it. The controlled test was the same scene, same settings,
same instrument, measured before and after Lumen was switched on:

| emissive 20, all lights 0 | wall @86cm | wall @261cm | ceiling |
| --- | --- | --- | --- |
| `CaptureViewport`, pre-Lumen | 11.278 | 0.000 | 0.993 |
| `CaptureViewport`, post-Lumen | 11.207 | 0.000 | 0.986 |
| **on-screen viewport, post-Lumen** | **70.3** | **3.14** | **13.3** |

The offscreen path did not move at all across the biggest renderer change the
project has ever made, while the on-screen viewport moved by a factor of six.
Offscreen scene captures have no temporal history and Lumen is temporal, so
direct light and bloom survive the trip and indirect light does not.

**Why this one is nastier than entry 7.** A stale screenshot repeats a frame,
so a hash check catches it. This returns a *fresh, correct, different* image
every time, responds properly to every parameter you change, and produces a
clean monotonic ladder. Every internal consistency check passes. The only way
to catch it is to measure a quantity you already know the answer to on another
instrument.

**So: never measure global illumination with an offscreen capture.** Measure
from the on-screen viewport, which means the editor window has to be genuinely
visible — `CaptureEditorImage` photographs the window, so anything covering it,
this app included, silently freezes the frame (entry 7 again, by a different
route). Two capture paths, each broken in a way the other is not.


---

## 13. While PIE runs, the scene tools answer about a world you are about to throw away

Found 2026-11-02, while proving a newly tagged `LLK.StockBay` actually received
a filed relic.

Querying the level during a PIE session returns the **PIE duplicate**, not the
editor world. The refPath says so plainly once you look:

```
/Game/UEDPIE_0_LLK_Boswells_FirstRun.LLK_Boswells_FirstRun:PersistentLevel.LLKRelicActor_1
```

**For reading, this is the feature.** It is how the relic's landing position was
confirmed at `(450, 450, 50)` while the game was still running - the editor
world's copy had not moved and never does, because `SetActorLocation` happened
in the play world.

**For writing, it is a trap with the familiar shape.** A `set_actor_transform`
issued while PIE is up edits the duplicate, which is discarded on `StopPIE` -
and the read-back *confirms the write*, because it reads the same doomed world.
The claim and its check are once again the same act, and this time both are
about the wrong world. A restore done one call too early would have reported
success and silently left the editor map holding a test value, which is exactly
the state that then gets committed.

**So assert the world before any write to a map:**

```
IsPIERunning() == False        # or abort
get_current_level() == <the map you mean>
"UEDPIE" not in actor.refPath  # belt and braces, since the path tells you
```

This is the same guard as trap 6 - an answer about the wrong map - except that
here the wrong map is one the tooling created for you, is named almost
identically, and exists for only a few seconds.

## 14. A failing automation test exits 0

Found 2026-11-02, while checking a green by inverting an assertion.

`UnrealEditor-Cmd.exe <project> -ExecCmds="Automation RunTests <name>"
-TestExit="Automation Test Queue Empty"` reports the outcome in the log and
**not** in its exit code. Measured, same command, same session, two runs:

| run | log says | exit code |
| --- | --- | --- |
| assertion intact | `Result={Success} Name={SweeperBrain}` | **0** |
| assertion inverted | `Result={Fail} Name={SweeperBrain}` | **0** |

So any hook, CI step or agent that branches on `$?` or `$LASTEXITCODE` reads a
**failed test suite as a passing one**. The failure is fully reported - by name
and by line - in the log the exit code is hiding.

**Use `.\Tools\run_automation.ps1`, which already does this.** Codex's runner
parses the log rather than the exit code, and the comment at its line 67 said
so before this was measured. The failing run above emitted
`LogAutomationController: Error: Test Completed. Result={Fail}`, which that
script's pattern catches - so this trap was already mitigated and the way to
step in it is to hand-roll `UnrealEditor-Cmd` instead of calling the script,
which is what happened here. One caveat while it stands: that runner also
hard-requires a marker only `LLK.Harness` emits, so a narrowed `-TestFilter`
currently fails a green run. Handed to Codex in `NOTES-FOR-CODEX.md`.

**If you must parse it yourself: parse the log.** `Result={Success}` or
`Result={Fail}` per test, plus `N tests performed`, is the signal. Grep for `Result={Fail}` and treat *zero
`Test Completed` lines at all* as a failure too, since a run that never started
the test also exits 0 and says nothing - the instrument's absence looking like
a reading, for the third time on this page.

**This is trap 1's sibling and the stronger case.** Trap 1 is about a *build*
whose exit code could not be trusted in a known direction; this is a *test*
whose exit code carries no information whatsoever. The two together mean the
rule for this toolchain is simply: **exit codes are not results here. Read what
the tool said.**

## 15. Live Coding blocks the EDITOR target, not the game target

Found 2026-11-02 by the other session, and it retires a blocker this project
has hit four times.

`UnrealBuildTool` refuses with *"Unable to build while Live Coding is active"*
only for the target the editor is holding - `LastLightkeepersEditor`. The
**game** target, `LastLightkeepers`, builds the same `Source/LLK` module and
is not blocked. So with the editor open:

| target | blocked by Live Coding | covers |
| --- | --- | --- |
| `LastLightkeepersEditor` | **yes** | `Source/LLK` + `Source/LLKEditor` + automation tests |
| `LastLightkeepers` | **no** | `Source/LLK` - the whole runtime module |

That is real compile coverage rather than a near-miss: anything outside
`LLKEditor` is genuinely checked. `bb50592` was committed unbuilt for exactly
this reason and then confirmed clean against the game target without anyone
closing the editor.

**What it does not cover:** `Source/LLKEditor` (the import commandlet, the
validators) and the automation tests, which live in the editor target. A green
game target is not a licence to claim `LLK.Enemies.SweeperBrain` passes - that
still needs the editor closed, and trap 14 still applies to how you read the
result.

## 16. "The editor is closed" is a precondition, not a blocker

Mine, 2026-11-04, and it cost a whole requested task.

Asked to regenerate `DA_Lantern.uasset` and `DT_Enemies.uasset` after the
movement rescale, I checked for a running editor, found none, and reported the
work blocked. It was not blocked. `Tools/import_unreal_data.ps1` runs
**`UnrealEditor-Cmd.exe -run=LLKImportData`** — a headless commandlet — and its
line 22 is:

```
if (Get-Process UnrealEditor ...) {
    throw 'Unreal Editor is open. Close it before importing generated data assets.'
}
```

**No editor running is the required condition.** The task I called impossible
took 0.26 seconds and wrote 7 DataTables and 4 DataAssets.

The same is true of `Tools/run_automation.ps1`, which drives the same
`UnrealEditor-Cmd` binary. So the class of work I had written off covers data
regeneration and the entire automation suite — most of what this project
actually verifies.

**What makes this worth an entry is that the doc already said so.** Trap 2 is
about this very script. Trap 15 ends *"that still needs the editor closed."*
The fact was recorded twice. I did not fail to have the information; I failed
to ask the question it answers, because I had collapsed two different things
into one:

| The question I answered | The question I was asked |
| --- | --- |
| Is the MCP bridge reachable? | Can this Unreal work be done? |

The bridge needs a **live editor with a loaded level**, because it manipulates
actors in an open world. A commandlet needs the opposite. Treating "no editor"
as one state produced a confident, wrong blocker — the same shape as every
entry above, with the instrument's absence standing in for a reading, except
here the absent instrument was the only one that *could not* do the job.

**So the rule: before reporting Unreal work blocked, name the binary it
needs.** If the answer is `UnrealEditor-Cmd.exe`, a closed editor is the
green light. The three capabilities and their opposite requirements:

| Capability | Binary | Editor must be |
| --- | --- | --- |
| Scene reads/writes, actor placement, PIE | MCP bridge in `UnrealEditor.exe` | **open**, with the right level loaded |
| Data asset regeneration | `UnrealEditor-Cmd.exe -run=LLKImportData` | **closed** |
| Automation tests | `UnrealEditor-Cmd.exe` via `run_automation.ps1` | **closed** |
| C++ build | `Build.bat` / `build_editor.ps1`, editor target | **closed** (trap 15) |

Only the first row is what "the editor is closed, do what you can" rules out.

## The pattern, stated once

Each of these is a true answer to the wrong question. The build tool answered
"did I exit", the import script answered "were there any errors logged
anywhere", the test runner answered "did the binary on disk pass", the port
answered "is a server up", the guard answered "what is in the world right now",
and the level query answered "which world can I see from inside this play
session", the capture tool answered "what was drawn last", and the screenshot tool
answered "what is the user looking at".

So the question to ask of any green signal is not whether it is true. It is
**what would this have said if the thing I care about were broken?** Where the
answer is "the same thing", the signal is decoration. That is what a red proof
buys and a count does not.

### Most of these are one mechanism: the instrument's absence looks like a reading

Say it precisely, because the precise form is checkable and "be careful" is not.
**When the instrument is not there, it returns a value that is indistinguishable
from a measurement.** The failure mode is silence, and silence looks like an
answer:

| the instrument is absent | and it returns | which reads as |
| --- | --- | --- |
| build did not run (1) | exit 0 | success |
| binary was not rebuilt (3) | 33 tests pass | your code passed |
| wrong world queried (6) | empty list | no such actors |
| viewport not redrawing (7) | the previous frame | unchanged |
| nothing in front of the lens (10) | a black frame | a dark room |
| LFS content not fetched (§11 search) | no grep matches | no overrides exist |

**So the step-zero question is: what does this test return when the instrument
is not there at all?** If the answer is anything but an error, the test is
unfinished.

And the fix is always the same construction rather than more care: **carry a
control through the same path in the same run**, so absence produces a visible
failure instead of a null. The 0 cd versus 5000 cd gate, the capture-liveness
proof, and the LFS pointer-header check are all that one move.

**The rule, which is checkable in a procedure rather than left to the reader:
no measurement may record an empty or unchanged result as data. Any null
becomes an explicit failure before it reaches a table.**

### The exception, which needs a different guard

Three entries here are *not* silence, and the rule above will not catch them.
**2** is the inverse — a correct import reporting failure. **11** and **12** are
worse: a signal that is present, confident and wrong. Bloom is really there and
really brightens the wall; the offscreen capture really renders a lit, exposed,
plausible room. No null appears anywhere, so no null-check fires.

Those were caught a different way, and both times by the same move: **check one
quantity against something outside the instrument.** Bloom was caught against
inverse-square — two patches of one flat wall differing by 70x where physics
allows 9x. The offscreen capture was caught against the on-screen viewport — the
same scene reading 11.2 where the other instrument read 70.3, and 99.06 where
it read 10.33.

So there are two questions, not one. *What does this return when the instrument
is absent?* catches the silent majority. The second catches the confident
minority, and it is the only thing that ever has — but it has to be put
precisely, because the obvious version is weaker than it looks.

### Forced, not merely known

**A second instrument raises the question. Only something that cannot be
otherwise answers it.** The offscreen capture was first suspected because it
disagreed with the on-screen viewport, 11.2 against 70.3 — but a disagreement
tells you one of them is lying and not which. The on-screen viewport had no
more authority than the offscreen one; it was simply the other reading.

What actually settled it was an impossibility. With the shelf lip shading the
card, the offscreen capture read **99.06**. The *unshaded* card at the same
ambient reads **92.29**. Adding an occluder cannot make a surface brighter, so
the offscreen value was refutable **without reference to the other instrument
at all**. That argument was available at the time and was not the one used.

So the question is: **what does this read on a quantity whose answer is
forced?**

### Orderings are the cheap forced quantity

In practice the forced quantities here are not absolute values — nobody has a
correct absolute model of what the card *should* read at 6 cd. They are
orderings, and those are free:

- a card at 6 cd must read brighter than the same card at 3
- a shaded card must read darker than an unshaded one at the same intensity
- a surface at twice the distance must fall by roughly four

An ordering check costs **one extra capture**, needs no correct model, and
catches a confident failure as cleanly as a law does — because a wrong result
that happens to respect every ordering is far rarer than one that merely looks
plausible.

**So: every ladder includes at least one pair whose ordering is forced, and the
ordering is verified before any value is recorded.** This has already worked
once before it had a name: the 25 cd lantern row was discarded for breaking the
monotonic trend, which is how its wrong window size was noticed.

### Two rules that cover today's errors, including both of mine

**A check must be run where the measurement is sensitive, not where it is
convenient.** The capture-geometry error moves a structured region by 1.76x and
a uniform one by 5%. So a calibration patch on a flat swatch would have read
27-28 throughout and certified a broken instrument — the same shape as
byte-identical frames certifying a frozen one. Validating on the easy region
proves nothing about the region you are reading.

**A number read from the engine carries a frame of reference, and the frame has
to be read rather than assumed.** Three separate errors in one day were one
error:

| the number | the frame assumed | the frame it was in |
| --- | --- | --- |
| `18:18:44` in the log | local time | **UTC** — the restart preceded both runs |
| `bMegaLights = true` in DockBay | in effect | **dormant**, its override was off |
| a pixel inset in the crop | a known window size | **three different sizes**, silently |

Each produced a confident wrong claim, two of which reached commit messages.
"Do not assert causes" is the wrong lesson, because it does not say what to
check. **Read the frame** does.

### Why this is worth an extra capture every time

The two failure classes do not cost the same. **A silent failure wastes a run.
A confident failure gets written into a findings table and becomes the basis
for a design decision** — which is what the fixture band, the shelf lip and the
contract remedy all are. Entry 2, a correct import reporting failure, is the
cheap direction of the same axis: it costs a run and nothing else.

---

## 17. "The editor is open" is a state you can measure, not a wall

Trap 16 recorded that *the editor is closed* is a precondition rather than a
blocker. This is the same mistake from the other side, made by the same session
on the same day: **the editor was open, and I treated that as immovable for
hours without ever checking whether it was doing anything.**

It was not. The editor's own log said so, for free, with no editor needed:

```bash
# nothing has been modified or saved in this session
grep -cE "MarkPackageDirty|was modified|SavePackage|LogFileHelpers" \
  Saved/Logs/LastLightkeepers.log      # -> 0

# and the last four hours are housekeeping only
tail -20 Saved/Logs/LastLightkeepers.log
# LogDerivedDataCache: Maintenance finished ... (hourly, x4)
```

Zero dirty-package lines and four consecutive hours of hourly DDC maintenance
is an **idle** editor holding a lock, not a colleague mid-edit. UE also writes
its own verdict to `Saved/Autosaves/PackageRestoreData.json`
(`"RestoreEnabled": false, "Packages": []` when nothing needs restoring),
though that file is only rewritten when autosave evaluates, so it can be stale
— the log is the live signal.

### Closing it without risking anyone's work

`CloseMainWindow()` sends `WM_CLOSE`, which is exactly what clicking the X
does, so the editor runs its own shutdown and **prompts instead of discarding**
if anything is dirty:

```powershell
$p = Get-Process -Id <pid>
$p.CloseMainWindow()
$p.WaitForExit(90000)   # false => a modal dialog is up and nothing was lost
```

A clean exit with no prompt is the second, independent confirmation that
nothing was dirty. `Stop-Process` is a kill and gives neither the prompt nor
the confirmation — do not reach for it.

### Why this cost a whole session

Three capabilities need the editor **closed** (data import, automation, the
EDITOR build target — see trap 16's table) and exactly one needs it **open**
(the MCP bridge, for scene reads and PIE). The bridge also needs
`ModelContextProtocol.StartServer` typed into the editor console by a human at
the machine. So with Chris remote, holding the editor open preserved the *only*
capability that was unreachable anyway, while blocking the three that were not.

**The arithmetic is worth doing before waiting:** count what the current state
blocks against what it enables, and check whether what it enables is actually
reachable. Here it was 3 against 0, and a `grep` of a log file was the whole
investigation.

### The general shape

Both traps are the same error. A precondition was read as an obstacle because
it arrived worded as one — "the editor is open", "the editor is closed" — and
in neither case was the state itself examined. **A tool's required environment
is a fact to check, not a verdict to accept.**

## 19. A byte scan answers "is this value here", never "what value is here"

`M6` read a transform out of a `.umap` with a byte scan and it worked. On
2026-11-05 the same tool, pointed at the same package, produced three different
coordinates for one actor across two passes. The tool did not change. The
question did.

**The scan that worked asked a closed question.** It searched for the bay's
**known** authored triple, `(450, 450, 50)`, and it carried a control: the
crate's `(300, 250, 90)`, which was expected to be found and was. When the first
attempt found neither, the control said why — it had searched float32 against
double-precision data — and the finding was a corrected method rather than a
false loss. A hit is evidence the value is present. A miss with a live control is
evidence it is absent.

**The scan that failed asked an open one.** *"Where is `ALLKExtractionZone_0`?"*
has no expected answer, so there is nothing for a control to control. The method
used proximity to the actor's name as the link between a name and a number, and
proximity in a serialized package is not ownership: the window caught
neighbouring actors' data, and three plausible triples came back with no way to
rank them. Widening or narrowing the window changes the answer, which is the tell
that the method has no ground truth in it.

| Question | Sound? | Why |
| --- | --- | --- |
| Is `LLK.StockBay` still in this package? | **Yes** | Presence of a known string, and absence is meaningful |
| Is the bay still at `(450, 450, 50)`? | **Yes, with a control** | Known value, and a second known value proves the scan can see its own data type |
| Does this actor's `ExtractionPointId` read `LoadingDock`? | **Yes** | One authored string, and the set of legal values is tiny and known |
| What is this actor's transform? | **No** | Open question. Nothing links the number to the name except proximity, and proximity is not ownership |
| Was this property overridden in the map? | **No** | A literal-string count cannot see the name table, so it reads the same whether or not an override exists |

**The rule.** Use a byte scan to confirm or refute something you can already
state. The moment the output is a number you did not bring with you, you are
reading tea leaves in hex, and the output is worse than nothing because
coordinates look like findings. Open the editor, or say it is unknown.

That last row is its own small lesson: the first draft of
`FINDINGS-EXTRACTION-NEVER-COMPLETES.md` argued from `BoxExtent` appearing zero
times in the package. It reached the right conclusion through an instrument that
could not have reached any other one.


---

## 20. `unreal.log` does not reach stdout, and row 19's ban on reading transforms has one structural exception

Both found on 2026-11-09, applying phase 1 of
`docs/PLAN-AUDIT-BOSWELLS-ANCHOR.md` through
`Tools/run_editor_python.ps1`, with no editor open.

**The signal.** A headless `-run=pythonscript` run was piped to a log. The
log contained the engine's own startup chatter and **not one line the script
printed**. The obvious reading is that the script did nothing. It had
re-seated 22 actors in memory and was about to save.

`unreal.log`, `log_warning` and `log_error` go to
`Saved/Logs/LastLightkeepers.log`. They do not go to stdout, even with
`-stdout`, which forwards the engine's log categories and not the Python
shim's. Every script in `Tools/` that runs this way now prints to both.

**The near-miss beside it.** The first run exited **255** on a Python
exception, so a non-zero code here is real - better than row 1's `Build.bat`.
What is still not safe is reading **0** as success: a script that loads the
wrong map, refuses its own precondition check, or applies every edit and then
fails the save all exit 0. Read the line the script prints about whether it
saved. `fix_anchor_map.py` prints `SAVED` with counts, or an error saying the
edits were applied in memory and the file on disk is unchanged.

### The exception to row 19, and why it is one

Row 19 says a byte scan can confirm a value you brought with you and must
never be asked *"what is this actor's transform?"*, because the only link
between a name and a number is proximity, and proximity in a serialized
package is not ownership. **That reasoning is correct about the method it
describes and does not cover a structured record.**

A saved non-partitioned level carries one descriptor per actor: a
`path,class,base64,None` record whose decoded bytes are the actor's guid,
its class name as a length-prefixed string, then **exactly ten doubles** -
quaternion, translation, scale - then its editor label. The label and the
numbers are inside one record of known layout. Nothing is inferred from
distance. `Tools/read_map_transforms.py` reads this, and it is what produced
the audit's part A.

**It was validated rather than asserted.** The decoder predicted 22 specific
wrong Z values; the editor, loading the same map through its own API,
reported those same 22 values as it corrected them; the decoder then read the
corrected map back and agreed with the editor's arithmetic to the centimetre
on all 22. Three independent passes over one fact.

Two limits stay, and they are the reason this is an exception and not a
replacement for opening the editor:

- **It reads the descriptor index, not the export table.** The descriptors
  are written from the actors on save, so they agree on a map the editor
  saved. A map written by anything that does not refresh them would read
  stale here while being right in game.
- **A transform is not collision and not bounds.**
  `FINDINGS-BOSWELLS-GEOMETRY.md` already separates those two; this is a
  third thing again, and all three can disagree. The audit's claim that a
  body cannot climb the west ramp rests on `/Engine/BasicShapes/Cube`
  carrying its default collision, which is an assumption a trace would
  settle and a decode never can.

**The rule, amended.** Ask a byte scan for a value you brought with you, or
for a field inside a record whose layout you can state and whose name sits in
the same record. Never ask it for a number you found by looking near a name.
