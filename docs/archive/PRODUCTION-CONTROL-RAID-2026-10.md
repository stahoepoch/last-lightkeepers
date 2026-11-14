# Production control — how Last Lightkeepers is actually made

Status: **active operating plan**, 2026-10-30. This is the small-team version
of a studio production cadence: evidence gates, a single accountable owner,
small work packages, and a playable integration build at a fixed rhythm. It
does not replace the creative authority of `docs/DECISIONS.md`.

## The authority stack

When documents disagree, use this order:

1. `docs/DECISIONS.md` — what the game is allowed to be. Only Chris changes a
   LOCKED decision.
2. `docs/ROADMAP.md` — milestone gates and acceptance criteria.
3. This document — current sequence, dependencies, and operating cadence.
4. `docs/LANES.md` — who may change each path.
5. Feature documents — specifications, not schedules.

`docs/STEAM-PRODUCTION-PLAN.md` remains useful research and a capacity model.
It is not a second calendar. Its former sixteen-week outline is a forecast to
revise after the first measured vertical-slice velocity, never a promise that
can overrule a gate.

## Accountability

Chris is the product owner and final approver: creative calls, values,
`Content/`, external testers, and anything published. Claude owns runtime
gameplay rules, tests, instrumentation, design documents, and the decision
record. Codex owns editor/build/tooling/configuration/CI and validates the
project can be built, packaged, and diagnosed.

The exception that needed making explicit is **M1.6, Enhanced Input**:

| Piece | Responsible | Why |
| --- | --- | --- |
| Input-action contract, Config, validator, and CI check | Codex | Text/build lane |
| `Content/Input` assets and their mappings | Chris | They are binary `.uasset` content |
| Runtime consumption and fallback behavior | Claude | Runtime lane |
| Final two-player proof | Chris + Codex | It is a real-machine integration check |

No one silently crosses a lane to close a blocked task. They write a handoff
with the missing input, file/path, expected behavior, and acceptance proof.

## Work-package rule

Every task entering active work must fit on one card and name all six things:

- one accountable owner;
- a dependency and an explicit "not blocked" check;
- a playable or observable acceptance criterion;
- the test modes it needs: solo, listen host + client, dedicated-server
  simulation where relevant, and packaged build where relevant;
- the evidence to retain (test log, capture, profiler trace, or playtest
  observation); and
- the files/assets it is permitted to change.

One active work package per owner, with at most three active packages across
the project: Chris's content package, Claude's runtime package, and Codex's
tooling/integration package. A fourth starts only when one finishes, is
blocked by a named external dependency, or is explicitly traded for another.
This is the WIP limit that protects a one-person project from looking busy
while nothing becomes playable.

## Current critical path

The repository contains parts of M1 and M2, but a milestone is **not green**
because code exists. It is green only when its ROADMAP acceptance evidence is
recorded. The immediate schedule is therefore:

| Order | Work package | Owner(s) | Unblocks | Exit evidence |
| --- | --- | --- | --- | --- |
| 0 | M0.8 visual proof: open the UE project, play a cube/keeper, record the result | Chris | M1 closure | Short capture or dated note that the editor opened and Play worked |
| 1 | M1.6 input contract and authored Input assets | Codex + Chris + Claude | M1.1 and all real play | Validator passes; movement, lantern, interact and controller bindings work in editor |
| 2 | M1 integration proof: two local machines/clients, host and client agree on movement, lantern, and pressure HUD | Chris + Codex + Claude | M2 behavior | Captured LAN result; this is LAN first, not blocked on a Steam App ID |
| 3 | M1.5 ground-floor teaching greybox | Chris | M2.5 and M2.6 | The six questions in `THE-STORE.md` pass in a fresh-player walkthrough |
| 4 | M2.5 navigation, occlusion, routes, and spawn points | Chris + Codex | First physical enemy | `LLK.CheckMap` green and one route works in play |
| 5 | M2.1–M2.4 + one complete M2.6 enemy, including its visual and audio tell | Claude + Chris | M3 loop | A player can identify the escalation, make a costly escape choice, be downed and revived |
| 6 | M3 complete raid: loot, burden, extraction, win/loss and run summary | Claude + Chris + Codex | M4 | A 10–12 minute raid can end correctly in solo and two-player tests |

M2 rule/data preparation is permitted while the ground floor is being built;
enemy behavior, navigation tuning, traversal, and art production are not
credited as progress toward M2 until rows 2–4 are complete. That preserves the
useful parallelism already identified in `ROADMAP.md` without letting it hide
the critical path.

## Integration rhythm

This project needs a **weekly playable build**, even when it is ugly. At the
end of each work week:

1. Run the fast data/tooling gate on the shared revision.
2. Build and run the Unreal automation suite locally.
3. Open the editor and run the newest playable path for ten minutes.
4. Test any changed replicated feature as host plus client.
5. Record only three outcomes: what changed, what failed, and the single next
   risk to retire.

No feature is called done from a green compile alone. A changed gameplay rule
needs automated coverage and a play observation; a changed `Content/` asset
needs Chris's visual inspection; a changed build/tool needs a clean-clone or
packaged verification when applicable. Work stays on its branch until that
evidence is attached to its handoff.

## M4 is a decision gate, not a demo day

The existing gate is right but four testers are discovery evidence, not enough
to greenlight production. Use two rounds:

| Round | Participants | Purpose | What advances it |
| --- | --- | --- | --- |
| M4a — moderated discovery | 5 people, including players unfamiliar with the project | Find comprehension and pacing failures | Recorded runs, telemetry, and a ranked issue list. Fix only the repeated or loop-breaking findings. |
| M4b — confirmation | 5 new people after the targeted fixes | Check that fixes worked rather than merely familiarizing the first group | The go/no-go review below. |

The M4 review asks four outcome questions, first solo and then co-op:

1. Do players understand that the lantern reveals opportunity **and** creates
   risk, without a tutorial wall?
2. Do they make a meaningful light/off, advance/leave, or rescue/escape choice
   because of it?
3. Does each threat telegraph through two distinguishable channels, including
   a playable sound-off check?
4. After a completed run, do they voluntarily choose or ask for another room
   or run?

No crash, repeatable desync, lost progression, or accessibility-blocking
failure can be waived by a positive survey. If M4 fails, return to M1–M3 and
change the trade; do not start M5 progression, M6 fidelity, or a second space.

## Change control and forecast

- New feature ideas enter a **later** list. They do not enter the active
  milestone unless they directly retire a documented milestone risk.
- Any change to a LOCKED decision needs Chris's explicit decision-log entry,
  a named player behavior it improves, and the rule it weakens.
- Time is forecast in milestone bands only until M4. After the first two
  weekly playable builds, use observed throughput to forecast M1–M4; never
  use the old sixteen-week outline as a commitment.
- Concept art and visual exploration may continue as low-cost preproduction.
  Final capsule art, a public store page, trailer production, and a marketing
  schedule wait until M4 says the loop earns them.
- A missed forecast is information, not a reason to lower an acceptance bar.

This is deliberately how a disciplined small studio works: prove the loop,
then buy fidelity; keep content safely owned; integrate constantly; and let
evidence, not a date, authorize expansion.
