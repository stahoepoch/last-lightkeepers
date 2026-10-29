# Lanes — who owns what on the Unreal build

Written 2026-10-27, on Chris's instruction: "divide the work between you and
codex. Keep specific lanes. And responsibility. Everything accounted for."

This **replaces `COORDINATION.md`** for all Unreal work. That file still governs
the frozen Roblox archive and nothing else (`docs/DECISIONS.md` rows 1, 33).

Three parties. One owner per path, no exceptions, and the reason is harsher in
Unreal than it was in Roblox — see the next section before reading the table.

---

## The rule everything else is built on

**Unreal's content is binary, and binary does not merge.**

`.uasset`, `.umap`, `.uexp` are opaque to `git diff` and to `git merge`. If two
parties change one map, there is no conflict to resolve — there is a winner and
a silently destroyed afternoon. Git cannot help, review cannot catch it, and
the loss is invisible until someone opens the level and their work is gone.

Three consequences, and they shape every lane below:

1. **Chris owns `Content/` exclusively.** Every asset, every level, every
   material, every Blueprint. Not because agents shouldn't be trusted with it,
   but because we cannot read it, cannot diff it, and cannot merge it. An agent
   touching a `.uasset` is a coin flip on somebody's work.
2. **Gameplay logic lives in C++, not Blueprint.** This is the decision that
   determines whether agents can contribute at all. A Blueprint is a binary
   asset; a rule written in one is a rule no agent can read, review, test or
   fix. So anything that is a *rule* — the storm charge, the lantern cost, loot
   burden, extraction, the ladder — is C++ in `Source/`, in text, in version
   control, diffable and testable. Blueprints are allowed as thin glue: an
   actor that inherits a C++ class and sets which mesh and which sound. Logic
   in a Blueprint is a bug report.
3. **Both agents stay in text, permanently.** Which makes agent-vs-agent
   conflict in `Content/` structurally impossible rather than merely
   discouraged. The remaining risk is two agents in one C++ module, and the
   table below splits modules so that cannot happen either.

---

## The table

| Path | Owner | Notes |
| --- | --- | --- |
| `Content/**` — all of it | **Chris** | Levels, meshes, materials, lighting setups, Blueprints, animation, audio, UI assets. Agents never open these |
| `Source/LLK/**` — runtime module | **Claude** | Gameplay systems in C++: storm charge, lantern, loot, burden, extraction, enemies, the ladder, save data |
| `Source/LLKEditor/**` — editor module | **Codex** | Importers, commandlets, editor tooling, graybox generation, validation |
| `Source/*.Target.cs`, `Source/**/*.Build.cs` | **Codex** | Build configuration, module dependencies, plugin enables |
| `Config/*.ini` | **Codex** | Engine, input, packaging, Steam config. Gameplay numbers never live here — they live in `data/` |
| `Tools/**` | **Codex** | Python editor scripts, the test runner, local build scripts |
| `.github/workflows/**` | **Codex** | CI |
| `data/*.json` | **Claude** schema, **Chris** values | Unchanged from the Roblox project and deliberately so. See "The data pipeline" |
| `tests/**` | **Claude** | Data and schema tests that need no engine |
| `Source/LLK/Private/Tests/**` | **Claude** | Unreal automation tests for gameplay rules. `Private/Tests`, not `Tests` - the table named a directory that has never existed, corrected 2026-10-29 |
| `docs/**` | **Claude** | Design docs, the decision log, this file. Chris ratifies; Codex reads |
| `*.uproject` | **Codex** | It is text, it is build configuration, and it must have exactly one owner |
| `.gitignore`, `.gitattributes`, LFS config | **Codex** | One owner, because getting this wrong commits 40GB of cache |
| `src/**`, `default.project.json`, `COORDINATION.md` | **nobody** | Frozen Roblox archive. Row 33. Read for reference, never edit |

**If you need a change outside your lane, write it down for the owner rather
than making it.** That rule survived the platform change intact because it is
the only reason three parties have been able to work in one repo at all.

**Breached 2026-10-29, by Claude, in `Tools/**`.** Reviewing Codex's
`collect_playtest_diagnostics.ps1` turned up four real gaps, and I fixed them
in his file across four commits instead of handing them over. Chris restated
the lane the same day - *"Tools/** is now Codex-only; Claude should hand off
tool requirements, not edit the files"* - which is this table's existing row,
not a new rule.

It cost nothing, and that is the point worth recording rather than the apology.
Codex was reading the file from a commit behind while I was writing it, and
reported a defect I had already fixed as still outstanding. Nothing was lost
because git serialised us and because his note was a note rather than an edit.
Two writers in one file is not a conflict that gets resolved; it is a winner
and a silently destroyed afternoon, which is the whole argument of row 41
applied to text instead of binary.

**The thing that was genuinely missing is narrower than the rule**: there was
no stated channel for *review findings* on somebody else's lane. Running
another owner's script and reading its output is how all four gaps were found,
and nothing about the lanes forbids that or should. See the handoff list below.

---

## Responsibility — the things that get dropped

A lane table says who edits which file. It does not say who is *responsible*
when something is nobody's favourite job. This is that list, and it is
deliberately exhaustive, because every one of these has sunk a small project.

| Area | Responsible | The failure it prevents |
| --- | --- | --- |
| **Networking topology** | Claude | Co-op retrofitted onto single-player code. Decided now, not later: see "Multiplayer" below |
| **Save format and migration** | Claude | A playtest that invalidates everyone's save and cannot be rolled forward |
| **Performance budget** | Codex | Finding out at M4 that the store runs at 22fps on a tester's machine |
| **Input and rebinding** | Codex | Enhanced Input added late, after every action is hardcoded |
| **Audio bus structure** | Chris | Mixing a horror game with no busses, where the lantern click is louder than the Warden |
| **Accessibility** | Claude writes it, Chris signs it off | `DECISIONS.md` row 16 is already LOCKED: every enemy telegraphs with sound before sight, distinguishably. That is a build requirement, not polish, and it needs a visual channel too |
| **Asset provenance** | Chris | Row 24: everything we ship, we own. On Steam this is a legal exposure, not a principle. Every asset needs a recorded source |
| **Trademark and real-world names** | Claude flags, Chris decides | Rows 18 and 19. No real town, no real fraternal order, no real store names. A Steam store page is published; a Roblox place was not |
| **Build and packaging** | Codex | "It works in the editor" shipping to a tester who cannot launch it |
| **Crash reporting and logs** | Codex | A tester says "it crashed" and there is nothing to read |
| **Playtest logistics** | Chris | Row 37: the Playtest child app, the builds, the testers, the questions asked |
| **Playtest instrumentation** | Claude | Row 3's question answered with opinions instead of data. The Roblox A1 slice was written for this and never built |
| **Store page, copy, capsule art** | Chris | Nobody else can; it is published under his publisher account |
| **The decision log** | Claude | The reason this platform change cost two days of docs instead of two weeks of argument |

---

## Multiplayer — decided now, on purpose

`DECISIONS.md` row 36: solo, or private co-op for 1–4 friends, no public
matchmaking.

**Build with replication from the first commit.** Listen server, host is a
player, no dedicated servers. Every gameplay system in `Source/LLK` is written
server-authoritative from the start, even while there is only one player to
test with.

This is not premature engineering, it is the opposite. Adding replication to a
finished single-player game means rewriting every system that owns state — and
in this game that is *every* system, because the storm charge, the lantern, the
loot and the extraction are all shared. The Roblox prototype was
server-authoritative throughout for exactly this reason, and that architecture
is the one thing about it worth copying.

Steam sessions come from Online Subsystem Steam, configured in
`Config/DefaultEngine.ini` (Codex's lane) and needing one thing we do not have
— see "Open and needed from Chris".

---

## The data pipeline — why `data/*.json` survives

The eight JSON files in `data/` are the project's values layer, and they are
engine-neutral by construction. They stay, and they stay in the same place with
the same rule: **Claude owns the schema, Chris owns the numbers.**

The flow:

```
data/*.json  →  Tools/import_data.py  →  Content/Data/*.uasset (DataTable)
  (text, in git)       (Codex)              (binary, build artifact)
```

- The JSON is source of truth. It is reviewed, diffed and tested.
- The DataTable assets are **generated, never hand-edited.** They are build
  output that happens to live in `Content/`, and they are the one exception to
  "Chris owns `Content/`" — nobody owns them, because nobody authors them.
- `tests/balance.test.mjs` keeps running on the JSON with no engine present.
  Ninety-eight assertions that already encode the game's balance rules, kept
  for free across an engine change. They were worth writing.

Values do not live in `Config/*.ini`, do not live in Blueprint defaults, and do
not live in C++ constants. One source, one owner, one diff.

---

## How the three of us actually hand off

- **Claude → Chris:** a C++ class that needs a mesh, a sound or a level
  placement is landed with the class complete and a written note saying exactly
  what to make and what to set. The system must degrade to nothing — not crash
  — when the content is absent. The Roblox project's rule was that every
  service is a no-op without its geometry; that rule carries over verbatim.
- **Claude → Codex:** a new module dependency, a plugin, a config key or an
  import step gets written into this file's open items rather than edited into
  `*.Build.cs`.
- **Codex → Claude:** a tooling change that alters what the gameplay code can
  assume gets written down, not assumed.
- **Review findings, in any direction:** running another owner's script,
  reading their output and reporting what is wrong is always in bounds -
  **editing their file to fix it is not.** A finding is handed over as the
  symptom, the evidence and the file and line, and the owner decides. Added
  2026-10-29 after I found four real defects in `Tools/**` by running it and
  then fixed them myself, which was the wrong half of a good instinct.
- **Reading each other's work means reading it at HEAD.** A review written
  against a stale checkout reports fixed defects as open, which happened the
  same day in the other direction. `git log --oneline origin/master..HEAD`
  before reporting, or the finding may already be history.
- **Anyone → Chris:** balance values, creative calls, anything binary, anything
  published. He is the only one who can press Play and the only one who can
  release.
- **One writer per lane at a time** (row 27). Two agents in `Source/` at once
  is fine when they are in different modules; two agents in one module is not.

---

## Open, and needed from Chris

1. **The Steam App ID.** The packages he reserved are `1903488`, `1903489`,
   `1903490`, and store item `1409731` — those are packages and a store item,
   **not an App ID.** Online Subsystem Steam needs the App ID, and so does
   every build that wants friends-invite to work. One number, from the
   Steamworks dashboard.
2. **Unreal Engine is not installed on this machine.** Verified today: the
   Epic Games Launcher is present, no `UE_5.*` is. Nothing in the roadmap's M0
   can finish until it is. See `docs/ROADMAP.md`, task M0.1 — it is his, and it
   is the only true blocker in the project right now.
3. **The singular/plural title**, still open from `docs/PLATFORM.md`.
