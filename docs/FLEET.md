# Fleet protocol — how four parties work one repo

Status: **proposed**, 2026-11-01. The ownership table needs Chris's
ratification before it is binding; everything else is description of what
already happens, written down so it stops being re-invented.

`COORDINATION.md` was the Roblox-era agreement and its lane table still
describes Luau services. This file replaces that table for the Unreal build.
`COORDINATION.md` is kept for its tag contracts, which are still a precise
statement of what pieces of a map must *do*.

## The parties

| Party | What it is | What it does best |
| --- | --- | --- |
| **Chris** | The human. Sole creative and balance authority | Decides. Plays. Relays between the agents, because they cannot see each other |
| **Codex** | Drives code and the editor directly | Lands C++ and Blueprint work, builds maps, runs the build |
| **Claude (MCP)** | Drives Unreal through the MCP plugin | Instrumented work in the editor: the light lab, measurement, capture |
| **Claude (this)** | Chat session, no editor | Critique, adjudication, audit, design argument, docs |

**The agents cannot see each other.** Every cross-agent fact arrives as
something Chris pasted. That is the single constraint this whole protocol is
built around: an agent's claim about another agent's work is hearsay until it
is in the repo.

## The one rule that matters

> **Uncommitted work does not exist.**

Not "is at risk" — does not exist. The other agents read the repo, not each
other's chat. A design file, a map, or a finding that is not committed cannot
be built on, cannot be cited, and will be re-derived from scratch in a
conversation next week.

This is not theoretical. As of 2026-11-01 the repo had eleven untracked design
documents and an untracked map. See `DRIFT.md`.

**At the end of any session that produced a durable artifact: commit it.**
Pushing is Chris's call. Committing is not.

## Authority

Four tiers, from `README.md`:

1. `DECISIONS.md` — numbered binding rows. Only Chris promotes into it.
2. `WORLD-SYSTEMS-BOARD.md` — the living board. New ideas enter here.
3. `FLEET.md`, `PRODUCTION-CONTROL.md`, `LANES.md` — process and ownership.
4. Everything else — descriptive, binds nothing.

**Cite rows by number.** "Row 5" is checkable; "the light rule" is not, and a
paraphrase is how a locked decision gets quietly softened.

## Lanes, Unreal build

Proposed. The pattern is: **one active writer per artifact**, which is
`DECISIONS.md` row 41 generalized from `Content/` to everything.

| Artifact | Active writer | Others may |
| --- | --- | --- |
| `Source/` C++ | Codex | read, propose a handoff |
| `Content/` authored assets | whoever Chris named for that asset (row 41) | read |
| Maps | Codex, or the MCP session when it is a measurement rig | read |
| `data/*.json` balance values | **Chris only** | propose with reasoning |
| `tests/` | the author of the system under test | add a test that fails |
| `docs/DECISIONS.md` | Chris | propose a row |
| `docs/WORLD-SYSTEMS-BOARD.md` | append-only, anyone | never rewrite another's entry |
| Everything else in `docs/` | anyone, append-first | correct with a dated banner, not a deletion |

**Balance values are Chris's and always have been.** `data/lantern.json`
carries four "Chris's call" notes, one of which removed the lantern's fuel
gauge entirely. An agent that invents a number in a service file overrules a
decision it never saw.

## Two sessions in one checkout — git stops being an instrument

Added 2026-11-02, from a collision that produced a false document.

The MCP session ran `git status` mid-session and saw
` M Source/LLK/Private/UI/LLKDebugHUD.cpp`. A `git diff` moments later came
back empty. It recorded that as "a stat race that resolved to clean" and
carried on — and then shipped `cc04e5c`, a document asserting that nothing in
the project binds the four Sweeper cue delegates, which was **already false**
when it was written.

Nothing was wrong with either command. The modification was this session
editing those two files in **the same working tree at the same time**, and the
empty diff was the commit landing between the two calls.

### Why this is a protocol defect and not an attention lapse

The lane table above assigns one active writer per **artifact**. It never said
what happens when two sessions share a **working tree** — and in a shared
tree, the index and the working copy are shared mutable state, so:

> **`git status` and `git diff` describe a world another writer is changing
> underneath you.** Two of them taken a moment apart are two observations of
> different worlds, and the disagreement between them carries no information.

This is the stale-input class from `ADVERSARY.md` at a sub-second timescale,
and worse than the documentary version: a stale document at least holds still
while you read it.

It also fails quietly in both directions. A path that looks modified may have
been committed; a path that looks clean may be mid-save. Neither produces an
error.

### The rule

**The structural fix is one working tree per writing session**, which the app
can do, and it is Chris's call whether to spend it. Until then:

- **Announce before editing a shared path.** Say which files, before touching
  them, to whoever else is working. This session did not, and should have —
  its own lane table predicted exactly this.
- **Never conclude anything from two git calls taken at different instants.**
  If `status` and `diff` disagree, the answer is "another writer is here", not
  "the first reading was noise."
- **Check `git log` before recording a claim about what the tree contains.** A
  commit is a fact with a timestamp; a dirty-file list is a guess about now.
- **Read a collision as a collision.** Row 27 exists because this happens; the
  signal appeared in a terminal and was explained away rather than believed.

### Why it matters more than the one document it spoiled

Both agents are fast, both commit often, and neither can see the other except
through the repo. The repo is therefore the shared instrument — and an
instrument two parties write to concurrently needs the same discipline as the
light lab's: know what you are measuring, and do not average two readings of
different things.

## Handing work across a lane

A handoff is a committed file, `docs/HANDOFF-<party>-<subject>.md`, and it
names four things or it is not a handoff:

1. **The job**, small enough to be one commit.
2. **Done when** — an observable condition, not an intention.
3. **What is already ruled out**, so the receiver does not re-walk it.
4. **What it must not touch**, with the row number that says so.

A handoff is *released* only when Chris says it is. Written-but-unreleased is
a proposal, and the file says which it is at the top.

## Correcting a document instead of deleting it

The house pattern already exists in `BANKING.md`:

```text
Status: proposed, 2026-10-21.
**SETTLED AGAINST THIS DOCUMENT, 2026-10-26.** Chris locked the opposite rule
and the recommendation here is dead. [what is true now] See DECISIONS.md O3.
```

Banner at the top, the reason, the pointer to what replaced it, and the body
left alone. **Deleting a superseded doc destroys the record of why the dead
idea was attractive**, which is exactly what makes it come back next month as
new scope.

Stale docs go in the register: `DRIFT.md`.

## The log

`LOG.md` is append-only, one dated entry per session that changed something
durable. It exists because three agents and a human cannot reconstruct a week
from `git log` alone — the log says *why*, and which party was driving.

## What no agent does without Chris

Push. Publish. Touch an account, a payment, or a release. Promote a row into
`DECISIONS.md`. Change a balance value. Overwrite another writer's artifact.
Rewrite git history — LFS makes that irreversible in practice.

## Related

`ADVERSARY.md` — how the agents check each other, and what counts as a claim.
`DRIFT.md` — the stale-concept register.
`LOG.md` — the running log.
