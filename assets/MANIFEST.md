# Asset manifest

Every `rbxassetid://` the project references, what it is, and the source file it
came from. See `docs/ASSETS.md` for the rule.

**If an id appears in `src/` and not here, it is an unlicensed dependency and
must not ship.**

## Shipping

*(none yet — nothing has been uploaded from a master we own)*

| Asset id | Used for | Source file | Made by |
| --- | --- | --- | --- |

## Owed — placeholders that must not ship

| Asset id | Used for | Problem |
| --- | --- | --- |
| `rbxassetid://9118959688` | Telegraph sound in `SweeperRig`, `GlimmerRig` and `WindowWalkerRig` | **Not ours.** Identified as "Servo Motor 7 (SFX)", Pro Sound Effects — verified real and loadable by the gameplay session via Studio asset search and a live playtest (`IsLoaded` true, `TimeLength` 1.11s). Verifying an asset exists is not owning it: it lives on Roblox's servers under someone else's licence and cannot port. Replace with our own recording. |

An earlier version of this row said "origin unverified", which was wrong — the
gameplay session had checked it and I had not asked. Corrected 2026-10-20.

## Source files held but not yet shipped

Concept art, reference only. Not for upload.

- `assets/enemies/sweeper-concept-v1.png`
- `assets/enemies/glimmer-concept-v1.png`
- `assets/enemies/glimmer-scan-glyph-v1.png`
- `assets/enemies/window-walker-mannequin-concept-v1.png`
- `assets/enemies/window-walker-mannequin-concept-v2.png`
- `assets/enemies/window-walker-mannequin-concept-v3.png`
- `assets/enemies/ficus-stalker-concept-v1.png` — **queried.** No "ficus
  stalker" exists in the roster, which is Sweepers, Stockers, Glimmers, Window
  Walkers and the Warden. Either it is a Stocker concept under a working name,
  or it is a sixth enemy nobody approved. Chris to confirm which.
- `assets/loot/relic-access-device-concept-v1.png`
- `assets/loot/restoration-crate-concept-v1.png`

## Audio still to make

Each enemy needs a cue distinguishable from the others by ear alone — that is
the handoff's accessibility requirement, not polish. Brief in `docs/ASSETS.md`.

- [ ] Sweeper telegraph
- [ ] Stocker telegraph
- [ ] Glimmer scan tone (must make the 2-second window audible)
- [ ] Window Walker wake
- [ ] Warden approach
