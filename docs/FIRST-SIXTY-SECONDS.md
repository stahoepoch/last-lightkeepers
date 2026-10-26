# The first sixty seconds

Status: **proposed**, 2026-10-26. Closes `WORKLIST.md` C4, whose done-criteria
is a beat-by-beat spec rather than code — the CHI 2012 finding behind it is
that tutorials are only justified in the most complex games and that
*context-sensitive* help is the form that works. So the first run teaches **by
arrangement**: what is lit, what is reachable, and what order things appear in.
Most of the arrangement is map-side, so every beat below names the lane that
builds it.

Written against the locked loop: only extraction secures loot, calling the exit
is loud, light is how you see and how they find you.

**Nothing in here is a text box.** If a beat can only work by telling the
player something in words, it is the wrong beat.

## Part one — arrival at the hall, 0:00–1:00

| Time | What the player sees | Lane |
| --- | --- | --- |
| 0:00 | One room lit, the side room. Everything beyond it is visible through doorways and dark. No corridor choice, because there is only one lit way to stand. | map |
| 0:05 | Their own lantern on a hook at eye height, the only interactable in reach. Taking it is the first prompt in the game. | map + hub |
| 0:15 | With the lantern in hand, the room reads differently: the roll board on the wall has names on it and empty lines under them. Nobody explains the roll. | hub |
| 0:30 | The kitchen counter, with nothing to hand in. The prompt says so rather than staying silent — the rule the whole hub follows. | hub |
| 0:40 | The departure board, lit, offering alone or with friends as equal choices. Solo is never the tutorial. | expedition |
| 0:55 | Three landmark plates, unlit stone, each reading `0 of 10 crates`. The first thing the player learns about the town is that it is not finished. | hub |

**What part one teaches without saying anything:** this place is yours, it is
mostly dark, darkness here means unfinished rather than dangerous, and the way
to change that is the board.

## Part two — the first run's first sixty seconds

| Time | What happens | Lane |
| --- | --- | --- |
| 0:00 | Arrival inside the mall, lantern **off**, in a space with one obvious way forward and enough ambient light to walk by. The first ten seconds must be safe and must not look safe. | map |
| 0:08 | Loose supplies in plain sight, no lantern needed. The first pickup teaches the hold — a short one — and that taking things is a commitment of time. | map + loot |
| 0:20 | A dormant Window Walker in a shopfront, passed at close range in the dark, doing nothing. It is scenery until it isn't. No cue, no music sting. | map + enemy |
| 0:30 | A dead end that is visibly not a dead end: something catches the eye and cannot be resolved without light. This is where the lantern is first *wanted* rather than mentioned. | map |
| 0:34 | Lantern on. The cache resolves into a relic. The meter starts moving and the HUD shows it moving for the first time, because it never moved before this. | lantern + HUD |
| 0:40 | The relic's hold is long — seconds of standing still, lit, in the open. The first genuinely frightening thing in the game is a progress bar. | loot |
| 0:48 | Something answers. Not an attack: a patrol's noise in the next space, and the Window Walker behind the player is now facing a different way. Nobody is told that light did this. | enemy |
| 0:55 | Lantern off, and the relief is physical. That is the lesson, and it was never stated. | — |

**What part two teaches:** off is safe and poor, on is rich and loud, taking
costs time, and the building reacts. Four rules, no text.

## Part three — the one beat that closes the loop

| Beat | What happens | Lane |
| --- | --- | --- |
| The call | The exit is called. A flare, a siren, every patrol turning, and the meter jumps whatever it was sitting at. The player did that on purpose and it was the loudest thing they have done. | loot + enemy |
| The hold | They hold one room with the haul on them, knowing it is not theirs until the ride comes. | loot + enemy |
| The hand-in | Back at the hall, the crate goes over the counter, their name goes on the roll, and **a door that was dark opens.** | hub |

That last beat is the reason there is a run two, and it is the one piece of
this document that is not built. `HubService` has the lamp wall, the plinths,
the roll board and the plates, and nothing that opens. See
`docs/VERTICAL-SLICE.md`, element 5.

## What this spec forbids

- **No tutorial text, no arrows, no forced camera.** If a beat needs any of
  them, redesign the beat.
- **No scripted safety.** The dormant Window Walker at 0:20 must be genuinely
  able to wake if the player lights up early. A first run that cannot go wrong
  teaches nothing about a game whose whole subject is risk.
- **No gating.** A player who sprints past every beat is allowed to. The beats
  are an arrangement, not a corridor.
- **No first-run-only content.** Everything above is the mall behaving
  normally. The moment it becomes a special opening, it stops teaching the
  game and starts teaching the opening.

## The one number this needs

Beat 0:34 assumes the meter visibly moves the moment the lantern comes on. At
the tuned curve a lit solo keeper reaches first pressure at 2:34, so inside the
first sixty seconds the needle moves but nothing fires. That is correct: the
player should *see* the cost before they *pay* it. If the HUD cannot make that
movement legible at low values, the beat fails and the HUD is what changes —
not the curve.
