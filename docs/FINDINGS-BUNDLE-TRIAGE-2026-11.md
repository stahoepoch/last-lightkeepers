# Two of the three bundles are music this game must not have

**Written 2026-11-11**, triaging three Humble bundles Chris relayed, against
`DECISIONS.md` row 24, row 16, `docs/SOUND.md`, and the precedent already set
by `assets/MANIFEST.md` on 2026-10-28. Nothing was bought and nothing is
Chris's to be told — asset provenance is his lane (`LANES.md:108`).

**Short version: skip the two audio bundles, and the third is a row 24
decision rather than a value decision.** None of the three unblocks anything.

## The precedent that already decides most of this

`assets/MANIFEST.md` records a Humble audio bundle bought on 2026-10-28 —
Function Loops, $25 tier, 38 packs — cleared to ship on three conditions, and
with two categories **declined on design grounds, not licence grounds**:

> *"**Monsters, Boss & Zombies** is **voiced English dialogue** — 'Fresh
> meat', 'I can smell your fear'... Row 47 is LOCKED: the machines want
> nothing and hate nothing. One of these in the build inverts the design's
> best idea. **Same for *Horror Screamers* against `docs/SOUND.md`'s telegraph
> rule.**"*

And the scale lesson from the same entry: *"~2,060 of the ~3,100 files are
music-production content the roadmap rules out for now. This is a seasoning
library, not a foundation, and it replaces none of the six cues in
`assets/audio/make_cues.py`."*

Both findings apply directly below.

## 1. Sounds to Die For: The Horror Audio Collection — **skip**

$20 for 10 items, MSRP $414.90. Ends about 2026-11-18.

Ten packs, every one of them a **themed horror score**: *Occult Metal*,
*Psychological*, *Space & Isolation*, *Tech-Horror*, *Found Footage*,
*Gothic*, *Occult Folk*, *Eldritch*, *Retro Pixel*, *Slasher*.

This is the wrong instrument for this game, and the reason is a LOCKED row.

Row **16** requires that *"every enemy telegraphs before sight through at
least two distinguishable channels"*, and `PITCH.md` promises *"everything
makes a sound before you see it."* **In a game whose core verb is listening
for a machine you cannot see, a continuous horror score is a mask over the
mechanic.** `SOUND.md` is built entirely around telegraphs, afterimages and
the storm as a three-channel system; there is no layer in that design where a
scored cue belongs.

`MANIFEST.md` already declined *Horror Screamers* on exactly this rule. Ten
more horror scores is the same decision at ten times the volume — and buying
them is buying the pressure to use one.

**The one music slot this game actually has is diegetic**: a store PA still
looping a tape nobody turned off, a demo TV in the teens area. None of these
ten packs is that. A 1992 mall does not play eldritch horror.

## 2. The Complete Works: Royalty-Free Audio Tracks — **skip**

$15 for 128 items, MSRP $5,906. Ends about 2026-11-16 — **the first of the
three to expire.**

128 packs of library music: *Pop music*, *Orchestral*, *Guitar music*,
*Country dreams*, *Sad songs*, *Happy days*, *Flower power*, *Summertime*,
*Dance away*, *Pastoral*, plus 20 loop packs.

Same objection as above for score, and the same MANIFEST lesson about music
being roadmap-excluded. But this one is worth one honest sentence the first
is not:

**It is the only thing on the table that resembles the game's one legitimate
music need.** *Happy days*, *Flower power*, *Summertime*, *Pop music*,
*Country dreams* is what production-library muzak sounds like, and muzak over
a dead mall PA is a genuinely strong horror instrument — the building still
doing its job for nobody, which is row 47 in sound.

Two reasons it still comes out as skip:

- **Era.** Library music is written to sound current, not to sound like 1992.
  The thing that would sell this is period-correct, and nothing on the page
  claims that. Buying 128 packs to maybe find one usable loop is backwards.
- **Volume.** The game needs *one* tape. Not 128 packs, and not at the cost of
  reading a second vendor's terms.

If Chris wants that PA loop, the cheaper and more period-honest routes are
`make_cues.py` doing it procedurally, or one licensed track chosen on purpose.

## 3. The Ultimate Cyber November Game Dev Bundle — **a row 24 decision**

$30 for 75 items, MSRP $7,790. Leartes Studios. Ends about 2026-11-26. **Keys
redeem through `cosmos.leartesstudios.com`, not Humble's library, and expire
2027-11-10.**

This is the only one of the three with real matches. Of 75 items, the ones
that touch something this project has actually written down:

| Item | What it is for here |
| --- | --- |
| **SUV / Sedan / HatchBack / Classic Car**, driveable | **The convoy at dusk, and Sayers Auto Centre.** Chris's crossing ends in a room full of cars |
| **SFX Footsteps Crouch**, **SFX Character Movement** | The player's own foley. A crouching child is a sound the game does not have |
| **Abandoned / Post-Apocalyptic Hospital** | A named later chapter |
| **Post Apocalyptic Playground Equipment** ×3 | A mall play area, and §13's kids department |
| **Container Port Environment** | The railyard and the dock |
| **Post Apocalyptic Coffee Shop** | A food-court tenant |
| **Military Drone**, **Military Barriers** | The airport perimeter chapter |

That is roughly **eight of seventy-five**. The rest is Neo Tokyo, Demonic
Village, Merlin's Cave, medieval taverns, Roman statues and four packs of
guns — for a game with no combat.

### The licence, read rather than guessed at

This is where the Function Loops lesson applies: *"A licence you cannot
produce later is a licence you do not have."* Leartes publishes a **Digital
Asset Licensing Contract**, effective 2 June 2024. What it says:

- **Commercial use is granted.** *"This license grants permission for use in
  both commercial and non-commercial projects."*
- **No transfer of ownership, in the vendor's own words.** *"The purchase of a
  Digital Asset License shall not in any way be construed as a transfer of
  title and intellectual property rights."* The grant is *"non-exclusive,
  non-transferable, and limited."*
- **No redistribution as standalone files** — the licensee *"shall not make
  the Digital Assets available in a manner that allows third parties to
  download, extract, or access the assets as standalone files."* This is the
  same condition Function Loops imposed, and the project already satisfies it
  by cooking.
- **Licences survive delisting**, which is the durability `DECISIONS.md` row
  23 cares about.
- **The revenue cap: asked and answered, in our favour.** A *"$100,000"*
  ceiling appears on the Individual tier, and this file first recorded that
  whether the Humble tier carried it was not established. Codex found the line
  that settles it, and the FAQ confirms it verbatim: *"Humble Bundle's Bundles
  are licensed under Studio License."* Studio is *"annual revenues under 2
  million USD or has employees between 2 to 11."*
- **And for a one-time pack purchase the ceiling does not bite on growth at
  all.** The FAQ distinguishes subscriptions from pack purchases and says of
  the latter: *"No, you do not need to upgrade your pack license or make any
  additional payments once your revenue exceeds 2m."* The cap-on-success
  objection applied to subscriptions, not to a bundle.
- **The licence transfers with the game.** *"When you sell your game or your
  entire company to a new owner, you can also transfer the license rights
  associated with the game"*, on written notice to `info@leartesstudios.com`.
  This is the clause row 24's *reason* actually cares about, and it is the one
  CC0 and AI packs cannot offer at all.
- **No AI/ML training use**, either of their assets or — they claim — in making
  them: *"we do not use AI in our models and textures."* Relevant because
  AI-generated content is the one category row 24 rules out on both the letter
  and the reason.

### Why this is row 24's problem and not mine

Row 24 is *"everything we ship, we own"*, and `LANES.md:108` gives its reason:
*"On Steam this is a legal exposure, not a principle."*

**The letter and the reason point opposite ways here**, which has not happened
before in this project:

| | Letter of row 24 | Reason behind row 24 |
| --- | --- | --- |
| CC0 (Poly Haven) | fails — unowned, not ours | weak: no counterparty, no indemnity |
| AI-generated pack | fails — unownable | fails outright |
| **Leartes, paid** | **fails — vendor says so explicitly** | **strongest yet**: named company, dated contract, commercial grant, invoice, survives delisting, Studio tier to $2m, **and transfers with the game if it is ever sold** |

A paid commercial licence from a named vendor is the ordinary mitigation for
the exposure row 24 was written about. It still is not ownership. **That
tension is Chris's to resolve and nobody else's**, and it is worth resolving
as a row rather than per-purchase, because it will recur every time a bundle
appears.

### If he does buy it

`docs/ASSET-INTAKE.md` already has the process, and the Function Loops entry
has the shape. Before anything is referenced by the build: save the Leartes
contract text into `assets/props/licences/` with the date and URL, **and save
the FAQ's Humble/Studio line with it**, because that line is the only thing
tying the $30 Humble purchase to the Studio tier and a FAQ page can be edited;
record per-item rows for the items actually used, not all 75; and remember the
keys expire 2027-11-10, which is a real date on a third-party site rather than
a Humble library entitlement.

## What none of these do

The build has one room, 103 actors, 3.7% fixture coverage, and nothing in it
that opens a route. Rung 1 is unbuilt. The audio that exists was synthesised
from arithmetic and works.

**Assets are not what is blocking, and $65 of bundles will not change the
state of the game by one step.** That is not an argument against buying them —
a car for Sayers Auto at $30 is cheap and the crossing ends in a room full of
cars — but it is the honest frame. The thing between here and a playable
chapter is a generator, a door, and a reason to walk 396 m.

## Turned over

**Operand.** These three bundles against the precedent set on 2026-10-28, and
against spending the next session reading EULAs instead of building rung 1.

**Floor.** `MANIFEST.md`'s Function Loops entry is a floor and it already
declined a horror pack by name. Leartes' published contract is a floor. Row 24
is a floor whose letter and reason disagree here, which is a finding rather
than a verdict.

**Clock.** Bundle 2 expires about 2026-11-16, bundle 1 about 2026-11-18,
bundle 3 about 2026-11-26. The row 24 question has no deadline and should not
be decided by one.

**Falsifier.** The horror-score objection falls if the game turns out to want
non-diegetic music at all — which is testable the first time anyone plays the
crossing with and without. **The Leartes falsifier has now been run and it
passed**: it was the revenue cap, and the FAQ puts Humble bundles on the Studio
tier with no upgrade required on a pack purchase even past $2m. What is left to
falsify is narrower — the cap lives on a FAQ page rather than in the signed
contract, so the thing that would still kill it is the contract text itself
contradicting the FAQ, which is why both get saved.

**Upside down.** The cheapest audio in this project was free and is already
the best: six synthesised cue designs whose master is a script that diffs,
re-renders at any sample rate, and carries no licence at all because nobody
else made it. Every bundle on this page is an argument for `make_cues.py`.
