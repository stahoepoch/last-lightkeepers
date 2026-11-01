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
is absent?* catches the silent majority. *What does this read on a quantity I
already know the answer to?* catches the confident minority, and it is the only
thing that has ever caught the confident ones.
