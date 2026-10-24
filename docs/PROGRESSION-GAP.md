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
decision. So the catalogue is empty rather than guessed.

## The decisions needed to open it

1. **What is on sale.** Upgrades, gadgets, cosmetics, or all three? The
   handoff's language ("the more you unlock, the more you can buy per floor")
   suggests unlocks gated on floors — is that the shape?
2. **What an upgrade does.** It must move something the code already reads.
   Candidates that exist today: carry slots per tier, revive hold seconds,
   sprint speed, Sweeper hits-to-knockdown. The lantern's burn time is NOT a
   candidate — it is deliberately infinite.
3. **Prices**, in which tiers. Supplies and Components are the obvious
   currencies; Relics and Crates already have homes (the Keeping Room and the
   Club) and may be better left as prestige than as money.
4. **Whether upgrades are per-player or per-crew.** The Club roll is explicitly
   "credited by name, never removable" and shared; the Workshop may want to be
   the opposite.
5. **The three landmarks** — THE WELL, THE BELL TOWER, THE LONG BRIDGE — need
   a crate threshold each, and a decision about what completing one gives
   beyond the sign.

Answer 1 and 2 and the rest follows quickly; the plumbing is all in place.

## When it is built

It goes in `WorkshopService`, which already binds the Quartermaster counter
and the four pending fixtures. The catalogue belongs in `data/` next to the
other tuning, so the values sit where Chris edits balance rather than in Luau.
