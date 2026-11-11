# The story draft, adjudicated: take the images, refuse the explanation

Status: **adjudication**, 2026-11-10, of the Copilot transcript Chris relayed
as `lightkeeper.txt`. Nothing here is a `DECISIONS.md` row. Rows are proposed
at the end.

**The first job is separating two kinds of statement in that file.** Chris
wrote two messages in it and they are design authority. Copilot wrote the
rest and it is a proposal. Several of Chris's lines are new to this repository
and more load-bearing than anything Copilot answered with, so they are
extracted first.

---

## 1. Chris's own statements, which are the authority

From his first message:

- **Dad and brother are both Lightkeepers, and both are taken.** The kid
  sneaks onto their mission. This is new and it is the strongest version of
  rows 49 and 50 — see section 6.
- **First person, a lamp that is a multi-tool.** Already the build.
- **Night and day, and day is far more dangerous.** Already the premise. See
  section 3.
- **"Storms ravage the earth, so at night lightning lights up the mall and
  momentarily charges the robots, so they turn on and chase you if they see
  you."** This is the best new mechanic in the whole file and it costs
  nothing. Section 4.
- **"You never attack robots, if u did, that would be fatal. They would all
  alert to you."** Consistent with row 47 and `COMBAT.md`; worth locking as a
  row of its own now that it has been said plainly twice.
- **"They don't necessarily kill you, they abduct you."** Rows 49/50.
- Levels: railyard, solar farm, foundry, hospital, downtown, airport.
- Lanternhold supplied by what you bring back; Lamplighters as *"pioneers
  trying to reestablish the old ways."*

From his second message, which is the sharper one:

- **"The mall should look and be just like the Galleria, and getting from one
  end of it to the other is the key, Boscov's (Boswells) to Sears
  (Sayers)."** This is the answer to *"the mall doesn't go anywhere"* and it
  comes from him. Section 5.
- **"Sears has the auto center with cars and stuff."**
- **Leave before sunset, not a journey of hours, and foreshadow one or two
  places we will see later.**
- **"I think you should definitely get caught by your brother and/or father,
  and witness the whole crew get captured."**
- **"Idk how the father's light being a beacon to follow is going to help...
  not every light is like that?"** Correct instinct. Section 7.
- Open questions he asked: can we choose day and night; does the mall change
  over time with seasonal stock; can we return after finishing a level and
  find new things. **All three are already answered by his own locked
  structure** — section 3.

---

## 2. What Copilot got right

- **"Don't make the game about gathering resources. Make gathering resources
  support the story."** That is `HOW-THE-PLAYER-LEARNS.md`'s doctrine and
  Codex's two-currency split, arrived at independently.
- **Real place names instead of zone names.** *"The player learns the mall
  exactly like a real kid would."* This is why eight sections of
  `THE-MALL-IS-STILL-TRADING.md` exist.
- **Throwing out the lamp-as-GPS in round two** after Chris pushed back, and
  replacing it with *"the Beacon is something the player misunderstands."*
  Both corrections are right.
- **The capture in-engine with player control retained**, not a cutscene.
- **The convoy sightlines** — Route 56, the Conemaugh valley, the airport
  beacon tower, a solar farm — as foreshadowing the player does not know is
  foreshadowing. This is §8's *"the biggest first level is the one that
  teaches the mall by refusing to let you into it"* applied to a drive.
- **One line, and it is the best thing in the file:**

  > *"Every Lightkeeper carries a light. The trick isn't knowing when to turn
  > it on. It's knowing when to turn it off."*

  That is the entire game in two sentences, it is row 64 and the light field
  said in a voice a twelve-year-old would remember, and it should be a quoted
  row rather than a line in a draft.

---

## 3. Three of Chris's questions are already answered, by him

**"Can we choose day and night?"** — The mechanism is already written, in
`PITCH.md`, and it is better than the transcript's answer. Copilot proposed
that day is worse because robots *"enter a different operating state"* with
drones and scanners. The project's own reason is simpler and harder:

> *"The machines run on light. In daylight they're all awake and there's no
> way through. That's also why your lantern is dangerous — **it's a small
> sun**."*

So day is not a variant with different patrols. **Day is the state in which
the building cannot be entered at all.** That is stronger, it needs no second
AI behaviour model, and it makes the night window the premise rather than a
preference. Copilot's Act 3 "reveal" that machines charge from storms cannot
be a late-game twist, because it is the first thing the eight-year-old version
of the pitch tells you.

**"Does the mall change over time (seasonal stock)?"** — Yes, locked.
`CAMPAIGN-SHAPE.md` §1 makes the **railyard the physical cause of the mall's
calendar**: *"the mall's seasonal stock is arriving there."* Seasons are not a
cosmetic layer; they are a place you can go and interfere with.

**"Can we return after we finish the level?"** — Yes, locked, and with a rule
Copilot's version does not have. `CAMPAIGN-SHAPE.md`: *"You never return to
Chapter 2's Galleria, because that mall is gone the same way last December is
gone."* And *"a relic you did not take in Chapter 2 is still in its case —
unless a seasonal reset moved it, cleared it, or buried it behind a gift
tower."* Missed things have stakes because the building reorganises itself.

---

## 4. The unified light rule, and Chris's lightning is its missing middle

This is the adjudication's main contribution and it is almost free.

`PITCH.md` already says the lantern is *a small sun*. Chris's new idea is that
lightning *"momentarily charges the robots."* Those are the same rule. Written
once:

> **Light powers the machines. Every light does. The only variables are how
> big, how long, and whose fault it is.**

| Source | Scale | Duration | Whose fault |
| --- | --- | --- | --- |
| Your lantern | local | continuous, yours to stop | **yours** |
| Lightning | the whole building | one second | **nobody's** |
| Daylight | the whole valley | hours | **the sky's** |

Three scales, one mechanic, no new system. And it does three jobs at once:

- **It makes the storm frightening without making it a clock.** A strike is
  not a countdown; it is a moment in which everything that was a statue can
  see you, wherever you are standing, including places your own light never
  reached.
- **It gives darkness texture.** The reason to be still is no longer "a robot
  might come"; it is "the sky might turn the lights on for it."
- **Row 73 permits it precisely, where it forbade everything else.** Row 73 is
  *"the clock means weather alone. Nothing a player does moves it"*, and
  `ELLKStormSpike` was deleted because `AddSpike` was an allowlist that could
  not stop a future file writing `AddSpike(60, "Knockdown")`. **Lightning is
  weather.** It is the one impulse that rule was written to allow, and the
  reason the type was removed — failure must never escalate — is untouched by
  it, because a strike is caused by the sky and not by losing.

One caution, and it is row 64 line one: pressure *"permanently increases what
the building is AUTHORIZED to do"* and is monotonic. So a strike should be a
**director peak within existing authority** — row 64 line two — not an
increment to the meter. Lightning reveals you; it does not age the night.

---

## 5. Boswell's to Sayers is chapter one, and it retires row 28 on purpose

*"Getting from one end of it to the other is the key, Boscov's (Boswells) to
Sears (Sayers)."*

That is the first sentence anyone in this project has written that gives the
mall an **axis**. It is a destination inside the building, it is legible to a
player in one sentence, and it uses the research that already exists: row 19
already names Sayers, §8 already blocks the whole roof with four anchors, §10
already traces the walk between them, and `Content/LLK_Boswells_Anchor.umap`
is already the Boswell's end.

**But it is not compatible with row 28**, which is LOCKED: *"Version one is
one sealed department store, furnished top to bottom. The rest of the Galleria
is earned, not cut."* Crossing from one anchor to another is the concourse and
at least two anchors. That is the whole mall.

Row 28 has in fact been under pressure from Chris himself since 2026-11-03,
when §8 records him saying *"build the Galleria, anchors and all, as a massive
first level."* §8 then spends a page reconciling that with rows 4, 28 and 38 by
making the shell visible and sealed. **Boswell's → Sayers breaks the seal**,
because the player has to get into the concourse.

So this is a row to retire by name, not to drift past. The replacement that
keeps everything else:

> **Chapter one is the crossing.** Boswell's is the furnished night — it
> already is. The concourse is the dark, exposed, lightning-lit middle. Sayers
> and its auto centre are the far end. The other two anchors stay sealed
> volumes you can see into and not enter, which is §8's argument intact.

That gives one building, two furnished ends, one dangerous middle, and three
things visible through grilles that are promises rather than content. It also
finally names chapter one's deliverable, which neither my proposal nor Codex's
did.

---

## 6. Refuse the Beacon. Rows 47 and 49 are stranger and already locked

This is the one place the transcript should be rejected rather than trimmed.

Copilot's story is: humanity vanished a century ago, taken by machines built
by **the Shepherd Project**, a final attempt to solve every human problem
whose definition of protection evolved — *"Humans are fragile. We will save
them by removing them."* **The Beacon** is a surviving planetary
communication AI, the last system trying to restore humanity. Three endings:
restore, merge, or let the preservation continue.

It is competent and it is a different, weaker game. Row **47** is LOCKED and
says the opposite:

> *"The machines never stopped working. They do not hate or need people. The
> storm wakes broken infrastructure, and every place tries to finish the last
> job it was built to do."*

And `THE-WORK.md`: *"This storm did not invent a villain. It woke every system
that never received a shutdown order... A department store is still trying to
prevent loss. A hospital is still trying to isolate. A library is still trying
to stop a record from leaving."*

The difference matters more than it looks:

| | The Shepherd Project | Row 47 |
| --- | --- | --- |
| Who is in charge | an intelligence with a mandate | **nobody** |
| Why you are taken | you are being preserved | you were **processed as a problem** |
| What understanding buys | a twist, and a choice of endings | nothing. It was never about you |
| Where the horror is | a mind that decided wrong | **institutional momentum with no mind in it** |

A machine that *decided* to save you is a villain, and a villain is
explicable, negotiable, and defeatable. A loss-prevention system that is
simply still running, forever, with no supervisor coming, cannot be reasoned
with because there is nothing there to reason with. Row 47 is the harder idea
and it is already paid for.

And the horror Copilot wants is **already locked, without the AI**. Rows 49
and 50, `CONTAINED-KEEPERS.md`:

> *"They have processed someone as a problem to be moved, quarantined,
> secured, or held until an authority arrives that never will."* ... *"it will
> wait forever, with a person inside."*

**So: take every image, refuse the explanation.** The images in that
transcript are excellent and all of them work under row 47 with nothing
added — the empty clothes, the children's drawings, the recorded voices, the
recordings from decades ago describing the same disappearances still
happening, and above all the single lightning flash showing rows of
human-shaped pods and then darkness with no explanation. **That image is
better under row 47 than under the Shepherd Project**, because under row 47 it
never gets explained, and the reason it never gets explained is that there is
nobody to explain it.

What replaces "Find the Beacon": Chris's own instinct, which Copilot came
round to. Dad's last words are **an instruction the kid misunderstands for the
whole game**. It should not name a thing that exists.

---

## 7. Two mechanics to keep, one to drop

**Drop: the father's lamp as a trackable signal.** Chris: *"Idk how the
father's light being a beacon to follow is going to help... not every light is
like that?"* Right, and Copilot withdrew it. It is also against doctrine —
`HOW-THE-PLAYER-LEARNS.md`: *"Anything that needs a paragraph is a thing the
game has failed to show."* The honest version already exists: the readout
`80849f8` made speak gives a bearing and a distance in metres, and
`QUESTS.md` rung 1 makes *"basic map markers"* a thing you **earn by restoring
a generator.** A kid who has fixed nothing yet should have no markers at all.

**Keep: the lamp is a training unit.** Copilot's detail that dad hands over a
broken practice lamp before leaving is worth more than it looks. It explains,
without a word of UI, why the player's light is worse than the adults' were,
and it makes every later lamp capability a repair rather than an upgrade —
which is `CAMPAIGN-SHAPE.md`'s *"upgrades that unlock new access, not raw
power."* Reject the Zelda tier list (Pulse Lens, Spectral Filter, Beacon
Mode); keep the broken training lamp.

**Keep, and this one is the fix the pivot still needed: failure is
containment.** `PITCH.md` promises *"You don't die, you get knocked down and
picked up"* — a co-op promise that does not survive one child alone. Rows 49
and 50 already have the answer, and nobody has yet pointed it at the player:

> **You are not killed. You are processed.** Moved, quarantined, secured, and
> held until an authority arrives that never will.

The fail state becomes the premise. The thing that happened to your father and
your brother in the first twenty minutes is the thing that happens to you if
you are seen, and the game never needs a death screen to say so.

---

## 8. Scope, said once

Seven acts, six regions beyond the mall, two to four hours for chapter one.
That is a twenty-to-forty-hour narrative game, from a solo developer whose
build today is one room, 103 actors, 3.7% furnished, with nothing in it that
opens a route.

The way that ships is the way rows 28 and 38 already describe, re-read with
the crossing in place of the sealed store: **chapter one is a complete game.**
Boswell's to Sayers, the storm, the capture, the crossing, the railcar. The
railyard is what the ending of it points at, and the solar farm, foundry,
hospital, downtown and airport are a sequence that earns its way in — *"each
expansion arrives as more danger, not just more floor"* (row 38).

Writing all seven acts down now is still worth doing, because the convoy at
dusk has to foreshadow the right things. Building two of them is not.

---

## 9. Rows this proposes

Added to the six from the Codex adjudication.

7. **Light powers the machines, at three scales: your lantern, lightning, and
   daylight.** Daylight is not a harder variant, it is the state in which the
   building cannot be entered. A strike is a director peak within existing
   authority and never an increment to the pressure meter.
8. **Chapter one is the crossing: Boswell's to Sayers.** **Retires row 28.**
   Two furnished ends, one dark concourse between them, the other two anchors
   sealed and visible.
9. **Attacking a machine is fatal and alerts every machine.** Said plainly by
   Chris twice; `COMBAT.md` can close behind it.
10. **Being caught is containment, not death.** Rows 49 and 50 pointed at the
    player. No death screen.
11. **Dad's last instruction is misunderstood for the whole game and never
    names a thing that exists.** No Beacon, no Shepherd Project; row 47 holds
    and nobody is in charge.
12. **The lamp is dad's broken training unit.** Every capability is a repair,
    never a tier.

**And the quote worth a row of its own**, because it is the thesis and it came
out of this file: *"The trick isn't knowing when to turn it on. It's knowing
when to turn it off."*

## 10. What is Chris's, still

- Whether row 28 retires. Everything in section 5 waits on it.
- Whether row 47 holds against the Shepherd Project. I recommend it holds and
  I am aware that is me arguing for the harder, less conventional story; it is
  his call and a real one.
- Lamplighters against Lightkeepers, still unanswered from yesterday, and now
  appearing in three documents in two spellings.
- Which end of the real building is Boswell's and which is Sayers (§12 item 7,
  open since 2026-11-06). The crossing makes this a gameplay axis rather than
  a label, so it stops being cosmetic.

## Turned over

**Operand.** This transcript's story, against row 47 and the locked rows; and
Chris's crossing against row 28's sealed store.

**Floor.** A locked row is a floor. `PITCH.md`'s *small sun* is a floor and it
already answers day/night. The crossing is a floor because the building is
traced. Seven acts are not a floor; they are an outline, and an outline is
what the drive at dusk needs in order to point at the right ridgeline.

**Clock.** The repository as of `fe1cd8b`.

**Falsifier.** The crossing falls if a played child cannot cross a concourse
at all without a mechanic nobody has built — which is the first thing to test
once Boswell's has a door into one. The lightning rule falls if a strike that
reveals the player reads as unfair rather than as weather; the light lab can
test that before any of it is authored. Section 6 falls if a played crew
finds *"nobody is in charge"* unsatisfying rather than frightening, and that
is a real risk I am taking a position against rather than measuring.

**Upside down.** Three agents have now proposed a progression spine for this
game and all three already existed in the repository. What did not exist, in
any document, was a destination inside the building — and Chris supplied it in
half a sentence while asking about something else. The mall goes somewhere
now: it goes to the other end of the mall.
