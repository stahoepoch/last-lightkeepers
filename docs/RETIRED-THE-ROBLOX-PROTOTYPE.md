# The Roblox prototype is retired, and here is where it went

> **Status: DONE 2026-11-03**, on Chris's instruction — *"roblox is over with,
> so retire what we need to."* Nothing was lost. Everything removed is
> addressable at the annotated tag **`roblox-prototype-final`**.

## Why this needed doing rather than just noting

The shipping game has been Unreal since 2026-10-27. The prototype stayed in
the tree, and it was not inert — it was **authoritative-looking and wrong**:

- `npm test` ran **392 green checks** against Luau and a Rojo `DataModel`.
- A **pre-commit hook** gated *every commit in an Unreal project* on
  `Ambient` and `EnvironmentDiffuseScale`, which are Roblox Lighting service
  properties. It blocked an unrelated commit on 2026-11-03 for that reason.
- The one document titled *"read this before you touch lighting"* described a
  renderer the game does not use, while the real law — candelas, measured in
  `LLK_LightLab` — lived in two other files that nothing enforced.

A green signal about the wrong world is worse than no signal.

## What was removed from the tree

| | |
| --- | --- |
| `src/` | 36 Luau files — services, client, shared, map builders |
| `default.project.json` | the Rojo project (`$className: DataModel`) |
| `selene.toml`, `rokit.toml` | the Luau toolchain configs |
| `tests/prototype/` | the 392 checks that guarded the above, plus their README |
| `.mcp.json` → `Roblox_Studio` | the Studio bridge server |
| `package.json` | `test:prototype`, `test:lighting`, `test:wiring`, `test:map-contract` |

**45 files.** They are not gone; they are one `git show roblox-prototype-final`
away, and the tag's own message says what it holds.

## What was kept, and why

**`DONTFUCKUPTHELIGHTS.md` stays, marked retired at the top.**
`docs/LIGHTING-IN-UNREAL.md` cites it as its ancestor and says it is *"still
right about its subject"*. Deleting it would break the lineage of a design
decision, which is the exact failure `docs/HANDOFFS.md` exists to prevent. One
markdown file costs nothing to keep and its thesis is still the reason the
guard exists at all:

> The lighting has been broken and re-broken several times, each time by
> someone changing one number in good faith.

**The pre-commit hook kept its job and changed its target.** It now runs
`tests/light-authority.test.mjs` — the Unreal guard — and its failure message
points at `LIGHTING-IN-UNREAL.md` and the LOCKED `FIXTURE-LIGHT-CONTRACT.md`.
The intent was always right; only the engine was wrong.

## What is deliberately left undone

**About twenty files under `docs/` still mention Roblox, Luau, Rojo or
Studio.** Most do so historically and correctly — `PLATFORM.md` and
`DECISIONS.md` record the move itself, and that record should survive. A sweep
to find the ones that still speak in the present tense is a separate job, and
doing it by bulk search would damage the history it is meant to clarify.

**The design reasoning in the deleted tests was not extracted.** `wiring.test.mjs`
held real findings — per-player relic visibility, the enemy telegraph rigs, the
regressions the first playtest found. Where an Unreal equivalent gets written,
read the Luau version at the tag first to see what the question was. It is
cited here so the next person knows it exists rather than rediscovering it.

## After

`npm test` means the shipping game and nothing else: the portable data
contracts, the importer contract tests, and the Unreal source contracts —
eleven node checks plus three Python ones.
