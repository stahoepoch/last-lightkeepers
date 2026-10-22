# Codex integration runbook

Scope: Codex only. This is deliberately not a gameplay or map-design backlog.
Those files have separate writers.

## Active

- [ ] Reconnect the existing Rojo plugin in Studio to the already-running local
  server at `127.0.0.1:34872`. **Blocked:** Studio logged a forced WebSocket
  disconnect; the desktop Studio window is not exposed to the automation
  surface, so its plugin button cannot be operated here.
- [ ] Confirm a source edit appears in Studio without hand-pasting.
- [ ] Run one clean playtest after sync: hub departure, raid arrival, a pickup,
  one enemy encounter, extraction, return to hub.
- [ ] Capture the console and record only reproducible integration failures in
  `NOTES-FOR-CLAUDE.md`; do not directly edit gameplay-owned files.

## Every change handoff

- [x] Build `default.project.json` with Rojo — 2026-10-21.
- [x] Run the Node checks — 216/216 passed on 2026-10-21.
- [ ] Verify Studio is connected before claiming a change is live.
- [ ] Keep Studio edits limited to integration settings and never hand-paste
  gameplay code over a source-controlled module.

## Explicitly out of scope

- Gameplay logic: `src/server/**`, `src/client/**`, `src/shared/**`.
- Map layout, props, tags, fixtures, and lighting placement.
- Balance values in `data/*.json`.
