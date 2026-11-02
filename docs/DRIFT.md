# Drift register — stale concepts, divergence, and what to do about each

Status: **audit, 2026-11-01**, by Claude (chat session). Every item was
verified against the repo as it stands, not remembered. Nothing here is fixed
by this file; it is the list, with the owner of each fix named.

Method and conventions live in `ADVERSARY.md`. Correction pattern — a dated
banner, not a deletion — lives in `FLEET.md`.

---

## A. Two LOCKED rows contradict each other

**This is the serious one.** `DECISIONS.md` opens by saying it exists because
an unfindable rule "gets deleted by someone who never knew it was one." It now
contains the same failure internally.

| | |
| --- | --- |
| **Row 6**, LOCKED, from the handoff | "Storm Charge only ever rises. **Darkness slows it**; nothing reverses it" |
| **Row 73**, LOCKED, Chris 2026-10-29 | "The clock means weather alone. **Nothing a player does moves it.**" |
| **Row 72**, LOCKED, Chris 2026-10-29 | "**Light never changes its rate**; light creates only local consequences" |

Row 73 is later, is explicitly closing out rows 70 and 72, and is the one the
build follows. Row 6 carries no banner — so an agent reading `DECISIONS.md`
top-down finds the dead rule first, in the sixth row, under the heading "The
core rules."

**Fix:** one line from Chris, struck-through in row 6 with a pointer to 73,
exactly as row 8 was handled on 2026-10-26. **Owner: Chris.** No agent may
edit a locked row.

### A2. Status fields that outlived their own consequences

Rows **68** and **70** are still marked `PROPOSED`, but row 73 (LOCKED) says
it closes them out, and row 72 (LOCKED) implements the one-clock rule row 70
proposed. Either they were ratified and the status is stale, or something in
them was not — and no agent can tell which from the file.

**Fix:** Chris sets each to `LOCKED` or names what is still open.
**Owner: Chris.**

---

## B. Untracked durable work

> **The map is safe, `cfae411`, 2026-11-01.** The MCP session committed
> `Content/LLK_Boswells_FirstRun.umap` as "Build Boswell's first extraction
> run" — a playable loop in `/Game/LLK_Boswells_FirstRun`, build verified on
> Unreal's own `Result: Succeeded`, PIE registering the Sweeper, the patrol
> and the LoadingDock zone. **The eleven root documents below are still
> untracked.**

`git status` on 2026-11-01, after this session committed the three design docs
it found yesterday:

```text
?? Content/LLK_Boswells_FirstRun.umap        [RESOLVED cfae411]
?? CAMPAIGN-SHAPE.md          ?? MAKE-IT-FEEL-LIKE-A-LANTERN.md
?? FIELDBOARD-OBSERVATIONS.md ?? PERMISSIVE-HOSTILE.md
?? FIXTURE-LIGHT-CONTRACT.md  ?? THE-MALL-IS-STILL-TRADING.md
?? FOUR-ANCHORS.md            ?? THE-STORE-STILL-RUNS.md
?? KNOWLEDGE-SPACE-TIME.md    ?? UI-NOVELTY.md
?? LIGHTLAB-SPEC.md
```

Eleven design documents and **a map**. None of it is ignored by
`.gitignore` — it was simply never added.

~~`Content/LLK_Boswells_FirstRun.umap` is the urgent one. It is binary, it is
inside the LFS path, and under row 41 it has an owner who is the only party
who may write it. An uncommitted `.umap` is one editor crash from gone, and
nobody else can see it exists.~~ **Landed in `cfae411` by its own writer,
which is how row 41 is supposed to work: the register names the risk, the lane
owner clears it.**

**Fix, outstanding:** commit the eleven root documents, or move them into
`docs/` first — see C2, which is the better order, since two of them may be
shadows of tracked files and committing a divergent duplicate makes the
divergence permanent. **Owner: Chris's call on move-then-commit.**

---

## C. One contract exists twice, with different content

```text
FIXTURE-LIGHT-CONTRACT.md       ead58546…   (untracked, root)
docs/FIXTURE-LIGHT-CONTRACT.md  5fc21b80…   (tracked)
```

Different hashes. Two copies of a document whose entire purpose is to be the
single statement of what a fixture light must do. Something has been tuned
against one of them and it is not knowable which.

**Fix:** diff them, keep the tracked one as the contract, fold in anything the
root copy added, delete the root copy. **Owner: whoever owns the fixture light
contract** — the root copy's content decides that.

### C2. Candidate shadow documents

Nine of the eleven untracked root files have titles that look like they shadow
a tracked `docs/` file. **This is a hypothesis from titles, not a diff** — each
needs one read before anything is merged or deleted:

| Untracked root file | Possible tracked counterpart |
| --- | --- |
| `KNOWLEDGE-SPACE-TIME.md` | `docs/KNOWLEDGE-OF-PLACE.md` |
| `THE-STORE-STILL-RUNS.md`, `THE-MALL-IS-STILL-TRADING.md` | `docs/THE-STORE.md` |
| `CAMPAIGN-SHAPE.md` | `docs/SPINE.md` |
| `LIGHTLAB-SPEC.md` | `docs/LIGHTLAB-PROTOCOL.md` |
| `FIELDBOARD-OBSERVATIONS.md`, `UI-NOVELTY.md` | `docs/HUD-STATUS-SCREEN.md` |
| `MAKE-IT-FEEL-LIKE-A-LANTERN.md` | `docs/THE-LANTERN.md`, `docs/LANTERN-LOOKDEV.md` |
| `FOUR-ANCHORS.md`, `PERMISSIVE-HOSTILE.md` | no obvious counterpart |

The structural problem underneath: **the repo root is an unmanaged second
docs directory.** Eight tracked `.md` files and eleven untracked ones sit
beside `docs/`, which has its own index. Ideas written at root are invisible
to anyone following `docs/README.md`.

**Fix:** root keeps only what belongs there — `README.md`,
`DONTFUCKUPTHELIGHTS.md`, `COORDINATION.md`, and the three `NOTES-FOR-*.md`
relay files. Everything else moves into `docs/` and gets an index line.
**Owner: Chris's call on whether to move them; mechanical once decided.**

---

## D. Superseded platform, still written in the present tense

The Roblox lanes closed on 2026-10-27 (rows 1 and 33). `COORDINATION.md` has a
STOP banner and root `README.md` correctly says "archived." These do not:

**`MORNING.md`** — the worst offender, because it is called *Morning, Chris*
and reads first. Written 2026-10-20, present tense, and it asserts:

- "**Lantern charge drains** — 150 seconds of light per full charge, then it
  cuts out." Row 10, LOCKED the same day, says charge is unlimited: *"i don't
  think the lantern should ever run out."*
- "lanterns **accelerate** [the storm] 2.5× each, capped at 5×." Rows 72 and
  73, LOCKED, say light never moves the clock.
- Rojo setup, Studio Play, tagged-instance counts in the Output window.

Two reversed locked rules and a dead toolchain, on the page named for the
first thing he reads.

**`NOTES-FOR-CLAUDE.md`** — marked `IN-FLIGHT — 2026-10-19`. It is a Luau
systems audit of `src/`, which row 33 froze. Its P0/P1/P2 items are void.

**`COORDINATION.md` lane table** — still assigns "Storm Charge, lantern state,
loot/banking" as Luau service ownership. `FLEET.md` replaces the table; the
banner should say so.

**Fix:** a dated banner at the top of each, in the `BANKING.md` pattern. Keep
the bodies — `MORNING.md` is a useful record of what the prototype did.
**Owner: this session; it is doc work in no agent's build lane.**

---

## E. Pending rename — not drift, do not mass-edit

"Storm Charge" appears in roughly thirty files. It is **not** stale in the
sense above: row 73 renamed the types to **Zone Pressure**, and
`docs/HANDOFF-CODEX-ZONE-CLOCK.md` — released by Chris 2026-10-30 — owns the
rename. Two fields must survive it (`SirenSeconds`,
`bConvergeAllPatrols`), and `StormChargeSpike` is the one to delete, guarded
by `tests/extraction.test.mjs`.

**A helpful find-and-replace across the docs would break a released handoff.**
Leave the vocabulary alone until that handoff lands, then sweep in one commit.

### E2. "Studs" is also not drift

"Studs" appears in 23 files and `data/*.json` is still authored in studs on
purpose — one documented constant, `StudsToCm = 28` in
`Source/LLK/Public/LLKUnits.h`, converts at the boundary. 98 balance
assertions read those values. Re-authoring them in centimetres invalidates
every one.

Named here so that a future agent tidying units does not helpfully destroy the
portable data layer. The real cost is logged already: every new range has to
remember the conversion.

---

## How to use this file

Work top down — section A is a guardrail contradiction and everything else is
housekeeping. When an item is fixed, strike it through with the commit that
did it rather than deleting the row, so the register also records what has
already been swept.

Re-run the audit from `ADVERSARY.md`'s standing checks.
