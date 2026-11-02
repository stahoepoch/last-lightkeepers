# The Fieldboard records observations, not state

For folding into `docs/HUD-STATUS-SCREEN.md`. This specifies the one idea
Chris picked: the board shows **what the crew found out, and when** — never
what is true right now.

---

## The invariant

> **Every row on the Fieldboard was put there by a player, and carries who
> and when.**

No row is written by the game. The moment the game authors one, it has become
a narrator, which is the same rule the lantern already obeys from the other
direction: *the lantern makes existing things legible and causes visible
machine responses; it never supplies hidden information or narrates the world
for the player.*

The radio is the only channel permitted to narrate, because a human is
talking. The board is the written form of that same permission.

---

## Two kinds of row, and the difference matters

This is the distinction that keeps the board honest and makes it
implementable.

**Attested rows — what your own body did.** Generated automatically, because
the server already knows they happened and the player cannot be mistaken
about them.

- *You* identified a relic (`bLegible` latched — the record already exists)
- *You* reached an extraction point
- *You* were downed, or revived someone
- *You* read a directory board

These cannot be wrong. They are history.

**Claimed rows — what you say about the world.** Created deliberately, by a
player pressing a key while looking at something.

- "Sweeper seen here"
- "Route blocked"
- "Nothing down here"

These can be wrong, and can become wrong. That is the point.

The split gives you a free design rule: **the board never auto-writes a claim
about the world.** Anything the game would have to infer, a player has to
assert. That single line prevents the whole feature from quietly drifting
into a radar.

---

## The record

```
FLLKObservation
    Kind          attested | claimed
    Subject       enum — Relic, Sweeper, Route, Extraction, Directory, Downed...
    Location      FVector, or a named place where one exists
    ObserverId    the player who made it. NEVER null. See "Tests", below.
    RunSeconds    when, in run time
    StormStage    the storm stage current at the moment of observation
    Note          optional short text, claimed rows only
```

Server-authoritative and replicated like everything else in this game.
`ALLKGameState` is the natural owner: it already carries `StormSnapshot`,
`ExtractionSnapshot` and `bCrewWiped`, which are the same shape of thing —
crew-wide facts the server decides.

---

## Staleness has two clocks, and the second one is the good one

**Elapsed time** is the obvious one. "6 minutes ago." Fine, necessary, and
not very interesting.

**Storm stage** is the better one, because the storm is the thing that makes
old information wrong. Storm Charge only ever rises, so stage is a monotonic
clock the fiction already owns. A sighting from before the last surge is
stale *for a reason the player understands*, rather than because a number got
big.

So a row that predates the current stage is marked as such, in words:

> Sweeper — toy department — *Mara, 4m ago, before the surge*

"Before the surge" does more work than "4m ago" because it tells you **why**
to distrust it. The world changed between that observation and now, and the
player knows exactly what changed.

This also gives you free pacing: every stage change ages the entire board at
once. The crew's shared knowledge degrades in steps that coincide with the
moments the game gets harder, with no decay timer to tune.

---

## Rows never disappear

Deleting a stale row is the game making a judgement about what is still true,
which is the narrator rule again wearing a different hat. Age them, sort
them, push them down, grey them — never remove them.

A six-minute-old sighting is not noise. It is the reason the player decides
to go a different way, and taking it away is taking away a decision.

The practical consequence is a cap, since four players over a long run
produce an unbounded list. Cap **per subject**, newest first, and when rows
are dropped say so in a line rather than silently truncating:

> Sweeper — 3 shown, 7 older

A board that hides its own truncation is lying by omission, which is the one
failure mode this design exists to avoid.

---

## Contradictions get shown, not resolved

Two players report different things about the same place. Show both rows.

The game has no way to know which is right, and pretending otherwise is the
narrator rule a third time. Two contradictory sightings is *information* — it
means something moved, or somebody was wrong, and either is worth knowing.
Resolving it silently throws that away.

---

## Accessibility, because row 16 reaches here too

Age cannot be carried by colour alone. Greying out a stale row is a visual
convention that fails for a meaningful share of players, and row 16 exists
precisely to stop single-channel communication.

So: the **words** carry the age ("before the surge", "4m ago"), position
carries recency (newest first), and colour is decoration on top of both.
Remove the colour entirely and the board must still read correctly.

Same rule the map markers already got in `e0a91fb` — text and shape alongside
icon and colour.

---

## Disconnects don't erase history

If a player leaves, their rows stay. The observation happened. Attribution
becomes the name they had, not a live player reference.

This matters more than it sounds in a 1–4 drop-in game: a crew that loses
someone keeps what that person learned, which is both correct and kind.

---

## Tests worth planting before the feature exists

This project's habit is to plant the test that catches the quiet failure, so
here are the three that matter. Each should be proven red first.

1. **No row without an observer.** Every `FLLKObservation` carries a non-null
   `ObserverId`. This is the anti-narrator rule made mechanical: a future edit
   that has the game helpfully write a row fails here. Invert it by
   constructing a row with no observer and requiring the failure.

2. **No automatic claims.** Count the code paths that can create a `claimed`
   row and require that every one of them originates in player input. If a
   second path appears, the test fails. Same shape as the existing guard that
   counts the paths into `Pursue` and fails if a third appears.

3. **Truncation is visible.** When rows are dropped by the per-subject cap,
   the board reports the count that was dropped. Red-prove by capping at one
   and requiring the "N older" line.

The first one is the important one. It is a single assertion that protects
the whole design philosophy, and it will still be protecting it in a year
when nobody remembers why the rule existed.

---

## What this replaces

`HUD-STATUS-SCREEN.md` currently defers the named relic ledger until there
are server-owned relic records. That deferral can close: `bLegible` already
latches, server-decided and sticky on purpose, which means **the record
already exists.** A latched flag is a record of something that happened at a
moment, which is exactly an attested row.

The ledger is not a separate feature waiting on infrastructure. It is the
first `Subject` on this board, and the infrastructure is the latch.
