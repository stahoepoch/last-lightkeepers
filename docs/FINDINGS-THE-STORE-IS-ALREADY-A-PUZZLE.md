# The store is already a puzzle, and nobody has spent it

**2026-11-22.** Chris asked for Tomb Raider / Stray style interaction — *"you're
going to push something away or do something"* — and for the scaffolding to be
laid so the work can go on bit by bit. This is the scaffolding and the first
verb. `HANDOFFS.md` M21.

---

## The thing that was already true

`LLKStoreFloorPlan` authors `CounterHeightStuds = 3.2` (89.6 cm) and
`ShelfHeightStuds = 6` (168 cm). The keeper's eye sits at 125 cm and their
reach is 150 cm of slant range.

**The top shelf of every fixture in Boswell's is already out of reach by 18 cm,
and nobody designed that as a puzzle.** The store was authored at adult scale;
the keeper was later scaled to a child. The gap between those two decisions is
free level design that has been sitting in the repo unspent.

It is also the same measurement that produced a *defect* a while back — the
lantern placed 189 cm above a 150 cm reach. That was a bug because nothing in
the game could climb. The difference between that bug and a puzzle is one verb.

So the loop is: **cannot reach → move something → stand on it.** This change
builds the middle link and the scaffolding under all three.

## The structural problem that had to be fixed first

`ULLKInteractorComponent` resolved focus for five types by name — casualty,
relic, extraction zone, lantern pickup, opening grille. Each cost the same
three edits: a member, a resolver, a server RPC, plus a new arm in
`BeginInteract` and a new clause in the condition that suppresses the zone
prompt. Five types, five copies of one shape, in a component that already
carries a comment warning that the fix for a hidden prompt is never to reorder
those arms.

A puzzle with three parts is that surgery three times. So `ILLKInteractable`
is one interface, one focus slot, one RPC pair, and anything added from here
arrives through it.

**The existing five are deliberately not migrated.** They work, they are
covered, and the grille has another writer in it right now. Doing both at once
would put a regression in the old path and a bug in the new one into one
commit, indistinguishable from each other. This stops the bleeding; migrating
is a separate change with its own proof.

**Each actor owns its own reach and aim.** The grille wants a read at its mark;
a pushable wants a shove at hand height; a thing on a top shelf wants a reach a
child does not have. One reach constant on the interactor would have to be the
loosest of them and would then apply to all of them.

## The verb

`ALLKPushableFixture` — a shop fixture a child shoves along an authored rail.

**It is not physics, and that is a deliberate engineering call.** There is no
rigid-body simulation anywhere in this runtime; `grep -rl
"SetSimulatePhysics\|AddImpulse"` over `Source/LLK` returns nothing. Opening
that door for a puzzle object means simulated bodies on a replicated pawn's
collision, and the first thing it costs is the ability to assert any of this
without a world. A fixture slides along an axis its author chose, between an
authored start and limit, at an authored speed. That is what Tomb Raider did,
it is legible — a player can see where a thing will go before committing — and
a crate that could be nudged anywhere is a crate that can be wedged in a
doorway to soft-lock the chapter.

Travel is a single replicated float, so the fixture cannot leave its rail,
cannot rotate, and cannot end a session anywhere its author never looked.

| number | value | against |
| --- | --- | --- |
| push speed | 1.2 studs/s | slower than a crouch (3); a Sweeper covers 5 studs for every 1 it moves |
| noise while moving | 16 studs | louder than a sprint (12.5), quieter than lit sight (38) |
| reach to start | 150 cm eye-to-body | the same child's reach the lantern pickup uses |

**Pushing is the loudest thing a player can do, and that is the cost.** Not
that it is hard — that it takes time you can be found in. That only became a
real cost yesterday, when machines learned to hear.

**Focus is re-checked every frame, not just at the press.** Otherwise a player
starts a shove, runs for cover, and the fixture finishes the job and makes all
the noise for them — the entire price of the verb, avoidable.

## Machines can now hear the world, not just the player

`ILLKNoiseSource` is the other half of yesterday's sense.
`FINDINGS-NOTHING-CAN-HEAR-THE-PLAYER.md` closed with the observation that only
the keeper emitted noise, which made hearing purely a tax: it could get you
found and it could not help you. A thrown bottle, a breaker in the service core
and a fixture grinding across tile all have the same shape, and now one of them
exists.

Sources are **asked, not pushed** — a listener iterates and queries, exactly as
`FindMisplacedItem` already does. A queued sound nobody drained would have a
machine investigating something that stopped two minutes ago, which is the bug
a player describes as the game cheating.

The scan runs **before** the keeper early-returns, so a fixture grinding in an
empty building is still a sound. Row 47's machine does not care whether anyone
is there to have made it.

### Salience, and the claim my own test refuted

When two noises both reach a machine, it goes to the more salient, and salience
is **excess reach** — how far past the listener the sound carried.

The first version of `LLKPushTest` section 6 asserted that a fixture beats
footsteps outright. **It failed, and the claim was wrong rather than the rule.**
With the keeper sprinting 2 studs from the machine and the fixture 9 studs away,
excess gives 12.5 − 2 = 10.5 for the sprint against 16 − 9 = 7 for the fixture.
The sprint wins — and it *should*. A keeper running past a machine's shoulder is
the more urgent thing in the room, and a rule that sent it to look at furniture
instead would be the worse rule.

So the true claim is narrower, and it is the one that matters:

- **A fixture outranks the noise of whoever is pushing it**, because the pusher
  is standing next to the thing. Without this, a distraction could never work.
- **It outranks a keeper creeping much nearer the machine.**
- **It does not outrank a sprint going past that machine's shoulder**, and the
  test now asserts that explicitly so nobody later "fixes" it.

Nearest-wins and loudest-wins both fail one of those three. Excess passes all
three, which is why it is the rule.

## The prompt comes off the interface, not out of the HUD

`LLKDebugHUD` draws the generic arm in the same position it occupies in
`BeginInteract` - below the authored openings, above loot - and takes the
label from `FocusVerb()`. That is the point of the seam reaching the HUD as
well: the five existing prompts are each a type this file was taught by name,
and the next interactable draws itself without another branch. A prompt
written in the HUD is a second opinion about what E does, and the whole reason
`ULLKInteractorComponent` exists is that there is only ever one.

## Verified

```
LogLLK: Warning: No pushable fixture in LLK_Boswells_OpeningPass. The push verb
        exists in code and there is nothing in this map to use it on, so no
        player can discover it here.
LogLLK: Display: LLKSweeper_0 has ears: from 173.2 studs away...
LogTemp: Display: Map readiness: ready.
```

`LLK.World.Push` reported `Result={Success}` by name in
`Saved/Logs/LLK-Automation.log` — 38 successes, 0 failures. `npm test` green
across fifteen suites; `tests/push.test.mjs` 16/16.

**That warning is the most useful line in this change.** The runtime, the
interface, the noise model and both test layers can all be correct while no
level contains a single pushable — which is this project's recurring defect
exactly, and nothing else in the build would have said so.

## What this does not claim

- **Nobody has pushed anything.** No level contains a pushable, so the chain
  from focus to prompt to press to travel to noise to a Sweeper changing state
  has never run end to end in a world. Every part is proven; the seam between
  them is not. Placing one is the map writer's lane, and the opening `.umap`
  has another writer in it.
- **The climb does not exist.** A fixture pushed under a shelf is 89.6 cm of
  floor to stand on, and 89.6 + 125 puts a child's eye above a 168 cm shelf.
  That arithmetic is the reason the verb is worth having and none of it is
  built. Until it is, a pushable is a thing you shove into a doorway or out of
  a sightline, which is real but is not the loop.
- **One object, one shape.** Sliding along a rail is the only kind of pushable.
  A thing that topples, a thing you drag behind you and a thing that breaks are
  all different actors, and the seam is what makes them cheap rather than what
  makes them exist.
