# The progression gap — what a run pays and what it buys

Short version: **a finished run pays out, and there is nothing to spend it
on.** Materials have been accumulating in every profile since banking landed.

This file exists so the decisions that would open the shop are in one place
for Chris, instead of being guessed at in code. Nothing here is built.

## What already works

| Piece | Where | State |
| --- | --- | --- |
| Run payout | `PlayerDataService.grantPayout` | credits `profile.materials` per tier on every completed run |
| Persistence | `PlayerDataService` | `materials`, `upgrades`, `cosmetics`, `contributions`, `runsCompleted` |
| Lamp Wall | `HubService` | one lantern per keeper out, lit/taken down/put out |
| Crate hand-in | `HubService` | crates become the Good Cheer Club's, credited by name on the roll |
| Relic plinths | `HubService` | shelved, with the finder named on the plaque |
| Stock readout | `WorkshopService` | the Quartermaster tells you what you are carrying |

## What is declared and dead

- **`profile.upgrades`** is written by nothing and read by nothing. It has
  been in the Profile type since the start.
- **`profile.cosmetics`** likewise.
- **The four sinks named in `data/loot.json`** have no consumer:

  | Tier | `feeds` | Fixture in the hub | Wired? |
  | --- | --- | --- | --- |
  | Supplies | `WorkshopUpgrades` | WorkshopBench, WorkshopPlot | no |
  | Components | `GadgetCrafting` | WorkshopBench | no |
  | Relics | three feeds | RelicBench, RelicPlinth | plinths only |
  | RestorationCrates | `LanternholdLandmarks` | LandmarkSite ×3 | no |

Those five fixtures are placed, named and signed in the map. Since this
commit they say "not open yet" when you press them rather than doing nothing,
which is the difference between unfinished and broken — but they are still
unfinished.

## What I did not decide, and why

Every number that would open the shop is a balance value, and this repo is
deliberate that those are Chris's. `data/lantern.json` alone carries four
"Chris's call" notes, one of which removed the lantern's fuel gauge entirely:

> *"i don't think the lantern should ever run out of light... the risk is
> always having it on."*

An upgrade shop invented in a service file would quietly overrule that kind of
decision. So nothing below was guessed - Chris decided it.

## Decided, 2026-10-24

Chris locked these:

- Permanent, **per-player** workshop upgrades.
- Currency is **Supplies and Components only**. Relics stay named prestige on
  the plinths; crates stay town restoration on the Club roll. Neither is money.
- Landmarks are **spectacle only**, at **10 / 25 / 50** crates.
- **Not for sale**, ever: revives, sprint speed, enemy durability, anything
  touching the lantern. Those are core tension, not shop stats. Nor
  `relayBanksPerRun` - it is read by nothing and is baggage now that the relay
  is a route board.

## The three proposed knobs, checked against the live code

He asked for the knobs to be verified before anything was built on them. Two
of the three were not what they looked like.

| Knob | Verdict |
| --- | --- |
| **Carry capacity** (`carrySlots`) | **Not a knob.** `carrySlots` is in `loot.json` and is read by *nothing*. There is no carry cap in the game at all. Selling "more capacity" would mean first adding a limit, which makes the base game more restrictive so the shop can sell the restriction back. |
| **Interaction speed** (`pickupSeconds`) | **Enforced, but not per player.** It sets `ProximityPrompt.HoldDuration`, which is one value shared by everyone looking at that prompt. A per-player version needs per-player prompts plus server-side hold timing - a mechanism, not a config read. |
| **Relic carry penalty** (`burden`) | **Works today.** `RunStateService` already computes burden per player from their own haul and applies it to their own humanoid. This is the one that shipped. |

## What is built

`data/workshop.json` holds the catalogue; `WorkshopService.purchase` sells;
`RunStateService` applies the effect. Two upgrades on one line, the second
gated behind the first:

| Upgrade | Cost | Effect |
| --- | --- | --- |
| Relic Harness | 14 Supplies + 4 Components | relic penalty x0.65 |
| Padded Cradle | 30 Supplies + 12 Components | relic penalty x0.40 |

Carrying three relics: **-12% unupgraded, -8% with the Harness, -5% with
both**. It never reaches zero - carrying relics out is meant to cost
something.

**The shop needs a published place.** `PlayerDataService` refuses to save a
profile that did not load cleanly, and DataStore is unavailable until the
place is on the web, so in Studio every purchase is refused with "shop needs a
published place - no profile to save to". That is deliberate: selling against
a profile that cannot be saved takes real materials and hands back an upgrade
that evaporates on the next join.

## Still open

1. **Interaction speed** needs the per-player prompt mechanism above before it
   can be sold. Worth it, or drop the knob?
2. **The landmarks need a shared total.** Each player's own crate count
   persists in `profile.contributions`, but the *town's* total only exists as
   a session variable in `HubService` - it resets when the server restarts.
   10 / 25 / 50 against a number that resets is not progression. This needs a
   server-wide persisted counter, which is a new persistence surface
   (`PlayerDataService` is per-player only). Your call on whether the town
   total is global across all servers or per-server.
3. **What else goes on the shelf** once the first line proves out.

