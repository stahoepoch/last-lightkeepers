# The frozen prototype's checks live here, outside `npm test`

Everything in this folder tests the **archived Roblox build** — `src/`, which
is 36 Luau files, and `default.project.json`, which is a Rojo `DataModel`.
None of it tests the shipping game, which is Unreal Engine 5.

## Why they were moved, 2026-11-03

They were in `npm test`, green, detailed, and about the wrong engine:

| File | Checks | Reads |
| --- | --- | --- |
| `wiring.test.mjs` | 297 | `src/**.luau` — its own header says *"Static wiring checks for the Luau"* |
| `lighting.test.mjs` | 89 | `default.project.json` and `src/server/services/LightingService.luau` |
| `map-contract.test.mjs` | 6 | `src/map/GalleriaBuilder.luau` |

**392 green checks, none of them about the renderer or the map that ships.**

The cost was not wasted CPU. It was misdirection. `lighting.test.mjs` is the
most emphatic document in this repository — *"DONTFUCKUPTHELIGHTS.md explains
why these values are what they are"* — and it asserts things about
`Lighting.Ambient`, `EnvironmentDiffuseScale`, `GlobalShadows` and
`ClockTime`. Those are Roblox Lighting service properties. **They do not exist
in Unreal.** Any reader, human or agent, who went looking for this project's
lighting law would find a thorough, passing, authoritative-looking guard over
settings the game does not have, while the real law — measured in candelas in
`LLK_LightLab`, written down in `docs/FIXTURE-LIGHT-CONTRACT.md` — sat
somewhere else entirely.

A green signal about the wrong world is worse than no signal, and this
project has already learned that lesson twice in other forms.

## What was deliberately *not* done

**They were not deleted.** The prototype is a reference, and these checks
encode real design reasoning that was paid for in playtests — the asymmetry
between `aggroRadiusDark` and `aggroRadiusLit`, the per-player relic
visibility, the enemy telegraph rigs. When an Unreal equivalent is written,
this is where to read what the question was.

**The boundary was drawn by audit, not by guesswork.** All fourteen node
checks were examined for prototype references; exactly these three had any.
The other eleven read `Source/`, `data/` or `docs/` and stay in `npm test`.
`wiring.test.mjs` also touches `data/`, which is shared with the shipping
game — but it touches it only to resolve Luau `Config.<Section>.<key>`
lookups, so it moved whole. The Unreal side of that data is covered by
`Tools/import_data.py` and its contract tests, plus `units.test.mjs` and
`balance.test.mjs`.

## Running them

```
npm run test:prototype
```

`npm test` means *the shipping game*: portable data contracts plus Unreal
source contracts. It must stay that way. If you add a check here, it does not
belong in `npm test`, and if you add a check about Unreal, it does not belong
in this folder.
