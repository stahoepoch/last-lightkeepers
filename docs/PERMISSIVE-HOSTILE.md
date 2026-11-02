# Permissive about shopping, hostile to exceptions

Chris's sentence is the cleanest statement of this game's rule set anyone has
written, and it should be the line the whole mall is built against:

> **The mall's idea of "normal" is still executing without people to correct
> it. It is permissive about shopping, but hostile to exceptions.**

Everything below follows from that, plus his constraint that seasons change on
the mall's own calendar so the living mall is **legible rather than
arbitrary.**

---

## 1. The anchors, settled

| Game | Real | Identity |
|---|---|---|
| **Boswell's** | Boscov's | Mirrors, neon, glass cases, dense department-store spectacle |
| **Sayers** | Sears | Automotive and service, tight utilitarian stock and repair |
| **Penhurst** | JCPenney | Crooked, two mall entrances, salon and beauty concession |
| **Bellevue** | Bon-Ton | Its own full identity — a complete store, not a shell |

Four anchors, all real, all stocked, all active. No fifth, no missing store,
no empty construction shell. I had introduced one and it was wrong.

**Naming note on the fourth.** `DECISIONS.md` row 64, `THE-STORE.md` and
**two rows in `data/relics.json`** already say *Bellevue*. Changing it to
Belview or Bellmont is a data edit plus a DataTable regeneration, not a prose
change. Recommend keeping Bellevue — *bon ton* means "good style," and
Bellevue carries the same slightly-above-itself provincial elegance, which is
what Bon-Ton actually was. Chris's call.

---

## 2. The asymmetry: front is gated, back is unpermitted

"Permissive about shopping, hostile to exceptions" resolves into two spaces
with opposite rules, and this is the part that makes routing interesting.

**Front of house is permissive and gated.** You are a customer. You may walk
anywhere, look at anything, pick things up, put them down. The mall does not
mind you at all — until you carry something through a gate, which is an
exception, and the gate says so.

**Back of house is ungated and unpermitted.** There are no anti-theft
pedestals in a service corridor because customers do not belong there. Nothing
needs to check what you are carrying, because you are already the exception
simply by standing in it.

| | Front of house | Back of house |
|---|---|---|
| Your status | Customer, tolerated | Exception, by presence |
| Goods check | At the gate | None |
| Light | Store floor dark, concourse lit | Black |
| Who works here | Window Walkers, Sweepers | Stockers, receiving |
| Cost of being there | None, until you take | Immediate and constant |

So the quiet route is not the safe route. It is the route with no alarms and
no tolerance — and `data/enemies.json` already supports the split, because
Stockers are *shelf stackers* with strong line of sight down long straight
runs, which is precisely a service corridor.

---

## 3. The second currency of attention

This is the biggest structural gain from the alarm rule and it is worth
naming.

Until now, everything in this game costs **light**: continuous, personal,
carried by one player, read as brightness at a position.

The gate introduces a second, completely different currency: **alarm.**
Discrete rather than continuous. Locational rather than personal. It happens
once, loudly, at a known place, and then it is over.

That gives routing a reason to exist independent of the lantern. A crew can be
perfectly dark and still make the worst possible noise, and a crew can be lit
the entire run and never trip anything. Two axes, two kinds of mistake, and
decisions that are not all the same decision wearing different hats.

It also gives the Sweepers' `swarmCallRadius: 45` something to be called by
that is not a player.

---

## 4. Size decides the channel, and burden already exists

- **Large items** leave through freight or the automotive bays only.
- **Small items** can take a side route or parcel channel, faster.
- **Anything** can go through the front, loudly.

`DECISIONS.md` row 77 already has burden as authored-and-now-read data, and
`THE-STORE.md` already stamps restoration crates with the department they came
from. Size class is a third field on the same object, and it is the one that
turns "what do we take" into "how do we leave" — because they become the same
question, asked four rooms earlier than the extraction button.

The crate's department stamp says *which room at home this finishes.* The size
class says *which door it can get out of.* A crew that wants the dormitory
bedding from the second floor is committing to a freight route before they
pick it up.

---

## 5. Seasons are authored circulation states

Chris's constraint — the mall's own calendar, never random mid-run — has a
strong implication worth stating directly:

> **Each season is a fixed, learnable circulation map of the mall. Not a
> generator.**

Five or six authored states of the same familiar building. Christmas blocks
the toy route with gift towers and opens the furniture wing. Summer's patio
displays open sightlines and close hiding places. Back-to-school builds rack
mazes. Clearance reclaims floor with carts, signage and temporary walls.

Same mall, different circulation and hiding logic — and because it is authored
rather than generated, a crew can *learn* it. "Christmas means the toy route
is blocked but furniture is open" is exactly the kind of sentence players
should be saying to each other, and it is only possible if the states are
fixed.

**This also means knowledge is season-scoped**, which gives the Fieldboard a
third clock on top of the two it already has:

| Clock | Scope | What it means |
|---|---|---|
| Elapsed time | Minutes | How long ago |
| Storm stage | Within a run | The world changed since — "before the surge" |
| **Season** | Between runs | **A different mall** |

A route note from Christmas is not stale in summer. It is **historical** —
accurate about a building that is not currently there. The board should say
so, because "blocked" and "blocked last Christmas" are different facts and a
crew that confuses them walks into a wall.

That is a free depth gain from a constraint Chris imposed for legibility, and
it is the kind of thing that only works because the seasons are authored.

---

## 6. Conceded: no mid-run changeover

I proposed a rare event where the mall reorganises its floor while the crew is
inside it. Chris's constraint rules it out and he is right: it is the
arbitrary-mid-run case exactly, and it would break the one thing that makes
the living mall worth having — that a crew can learn it.

The principle underneath is the keeper: **the mall may change, but never
without having announced it.** If a changeover is ever built, it belongs on
the calendar in advance, so a crew can choose to be there on the turn of the
season rather than be ambushed by furniture. Legible first. That is a much
later idea and nothing should wait on it.

---

## 7. What still has to be decided

- **The fourth anchor's identity.** Boswell's, Sayers and Penhurst have
  characters. Bellevue has a name, two relics and no personality yet. Under
  the knowledge taxonomy it is the open slot, and nothing in the set currently
  gives *vertical* knowledge — a store built around an open well or a grand
  staircase would complete it. Entirely Chris's call.
- **What advances the season.** Real time, runs completed, or story progress.
  Real time makes the mall genuinely indifferent and makes waiting real, at
  the cost of locking a player out of what they need. Runs-completed keeps it
  in the crew's hands and will be farmed. Story-driven is predictable and
  costs the indifference that makes the idea work at all.
- **Whether locked stores open on a schedule or on a condition.** Chris said
  "on schedules or after some visible system condition is met." Both are good
  and they are different games: a schedule is something you learn and wait
  for, a condition is something you go and do. The breaker in
  `data/loot.json` is already the second kind.
