# Asset policy — everything we ship, we own

Decided by Chris, 2026-10-20: *"we make our own assets and sounds then to
carry."*

**Rewritten for Unreal 2026-10-28.** The rule below is unchanged and was never
in question. What changed is every mechanism that enforced it: this page used
to describe `rbxassetid://` uploads, and the Steam pivot (row 32) left it
describing an engine we no longer use. A policy document that describes the
wrong engine is how a rule quietly stops being followed, which is why this was
flagged in `docs/FIELD-KIT.md` and then fixed rather than left.

---

## The rule

**Every asset that ships traces to a source we own or are licensed to ship.**

Two ways to satisfy it, and only two:

1. **We made it.** The master lives in `assets/`, outside the engine, in a
   format that survives an engine change.
2. **We bought it, and the licence permits shipping it inside a sold game
   binary.** The grant is saved under `assets/<kind>/licences/` with its date
   and source, and the pack has a row in `assets/MANIFEST.md`.

Anything else is an unlicensed dependency and must not ship. There is no third
category, and "it was free on the internet" is not one.

## What changed with the engine, and what did not

| | Roblox (frozen, row 33) | Unreal |
| --- | --- | --- |
| How a shipped asset is named | `rbxassetid://123` pointing at an upload | A content path under `/Game/`, cooked into the build |
| Where the file lives | Roblox's servers | `Content/`, versioned in Git LFS (`.gitattributes`) |
| How to check the rule | grep `src/` for ids, match against the manifest | Match referenced content paths against the manifest |
| What an agent may touch | `.rbxmx` was text and merged | **Nothing binary.** Row 41: no agent opens a `.uasset` |

**What did not change is the part that mattered.** The whole argument for
keeping masters outside the engine was that the portable layer has to survive
an engine change — and then it did. Design, data, audio masters and documents
came across; Studio maps, Luau and Roblox networking were rebuilt. The
`.wav` files in `assets/audio/` are still usable today precisely because they
were never only an upload id.

## Where masters live

| Kind | Master | Shipped as |
| --- | --- | --- |
| Audio we made | `assets/audio/*.wav`, and the script that renders them | Imported `USoundWave` under `/Game/Audio/`, cooked |
| Audio we bought | The pack, kept out of the repo; licence in `assets/audio/licences/` | Cooked audio only — never loose `.wav` in the pak |
| Models | `assets/models/*.blend` | Exported `.fbx`, imported, cooked |
| Textures | `assets/textures/` at full resolution | Imported, compressed by Unreal |
| Concept art | `assets/<area>/*.png` | **Reference only, never shipped** |
| Generated data | `data/*.json` | `Content/Data/*.uasset`, built on demand, never committed |

**`assets/audio/make_cues.py` is the model for all of this.** The master is a
script, not a `.wav`: it is text, it diffs, it re-renders at any sample rate,
and it survived the engine change without anyone touching it. Where a master
can be code, make it code.

## The manifest

`assets/MANIFEST.md` is the ledger, and its job is to answer one question:
**may this ship?** It carries every bought library with its licence status,
every placeholder that must not ship, and every source file held for
reference.

A row is added when the build starts referencing a pack, not when the pack is
bought. A ledger listing 38 packs nobody uses has 38 answers nobody needs.

**The enforcement check is owed and does not exist yet.** On Roblox it was a
grep of `src/` for `rbxassetid://`. The Unreal equivalent is a scan of
referenced content paths against the manifest, and it belongs in the fast CI
gate beside the data checks. Nobody has written it; saying so here is better
than implying it runs.

## Placeholders

Placeholders are allowed. **Silent placeholders are not.** Mark them in code
and list them in the manifest under "Owed", so nothing borrowed reaches a
build by being forgotten.

## Bought libraries: four questions before money leaves

Written out in `docs/FIELD-KIT.md` and summarised here because this page is
the policy: find the licence as a URL or file; confirm it names commercial use
inside a sold game; check what it says about AI; check whether it is one seat.
Then save a copy. **A licence you cannot produce later is a licence you do not
have.**

That sentence is not theoretical. The Function Loops bundle shipped no licence
in the archives, the Humble page carried none, and the receipt carried none —
the grant was on the vendor's own site, and is now retained verbatim beside
its summary.

**One clause deserves repeating on this page, because this project is built by
AI agents:** Function Loops prohibits all generative and training use of its
content. Those files are raw material for humans and scripts, never input to a
model. `make_cues.py` is unaffected — it is arithmetic, not a model.

## Audio has a second requirement, and it is not polish

The handoff makes enemy audio an accessibility rule: **telegraph every enemy
with sound before sight.** Each enemy needs a cue a player can tell apart by
ear alone — and, since enemies converge on the brightest position rather than
the nearest player, a cue they can *locate*.

The six cues in `assets/audio/make_cues.py` each use a different *mechanism*
rather than a different setting on one sample, which is what makes them
distinguishable:

- **Sweeper** — many small things moving, low and scuttling
- **Stocker** — slow, heavy, a single source
- **Glimmer** — a scan tone with an audible 2-second shape, so the window to
  go dark can be heard rather than guessed
- **Window Walker** — stillness breaking
- **Ficus** — the one that is not a machine: dense foliage that fuses into
  texture rather than countable events, with no mechanical content anywhere
- **Warden** — unmistakable, and audible from much further than the rest

**Positional audio wants mono.** Unreal does spatialize stereo, but places the
two channels a `StereoSpread` apart in world space, so direction is smeared.
In-world sounds go mono, or the attenuation asset sets `StereoSpread` to 0;
UI and ambience beds stay stereo. See `docs/FIELD-KIT.md` for the measurements
behind that.

## Why this matters more here than on a normal project

Borrowed assets are the one category that quietly fails to transfer, and the
cost of unpicking them scales with how long nobody noticed. We have now
changed engines once. The things that survived were the things we owned, in
formats we chose. That is the whole argument, and it has already been tested
once rather than merely asserted.
