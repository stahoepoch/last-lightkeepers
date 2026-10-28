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
to `C:/Users/chris/Downloads/`. Three tier archives, **all complete and
verified 2026-10-28**: `bt5_…` (192 entries), `bt15_…` (1,476) and `bt25_…`
(6,944). All three open cleanly; the earlier "contents unverified" note is
resolved. 38 packs as advertised.

**Status: CLEARED TO SHIP, with three conditions.** Resolved 2026-10-28 -
the grant was found on Function Loops' own site and recorded in
`assets/audio/licences/function-loops-2026-10-28.md`. Commercial use in a
**video game** is explicitly permitted and royalty-free, which was the open
question. The conditions:

1. **Ship cooked audio, never loose `.wav` files.** Shipping a work containing
   the sounds is granted; redistributing the sounds is not. Unreal's default
   cooking is already correct, so this costs nothing - just never add an
   uncooked-audio packaging step.
2. **No AI use, at all.** Function Loops prohibits AI training, fine-tuning,
   dataset creation and generative use without written permission. This matters
   here more than anywhere: these wavs are raw material for humans and scripts,
   **never input to a model.** `make_cues.py` is unaffected - it is procedural
   synthesis with no model in it.
3. **One licensee.** The grant is to Chris. A second human needing the raw
   library needs their own licence.

**Nothing outstanding.** Chris checked the Humble receipt on 2026-10-28 and it
carries no terms of its own, so the Function Loops page is the sole governing
grant, and its text is retained verbatim beside the record in
`assets/audio/licences/`.

The original finding that forced this check, kept because it is the reason the
ledger exists:

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

**What this is good for, now read rather than guessed at.** The full
pack-by-pack judgement lives in **`docs/FIELD-KIT.md`**; the short version is
that **Doors** (mostly sliding and garage — a store and a loading dock),
**Household Foley & Devices** (switches, latches, buttons), **Clothing, Fabric
& Materials** (backpack, bandage, cable), **Alarms & Sirens** (the extraction
siren already has `sirenSeconds: 8`), **Ui Clicks**, **Kitchen, Cooking** and
**Foley** are the ones that match something the design already asks for.

Two things the earlier guess got wrong, both worth keeping visible:

- **Scale.** Every SFX pack is exactly **30 files**, and ~2,060 of the ~3,100
  files are music-production content the roadmap rules out for now. This is a
  seasoning library, not a foundation, and it replaces none of the six cues in
  `assets/audio/make_cues.py`.
- **"The creature packs are wrong on principle" understated it.** *Monsters,
  Boss & Zombies* is **voiced English dialogue** — "Fresh meat", "I can smell
  your fear", "Come out little mouse". Row 47 is LOCKED: the machines want
  nothing and hate nothing. One of these in the build inverts the design's best
  idea. Same for *Horror Screamers* against `docs/SOUND.md`'s telegraph rule.

`assets/audio/make_cues.py` is unaffected and stays the method of record for
enemy telegraphs. A bought library does not replace the six synthesised cue
designs; it supplies the world around them.

| Pack | Used for | Licence on file | Cleared |
| --- | --- | --- | --- |
| *(none referenced by the build yet)* | — | `function-loops-2026-10-28.md` | **yes, on the three conditions above** |

Add a row per pack as the build starts referencing them, rather than listing
all 38 - the ledger's job is to answer "may this ship", and a pack nobody uses
has no answer to give.

## Source files held but not yet shipped

Concept art, reference only. Not for upload.

- `assets/enemies/sweeper-concept-v1.png`
- `assets/enemies/glimmer-concept-v1.png`
- `assets/enemies/glimmer-scan-glyph-v1.png`
- `assets/enemies/window-walker-mannequin-concept-v1.png`
- `assets/enemies/window-walker-mannequin-concept-v2.png`
- `assets/enemies/window-walker-mannequin-concept-v3.png`
- `assets/enemies/ficus-stalker-concept-v1.png` — ~~queried~~ **answered, and
  the query was stale when it was written.** `DECISIONS.md` row 13 is LOCKED at
  **six** enemies including the Ficus, approved by Chris on 2026-10-20
  (*"Codex did it, I like the ficus"*). The roster of five quoted here was
  already out of date. `data/enemies.json` carries a complete `Ficus` row and
  `ELLKEnemyFamily` carries the value. Closed 2026-10-28.
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
