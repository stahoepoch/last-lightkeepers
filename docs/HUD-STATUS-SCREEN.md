# Held status screen — the screen you consult, not the screen you live in

Status: **proposed implementation brief**, 2026-10-31. This follows Chris's
playtest reaction to the always-on debug panel: the ordinary screen should
belong to the place and its objects; a keeper deliberately checks their status
while the world continues around them.

## Interaction rule

**Hold Tab to inspect; release Tab to return. It never pauses the run.**

This is a private 1–4 player game. A toggle would leave an interface over the
world while a player tries to flee; a pause would desynchronise a co-op night.
The screen therefore has to be readable in a few seconds and must close on the
same gesture that opened it. Its job is a decision check, not menu management.

World-space prompts remain outside it: relic brackets, `[E] Take`, revive
progress, the downed panel and extraction prompts are all immediate action
language. The existing `LLK.ShowStormHUD` panel remains a developer-only
diagnostic, off by default.

## First-pass layout

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
 │ IDENTIFIED RELICS — a short, named ledger; no grid inventory      │
 └───────────────────────────────────────────────────────────────────┘
```

The visual implementation is a future authored UI asset. This document fixes
information hierarchy, not fonts, colours or final layout.

## What each region may say

| Region | Runtime truth already available | Never infer or show |
| --- | --- | --- |
| Night | `ALLKGameState::GetStormSnapshot()` and locally loaded static pressure rules | An exact enemy position, hidden spawn, or a fabricated safety score |
| Lantern | `ULLKLanternComponent::IsLanternOn()`; later, the replicated named lantern state | A fake battery meter while ordinary light remains unlimited |
| Haul | `ULLKInteractorComponent::GetCarriedCount()` and its burden rule | Slots, stacks or item quantities that do not exist |
| Crew | Replicated player/vitals state; downed and revive state are authoritative | A crew member's private focus target or an enemy's target choice |
| Extraction | `ALLKGameState::GetExtractionSnapshot()` | A countdown projected through a hold stall; only the server knows whether it is stalled |
| Identified relics | Future per-keeper record of the server-latched relic identity | A list reconstructed from nearby objects or from someone else's discoveries |

The last row is intentionally a dependency. Today the game carries only a
replicated `CarriedCount`, not a per-keeper list of named carried relics. The
screen must show the honest count first. A named "identified relics" ledger
requires a server-owned, replicated record with add/remove/wipe semantics;
it must not be faked from client focus or from a DataTable lookup.

## Input and accessibility contract

The screen needs a real **eighth Input Action**, `IA_LLK_Status`, Digital
(bool), mapped to Tab and the appropriate gamepad menu/view button. It cannot
be a one-off raw Tab binding: all input stays in `IMC_LLK`, player-mappable and
validated together. Adding it means updating the input validator, soft asset
contract and authored mapping context in one change.

The same screen supplies a readable counterpart to radio and voice chat:
crew downed state, active extraction state and the night’s direction are visual
facts. It must not require hearing a radio cue or a threat sound to understand
what the crew is currently doing.

## Acceptance questions

1. Can a keeper open, understand and release the screen while moving through
   a dangerous aisle without losing control?
2. Does it answer the only immediate choices: *how bad is the night, what am I
   carrying, who needs help, and is extraction live?*
3. Does it remain truthful when data is unknown or pending replication?
4. Does it expose no hidden enemy, map or teammate information?
5. With all UI hidden, do the world prompts still teach taking, reviving and
   extracting on their own?

## Build order

1. Add the complete authored input action and mapping through the input
   contract.
2. Add a C++ held-state seam and tests; no gameplay state changes because a
   menu is open.
3. Add the minimal count-based screen against existing replicated snapshots.
4. Add the named relic ledger only once its server-owned data model exists.
5. Art-pass the widget after the first lantern-in-hand and ambient-light pass,
   so its contrast is judged against the real scene rather than a debug floor.
