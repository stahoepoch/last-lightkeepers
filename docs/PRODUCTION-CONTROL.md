# Production control — current chapter

Status: **active operating plan, 2026-11-14.** The raid-era schedule is at
`docs/archive/PRODUCTION-CONTROL-RAID-2026-10.md`. `GAME-THESIS.md` states the
current product; `ROADMAP.md` defines playable acceptance. This file assigns
the next work and the evidence required to call it done.

## Authority and ownership

Chris decides product direction and authored values. The 2026-11-14 direction
is **solo horror RPG / stealth / environmental puzzle**, not a two-player
extraction game or a Roblox release. `DECISIONS.md` holds binding calls and
their history. Older locked rows that conflict with this explicit direction
need a recorded supersession; they cannot keep assigning work in the meantime.

`LANES.md` still governs file ownership and binary map handoffs. A single
writer owns a `.umap` or `.uasset` at a time. Claude owns runtime rules and
their tests, Codex owns build/editor tooling and integration, Grok owns its
assigned narrative files, and Chris owns creative and visual approval. An
explicit user assignment can change a lane for a task. Existing replication
code is technical debt to evaluate, not a co-op acceptance gate.

## Current integration order

| Order | Package | Done when |
| --- | --- | --- |
| 1 | **Boswell's traversal and scale** | The two-floor store has connected playable routes, usable cover, exits from tight spaces, a working vertical connection, and no fall-off path. Character and Sweeper footprints are measured in it. |
| 2 | **Lighting and lantern in that space** | Emergency fixtures show navigation; the carried lantern reveals one needed detail and operates one authored reader. Its notice cost is perceptible through the machine. Art tuning does not change gameplay radius. |
| 3 | **Threat and sound** | A machine has a legible job, directional sound and a visible commitment; the player can hide, detour or flee. Contact never removes input for a long recovery or pins the player in geometry. |
| 4 | **Chapter objective** | Clues and environmental changes carry the player across the whole overgrown, scavenged, dangerous Galleria to Sayers; the result persists and the path has an end. |
| 5 | **Fresh-player proof** | A reproducible build is played without coaching. Record what the player actually did, what blocked them and what they wanted next. Revise the chapter before enlarging the map. |

Art, animation, texture and audio work belong inside packages 1–4. They are
part of testing this game's physical fantasy and cannot all be postponed to
the final package.

## Work-package rule

Each active card names its owner, asset or file boundary, dependency, player
behavior to prove and evidence to retain. Keep one active card per owner and
integrate often. A source change needs a compile and relevant test; an Unreal
content change needs an editor or packaged play observation. A passing script
that examines old extraction data cannot certify the new chapter.

## Minimal operating loop

This is deliberately the smallest process layer. Do not add a second backlog,
agent hierarchy, template catalogue, or status report on top of `HANDOFFS.md`.
The project already has enough historical documentation to create a convincing
but wrong plan.

- **One live checkpoint:** `CURRENT-STATE.md` names the current chapter proof,
  active boundaries, last *observed* play result, and the next risk. Replace it
  at a work-package boundary; do not append a diary. Git history is the diary.
- **One ledger:** `HANDOFFS.md` owns assigned/open work and closure evidence.
  An item is not duplicated in the checkpoint.
- **One playable claim:** work affecting player experience is only *verified*
  after a person moves through it in the editor or a reproducible build. A
  compile, an automation check, screenshot of an empty map, or an MCP response
  is evidence about a seam, not the chapter.
- **One asset writer:** `LANES.md` governs `.umap` and `.uasset` claims. A
  work package can have several collaborators, but never two concurrent binary
  writers.

The guard against stale status is part of the loop: the checkpoint distinguishes
**observed**, **automated**, and **unverified** statements and carries its date
at the top. If it stops reflecting `HEAD`, update or delete the claim rather
than planning around it.

At least once each work week, run the fast tooling checks, compile the target,
play the newest chapter path, and record three things: what changed, where
play stopped or felt wrong, and the next risk to retire. For a remote session,
code and content preparation can advance, but unobserved visual claims stay
unverified until Chris plays them.

## Open product work, not implementation guesses

The exact keeper age and eye height, camp-loss cost, chapter return rules,
ending mechanism and full RPG progression remain at their recorded decision
status. Do not turn a proposal into runtime behavior merely because this
production plan needs a next card. The first crossing can prove traversal,
light, threat and story direction while those larger calls are authored.
