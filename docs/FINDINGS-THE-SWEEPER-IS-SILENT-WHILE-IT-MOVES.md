# The Sweeper is silent while it moves

**Found 2026-11-21**, immediately after the navmesh work made it possible to
measure. **Fixed and verified in a running game the same evening**, once Chris
closed the editor that was blocking the import. `HANDOFFS.md` M19.

---

## The measurement

With navigation finally built into `LLK_Boswells_OpeningPass`, the startup
report prints the one number this finding is about:

```
LogLLK: LLKSweeper_0 can reach the keeper: 11962 cm of path over 4850 cm
        straight (2.47x), 71.2 s at 168 cm/s.
```

**Seventy-one seconds of approach, in total silence.**

`ALLKSweeper` makes sound in exactly one way: `MulticastPlayCue` calls
`UGameplayStatics::PlaySoundAtLocation` at a fixed world position when a cue
fires — telegraph, stock started, stock interrupted, stock filed. Four
one-shots. There is no audio component on the pawn, no footstep, no motor, no
looping anything:

```
$ grep -nE "SpawnSoundAttached|UAudioComponent|bLooping" \
    Source/LLK/Private/Enemies/LLKSweeper.cpp Source/LLK/Public/Enemies/LLKSweeper.h
(no matches)
```

So a player hears the telegraph once, from where the machine stood when it
noticed them, and then nothing at all for over a minute while it closes.

## Why this was invisible

`DECISIONS.md` row 16 is **locked**: every enemy announces itself with sound
before sight, distinguishably. The telegraph cue satisfies that *as written*,
and `tests/cues.test.mjs` proves the delegate is bound and the sound plays. Row
16 was green. It is still green.

What row 16 does not say, and what nothing in the repo was checking, is that a
single announcement is not a tracking cue. `ROADMAP.md` slice 3 asks that *"a
fresh player can identify a cue and choose hide, detour or sprint"* — and a
player cannot choose against something whose position they stopped knowing 70
seconds ago. The requirement that was met and the experience that was missing
are two different claims, and only one of them had a test.

This is the project's recurring failure shape, fifth instance: **a correct
system over an absent thing, with a clean log.** The crate in a recess, the
fixture collision never imported, the lantern 189 cm above a 150 cm reach, the
opening with no navmesh — and now a threat with no sound. In every one of them
the code was right and the log said so.

It was also *unmeasurable until now*. Before the navmesh existed the Sweeper
never took a step, so the silence had nothing to be silent during.

## The fix

A looping, spatialised motor attached to the pawn, with the brain's state
setting its volume and playback rate.

| file | what it is |
| --- | --- |
| `assets/audio/make_sweeper_presence.py` | the master: a 4.0 s loop synthesised from arithmetic — motor fundamental with odd harmonics, a brush head of band-limited noise amplitude-modulated at the rotor rate, and one castor squeak per revolution |
| `Content/Audio/Sweeper/sweeper-presence-v1` | today's render, imported and marked looping |
| `Source/LLK/Public/Enemies/LLKSweeperPresence.h` | the state → (volume, rate) mapping, as a pure function, plus the glide |
| `ALLKSweeper::Presence` | a `UAudioComponent` on the pawn, its own attenuation, started in `BeginPlay` |
| `ALLKSweeper::UpdateVoice` | called every authority tick, next to `UpdatePresentation` |

**The castor is the part that does the work.** A motor alone runs on the spot;
one squeak per wheel revolution is what a player can count to tell whether a
machine is getting closer, and playback rate moves the squeak and the motor
together, which is what a machine actually does when it speeds up.

### The design claim, stated so it can be argued with

**Entering the telegraph cuts the motor to exactly zero, in one tick.** Every
other state is louder than patrol as the machine gets more interested; the
telegraph is silent. Three reasons, in order:

1. The telegraph cue has to land. A one-shot played over a running motor at
   the same position competes with it, and the player hears one sound.
2. The brain already holds the Sweeper still through the telegraph. A machine
   that stopped moving should stop sounding like it is moving — and the motor
   cutting is what makes row 16's second read as a head start rather than as
   dead air.
3. A hole in a continuous sound is a stronger cue than a loud sound. A player
   who has spent a minute hearing the aisles hum learns the hum, and then the
   hum stops.

The glide is deliberately exempt there. Everywhere else the component ramps at
a constant rate — patrol to pursue in about 0.2 s — because a motor that jumps
in one tick reads as a *cut*, and the ear hears a cut as a new sound somewhere
else rather than as the same machine speeding up, which would lose the only
thing this system is for.

If Chris wants it to idle through the telegraph instead, it is one constant,
and `LLKSweeperPresenceTest` names what fails first.

### Loudness, and the one restraint

Pursue is 0.62 against patrol's 0.22. It is deliberately not louder: a motor
that drowns the player's own footsteps takes away the other thing they
navigate by, and a chase where you cannot hear yourself is disorienting rather
than frightening.

`PresenceAttenuation` reaches 60 studs where `CueAttenuation` reaches 38, and
that difference is a contract, not a tuning accident. A cue is bounded to the
authored bright reach so that hearing one means a machine has committed within
range of noticing you. The motor is the opposite: it must be audible *before*
you are anywhere near being noticed, because its job is to let you route
around a machine you have not met. **Hearing it must never imply being seen.**

## What is tested, and the gap no C++ test can see

`LLKSweeperPresenceTest` ("LLK.Enemies.SweeperPresence") pins the balance:
every state audible but the telegraph, the telegraph exactly zero and
immediate, pursue louder and faster than patrol, stock a just-noticeable 12%
above patrol, the glide frame-rate independent and non-overshooting.

**All of that is true of a Sweeper that makes no sound whatsoever**, because a
C++ test cannot see whether the asset the component asks for is on disk,
whether anything calls `Play()`, or whether `UpdateVoice` is reached from a
tick. `ConstructorHelpers::FObjectFinder` does not error on a missing asset —
`Succeeded()` is false, `PresenceSound` stays null, and the machine is exactly
as silent as before, logging identically to a working one.

So `tests/sweeper-presence.test.mjs` checks the three things the automation
structurally cannot, by reading the source and the filesystem: the path the
C++ asks for names a file that exists, something calls `Play()` after
`SetSound`, and `UpdateVoice` appears inside `Tick`. Plus that
`TelegraphVolume` is still exactly `0`, because that single constant is the
design claim above and a retune is where it would quietly go.

## Verified, in a running game

The import was blocked for an hour: `Tools/run_editor_python.ps1` refuses -
correctly - while an editor holds the project, and the same Live Coding lock
refused the editor-target build. Chris closed the editor; both ran.

```
LogLLK: LLKSweeper_0 has a motor: sweeper-presence-v1, running at 22% volume
        on patrol and 62% in pursuit, silent while it telegraphs, audible to
        1680 cm.
LogLLK: LLKSweeper_0 can reach the keeper: 11045 cm of path over 4850 cm
        straight (2.28x), 65.7 s at 168 cm/s.
LogTemp: Display: Map readiness: ready.
```

`LLK.Enemies.SweeperPresence` ran and reported `Result={Success}` - checked by
name in `Saved/Logs/LLK-Automation.log` rather than inferred from the runner's
"LLK automation passed", which earlier this same session exited 0 having never
run a thing. `node tests/sweeper-presence.test.mjs` is 8/8. The imported wave
is `looping = True` at `duration = 4.000 s`, read back from the asset after the
import rather than assumed from the task that wrote it.

**That first line is deliberately affirmative, and that is a lesson from the
navmesh.** A null sound, a component that never played, and a working motor
all look identical in a log that only speaks up on failure. "Nothing
complained" is not evidence, so the Sweeper now says what loaded and what the
player will hear. 1680 cm is 60 studs at the project's 28 cm scale, which is
the attenuation contract above, read out of the live component rather than
restated from the header.

## A side finding, fixed

`tests/enemies.test.mjs` guarded row 16 with `!/Mind\.State\s*=/` — the pawn
may not set its own state. That regex also matches the first `=` of
`Mind.State ==`, so **reading** the state to compare it tripped a guard about
**setting** it. `UpdateVoice` has to ask whether the state is `Telegraph` in
order to cut immediately, and it failed a correct rule for the wrong reason.

Narrowed to `/Mind\.State\s*=(?!=)/`, with a second check asserting the
narrowing did not defang it: a real assignment is still caught, with and
without surrounding spaces, and `==` and `!=` are still allowed. That second
check exists because the whole failure was a regex nobody had tested.

## What this does not claim

- **Nobody has heard it.** Every number above is a design argument, a unit
  test, and a log line from a `-nullrhi` boot that renders no audio at all.
  That the component loaded, looped and was handed the right volumes is now
  proven; whether 0.22 disappears into the room and 0.62 arrives as dread is a
  judgment in play, and the loudest thing this finding cannot tell you.
- **Occlusion is not modelled.** A machine heard through a gondola is as loud
  as one in the open. That is the right next refinement and the wrong thing to
  guess at before the basic presence has been listened to.
- **One machine.** The route is also not stable: the same query read 71.2 s
  before the evening's dressing and 65.7 s after, so this duration is a
  property of whatever was last placed in the aisles, not a tuned number.
  65.7 s of approach is a long time to be afraid of a single
  Sweeper on a two-floor sales floor, and the motor makes that duration
  *audible* rather than shorter. Whether the chapter wants a nearer patrol, a
  faster machine, or a second one is a slice-3 question this answers nothing
  about.
