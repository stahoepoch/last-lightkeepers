# A child can get on the counter

**2026-11-22**, the same day as the push verb. The second half of the loop
`FINDINGS-THE-STORE-IS-ALREADY-A-PUZZLE.md` described and could not yet build.
`HANDOFFS.md` M23.

---

## Why this one is playable immediately and push is not

The pushable needs a fixture somebody places in a level, and nothing in any
level has one — the boot gate warns about it every time. **The climb needs a
counter, and Boswell's is full of them.** `LLKStoreFloorPlan` has authored
`CounterHeightStuds = 3.2` since long before any of this, which is 89.6 cm
against a keeper whose eye is at 125 cm.

So this verb arrives working on geometry that is already in the map. That is
the whole reason it went second.

## The rule, and it is one derivation

**You can climb onto what you can see the top of.** The ceiling is the
keeper's own eye height, computed by `LLKLanternFeel` from the capsule and the
camera offset, both scaled from the single authored 145 cm. A keeper rescale
carries it; there is no second number to forget.

| surface | height | |
| --- | --- | --- |
| kerb, cable tray | under 45 cm | the walk already steps over it — **not a climb** |
| counter | 89.6 cm | **climbable**, with 35 cm to spare |
| shelf | 168 cm | **not**, by 43 cm. Push something to its foot and climb that |

The floor of the range is the movement component's own `MaxStepHeight`, passed
in rather than assumed. Hard-coding a floor would be a second opinion about
what walking can already do, and every kerb in the store would become a
1.1-second animation nobody asked for.

`tests/climb.test.mjs` asserts there is **real headroom on both sides** — more
than 20 cm each way — so the sorting is comfortable rather than marginal. A
retune that nudged counters or the keeper would otherwise flip what the store
allows without failing anything.

## It is on the jump key

One key does the obvious thing in both cases: mantle if there is a ledge, jump
if there is not. No new input action, no new asset, nothing in `Config/` —
which also keeps the whole verb inside the runtime lane.

`TryClimb` is cheap when there is nothing there: one downward trace in front of
the keeper and no RPC at all.

## Slow, and never silent

| | |
| --- | --- |
| speed | 4 studs/s — a counter takes about 0.8 s, the ceiling about 1.1 |
| noise | 8 studs — walk 5 **< climb 8 <** sprint 12.5 **<** push 16 |

That ordering is the chapter's whole acoustic vocabulary, and each verb has to
be distinguishable by ear or a player cannot judge what they just did.

**The climb reporting its own noise is not optional.** A mantle moves the
capsule with `SetActorLocation`, so the keeper's velocity is zero for the
entire climb. Without `GetCurrentNoise` the most helpless second in the chapter
would be the quietest thing in it — the exact shape of defect this project
keeps finding.

### The constraint that fell out of the numbers

```
climb noise 8 studs  <  Sweeper dark sight 12 studs
```

So **a climb can never be heard-but-not-seen in the open.** Anywhere close
enough to hear the scramble is somewhere the machine could already see you. The
climb's sound only matters when sight is blocked — which is also the only time
anybody climbs: behind a counter, out of view.

Push is the opposite (16 against 12) and so is a sprint (12.5), which is why
those are audible commitments in the open and this one is not. That is the
right arrangement, and it is not worth "fixing" by raising the volume:
climbing louder than sprinting would break the vocabulary above, and a player
hauling themselves onto a counter in full view of a machine has already been
seen, so the sound would add nothing.

`LLK.World.ClimbChain` section 4 is built on exactly this — a counter to climb,
a wall between, and a machine seven studs off on the far side of it — and
`climb.test.mjs` asserts the inequality so a retune has to argue with this
paragraph.

## The refactor it forced, which was overdue

The climb's noise reached nothing at first. `ULLKClimbComponent` implements
`ILLKNoiseSource`, and `ALLKSweeper::Observe` iterates **actors**. A component
is not an actor.

Worse, `Observe` also computed the keeper's own noise itself, inline, from
their velocity — so there were **two code paths for "what can I hear"**, and
the climb fell straight down the gap between them: not velocity-driven, and
not an actor.

`ALLKCharacter` is now an `ILLKNoiseSource` like everything else, returning the
loudest thing the keeper is doing, and `Observe` just iterates sources. One
path. `tests/hearing.test.mjs` moved its ground-speed guard to the new home and
gained a check that the Sweeper no longer computes keeper noise itself —
because that guard failing is what proved the refactor had landed.

## Verified

```
LogLLK: Display: The keeper can climb onto anything up to 125 cm, reaching
        42 cm in front. A 90 cm counter is climbable, a 168 cm shelf is not -
        push something to its foot and climb that. 8 studs loud while
        scrambling.
LogTemp: Display: Map readiness: ready.
```

`LLK.Characters.Climb` and `LLK.World.ClimbChain` both `Result={Success}`,
checked by name — **41 successes, 0 failures**. Sixteen JS suites green,
`climb.test.mjs` 15/15.

`LLK.World.ClimbChain` spawns a real counter-height block, a real keeper, and
asserts the probe finds it, the mantle runs, **the keeper ends up standing on
top**, the climb goes quiet afterwards, a 168 cm shelf offers no ledge at all,
open floor offers none, and a machine behind a wall hears the scramble.

### Three failures worth keeping

1. **The world fixture crashed the runner.** Ticking components meant ticking
   `USkeletalMeshComponent`, which asserts that the tick function it is handed
   is not null — and the fixture has no real one to give. It now ticks only
   components from this module, which is also the honest scope: engine
   component behaviour is not what the layer exists to prove.
2. **Section 4 asserted the wrong thing twice**, and the second version is the
   design finding above rather than a test fix. It first put the machine six
   studs away in clear view and the machine telegraphed — because it had *seen*
   the keeper, which is correct and is what `LLKHearingTest` asserts.
3. **A `UPROPERTY` spliced into the middle of another declaration** and a
   `#include` placed after `.generated.h`. Both caught by the compiler, both
   mine, both from editing a 1,100-line header by pattern match.

## What this does not claim

- **Nobody has pressed jump at a counter in a real game.** The chain is proven
  in a test world; the opening map boot reports the reach but ticks no player
  input. This is the same gap the motor and the ears still have, and the thing
  a single playtest would close for all three at once.
- **There is no animation and no camera work.** The capsule interpolates up and
  then across, which is correct and will read as the keeper floating. A mantle
  needs hands on the surface and a head dip to feel like effort, and that is
  art and animation rather than runtime.
- **The mantle does not check what it is climbing.** Any surface under the
  ceiling with room above it is a ledge, including an enemy's roof, a window
  sill over a two-storey drop, or a stack the player has no business on. The
  room check prevents landing *inside* something; it does not express authorial
  intent about where a player may stand.
- **One direction.** You can get up and there is no climb down — the keeper
  steps off and falls, which at counter height is fine and at the top of a
  gondola would not be. Nothing stops a player stranding themselves somewhere
  the fall hurts, because falling does not hurt yet.
- **No ledge prompt.** The HUD says nothing about a climbable surface, so the
  verb is discovered by pressing jump at things. `FindLedge` is `BlueprintPure`
  and side-effect free precisely so a prompt can call it, and nothing does.
