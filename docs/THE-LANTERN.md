# The lantern — what it is, how it works, and what it costs

Status: Chris's three-mode model is **his, 2026-10-29**, recorded as
`DECISIONS.md` row 67. Everything marked *proposed* below is Claude's
derivation and awaits his word. Everything marked **locked** is cited to the
row that locked it and is not up for grabs here.

Written answering four questions he asked on 2026-10-29: *"What happens if a
lantern loses all its charge? We can't add more fuel? How does a lantern work?
What is it made from, how does it work? Do they upgrade?"*

The constraint running through all four answers: **the lantern is the one
variable the whole game measures.** Row 3's gate asks whether testers kill
their lanterns. Every answer below is checked against whether it keeps that
question askable.

---

## What it is, and why the fiction already decided most of this

`PREMISE.md` is load-bearing here and it is not vague: **the machines run on
light.** They were service units in a city that had gone solar, and when the
storm took the grid they fell back on ambient light. In daylight they are fed,
all awake, everywhere — which is why nobody goes. At night they run on reserve
and some stand dormant exactly where they stopped.

> *"So a lantern is not only a thing that reveals you. It is a small sun, and
> you are feeding them."* — `PREMISE.md`

That single sentence fixes most of what a lantern can be:

- **It emits the thing they eat.** Not a signal they detect — a supply they
  consume. This is why light is not a stealth mechanic with a vision cone
  bolted on, and why there is no "light repels small enemies" exception
  anywhere in the design.
- **It cannot be electric.** The grid is dead and has been since the first
  flood. A lantern that ran on the grid would be dead with it, and a lantern
  that ran on a battery would make the storm its charger — which would hand
  the player a reason to want the storm.
- **It is amber, against the storm's electric blue** (`PREMISE.md`). That is
  the colour-blind readability rule carrying the premise rather than decorating
  it: their power is blue and yours is warm, and you can tell at a glance whose
  light you are standing in.
- **It belongs to the order, not to the scavenger.** The Lightkeepers'
  work, since the first flood, has been *"keeping the records and keeping a
  light burning"* (`PREMISE.md`). `LANTERNHOLD.md` makes it literal: *"while
  you are out there, your lamp burns here, where everyone can see it."*

### So what is it made from — *proposed*

**A sealed amber mantle in a brass-and-glass body, serviced at Lanternhold and
not in the field.** Incandescent rather than electric: a mantle brought to
heat, giving continuous warm light with no flicker and no cold start.

Three things that shape earns, which is the test for whether a piece of fiction
is worth keeping:

1. **It explains why a machine reads it as a sun.** A continuous warm
   broad-spectrum source is the thing a solar unit was built to drink. A cold
   blue LED would not be food, and then the premise stops working.
2. **It explains why the field cannot refill it.** The mantle is the rare part
   and the hall is where it is serviced, so "go home to the hall" is a
   mechanical necessity rather than a menu convention — and the hall's lamps
   and your lantern become the same technology, which is why the order's
   survival and your light source are the same problem.
3. **It explains the flicker we do NOT have.** A lantern that stutters would be
   a scare instrument, and `SOUND.md` already has one of those in the storm
   strike. The lantern is steady; the building is not. Keeping those apart is
   what lets a flicker mean something when we eventually want one.

The hall's two halves (`PREMISE.md`) give the fiction its weight: *the earliest
records burned.* An order whose first catastrophe was fire now keeps its light
in a sealed body and keeps it burning anyway. That is a group with a reason to
be careful with lanterns, which is the player's whole relationship with the
object.

---

## The three modes — row 67, Chris's, 2026-10-29

| Mode | Hands | What it does | What it costs |
| --- | --- | --- | --- |
| **Stowed, off** | Free | Full movement; darkness | You lose light and orientation |
| **Stowed, lit** | Free | A low, body-mounted pool for route-finding | Full exposure. The building sees light either way |
| **Raised, lit** | One occupied | You aim the cone, read relics, inspect shelves, light a route deliberately | Exposure, plus a hand |

His framing of the middle row is the part that makes this a design rather than
a convenience: **stowed-lit is convenience, not the best vision mode.** It
lights your feet and nearby route clues; it does not cleanly reveal a shelf
across the room and it does not substitute for raising the lantern to identify
a relic. And *it costs the same exposure*, because the building sees light
either way.

That last clause is what stops the middle mode from being strictly better than
the other two. Without it, stowed-lit would be free light and the other modes
would be for people who had not noticed.

**Bulky carry is the fourth state and it is a consequence, not a mode.** A
crate takes both hands, so the lantern is stowed by physics: no mantle, no
revive, no tool use until you set it down. `data/loot.json` already authors
this — `RestorationCrates` carry `carrySlots: 0`,
`blocksCombatWhileCarried: true`, and `restorationCratesArePocketable: false`.
The data was waiting for the verb.

### Peek — *proposed, and deliberately not built yet*

Hold the lantern past a doorframe or a shelf edge. The cone reaches the room
before you do; you learn what is in there, and the room learns about you.

Chris's reason for preferring this to a generic camera lean is the right one
and it is the project's own rule: **you gain information by risking light.**
A lean gives free knowledge, which is a corner-camera exploit with better
manners. This gives knowledge at the exact price everything else in the game
charges.

Not built until real shelves and doorframes exist, per his call. The mode model
is worth locking now because it decides what `ULLKLanternComponent` is a state
machine *of*; the verb that uses it can wait for geometry.

---

## What happens when the charge runs out

**Today: nothing, because it cannot happen.** `data/lantern.json` authors
`charge.secondsOfLightPerFullCharge: 0`, which means light does not drain.
The charge has exactly one spender in the whole game — the Anchor Revive, at
`chargesPerRun: 1` — and `autoOffAtEmpty: true` is therefore inert. Chris's own
note in the data says why:

> *"Not a fuel cost: with light unlimited, spending the charge would strand a
> player lightless for the rest of the run."*

So the honest answer to *"we can't add more fuel?"* is that there is no fuel to
add, by decision, and the field is a resource with one use rather than a tank.

### Recommendation: no fuel timer — *proposed*

Three reasons, in order of how much they would cost to get wrong:

1. **A fuel timer spoils the experiment.** Row 3's gate asks whether players
   *choose* darkness. If darkness arrives because a bar emptied, a dark run
   proves nothing — we would be measuring a tank, not a decision. The one
   question M4 exists to answer would stop being answerable, and we would not
   notice, because the data would look the same.
2. **It competes with the cost that already works.** Light already costs
   exposure, charge in the machines, and a hand. A fuel bar is a fourth, duller
   cost that pulls attention away from the three real ones — the same argument
   that ruled out a lantern speed penalty, and it applies with more force here
   because a timer is a thing players watch instead of watching the room.
3. **It would be the one upgrade everybody wants and nobody can have.**
   `REWARDS.md` locks *"anything touching the lantern"* off the shelf. Add
   fuel and the shop has an obvious hole in it where the fix should be.

**What to do instead, if light ever feels too free:** the levers already in
`data/` are the radius, the intensity and the storm multiplier — all authored,
all portable, all already read by the enemy rules that have to agree with them.
And the three modes give a fourth: make raised-lit cost more than stowed-lit.
None of those requires a bar.

---

## Do they upgrade?

**No, and this is already locked.** `REWARDS.md`: *"Never on the shelf,
locked: revives, sprint speed, enemy durability, anything touching the lantern,
and anything that removes or shortens the trip out. Those are the tension
itself."* `GAME-THESIS.md` says it as a rule: *"No upgrade may remove the
lantern decision."*

The reason is worth restating because it will come up every time the shop looks
thin: the lantern is the instrument the game is calibrated against. Every enemy
radius in `enemies.json` is tuned to its 30-stud reach; `aggroRadiusLit` 38 is
*eight studs past the edge of your own light*, which is row 5's trade made
physical. Sell a brighter lantern and every one of those numbers quietly means
something else.

**What upgrades instead, which is not a consolation:**

- **The hall.** Restoration at Lanternhold is visible, permanent and the thing
  the crates are for.
- **Everything you carry that is not the lantern.** Gadgets, kit, the relic
  harness and cradle that reduce the burden of carrying prestige out.
- **Verbs, not power.** The three modes, the mantle, the vault, and peek are
  new things you can *do* at full price rather than discounts on what light
  costs. `GAME-THESIS.md` calls this capability with cost, and it is the only
  axis the lantern is allowed to grow along.

---

## The one thing that needs Chris's word

**`DECISIONS.md` row 45 and `docs/THE-NIGHT.md` disagree about what the meter
is**, and his 2026-10-29 question — *"a lantern should not worsen or accelerate
the weather"* — is exactly that disagreement surfacing.

- Row 45, **locked**: the meter *"is now how much of tonight's storm has
  arrived."* On that reading a lantern cannot touch it, and the current
  `stormChargeMultiplier` is wrong.
- `PREMISE.md`: *"Storm Charge only ever rises — it is not weather. It is the
  machine population waking up."* And *"light accelerates Storm Charge — you
  are charging them directly."*
- `THE-NIGHT.md`, **proposed** and already written: the meter is **charge in
  the machines**, with two sources — the storm, which is nobody's and
  inevitable, and your lantern, which is yours, optional and immediate.

The proposed reading is the one that survives his rule without losing anything,
and it already has the sentence for it:

> **Every time you light the lantern, you do to yourself, early, what the storm
> is going to do to you anyway.**

See the closing section of `THE-NIGHT.md`. What it needs is a decision about the
**name**, because "Storm Charge" is what made this ambiguous in the first place
and the mechanics are not what is wrong.
