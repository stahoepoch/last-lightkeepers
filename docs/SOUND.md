# Sound, the storm, and how a threat announces itself

Status: Chris's framing and the lightning beat are **his, 2026-10-28**. Row 16's
rewrite is **LOCKED**. Everything below marked *proposed* is Claude
extrapolating from those and is not ratified.

Chris: *"Sound is a major major thing for the game. This is pivot horror
survival in a way."* That is a genre statement as much as an audio one, and it
is the first time the project has said out loud that horror is the register
rather than a side effect of the dark.

## The rule, in its new form

> **Every enemy telegraphs before sight through at least two distinguishable
> channels, one of which may be sound.** — `DECISIONS.md` row 16, rewritten
> 2026-10-28

The old form said *sound before sight*. It was written as an accessibility
rule and it was failing as one. Microsoft's Xbox accessibility guidelines ask
for key information on two or more channels and name audio-only danger cues as
an anti-pattern — and in co-op the audio channel is **already occupied by three
people talking.** A crew mid-argument is a crew that never received the cue.

Chris's test, and it is the whole rule in one line:

> **"Voice chat must never be the reason someone misses a threat."**

Sound stays **primary**, in his words, "because it is scary and directional."
Nothing here demotes it. The second channel is physical evidence in the world:
moving shadows, flickering fixtures, disturbed merchandise, footprints,
silhouette changes, and UI-safe visual or haptic indicators where they fit.

### The acceptance test, both ways

Codex's plan already asks that every important action and enemy state be
identifiable **with your eyes closed**. Row 16's new form adds the inverse, and
both halves have to pass:

| Test | Passes when |
| --- | --- |
| Eyes closed | You can name the enemy and roughly where it is from sound alone |
| Sound off | You can tell something is coming, and roughly from where, from the world alone |

A cue that only passes the first test is the rule's old failure. A cue that
only passes the second is atmosphere.

## The storm is the startle instrument

Chris's beat, quoted because the whole design below is downstream of it:

> *"the lightning hits outside, machines power on for a moment and move, their
> sound is enough to startle you, the flash, their movement, the sudden jolt.
> Fear."*

A strike hits somewhere out in the dark. For a moment the building has power
again. Things that had been dead furniture draw current, move, make the noise
they were built to make — and then it goes, and they are furniture again.

This is the best single idea the audio design has, and it is not only a scare.
Six things it does at once, all of them load-bearing:

**1. It satisfies row 16 by itself.** Flash, sound and physical movement, in
one event, simultaneously. The game's own weather is a three-channel telegraph.

**2. It is non-local, so it reaches the keeper with no lantern.** The flash
comes through windows from outside. Everybody in the building gets it at the
same instant, including the player who chose darkness and can therefore see
nothing else. That is the one piece of information the dark cannot deny you.

**3. It demonstrates the premise instead of explaining it.** `PREMISE.md` says
the machines run on light. A strike feeding them for a second is that
sentence, played rather than told — exactly the wordless channel row 43 is
built on.

**4. It makes darkness informative, not only costly.** During the flash the
room lays itself out: the aisle, the far door, the shape standing in it. The
storm periodically hands out free sight, and shows you what is standing in it.

**5. It gives the meter a rhythm the project already wanted.** `ZONES.md`
already has lightning stepping the Storm Charge at the solar farm. Bringing the
instrument into the store means the solar farm escalates something the player
has already learned to fear instead of introducing it.

**6. It is why the roofless fourth floor is the worst place in the building.**
Up there the flash is not filtered through glass. The curriculum's sixth lesson
— *light is not always yours to control* — is delivered by the sky.

### The rules it needs to not be cheap — *proposed*

A startle generator becomes a random-death generator very easily, and the
difference is entirely in these constraints.

- **A strike reveals and relocates. It does not kill.** Machines wake *briefly*
  and return to dormant. What the player loses to a strike is their picture of
  where things are, never their run. If a strike can put a woken machine on top
  of you with no warning, the game is rolling dice at you.
- **The strike telegraphs itself.** Pressure, a rising hum in the fixtures, the
  rain changing — a half-second of warning before the flash. Row 16 applies to
  the storm too, or the storm is the one thing in the game that cheats.
- **Rationed, and tied to the meter rather than to a dice roll.** Startles
  deaden fast under repetition. Strikes should get closer together as Storm
  Charge climbs, so the instrument doubles as the meter made audible and the
  player can *hear* that they are running out of night.
- **No instant return to full black.** The eye needs the afterimage; cutting
  straight back to nothing reads as a graphics bug rather than a storm.
- **The strike is the one light with no fixture.** Codex's rule — every light
  must have a physical reason — holds, and the storm is its single exemption,
  which is exactly why it feels like it comes from outside the rules.

### The one that is a real mechanic — *proposed*

**During a strike, going dark does not protect you.** Everyone is lit, by
something nobody chose. For that moment the careful keeper and the reckless one
are in the same position, and the Window Walker's locked rule — stands still,
wakes to light — means a strike is light enough to wake *every one of them in
the building* for a beat.

That is frightening, it is consistent with rules already locked rather than
needing new ones, and it is the clearest answer yet to a question
`PROGRESSION-GAP.md` raised: what makes a dark run tense rather than merely
slow.

## Accessibility obligations, which are not optional here

A game built on flashes in darkness has a specific duty of care, and it is
cheaper to build in than to retrofit.

- **Photosensitivity.** Repeated high-contrast flashing in a dark scene is a
  seizure risk, not a style choice. The storm needs an intensity reduction
  setting that keeps the *timing* of every strike — so the information survives
  — while cutting the luminance step. Codex's plan already lists
  motion/flicker reduction; this is the thing it is for.
- **Captions for cues, not just dialogue.** There is barely any dialogue in
  this game. Captions here mean naming the *sounds* — `[servo motor, close,
  left]` — which is the second channel for a deaf player and costs a string per
  cue.
- **Volume by sound class**, already in Codex's plan, matters more than usual:
  a player who turns music down to hear cues must be able to.
- **Colour-independent threat cues**, per the existing readability rule. Amber
  against electric blue is not a cue for a player who cannot separate them.

## What this asks of the build

Nothing here needs a technology Codex's plan has not already specified —
MetaSounds, spatial audio with occlusion, concurrency limits, Audio Modulation.
Three additions:

1. **The storm is a system, not an ambience track.** It owns strike timing,
   the pre-flash telegraph, the light event, the brief machine wake, and the
   step it adds to Storm Charge. It is server-authoritative, because all four
   keepers must get the same strike at the same instant — a client-side storm
   would mean four players seeing different rooms.
2. **Every enemy needs its second channel designed, not assumed.** The sound
   half is in better shape than the rest: `assets/audio/make_cues.py` already
   holds six separate synthesis designs - not pitch variants of one - including
   the Glimmer's accelerating ticks that make its two-second scan window
   audible. What none of them has is a physical tell to go with it. That is the
   new work, and it is per-enemy.
3. **Audio ships in the first playable**, which Codex's plan already schedules.
   Row 16 is now unbuildable as an afterthought: a telegraph with two channels
   is a gameplay system, and it cannot be added in the polish pass without
   re-authoring encounters around it.

The archived prototype's cue script (`assets/audio/make_cues.py`) is still the
right *method* — the master is the script, not the `.wav` — and the six cue
designs in it are a starting point rather than a loss.
