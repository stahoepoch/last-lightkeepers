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

## Purchased libraries — held, licence unresolved

Added 2026-10-28. Chris bought the **Ultimate SFX for Games** bundle (Humble
Bundle, $25 tier, 38 packs, developer **Function Loops**, DRM-free), downloading
to `C:/Users/chris/Downloads/`. Three tier archives:
`bt5_…` (complete, 192 entries), `bt15_…` and `bt25_…` (still filling over
BitTorrent — pre-allocated to full size, central directory not yet readable, so
their contents are unverified).

**Status: NOT CLEARED TO SHIP.** Not because anything is wrong with it, but
because of what is missing:

> **The archives contain no licence file.** The completed `bt5` tier is 180
> `.wav` files across *Cozy Mechanics*, *Swing & Dash* and *Guns*, plus
> `__MACOSX` and `.DS_Store` cruft. No `LICENSE`, no `README`, no `TERMS`, no
> EULA, in any form. The bundle's Humble page carried no licence text either.

So the commercial terms exist somewhere other than the files — Function Loops'
own site, or the Humble purchase receipt — and nobody has read them. Row 23 of
`docs/DECISIONS.md` and `docs/ASSETS.md` both turn on licence durability, which
is the one thing that cannot be checked by listening to a `.wav`.

**What has to happen before a single one of these is referenced by the build:**

1. Find the actual grant — royalty-free, perpetual, commercial, no attribution
   requirement, and specifically **permitted inside a sold game binary** rather
   than merely "for your projects".
2. Save a copy of it into `assets/audio/licences/` with the date and where it
   came from. A licence you cannot produce later is a licence you do not have.
3. Record per-pack rows below, as rows for the packs actually used rather than
   for all 38.

**What this is probably good for**, once cleared — the packs worth wanting are
in the two tiers still downloading: **Doors**, **Foley**, **Household Foley &
Devices**, **Alarms & Sirens** (the extraction siren already has
`sirenSeconds: 8` in the data), **Horror Ambience**, **Glitches**, **Granular**
and **Techno Textures** for machine voices, and **Kitchen, Cooking** for the
Community Kitchen, which has no audio at all today. The creature packs are
wrong for this game on principle: row 47 says the machines do not roar, they
work.

`assets/audio/make_cues.py` is unaffected and stays the method of record for
enemy telegraphs. A bought library does not replace the six synthesised cue
designs; it supplies the world around them.

| Pack | Used for | Licence on file | Cleared |
| --- | --- | --- | --- |
| *(none yet)* | — | **no** | **no** |

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

## Audio — made, not borrowed

Source: **`assets/audio/make_cues.py`**. Every cue is synthesised from
arithmetic in that script — nothing sampled, nothing downloaded. The master is
the script, not the `.wav`: it is text, it diffs, and it re-renders at any
sample rate or length when the game is rebuilt in another engine. The `.wav`
files are just today's render.

Each cue is built from a **different mechanism**, not a different knob on one
sample, because the handoff's telegraph rule is about telling enemies apart by
ear:

| Cue | Identity | Mechanism |
| --- | --- | --- |
| `sweeper-telegraph-v1.wav` | A crowd, not a thing | Dozens of irregular filtered-noise grains; the tell is event *rate* |
| `stocker-telegraph-v1.wav` | Weight, and singularity | One low impact plus a descending servo groan |
| `glimmer-scan-v1.wav` | Time running out | A tone rising across exactly 2.0s with ticks that *accelerate*, so the closing window is audible |
| `window-walker-wake-v1.wav` | Stillness breaking | Silence first, then a dry crack and a rigid creak |
| `warden-approach-v1.wav` | Dread, and distance | Two near-unison low fundamentals beating, with a long decaying tail |
| `extraction-siren-v1.wav` | A public commitment | Alternating rising/falling alarm sweeps over a mechanical low pulse; positional at the called exit. |

The Glimmer one is the one that mattered: the handoff gives the player two
seconds to go dark, and a single hit cannot say *you have time, and it is
running out.* Accelerating ticks can.

**Not yet uploaded.** These have no `rbxassetid` because nothing has been
uploaded — that is an account action and Chris's to take. Upload, then record
the ids in the Shipping table above and swap them into the rigs. Until then the
rigs correctly still carry the placeholder.

## Audio still to make

Each enemy needs a cue distinguishable from the others by ear alone — that is
the handoff's accessibility requirement, not polish. Brief in `docs/ASSETS.md`.

- [ ] Sweeper telegraph
- [ ] Stocker telegraph
- [ ] Glimmer scan tone (must make the 2-second window audible)
- [ ] Window Walker wake
- [ ] Warden approach
