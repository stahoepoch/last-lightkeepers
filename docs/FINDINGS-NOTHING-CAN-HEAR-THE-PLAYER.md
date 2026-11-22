# Nothing in this game can hear the player

**Found 2026-11-22**, one day after `LLKSweeperPresence` gave the Sweeper a
voice. Fixing one direction of audibility made the other direction's absence
obvious. **Built and verified the same day.** `HANDOFFS.md` M20.

---

## The measurement

`FLLKSweeperSenses` is the complete list of what a Sweeper knows each frame.
Before today it held: `Notice` (sight, concluded by `LLKPerception`),
`KeeperPosition`, `DistanceToKeeperStuds`, `bHasPatrolPoint`,
`bAtPatrolPoint`, `MisplacedItemPosition`, `DistanceToMisplacedStuds`,
`bHasStockBay`.

There is no sense of sound in it. There never was one, in any family:

```
$ grep -rniE "hear|noise|audib|sound" Source/LLK/Public/Enemies/
Source/LLK/Public/Enemies/LLKSweeperPresence.h  (the motor, added yesterday)
```

Every match was about the machine *making* sound. **Nothing in this game can
hear the player.**

## Why this matters more than it sounds

`LLKPerception`, `LLKVisibility`, `LLKLightField`, the lantern's whole notice
cost and five authored aggro radii exist to make *being seen* an interesting
risk. That is a lot of machinery on one half of stealth. The consequences of
the other half being missing:

- **Sprinting was free.** In a lit aisle a sprint is something that can be
  seen, so it carried a visibility cost. In the dark it cost nothing at all.
- **Crouch was a visibility tool and nothing else.** `data/lantern.json`
  describes it as *"a deliberate slow mode for tight shops and careful
  approach"*. It bought approach. It did not buy quiet, because quiet did not
  exist as a quantity.
- **`ROADMAP.md` slice 3 asks a fresh player to "identify a cue and choose
  hide, detour or sprint."** Two of those three are the same choice when
  movement is silent — detour and sprint differ only in how long you are
  visible.
- **No puzzle could cost anything acoustically**, which is most of what makes
  an environmental puzzle frightening to solve rather than merely fiddly.

This is a near relative of the project's recurring failure shape. Not quite *a
correct system over an absent thing* — this is **a correct system over half a
thing, where the missing half is the one that would have made the present half
a choice.** Light is a gamble you take against sight. Nothing was a gamble you
took against sound.

## The fix

| file | what it is |
| --- | --- |
| `Source/LLK/Public/Enemies/LLKHearing.h` | the model and the argument for it: a noise, its radius, and whether it reached a listener. Pure, no `UWorld` |
| `FLLKSweeperSenses::HeardNoisePosition` | a `TOptional<FVector>`, pre-resolved, exactly as `Notice` arrives pre-concluded |
| `FLLKSweeperStep::bNoiseInvestigated` | reported once, so a playtest can see a machine leave its route for a reason the player caused |
| `ALLKSweeper::Observe` | measures the keeper's noise from their real ground speed and asks `LLKHearing` whether it arrived |
| `ALLKTestGameMode` | the readiness gate now reports what each machine can hear from where it stands |

### The radius is one second of travel, and that is deliberately not a new number

```
RadiusStuds = (speed in cm/s / StudsToCm) x SecondsOfTravel
```

At `SecondsOfTravel = 1` and the gaits Chris authored in `data/lantern.json`:

| gait | studs/s | carries | vs. Sweeper's dark sight (12) |
| --- | --- | --- | --- |
| crouch | 3 | 3 studs (84 cm) | a quarter of it — **genuinely quiet** |
| walk | 5 | 5 studs (140 cm) | well inside it — **quiet enough to work with** |
| sprint | 12.5 | 12.5 studs (350 cm) | just past it — **a decision** |

**The third row is the mechanic, and it fell out of Chris's own numbers rather
than being tuned into them.** Running past a Sweeper in an unlit aisle is now
about as findable as standing in front of it.

`aggroRadiusLit` is 38 studs, three times the loudest noise a keeper can make,
so **sound never exceeds sight in the light.** Light stays the dominant way to
be found. That is the game's central gamble and a new sense must not quietly
overturn it; `tests/hearing.test.mjs` fails if it ever does.

It also rescales itself. `data/lantern.json` has been globally rescaled twice —
halved on 2026-11-04, then the child-scale pass on 2026-11-21 — and
`enemies.json` carries a `_retuneNote` explaining that every speed moved
together because *"a chase is a ratio"*. A radius derived from speed survives
the next rescale with no note. An authored `sprintNoiseStuds` would be the
third number somebody has to remember to move, and the one nobody would.

### Velocity, not gait

The speed read is `GetVelocity().Size2D()` — actual ground speed, not the
nominal speed of the movement mode. That matters three times:

1. Burden and stumble already multiply it, so a loaded keeper limping is
   correctly quieter than a clean one running.
2. **A keeper standing still makes no sound at all.** Stopping is the verb
   this gives the player, and it only exists because the input is how fast
   they are really going.
3. `Size2D` and not `Size`: a keeper dropping down a stairwell must not
   broadcast a sprint they did not run.

The third is a guard in `tests/hearing.test.mjs`, because it is a one-character
mistake that would make falling the loudest thing in the game.

## The design claim, stated so it can be argued with

**Hearing can start a search. It can never start a pursuit, spend the
telegraph, or extend a search already running.**

Three rules, in order of how much they matter:

1. **Sound is not sight.** A machine that heard a noise walks to the spot,
   looks for its authored interval, and gives up. If the keeper is there it
   will be *seen*, by the rules that already existed, and that is the only
   route to contact. This is the same contract `LLKSweeperPresence` states
   from the other side: hearing the machine must never imply being seen by it,
   and being heard must not either.
2. **It does not telegraph.** Row 16 buys a second of warning for a keeper who
   has been *found*. Spending it on a false alarm teaches a player to ignore
   the one cue that means they have actually been found.
3. **It triggers from `Patrol` only.** From `Search` it would let a sprinting
   keeper refresh the clock and be followed across the store, which slice 3
   refuses outright. From `Stock` it would mean sound interrupts the filing
   job, whose authored rule is that *being seen* is how you stop it. Widening
   that is Chris's call, not a side effect of adding a sense.

**The precise bound, stated honestly.** Rule 3 prevents a refresh *within* a
search; it does not prevent a new search starting the frame after one ends. So
a continuously-sprinting keeper can be re-investigated once per search
interval. In practice that self-limits on the escape invariant Chris already
tuned: sprint is 350 cm/s against the Sweeper's 168, so a keeper running away
leaves the 350 cm hearing radius inside about two seconds. A keeper sprinting
in circles around a machine keeps it searching, and that is them choosing to.

## What is tested, and the gap no C++ test can see

`LLKHearingTest` ("LLK.Enemies.Hearing") pins eight claim groups: the radius is
one second of travel for each gait, standing still is silent at zero distance,
the boundary is where the arithmetic says, hearing starts a search from patrol,
sight outranks sound, sound cannot extend a search, twenty seconds of constant
noise never causes a pursuit or a telegraph, and a silent floor behaves exactly
as it did before.

**All of that is true of a game where nothing can hear the player**, because
the test builds the senses itself. `HeardNoisePosition` could sit in the struct
forever with no pawn ever setting it, every assertion green, and the keeper as
inaudible as they were yesterday.

So `tests/hearing.test.mjs` checks the two halves C++ structurally cannot: that
something actually fills the sense and the brain actually reads it, and that
the derived radii still line up with the two JSON files they were derived from.
Those radii are a claim about `data/lantern.json` and `data/enemies.json`
*together*, and a C++ constant copied out of them is a copy that drifts.

## Verified, in a running game

```
LogLLK: Display: LLKSweeper_0 has ears: from 173.2 studs away it hears a keeper
        moving faster than 173.2 studs/s (crouch 3, walk 5, sprint 12.5).
        Standing still is silent at any distance.
LogLLK: LLKSweeper_0 can reach the keeper: 11045 cm of path over 4850 cm
        straight (2.28x), 65.7 s at 168 cm/s.
LogTemp: Display: Map readiness: ready.
```

`LLK.Enemies.Hearing` reported `Result={Success}` — checked **by name** in
`Saved/Logs/LLK-Automation.log`, 37 successes and 0 failures, rather than
inferred from the runner's "LLK automation passed", which earlier this week
exited 0 having never run a test. `npm test` is green across all fourteen
suites, `tests/hearing.test.mjs` 16/16.

### The absent log line that did its job

The first version of that report lived in `ALLKSweeper::Observe`. It never
printed. `Observe` is only reached from `Tick`, and `LLK.CheckMap strict` ends
the process before the game ticks — so the line could not fire in the only
harness that runs it.

**A keeper standing still is correctly silent, which means a Sweeper with
working ears and a Sweeper with none produce exactly the same empty log in
every unattended boot.** That is precisely the trap the navmesh taught: a clean
log is not evidence. The report moved to the readiness gate, which already has
both actors, runs in the gate, and needs no path — hearing does not depend on
one, and a machine with no navmesh to the keeper can still be sent looking by a
sound.

It reports the *threshold* rather than the model: the speed a keeper at this
distance would have to exceed to be heard from here. One number, read against
the three gaits, and a playtest can tell at a glance whether a machine is
reachable by a run, by a walk, or not at all.

## What this does not claim

- **No keeper has moved in front of a machine yet.** Every number above is
  arithmetic, a pure brain rule, or a log line from a `-nullrhi` unattended
  boot in which nobody takes a step. That the sense is wired to real positions
  and computing real answers is proven; that a sprint across an aisle turns a
  machine around is a judgment in play, and the loudest thing this finding
  cannot tell you.
- **The absolute reach is short.** 12.5 studs is 3.5 metres. At spawn the
  machine and the keeper are 173 studs apart and nothing the keeper does is
  audible — by a factor of fourteen. This sense bites in close quarters,
  aisle-to-aisle, and not across the sales floor. Whether that is enough is a
  play question; `SecondsOfTravel` is the single lever, and raising it scales
  every gait together so the relationships the gaits were chosen for survive.
- **No occlusion.** A noise is heard through a gondola exactly as well as
  across an open aisle, which is the same honest limit `LLKPerception::
  SenseReaching` carries and names. It is the right next refinement and the
  wrong thing to guess at before anyone has played with the basic sense.
- **One noise source.** Only the keeper's own movement makes sound. A thrown
  bottle, a breaker thrown in the service core and a dropped relic all have
  the same shape — `FLLKNoise` is a position and a radius for exactly that
  reason — but nothing in the world emits one yet. That is what makes an
  environmental puzzle cost something, and it is the next thing to build.
- **Only Sweepers listen.** The other five families have no hearing, and
  giving them one means deciding per family what each is listening for, the
  same way `LLKPerception` had to.
