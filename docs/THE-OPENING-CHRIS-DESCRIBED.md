# The opening, as Chris described it

**Recorded 2026-11-22, from Chris, verbatim in substance.** Written down
because it is the first time the opening has been described end to end by the
person who owns it, and because `AGENTS.md` forbids implementing the capture
sequence until Grok finishes and Chris accepts one opening. **This is a record
and a brief, not a work order.** Nothing in it is built.

---

## What happens

A cutscene, watched rather than played. At the end of it the camera becomes
the boy's eyes and the chapter begins from exactly where he is standing.

1. A Lightkeeper expedition is working Boswell's. A family is with them — at
   least a father, an older brother, and the boy.
2. **There is a lot of talk between the boy and the brother**, and then the
   father realises something.
3. The realisation spreads: *"oh crap"* — the whole expedition understands at
   once that the machines are coming.
4. **They hide the little boy, fast.** This is the key beat: an adult decision
   made in a hurry, which is why he survives and they do not.
5. The expedition tries to get away from the robots. It fails.
6. **The machines take them.** The boy watches, from where he was hidden, as
   his family is dragged out of the store.
7. **He has no idea where they go.** That not-knowing is the engine of the
   whole chapter.
8. Cut to first person. The boy is just *there* — boom — at the dock.

## Why the hand-off point matters

The cutscene ends where the player begins, with no cut in space. Whatever the
last frame of the cutscene shows is the first frame the player controls, from
the same spot, facing the same way. A player who has just watched their family
taken from *this* floor is standing on it.

That is also what makes the opening a tutorial without being one: the player
already knows the machines take people, already knows hiding works, and
already knows nobody is coming back for them.

## What already exists, as of today

The dock is built and the family is standing in it. The first rendered
screenshot anyone has taken of this project — 2026-11-22 — shows four figures
at the loading dock in a pool of light, a pallet truck, crates, and
`RECEIVING` lettered on the back wall. They are stock mannequins with no
animation, and they are placed, not performing.

So the staging exists. What does not exist is any of the performance: no
dialogue, no sequence, no camera, no robot arrival, no drag-out, and no
hand-off to first person.

## What this implies technically, when it is time

Noted so the shape is known, **not scheduled**:

- A `LevelSequence` for the cutscene, and a hand-off that possesses the player
  pawn at its final transform rather than teleporting to a `PlayerStart`.
- The family needs to be skeletal actors that can be animated and then
  removed — they already are `SkeletalMeshActor`s, which is the right shape.
- The hiding place has to be a real spot on the floor the player can later
  look at and recognise. It should be somewhere a child fits and an adult does
  not, which is the same body-scale argument the climb verb is built on.
- The machines that take them should be the Sweeper family or something
  visibly related, or the player learns a threat grammar in the cutscene that
  the chapter then does not use.
- Dialogue is Grok's lane and is not drafted here.

## The one thing that would change the plan

Chris, same conversation: the store itself has to read as a real Boscov's
before any of this is worth staging, because a cutscene about losing your
family in a department store is only frightening if the room is a department
store. `FINDINGS-THE-STORE-IS-UNLIT-NOT-EMPTY.md` is the first move on that
and there is more to do.
