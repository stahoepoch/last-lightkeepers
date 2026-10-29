# The field kit — what to buy, what to capture, what to refuse

Written 2026-10-28, answering Chris: *"do any of the assets look like something
we should be getting? anything i should look out for in my travels that we may
need? my own checklist?"*

Three parts. Part 1 is the bundle he already owns, verified rather than guessed
at. Part 2 is the checklist to carry — the things only he can get, because they
are not for sale. Part 3 is what to ask **before** money leaves, which exists
because the Function Loops purchase taught it the hard way.

`docs/ASSETS.md` is still the policy (*everything we ship, we own*) and
`assets/MANIFEST.md` is still the ledger. This is the acquisition side of both.

---

## Part 1 — The bundle, now that all three tiers are readable

All three archives finished downloading and open cleanly: **bt5** 192 entries,
**bt15** 1,476, **bt25** 6,944. The "contents unverified" note in
`assets/MANIFEST.md` is now resolved. 38 packs, as advertised.

### The honest shape of it, before the recommendations

Of roughly 3,100 files, **about 2,060 are music-production content** — the
DRUMKITS VST plugin (887), *1984 Retro Music Production Bundle* (739) and
*Modern Music Kits* (433), delivered as MIDI, Serum presets and loops. The
roadmap's "what we are deliberately not doing yet" list rules out music
entirely for now, so that is two thirds of the bundle by file count that this
project has no use for this year.

**Every actual SFX pack is exactly 30 files.** Not 30 variations of a door —
30 sounds covering the whole category.

That is not a complaint about a $25 purchase; it is the thing to know before
planning around it. **This is a seasoning library, not a foundation.** It will
not build the store's soundscape, and it does not replace a single one of the
six cues in `assets/audio/make_cues.py`. What it does is stop us hand-making
the fiftieth incidental noise.

### Take these — they match something the design already asks for

| Pack | What it actually contains | What it is for here |
| --- | --- | --- |
| **Doors** | Mostly **sliding** doors, plus locks, latches and **garage** doors | The best fit in the bundle. A department store's doors are sliding doors, and the garage rolls are the loading dock — which `storm-charge.json` already calls the early exit |
| **Household Foley & Devices** | Buttons, switches (`Interrupteur`), latches, fridge, microwave | Row 47's whole thesis in audio form. A building still doing its job is *switches and latches*, not growls |
| **Clothing, Fabric & Materials** | Backpack, bag-open, **bandage**, cable, drop, fabric | The carry-and-burden vocabulary exactly. Bandage reads as the revive hold; backpack reads as looting |
| **Alarms & Sirens** | 30 straight alarm sirens | `extractionCall.sirenSeconds` is already 8. Use as a **layer under** `extraction-siren-v1.wav`, not instead of it — the made cue is the master |
| **Ui Clicks** / **Ui Games Interface** | Clean interface clicks | The debug HUD (M1.7) and the workshop. Cheap, immediate, invisible when right |
| **Kitchen, Cooking** | Boiling, kettle, frying, shredding | The Community Kitchen has no audio at all today. This is the entire room, solved |
| **Foley** | Bottle, crunch, drop, glass, **keys** | General room dressing. Keys are worth having for a store |

### What the files actually are, read from their headers

Every `.wav` in every pack audited is **44.1 kHz, 16-bit, stereo**. No
exceptions across the eleven packs worth taking - 340 files, all two-channel.
Durations fall into three groups:

| Group | Length | Packs |
| --- | --- | --- |
| One-shots | 0.5-4s (median ~1s) | Doors, Household Foley, Clothing, Foley, Ui Clicks, Glitches, Granular |
| Sirens | **11.52s, every file** | Alarms & Sirens |
| Beds | 16s / 32s, every file | Horror Ambience / Ambience & SFX |

**The stereo question, checked rather than recalled.** The common wisdom is
that Unreal will not spatialize a stereo sound. On 5.8 that is **false** -
`AudioMixerSource.cpp:246-275` returns 3D for anything up to two channels
(*"3d panner can't handle more than stereo currently"*), so these files do
position in the world. Worth stating plainly, because believing the myth would
mean batch-converting 340 files for nothing.

**What is true is subtler and still matters here.** `SoundAttenuation.h:264`
defines `StereoSpread` as *"the world-space distance between left and right
stereo channels when stereo assets are 3D spatialized"* - so a stereo source
is placed as **two points separated in space**, not one. Direction is smeared
by design.

That is a quality problem rather than a blocker, but it lands on the one audio
requirement this project calls non-negotiable. `docs/SOUND.md` makes
telegraph-before-sight an accessibility obligation, and enemies converge on
the brightest position, so a player must be able to say *where* a Sweeper is,
not merely that one exists. A smeared source is a worse answer to the question
the design asks most often.

**So, per use:**

- **In-world** (doors, foley, machine texture, the siren at its exit):
  convert to mono, or set `StereoSpread` to 0 on the attenuation so the pair
  collapses to a point. Decide once and write it into the attenuation asset
  rather than per-sound.
- **UI and ambience beds**: leave them stereo. Interface audio is 2D by
  nature, and a room tone *wants* width. Stereo is the right form there, not a
  compromise.

**One mismatch to settle before the siren is wired.** Every siren file is
11.52s and `storm-charge.json` sets `extractionCall.sirenSeconds` to 8. The
asset outlasts the window. That is a design question, not a defect - either
the siren is trimmed to the call, or 8s is a fade window over a looping bed
and the data means something slightly different from what it says. Worth
deciding before M3 wires it, since `assets/audio/make_cues.py` already renders
`extraction-siren-v1.wav` as the master and this pack is a layer under it.

### Take these with care — raw material, not finished cues

- **Glitches** and **Granular** — these are `Motion_*` designer sounds, meant
  as transitions. Usable as *ingredients* for machine texture, never dropped in
  whole. The moment one is recognisable as a stock whoosh, it has cost more
  than it gave.
- **Horror Ambience** — 30 undifferentiated beds. Worth an audition pass, but
  `docs/SOUND.md` makes the storm the startle instrument, and a generic dread
  bed under everything is exactly the lazy move that makes a horror game sound
  like every other horror game.
- **Ambience & SFX** — all sci-fi, and all **keyed** (`_Cm`, `_Fm`). Keyed
  drones are music wearing an ambience label. Against the no-music rule for now.
- **Cozy Mechanics** — a farming/fishing pack. `Dig_Stack_Drop` is usable for
  crates and restoration; the bait and fish are not for us.
- **Checkpoints, Errors And Riddles** — possible for rung completion and the
  restoration credit, if they do not sound like a mobile puzzle game.

### Never — and one of these would do real damage

- **Monsters, Boss & Zombies.** This is not ambiguous and it is worth being
  blunt: the files are **voiced English lines** — *"Fresh meat"*, *"I can smell
  your fear"*, *"I see you"*, *"Come out little mouse."* Row 47 is LOCKED: the
  machines want nothing and hate nothing, they are finishing the last job they
  were given. A single one of these in the build converts the game's best idea
  into a generic monster closet. Not a near miss — the exact opposite of the
  design.
- **Horror Screamers.** Jumpscare stingers. `docs/SOUND.md` builds dread out of
  the storm and telegraphs threats *before* sight as an accessibility
  obligation. A screamer is the sound of giving up on that.
- **Guns, Medieval Fighting, Magic Spells, War Strategy, 8-Bit, Shooter Radio
  Commands, Orchestral Braams/Impacts/Whooshes, Dreamy Whooshes, Bluster
  Shots** — wrong genre. The keepers do not fight (`docs/COMBAT.md`).
- **The music kits and the DRUMKITS VST.** No music work before M4, and there
  is no reason to install a plugin binary from a torrent onto the build machine.
- **ABYSS – Dark Cinematic** is two files, one of them a nested zip. A sampler.

### Three conditions still apply to every one of the above

From `assets/audio/licences/function-loops-2026-10-28.md`, unchanged: ship
**cooked** audio only, **never feed any of it to a model** (these are raw
material for humans and scripts), and the licence is **to Chris alone**.

---

## Part 2 — The checklist to carry

This is the part worth printing. **Everything here is free, unavailable for
purchase, and better than the bought version** — because the game is set in a
specific valley and a specific kind of building, and stock libraries are
generic by definition.

A phone is enough to start. The limit is wind and handling noise, not the
microphone.

### Sound — record these, in roughly this order of value

The bundle has none of these, and they *are* the game.

**In any store still standing, or any dead mall:**
- [ ] Automatic sliding doors — the motor, the rubber seal, the pause
- [ ] HVAC drone from directly under a vent, 60 seconds unbroken
- [ ] Fluorescent ballast hum, close
- [ ] A shopping cart on hard tile, pushed and released
- [ ] Metal shelving — a hand on a gondola upright, a shelf flexing
- [ ] An escalator from the top, the bottom, and the plate
- [ ] A rolling security gate coming down
- [ ] A freight elevator, inside, doors and travel
- [ ] A loading-dock roller door, and the dock plate

**Rail yard, from a legal and public vantage:**
- [ ] Couplings, distant and close
- [ ] Rail squeal on a curve
- [ ] A horn at distance, and the way the valley returns it
- [ ] Ballast underfoot

**Weather and water, which row 45 makes the clock:**
- [ ] Rain on a metal roof, on glass, on a flat
- [ ] Wind in a structure — a stairwell, a gap, a wire
- [ ] Thunder at distance, and close
- [ ] The river, at a few different grades

**The rule for all of it:** record **ambience long** (a usable bed is 60+
seconds, not 5) and **events with silence either side**, so the tail survives.
Keep the raw file. Per `docs/ASSETS.md`, the master lives in `assets/` and the
master is what we own.

**And the obvious courtesy, which is also the practical advice:** ask the
manager before recording inside a business, and do not record conversations or
people — we want the building, not the public. A refused ask costs nothing; a
confrontation costs the trip.

### Photographs — the store needs five shapes, not a thousand

`docs/THE-STORE.md` is explicit that density comes from **repetition with
intent**: four kits per department, five fixture meshes total, and *signage
carries the realism, not geometry.* So photograph the fixtures and the signs,
not the merchandise.

- [ ] **Round rack**, **four-way rack**, **glass case**, **gondola shelf**,
      **wall standard** — each from three angles, plus one close shot of how it
      meets the floor
- [ ] **Signage, exhaustively**: department names, price cards, sale banners, a
      directory board, floor numbers on stair doors, exit signs lit and unlit
- [ ] Ceiling: tile grid, light fixture types, sprinkler heads, vents
- [ ] Floor transitions — tile to carpet is where a department boundary reads
- [ ] Emergency and after-hours lighting, if ever seen. **This is rare and
      valuable**: almost nobody photographs a retail floor lit only by its exit
      signs, and that is the game's actual lighting condition
- [ ] A fitting-room corridor, an escalator well, a lunch counter — the
      landmark objects players will name out loud

**Scale beats beauty.** One tape-measure or doorway-height reference in frame
is worth more than a sharp photo, because the store has to *teach*, and
legibility is proportion before it is art. Aisle width, ceiling height, rack
height, door width.

**If photogrammetry is ever on the table:** overcast day, no hard shadows,
heavy overlap, no people in frame. But reference photos are useful immediately
and photogrammetry is not first-slice work.

### The valley — row 57 decided *Johnstown-shaped, not a reconstruction*

The elevation comes from public USGS data, so the trip is not for terrain
geometry. It is for the things data has no record of:

- [ ] **Sightlines**: what you can see from the valley floor and cannot reach.
      Row 57's "visible-but-unreachable landmarks" are a feeling, and the
      feeling is a photograph
- [ ] The floodwall, the river grade, the inclined plane
- [ ] How the hills close the sky at dusk — the hour matters more than the day
- [ ] Brick, stone and the specific colour of wet masonry there
- [ ] Street furniture, signage typography, the civic vocabulary of the place

### What is worth *buying* later, and what is not

- **Worth it:** a department-store or retail fixture kit, if one exists at the
  right quality. Five meshes done well is the whole store, and geometry is
  Chris's lane (row 41) so a bought kit has no agent-merge problem.
- **Worth it:** a handheld recorder, only once the phone's limits are the
  actual blocker. The phone will get the first fifty sounds.
- **Not worth it:** more SFX bundles. We now own ~1,000 stock sounds and use
  none of them yet. Buying a second library before the first has shipped a
  single asset is how a library becomes a hobby.
- **Not worth it:** anything for a milestone past M4. The gate decides whether
  M5–M7 happen at all.

---

## Part 3 — Before money leaves, four questions

The Function Loops purchase was fine in the end, but it took a chain of four
checks to find that out: *the archives shipped no licence → the Humble bundle
page showed none → the receipt carried none → the vendor's own site was the
grant.* That is the normal case, not bad luck. So:

1. **Where is the licence, as a URL or a file?** If the answer is "it'll be in
   the download", assume it is not. Find it before paying.
2. **Does it name a sold game binary?** "For your projects" is not the same as
   commercial distribution inside a product you charge for. Look for the medium
   named — *video games* — and for royalty-free and perpetual.
3. **What does it say about AI?** This matters more here than almost anywhere,
   because this project is built by agents. Function Loops forbids all
   generative and training use. Anything bought has to be checkable against
   that line, and the answer has to be written down where an agent will read it
   before touching the files.
4. **Is it one seat?** Fine today. Worth knowing before it isn't.

Then: **save a copy of the grant into `assets/<kind>/licences/` with the date
and where it came from.** A licence you cannot produce later is a licence you
do not have — that sentence is the reason the audio ledger exists, and it
applies to every category, not just sound.

---

## The gap this page named is now closed

`docs/ASSETS.md` was still written for Roblox - its table mapped masters to
`rbxassetid://` uploads and its enforcement rule grepped `src/` for asset ids.
The *policy* survived the engine change intact; the *mechanics* did not.
Rewritten 2026-10-28 for Unreal: content paths and LFS instead of upload ids,
row 41's no-agent-opens-a-uasset rule, the licence questions generalised from
audio to every category, and the mono/stereo decision recorded where the
audio rule lives.

**One thing it now says plainly rather than implying:** the manifest
enforcement check does not exist. On Roblox it was a grep for
`rbxassetid://`. The Unreal equivalent - scanning referenced content paths
against `assets/MANIFEST.md` - belongs in the fast CI gate beside the data
checks, and nobody has written it. A policy page that implies a check runs is
worse than one that admits it does not.
