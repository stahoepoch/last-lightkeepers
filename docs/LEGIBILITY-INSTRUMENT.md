# The legibility instrument — row 43's question, measured

Built 2026-10-31 by Claude, in `LLKRunMetrics` and `ULLKRunTelemetrySubsystem`.
Two new events and five new summary numbers, all of them derived from state the
game already computed.

**What it is for.** `docs/ROADMAP.md`'s M4 gate asks three questions and
`LLKRunMetrics` answered all three: do testers kill their lanterns
(`LitFraction`), do they bank (`RelicsTaken` against `RelicsSecured`), do they
argue for one more room (`ExtractionCalledSeconds`, `CancelCount`,
`HoldStallSeconds`). Row 43 locks the department store as the tutorial and
judges it on whether it **teaches**, and nothing measured that. A player who
never understood that taking a relic costs light produces the same numbers as
one who understood it perfectly and left early: a low haul, a dark run.

So the fourth question — *did you understand what you could do* — was the
highest-stakes question in the design and the only one with no instrument.

---

## What it reads, and what it must not be read as

**These are behavioural proxies for understanding, not measurements of it.**
Nothing below separates a player who did not understand the light rule from one
who understood it and chose to leave a brooch on a shelf. No arithmetic ever
will. What they do is bound the question: a run with no blocked attempts and a
fast first take did not have a legibility problem, and a run with eleven
`NeedsLight` refusals did.

The division of labour is the honest one. **Chris answers whether it felt
confusing; the instrument answers what he did.** Self-report about
comprehension needs a human who was there, and the roadmap's own reason for
instrumenting at all — Grizzard et al., that arousal habituates before players
notice it has — cuts the same way here. A file that forgets this becomes an
argument that sounds like evidence, which is worse than no file.

---

## The three numbers

### 1. Interact attempts, with the reason the game refused

**The highest-value event, because it records a decision the game already
made.** `ULLKRelicComponent::GetInteractBlock` computes exactly why an interact
cannot proceed — `Unidentified`, `NeedsLight`, or `None` — and before this the
answer was computed, acted on, logged at `Verbose`, and discarded.

Recorded in `ServerTake`, on the server, for successes as well as refusals so
the denominator is the one the player experienced. The distribution is the
thing: a run full of `NeedsLight` is a player who found the relic and not the
rule; a run full of `Unidentified` is a player reaching for things they have
never lit at all. **Those are different failures and they want different
fixes**, and no other number in the file can tell them apart.

Three decisions worth knowing about:

- **Announced before the range check.** A take refused because the keeper was
  across the room is not a legibility failure. It records as `None` with no
  `RelicTaken` after it, which leaves it visible in the event list rather than
  filed as a rule somebody misunderstood.
- **An unrecognised reason counts as a refusal, not a success.** `None` is the
  one reason that means it worked. A block reason added by a later rule must
  not quietly improve `RefusedFraction` by being unknown to the arithmetic.
- **`InteractAttempts` is reported beside `RefusedFraction`, always.** A run
  where nobody pressed the key and a run where everything worked both give a
  refused fraction of zero, and only one of them is good news. The readout says
  `NOBODY REACHED FOR ANYTHING` rather than printing a clean zero.

### 2. Seconds to the first successful take

Blunt, and derived rather than recorded — it is the first `RelicTaken`, which
the file has always had. **The honest reading is a floor, not a score:** it
includes walking to the shelf, so a long time is ambiguous and a short one is
conclusive.

### 3. A relic that was takeable, and was left

**The purest available signal that an affordance was visible and not
understood**, because it is the only event in the file that fires when a player
did *nothing*. A relic that is lit, legible and in reach is the game making an
offer; one that goes dark again un-taken after seconds of availability is an
offer that was not read.

Two things keep it from measuring noise:

- **It times the debounced state, not a raw sample.** `CanBeTaken()` is the
  same answer the prompt and the outline are drawn from, and its light half has
  been stable over two samples since `ab9818f`. An instrument timing a raw
  trace would manufacture an abandonment every time a keeper stood at a shelf
  edge — which is exactly where the hooded reading loop happens.
- **A three-second dwell floor.** A lantern swung past a shelf makes every
  relic on it takeable for a moment, and none of those is an offer anybody
  refused.

**Polled rather than bound, and the reason is a limitation of the gameplay
delegate.** `OnInteractableChanged` fires on the relic and carries no sender,
so a listener cannot tell *which* relic went dark — and the signal is useless
without the id. The sweep walks the relics it has discovered a few times a
second, which is the same shape the stall poll already uses for the same
reason: a condition has nobody to ask.

**Its honest limitation.** It is tracked by the relic, not by the keeper
looking at it, because focus is resolved client-locally and a remote client's
aim is not visible to a server-only instrument. So this measures *a relic the
light field made available and nobody took*. On a solo run that is the signal.
On a crowded one it over-counts, because one keeper's lantern makes relics
available to a room.

---

## What it is not allowed to do

The constraint on the whole telemetry layer applies unchanged, and
`tests/telemetry.test.mjs` enforces it: **no gameplay module may include
`LLKRunMetrics.h`**, because a rule that reads a metric is secret
rubber-banding, which Chris's fifth replayability pillar forbids.

This pass adds nothing that helps the player. `OnInteractAttempted` is
broadcast after the decision, and the take path neither consults it nor waits
for it; the abandonment sweep calls one getter. **An instrument must not become
a tutorial** — if a player cannot work out that taking costs light, the fix is
in the store, the prompt or the lantern, and this file's job is to say which.

---

## Tests

`LLK.Run.Metrics` sections 12–14, and seven new checks in
`tests/telemetry.test.mjs`. Both sets were proven red before the arithmetic
existed: eleven failing assertions in the automation test, and three in the
node suite against a deliberately mutated metrics layer.

What they cover beyond the happy path:

- **The zero that is not good news** — a run with no attempts at all, asserting
  that the readout says so in a sentence rather than reporting a clean
  fraction. The assertion checks for the specific sentence, because the
  existing readout already says `NEVER CALLED` about the extraction and a
  looser search would pass without the legibility line existing.
- **An unrecognised block reason** raises `InteractAttempts` and
  `RefusedFraction` and lands in neither named bucket, so a later rule cannot
  misattribute a failure or flatter the numbers by being unknown.
- **First take is the first one**, not the last, against a run with two.
- **A take is not an abandonment.** The sweep checks `IsTaken()` before
  counting, or every successful pickup would also read as a relic the player
  failed to understand.

Full suite: all 33 `LLK.*` automation tests pass, and 907 checks across the
thirteen node suites. `Tools/test_startup_config.py` fails on a
machine-generated Android File Server token in the uncommitted
`Config/DefaultEngine.ini`, which predates this work and belongs to another
writer.

---

## What is still missing, and whose it is

**The attempt that never reaches the server.** `TryTakeFocused` returns early
when nothing has focus, so a player pressing the take key at empty air — or at
a relic their aim never resolved — records nothing. That is a real legibility
signal and it is the one this pass cannot see: capturing it would mean a client
RPC for a non-event, which is more than instrumentation. Whoever wants it
should decide deliberately that the traffic is worth it.

**Nothing writes the numbers anywhere new.** `WriteToDisk` already puts one
JSON per run under `Saved/LLKTelemetry/`, and the new fields ride in it. There
is no dashboard and should not be one until somebody has read a few files by
hand and knows which numbers they actually reach for.
