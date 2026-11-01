# Fieldboard and route projection — information without a backpack ritual

Status: **proposed implementation brief**, 2026-10-31. This follows Chris's
playtest reaction to the always-on debug panel: the ordinary screen should
belong to the place and its objects; a keeper deliberately checks their status
while the world continues around them.

## Interaction rule

**Tab toggles the Fieldboard. Tab again or Escape closes it. It never pauses
the run.**

This is a private 1–4 player game. A pause would desynchronise a co-op night,
but a toggle is not inherently a problem: a player needs time to read their
crew, haul and objective state. Closing on the same key or Escape makes that
decision reversible without asking them to hold a key while reading. Movement
and danger continue while it is open; the Fieldboard must therefore be useful
at a glance and never become a backpack-animation tax.

World-space prompts remain outside it: relic brackets, `[E] Take`, revive
progress, the downed panel and extraction prompts are all immediate action
language. The existing `LLK.ShowStormHUD` panel remains a developer-only
diagnostic, off by default.

## The two surfaces

### Fieldboard — Tab

The Fieldboard is a clean, toggleable status surface. It is an abstraction for
clarity, not a literal backpack the keeper removes in the middle of an aisle.
The fiction is the order's field record: a compact ledger of what the keeper,
their lantern and their crew presently know.

### Route projection — M

**Hold M to inspect a map; release it to return to the world.** The lantern's
interpreter hardware projects a dim amber route image just ahead of the keeper
or inside the raised lantern's hood. It is a cosmetic, screen-space display:
it is not a second `LLKLightField` source, never makes relics legible, never
changes enemy detection, and does not illuminate the room.

The map is not GPS and cannot reveal a building the crew has not surveyed. It
shows the current floor's discovered outline, known exits, route markers and
objectives the crew has actually received. It may show teammate positions only
when a later explicit radio/ping system communicates them; it never becomes a
free radar.

Holding M is intentionally a smaller commitment than Tab: it lets a keeper
check their bearings, but their view is occupied by the projection and the
world keeps moving around them. The tension is informational, not a hidden
movement penalty.

## Fieldboard first-pass layout

```
 ┌───────────────────────────────────────────────────────────────────┐
 │ NIGHT   patrols alert                         next shift  03:12    │
 │ pressure rising  •  storm outside worsening                     │
 ├──────────────────────┬────────────────────────────────────────────┤
 │ LANTERN              │ CREW                                       │
 │ raised / working     │ [keeper] standing       [keeper] down      │
 │ warm, active         │ [keeper] standing       [keeper] standing  │
 │                      │                                            │
 ├──────────────────────┼────────────────────────────────────────────┤
 │ HAUL                 │ EXTRACTION (only when active)              │
 │ relics carried: 2    │ recovery holding — return to the dock       │
 │ burden: 8% slower    │ hold / stall state                          │
 ├──────────────────────┴────────────────────────────────────────────┤
 │ OBJECTIVE — one current job and the next visible action            │
 │ IDENTIFIED RELICS — a short, named ledger; no grid inventory      │
 └───────────────────────────────────────────────────────────────────┘
```

There is no all-purpose quest list. The objective region has one active job,
its immediate next action and, when relevant, a named place the crew already
knows. Examples: *restore the sub-basement breaker*, *return to Loading Dock*,
or *find the fitting-room service door*. An objective must never describe an
unseen answer, enemy placement or a route the player has not earned.

The visual implementation is a future authored UI asset. This document fixes
information hierarchy and interaction, not fonts, colours or final layout.

## What each region may say

| Region | Runtime truth already available | Never infer or show |
| --- | --- | --- |
| Night | `ALLKGameState::GetStormSnapshot()` and locally loaded static pressure rules | An exact enemy position, hidden spawn, or a fabricated safety score |
| Lantern | `ULLKLanternComponent::IsLanternOn()`; later, the replicated named lantern state | A fake battery meter while ordinary light remains unlimited |
| Haul | `ULLKInteractorComponent::GetCarriedCount()` and its burden rule | Slots, stacks or item quantities that do not exist |
| Crew | Replicated player/vitals state; downed and revive state are authoritative | A crew member's private focus target or an enemy's target choice |
| Extraction | `ALLKGameState::GetExtractionSnapshot()` | A countdown projected through a hold stall; only the server knows whether it is stalled |
| Identified relics | Future per-keeper record of the server-latched relic identity | A list reconstructed from nearby objects or from someone else's discoveries |
| Objective | Future server-owned objective state and its known location | A route, objective or explanation the crew has not discovered |
| Route projection | Future discovered-map record plus explicitly communicated markers | GPS, a minimap, hidden rooms, or enemy radar |

The last row is intentionally a dependency. Today the game carries only a
replicated `CarriedCount`, not a per-keeper list of named carried relics. The
screen must show the honest count first. A named "identified relics" ledger
requires a server-owned, replicated record with add/remove/wipe semantics;
it must not be faked from client focus or from a DataTable lookup.

## Input and accessibility contract

The two surfaces need real input actions: **`IA_LLK_Status`** (Tab and the
appropriate gamepad menu/view button) and **`IA_LLK_Map`** (M and a gamepad
equivalent), both Digital (bool). They cannot be raw key bindings: all input
stays in `IMC_LLK`, player-mappable and validated together. Adding them means
updating the input validator, soft asset contract and authored mapping context
in one change.

The same screen supplies a readable counterpart to radio and voice chat:
crew downed state, active extraction state and the night’s direction are visual
facts. It must not require hearing a radio cue or a threat sound to understand
what the crew is currently doing.

## Acceptance questions

1. Can a keeper open, understand and close the Fieldboard while moving through
   a dangerous aisle without losing control?
2. Does it answer the only immediate choices: *how bad is the night, what am I
   carrying, who needs help, and is extraction live?*
3. Does it remain truthful when data is unknown or pending replication?
4. Does it expose no hidden enemy, map or teammate information?
5. Does the route projection communicate only discovered space and deliberately
   shared information, while remaining a cosmetic lantern display?
6. With all UI hidden, do the world prompts still teach taking, reviving and
   extracting on their own?

## Build order

1. Add the complete authored input actions and mappings through the input
   contract.
2. Add a C++ toggle-state seam for the Fieldboard and a held-state seam for
   the route projection; no gameplay state changes because either surface is
   open.
3. Add the minimal count-based screen against existing replicated snapshots.
4. Add the named relic ledger only once its server-owned data model exists.
5. Art-pass the widget after the first lantern-in-hand and ambient-light pass,
   so its contrast is judged against the real scene rather than a debug floor.
