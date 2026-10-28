# Rewards — what each thing is for

Status: **proposed**, 2026-10-26. Chris: "exactly what supplies, components,
relics, restoration crates, and upgrades are for — with no overlapping
currencies." No numbers here are new; the ones quoted are already in
`data/*.json` and stay his.

## The rule

**Four resources, three questions. Nothing answers two questions.**

| Question | Answered by | Never by |
| --- | --- | --- |
| *What can I do on a run?* — your kit | **Supplies** + **Components** | relics, crates |
| *What does home look like?* — your hall and the town | **Restoration crates** | supplies, components, relics |
| *What happened here?* — your collection and the story | **Relics** | anything |

Supplies and Components both feed the kit, and that is a **cost pair, not an
overlap**: Supplies are the bulk you always have some of, Components are the
scarce half that decides *when* an upgrade is reachable. One upgrade, two
costs, one purpose. The existing line already works this way — the Relic
Harness is 14 Supplies and 4 Components.

If a fifth resource is ever proposed, it has to answer a fourth question. If it
cannot, it is one of these four wearing a hat.

## Supplies — the bulk currency

**Found:** everywhere, in plain sight, no lantern needed. The thing a dark
cautious run brings home.

**Buys:** your own kit, permanently, from the Quartermaster.

**Never:** restores a room, buys a relic, or converts into anything else. There
is no exchange rate between any two resources in this game, on purpose — the
moment one converts into another, the three-way choice at the heart of the
middle game collapses into "take whatever is worth most."

## Components — the gate

**Found:** in and on machinery — fixtures, panels, dead units. Scarcer, and
more often somewhere you need the lantern lit to work.

**Buys:** nothing by itself. Components are the second cost on an upgrade, and
they are what makes the next tier a decision rather than a wait.

**Why not just make upgrades cost more Supplies:** because then a dark, safe,
boring run buys everything eventually. Components are how the kit asks you to
have taken some risk.

## Relics — never currency

**Found:** only with the lantern lit. This is the single most important
asymmetry in the game and nothing may soften it.

**Corrected 2026-10-28 by `DECISIONS.md` row 53, LOCKED.** This page said
relics *"do not exist to a dark crew"* and `data/loot.json` said *invisible*.
Both were wrong in the same way, and Chris caught it: **an invisible reward
cannot tempt anyone.** A dark crew was never refusing a relic — it never knew
one was there, which quietly turned the game's signature decision into a
hidden-object pass. Relics are **present and visible in darkness, and
unreadable**: the object renders, its value does not. You can see something is
on the shelf. Finding out what costs amber.

### Light gates legibility, not existence — and here is where each thing lands

Chris's slice requirement is that *"valuable relics, restoration crates, and
route clues only become legible under lantern light."* Read as tier visibility
that would break the loot table and a test, so the distinction has to be exact.
`tests/balance.test.mjs:101` asserts **exactly one** lantern-only tier and that
it is Relics — and that assertion stays true under all three:

| Thing | Dark | Lit | Tier change |
| --- | --- | --- | --- |
| **Relics** | Visible. Value unreadable | Legible, and worth the risk | None — already the one `lanternOnly` tier |
| **Restoration crates** | Visible, takeable, carryable | **Which room at home it serves** becomes readable — the department stamp from `THE-STORE.md` | **None.** Stays `always` |
| **Route clues** | Present | Legible | **None.** Not loot. Environmental legibility, no tier involved |

The crate row is the one that would otherwise get implemented wrongly. A crate
must stay findable and carryable by a dark crew — it is the bulk reward and the
hall depends on it. What light adds is not *whether* you can take it but
*whether you know what it is for*, which is exactly the route decision
`THE-STORE.md` introduced: you go to a particular floor tonight because of a
particular room at home. A dark crew carries crates home and finds out later;
a lit crew chooses.

That keeps one tier gated, one asymmetry sacred, and still delivers his
requirement in full: **darkness means leaving meaningful opportunity behind,
not simply playing safer.**

### Route clues are the rule that stops the lantern becoming a detector

Chris, closing this out: *"Route clues make light useful even when there is no
loot nearby."* That sentence prevents a degenerate strategy nothing else in the
design was guarding against, and it should be read as a requirement rather than
a flourish.

If light is only ever worth spending **at** a reward, a competent crew learns
one habit: run dark, flick the lantern on at a shelf, flick it off, move. The
lantern stops being a stance and becomes a **detector**, pulsed for a second at
a time. Every rule we have survives that — the meter still rises, relics are
still gated — and the game still gets quietly worse, because the pressure
depends on light being something you *live in* for a stretch, not something you
tap.

Route clues fix it at the root. If the way through the building is itself
legible only under light — which door is sound, which stair is flooded, where
the service corridor goes — then a dark crew is not merely poorer, it is
**lost**. Light buys *orientation*, continuously, in rooms with nothing in them
at all. That makes a lit stretch a decision with duration, which is the only
kind of decision a rising meter can price.

It also hands the solo keeper something they badly need: a reason to light up
that does not depend on finding anything, in a run where nobody else is
carrying.

**The four-line summary of the whole mechanic, his:**

- Dark crews can still survive and haul basic crates.
- Lit crews can identify which recovery actually matters before committing.
- Relics are an explicit temptation rather than a hidden collectible.
- Route clues make light useful even when there is no loot nearby.

**What they are for:** the collection, the plinths, the plaques, and the story.
A relic goes on a shelf with the finder's name on it and stays there.

**Never:** spent, sold, converted, consumed by an upgrade, or required by one.
Locked in `DECISIONS.md` and restated here because it is the thing a progression
pass will reach for first: the moment relics buy something, the lantern stops
being a choice and becomes an obligation, and "your light is how they find you"
turns into "turn your light on or fall behind."

They do cost something to carry — the burden penalty — and the first workshop
line reduces it without ever reaching zero. That is the right shape: carrying
your prestige out is meant to be felt.

### "Light is survival" — true at run scale, fatal at moment scale

Chris, 2026-10-28: *"light isn't optional, it's survival."* True in the sense he
meant it, and worth pinning down precisely, because the phrase is memorable
enough to end up in a store page and it is one word away from the failure this
page already warns about.

| Scale | Is light optional? | Why it must be that way |
| --- | --- | --- |
| **Across a run** | **No.** A crew that never lights up is lost and poor — no relics, no crate stamps, no route | If darkness were a complete strategy, the lantern would be a side dish and the meter would price nothing |
| **In any given moment** | **Always yes.** Every second lit is a choice with a cost | If light were ever *required* right now, there is no decision — just an upkeep bill |

So the accurate form is **light is necessary, and never safe.** You cannot win
without it; you cannot afford it continuously. The run-scale necessity is
precisely what makes the moment-scale choice tense, and collapsing either half
breaks the game in opposite directions.

What this rules out, concretely: a darkness timer, a mandatory-light objective,
a lit-only extraction, or any rule phrased *turn your light on or fall behind.*
If a player ever says "I had to have it on," something has gone wrong — the
sentence we want is **"we decided to light it."**

## Restoration crates — home

**Found:** heavy, awkward, unmistakable. A crate is a whole-trip decision.

**What they are for:** the hall and the town. Rooms opening, lights coming on,
the three landmarks. Credited by name on the Good Cheer Club roll, which is the
order's actual history — the building was finished a room at a time, funded by
name, and the roll is that.

**Never:** your kit. A crate must never make you stronger, or the hall becomes
a stat farm and "the point is not a number" stops being true.

**Spectacle only, Chris's call:** finishing a landmark lights it and says what
came back. No bonus.

## Upgrades — what the kit is allowed to be

**On the shelf:** things that change how you *carry and handle*, not how you
fight or survive.

**Built today:** the relic-burden line, Harness then Cradle, each a smaller
penalty for carrying prestige out.

**Never on the shelf**, locked: revives, sprint speed, enemy durability,
anything touching the lantern, and anything that removes or shortens the trip
out. Those are the tension itself.

## Two fields to retire

Both found by audit, both still in `data/loot.json`, both read by nothing:

- **`carrySlots`** — there is no carry cap anywhere in the game. Selling
  capacity would mean first *adding* a limit so the shop can sell it back,
  which makes the base game worse to create a product. Recommend deleting the
  field rather than leaving it to look like a decision that was made.
- **`relayBanksPerRun`** — dead with the bank. Chris: "that knob is dead design
  baggage."

Deleting either is a value change in a data file, so both are left for Chris.

## What a crate is for, more precisely — added 2026-10-26

A crate is **stamped with the department it came from**, and a room at home is
restored by crates from the department that would have supplied it. The
dormitory wants bedding off the second floor; the kitchen wants housewares off
the third; the meeting room wants the auditorium's chairs, four floors up with
one way in, which is why the hardest room at home is the last to open.

This adds **no fifth resource** and the rule at the top of this page still
holds: a crate answers exactly one question, *what does home look like?* It
gains one attribute — where it came from — and what that buys is a route
decision. Reasoning and the full mapping are in `docs/THE-STORE.md`; it is one
field on the crate spawn plus a cost line per room, and both figures are
Chris's.

## The hole this page could not close — now named, still unbuilt

Crates are the only thing that restores the hall, and **the hall currently has
nothing to restore**. The only progress visuals in `HubService` are the lamp
wall, the relic plinths, the roll board and three landmark plates. There are no
rooms that open.

That is the first session's whole payoff — "you come home and a light comes
on" — and it is the largest unbuilt thing in the design, bigger than any zone.
Specified in `docs/VERTICAL-SLICE.md`.

**What changed on 2026-10-26:** the rooms now have names and jobs rather than
being an abstract "rooms open". The ladder in `docs/QUESTS.md` gives each one a
repair, a visible home and a cost — the departure generator, the workshop
tools, the radio.

**And later the same day the hole stopped being a hole.** Four rooms are in
`data/ledger.json` and `LedgerService` opens them: the generator room, the
workshop, the lift house, the radio room, at 1 / 3 / 6 / 10 crates, which Chris
read and kept on 2026-10-27. The crates
are **committed, never deducted** — `contributions` is credit and the handoff
locked it as never removable, so what a keeper has left to spend is derived
from the total minus the rooms already opened. Which means the landmark plates
still read the gross figure and are untouched: the landmarks record what you
brought home, the rooms record what you did with it. All that is left is the
geometry, which is the map lane's.
