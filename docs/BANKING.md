# The relay bank — keep it, and buy its safety with light

Status: **proposed**, 2026-10-21. Resolves `docs/DECISIONS.md` O3.
Evidence: `research_notes/…/banking_design.md`.

Chris's worry was the right one — *"no banking loot if it is gonna make the
game suck and die"* — and the research says the danger is real but the fix is
not removal.

## Why removing it would be the other failure

Two findings put the bank back on its feet.

**Total loss drives Roblox players off outright.** DevForum consensus is blunt:
full-loss-on-death makes players "leave the server or stop playing altogether,"
and partial loss is the standard fix. No bank means every wipe costs
everything.

**And loss aversion gets worse as the haul grows.** *Risking Treasure*
(CHI PLAY 2020) found loss aversion operates on purely virtual currency and is
**significantly more pronounced when more is at stake**. Applied here: a crew
late in a run, carrying everything they found, becomes progressively unwilling
to light the lantern at all. Every run would collapse into a stealth crawl at
exactly the moment the decision should be hardest.

**The bank is what lets a player cash out and go back to being brave.** It is
not a safety valve bolted onto the greed loop; it is the thing that keeps the
greed loop running more than once per expedition.

## The real danger, named precisely

A bank that is **free, instant, and on the efficient route** is auto-loot with
extra steps. Badlanders removed stealth, raised movement speed and auto-collected
loot until "nothing really means anything" — which made its insurance system
pointless and hollowed out its economy. That cascade is the thing to avoid.

So the bank must cost something. The question is *what*.

## What the evidence says about cost shapes

**Diablo III is the best-documented case, and it tried most of the options.**
Jay Wilson called town portals "a crippling combat exploit" and cut them before
ship. Blizzard then tried three cost shapes to rescue them — cooldowns, one-way
portals, and making scrolls rare — and per Wilson **none worked**: *"They just
changed the nature of it."* What they eventually shipped is a free, unlimited
portal whose only cost is **exposure**: a five-second cast that movement or
damage cancels.

That is direct evidence against three of the obvious knobs, and for one.

| Cost shape | Verdict | Evidence |
| --- | --- | --- |
| **Exposure** — vulnerable while it happens | **Adopt** | The only shape Diablo III kept after discarding the others |
| **A real detour** — relay off the efficient route | **Adopt** | Dome Keeper's Habermann: the wave clock is what makes the trip a decision at all; without it "you wouldn't really need to decide anything interesting" |
| Limited uses / cooldowns / rarity | **Avoid** | Blizzard tried all three; they "just changed the nature of it" |
| A fee or percentage cut | Unevidenced | No designer commentary found either way |
| Solo-banker bonus | **Avoid** | Incan Gold rewards the lone leaver, which manufactures exactly the fragmentation below |

## The decision

**Keep the relay bank. Make its cost exposure, and make the exposure light.**

Banking requires the lantern lit for the duration of the transfer. That means:

- You pay in Storm Charge, which is the game's only currency of risk.
- You announce yourself at the moment you have most to lose.
- **The safe act and the dangerous act become the same act.** That reinforces
  the one rule the whole game rests on, rather than sitting beside it.

It also means the bank cannot be made free by any upgrade, because the cost is
structural rather than a number.

## The co-op risk I had not considered

Blizzard's *second* stated reason for cutting town portals: they "tended to
split players up a lot."

That lands harder here than it did there. A relay one player can run alone
splits the crew — and in our game the one who walks off is carrying a lit
lantern, so the split is also a Storm Charge spike and a beacon pulling
machines toward whoever stayed behind.

**Mitigation, proposed:** the relay banks for every Lightkeeper within its
radius, so going together is strictly better than sending a runner. That turns
banking into a crew movement rather than an errand, and it preserves the
handoff's requirement that co-op creates roles.

## A guardrail for the workshop

Dome Keeper shipped a late-game lift that removed the carry entirely, and
Habermann names the resulting problem as having to "balance the game so that it
does keep being a challenge."

**No upgrade, cosmetic, pass or landmark may ever remove or shorten the bank
trip.** The handoff already says upgrades buy expression and teamwork, never
raw damage. This adds: and never the trip itself.

## What is genuinely untested

The researcher found **no prior art for a bank whose use feeds the escalating
meter.** The light-for-safety coupling above appears to be novel, which means
it cannot be settled from evidence and has to be answered by playtest.

The specific thing to watch: whether banking-while-lit is so expensive that
crews simply never bank, which would reproduce the turtle problem the bank
exists to solve. If that happens, the lever to soften is the duration of the
transfer, not the exposure itself.

Also noted: at a 6-10 minute target run, there may only be room for **two or
three relay windows**. One-way commitment mechanics assume a longer run and are
probably not available to us.
