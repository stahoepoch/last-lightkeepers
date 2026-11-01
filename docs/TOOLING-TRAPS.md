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

---

## The pattern, stated once

Each of these is a true answer to the wrong question. The build tool answered
"did I exit", the import script answered "were there any errors logged
anywhere", the test runner answered "did the binary on disk pass", the port
answered "is a server up", the guard answered "what is in the world right now",
and the level query answered "which world can I see from inside this play
session", and the capture tool answered "what was drawn last".

So the question to ask of any green signal is not whether it is true. It is
**what would this have said if the thing I care about were broken?** Where the
answer is "the same thing", the signal is decoration. That is what a red proof
buys and a count does not.
