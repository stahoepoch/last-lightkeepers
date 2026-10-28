# Contained keepers — the rescue contract

Status: Chris's, 2026-10-28. `DECISIONS.md` rows 49 and 50. The contract is
**LOCKED as a direction and staged after the first slice**; row 50, which
protects co-op from it, is **LOCKED outright**.

## Not hostages

> *"I would call them contained keepers, not hostages. The machines are not
> bargaining or taking people for leverage. They have processed someone as a
> problem to be moved, quarantined, secured, or held until an authority
> arrives that never will."*

That last clause is the whole horror and it should be read twice. Nobody is
coming. The system is not cruel and it is not finished — it is **waiting for a
supervisor who died years ago**, and it will wait forever, with a person
inside.

This is row 47 arriving at its natural consequence. Once the machines
*contain, remove, restrain, reroute, retrieve and lock down* rather than kill,
containment is what happens to people who lose, and the world should have
people in it who lost.

## By location, because the containment is the building's own idea

Chris's four:

| Zone | How a keeper ends up held |
| --- | --- |
| **Mall** | Locked in an old loss-prevention office behind security shutters |
| **Hospital** | Sealed inside an isolation room, or strapped into automated "treatment" |
| **Train station** | Trapped behind emergency platform gates while the rail system keeps rerouting them |
| **Downtown** | Shut inside a utility chamber by a flood-control system |

Each one is that place doing its last job correctly, which is the test row 47
set. The remaining two derive without strain: the **library** returns a person
to storage along with everything else it is protecting from contamination, and
the **solar farm** does not contain anyone at all — it enforces an exclusion
zone, so its version is a keeper stranded *outside* a perimeter that will not
let them back through. Worth noting because it is the one zone where the
contract inverts, and that is exactly what `ZONES.md` asks every zone to do.

## The contract, in three beats

His structure, and the middle beat is the one that makes it a contract rather
than a chore:

1. **Find the contained keeper.**
2. **Disable or override the system holding them.**
3. **Get them to an extraction point — or restore a safe route so they can
   escape themselves.**

**Beat three already contains the escape hatch from the genre's worst mission
type**, and it is worth being explicit about why, because someone will
otherwise simplify it away. "Escort a slow NPC through a dark building while a
timer runs" is a miserable mission and this project would build it by accident.
The *or* prevents that: restoring the route is a valid completion, so the
rescue can resolve as **infrastructure work** rather than as babysitting. A
crew that would rather fix the freight lift than walk someone down four floors
is playing the game correctly.

It also means the contract scales to crew size without a special case. Four
keepers can split — two override, two cut the route — and a solo keeper can
pick whichever of the two solutions their night allows.

## What a rescue pays

His list, and none of it is a number:

- They **return to Lanternhold** and add a human presence to a hall that is
  deliberately too big for the people in it.
- They **open a practical capability or route.**
- They **tell you something they saw deeper inside.**

> **No new currency, and no "save ten NPCs" checklist.** — his constraint,
> and `REWARDS.md` already forbade the first half of it.

### The quiet thing this fixes

`HOW-THE-PLAYER-LEARNS.md` has a standing problem: nobody in this game can
legitimately tell the player anything. No narrator, no codex, no tooltips —
channel two is *one line attached to something you earned*, and until now the
only thing that qualified was a relic plaque about a dead stranger.

**A rescued keeper is the first character in the game with the standing to say
a sentence.** They were deeper in than you have been, they are alive because
of you, and what they say is about where you are going next. That is the only
sanctioned source of spoken words in the whole design, and it is earned by
definition — which makes the rescue contract the answer to a question row 43
could not solve on its own.

### And it completes a set nobody planned

| Recovered | What it is |
| --- | --- |
| **Relics** | Who we were — one person, two sentences, on a plinth |
| **The library and museum** | What we knew (row 46) |
| **Contained keepers** | **Who is left** |

Three kinds of recovery, one per scale, and the third is the only one that can
thank you.

## Staging — his call, and he is right

> *"For the first department-store slice, I would keep this as the next
> contract type, not add it immediately. First prove loot/extraction is fun;
> then the first contained keeper becomes the moment the world stops feeling
> empty and starts feeling like there are other people worth going back in
> for."*

This agrees with `ROADMAP.md`: M4 is the gate, and nothing after it is
scheduled. The rescue contract is M5 work. Building it before the loop is
proven would be adding a reason to care about a loop that has not yet earned
being cared about.

## Co-op is never gated — row 50, LOCKED

Chris: *"we can add keepers to our party, they help us, or co op so it makes
sense why more than one. It's progression. Immediately if a group wants to play
together, there should be a way. U know?"*

Both halves of that matter and they pull in opposite directions, so the rule
has to be written down sharply or it will be implemented wrongly:

> **Rescued keepers are the fiction for why a crew exists. They are never the
> unlock that permits one.** Four friends who buy this game tonight play
> together tonight, on their first launch, with nothing rescued and nothing
> restored.

The failure mode is specific and it is easy to walk into: *keepers rescued →
party slots unlocked* is a tidy progression design, and it would mean three
friends who bought the game together cannot play together until somebody grinds
them out. Row 36 is 1–4 private co-op and it has no prerequisite. The research
is blunt about the cost, too — a player who launches a co-op game and cannot
play it with the people they bought it with has two hours to ask for their
money back, and that failure arrives before any progression system gets a
chance to be clever.

So: **the fiction explains the crew retroactively, and never licenses it.** The
hall has keepers in it because keepers come home. Why the people in your party
are there at all is a question the game does not ask.

### The crew, decided — row 51

Chris, 2026-10-28, resolving it properly: *"Make the crew itself part of
progression — but never gate real friends behind progression."*

| | Rule |
| --- | --- |
| **Immediately** | A player can host or join a private 1–4 party. Friends arrive at Lanternhold as **visiting Lightkeepers** and launch a raid together right away |
| **Over time** | Rescued NPC keepers build out your own roster |
| **Optional** | **Empty** party seats can be filled by recruited keepers, if the host wants them |
| **Always** | Hard cap of **four total**. **Humans take priority.** Nobody is ever told "sorry, you have not unlocked co-op" |

So a crew is any of: you alone; you plus up to three friends; you plus one or
two rescued keepers; you, a friend, and a recruited keeper.

**The slot arithmetic does the balancing by itself, which is why this is a good
design rather than a generous one.** An NPC occupies a seat and humans take
priority, so a full human crew gets nothing from the system and a solo keeper
gets the most. The roster is therefore a **solo and partial-crew** system
without having to be declared one, and it can never make a four-friend raid
easier or harder. No balance coupling, nothing to re-tune when the roster
grows.

### Help that creates choices, not help that removes danger

His constraint, and his four examples — each one is a single use with a cost
attached, which is the lantern rule applied to assistance:

| Keeper | What they give | What it costs |
| --- | --- | --- |
| **Navigator** | Identifies one safer route or possible extraction point | One. Knowing the safer way is not the same as taking it |
| **Maintenance** | Holds open one damaged service door, or briefly operates a pump | Briefly. The building is still closing |
| **Field** | Helps carry a heavy objective | Needs protecting, and occupies a crew slot |
| **Radio** | Warns you about an approaching threat | **The transmission can also draw attention** |

The radio keeper is the best of the four and worth naming as the pattern for
every future one: **a warning that is itself a signal.** It is the lantern
rule moved from light to information — the thing that lets you see costs you
the same coin as the thing that lets you be seen. Any keeper added later should
have to pass that test.

The maintenance keeper also quietly connects to row 45's flood: *briefly
operate a pump* is the basement reprieve with a person attached, rather than a
purchased upgrade.

### The hardest AI problem, and why his design avoids it — *proposed*

A deployed AI companion in **this** game is a harder problem than in any other
co-op game, for one reason: a companion would have to obey the lantern rule.
Every other genre AI can follow and shoot. One here carries a light that feeds
the meter, reveals relics and pulls every machine toward the brightest
position. Get its lantern logic slightly wrong and it is either a griefing
simulator or a thing players learn to leave in a corner — and "leave the AI in
a corner" is the behaviour that deletes the feature.

**His four keepers sidestep it entirely, and I think it should be made
explicit: NPC keepers never carry a lantern.** A navigator, a maintenance
hand, a carrier and a radio operator need no light of their own. That:

- removes the hardest AI behaviour in the project before it is written;
- keeps **every** lighting decision the player's, which row 44 and row 5 both
  depend on;
- is fictionally obvious — they just came out of a locked room. They do not
  have a lantern. **You** are the one with the light, which is the entire
  reason they are following you.

The field keeper carries the objective, not a flame.

### Staging, his

> *"For the first slice, actual friend co-op is immediate. The first rescued
> NPC Keeper should be the first post-slice progression addition, after the
> basic human party loop works."*

Which puts it exactly at `ROADMAP.md` M5, on the far side of the M4 gate, with
M1.9's two-machine human party test as its prerequisite. The human loop proves
itself first; the roster is the first thing built on top of a loop that earned
it.
