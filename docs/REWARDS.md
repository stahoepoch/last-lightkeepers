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

**Found:** only with the lantern lit. They do not exist to a dark crew. This is
the single most important asymmetry in the game and nothing may soften it.

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

## The hole this page cannot close

Crates are the only thing that restores the hall, and **the hall currently has
nothing to restore**. The only progress visuals in `HubService` are the lamp
wall, the relic plinths, the roll board and three landmark plates. There are no
rooms that open.

That is the first session's whole payoff — "you come home and a light comes
on" — and it is the largest unbuilt thing in the design, bigger than any zone.
Specified in `docs/VERTICAL-SLICE.md`.
