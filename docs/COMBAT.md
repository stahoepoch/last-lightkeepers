# Can a keeper fight back

Status: **proposed**, 2026-10-26. Chris: *"we don't even get to like open that
up. We don't even have weapons to fight some things — like we're probably going
to need to fight enemies, maybe not just escape them, run away and hide from
them. We might need to fight them."*

The answer is **yes**, and it is not a reversal of anything. It settles a
contradiction the project has been carrying since the handoff.

## The contradiction, stated plainly

- **`data/enemies.json` gives five of the six enemies a health value** — 40,
  160, 25, 80, 55 — and `src/server/services/EnemyService.luau` writes them
  onto a real Humanoid in four places. **Nothing in the game can reduce them.**
  Every enemy in the build is walking around with a health bar nothing can
  touch.
- **`data/loot.json` says a crate carrier `blocksCombatWhileCarried`**, with the
  note *"a carrier cannot fight normally."* That field describes a combat
  system, and it is the only mention of one.
- **`docs/PITCH.md` says: "Is it a shooter? No. You can't really fight back."**

So the handoff assumed combat, the data shipped combat's numbers, the code set
them, and the pitch denied the whole thing. Chris is not adding a system here;
he is picking which of the three the project meant.

Only one locked decision touches this and it stays untouched: **`DECISIONS.md`
7, the Warden is unkillable and drops nothing.** It is the meter with a body.

## The one rule

> **You win the fights you start. Nothing a keeper carries can kill an enemy
> that has already seen them.**

That single sentence keeps everything the pitch promised while giving Chris real
combat. Before you are spotted you have options: remove the thing in the
doorway, break the one that is still dormant, clear a route. Once you are
spotted you are back in the original game — hide, run, distract, carry each
other out.

So "you can't fight your way out" stays true. What changes is that you can
**fight your way in**, which is a different verb and the one a salvage crew
would actually have.

## Hands — the balance, and it costs nothing to build

**The lantern or the tool. Never both.**

That is the whole of it. Light is already the game's one decision; making the
weapon compete for the same hand means every swing is taken in the dark, or
taken by someone who just put their light down in a building that notices.
No damage curve, no stamina bar, no numbers to tune — the tension is the
tension the game already has, pointed at a new verb.

It also makes a crew matter in the way `DECISIONS.md` O5 wanted: one keeper
holds the light, another holds the tool, and neither can do both.

## The three kinds, and they are not a weapon tier list

**1. The quiet removal.** The most on-theme combat in this game and the one to
build first. A dormant Window Walker in a doorway, a Ficus in a stairwell, a
Stocker standing dead-eyed in an aisle — unlit, unaware, in your way. You can
take it apart with a hold: several seconds of standing still, making noise, in
the dark. The scariest mechanic the game already has is a progress bar, and
this is that mechanic pointed at an enemy. No swing, no aim, no health bar
needed.

**2. The swing.** Found in the building, never bought: a pry bar, a stanchion,
a fire axe off a wall bracket. Heavy, slow, committed, and **loud**. It kills a
Sweeper (40) or a Glimmer (25) in a hit or two and it will not take down a
Stocker (160) or a Window Walker (80) before they reach you — which means the
existing numbers in `data/enemies.json` already encode exactly the right rule
and nobody has to retune them. Small things die. Big things do not.

The noise is the real cost. `swarmCallRadius: 45` is already in the data: kill
one Sweeper badly and you have called the rest.

**3. The made tool.** This is Chris's rung 2 — *"unlocks a simple defensive tool
or noisemaker"* — and the reason it is a rung rather than a starting item. It
is also what `data/loot.json`'s existing `GadgetCrafting` sink has been
pointing at since the handoff, with nothing on the other end of it. Consumable,
crafted from Components, thrown or placed:

- A **noisemaker** — a decoy with a position that is not yours.
- A **flare** — light somewhere you are not, which feeds the meter *and* pulls
  the swarm. Entirely in character for this game: the thing that saves you is
  the thing that costs you.
- An **arc tap or snare** — a few seconds of a big thing not moving, enough to
  get past rather than enough to win.

Consumables are how you survive something you *cannot* kill, and they stay
consumable so no amount of them turns into a power curve.

## What this must never become

- **A shooter.** `DECISIONS.md` 2 locks out PvP and the pitch's whole promise is
  that the skill is deciding when to light up. Ranged weapons move the skill to
  aim, and a crew that can shoot stops needing to hide.
- **A permanent upgrade line.** `DECISIONS.md` 23 forbids selling power and
  enemy durability. Tools are **found or made per run**, which keeps them off
  the shelf, off the store page, and out of the progression Chris already
  refused.
- **A reason to be loud on purpose.** If the best play becomes clearing every
  floor before looting it, light and risk stop trading against each other.
  Keeping big enemies genuinely unkillable by hand is what prevents this, and it
  is already true of the numbers.
- **Anything that touches the Warden.** It cannot be stunned, snared, slowed or
  hurt. Locked.

## What the systems lane needs, when Chris says go

Small, and most of it is already there:

- A damage path onto the enemy Humanoids, which `EnemyService` is already set up
  for — the health is written, the death branches (`humanoid.Health > 0`) are
  already the loop conditions in nine places.
- A `data/tools.json` on the same pattern as `enemies.json`: per tool, the
  damage, the swing time, the noise radius and whether it is consumed. **Schema
  mine, every value Chris's**, as with every other data file.
- A held-item slot that the lantern already competes for, and the one line that
  makes `blocksCombatWhileCarried` finally mean something.
- The dormant-removal hold, which is `InteractionService`'s existing hold
  pattern pointed at an enemy instead of a cache.

Nothing here is a new engine. It is a verb bolted to numbers that have been
sitting in the data unused since the first commit.
