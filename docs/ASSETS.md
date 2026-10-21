# Asset policy — everything we ship, we own

Decided by Chris, 2026-10-20: *"we make our own assets and sounds then to
carry."*

This is not a new rule. The handoff already says it — keep original Blender
files, textures, audio masters and design docs outside Roblox from day one, and
never rely on Marketplace or Creator Store assets unless the licence expressly
supports the future Steam build. Today was the first time it was broken, which
is why it is now written somewhere enforceable.

## The rule

**Every asset that ships traces to a source file in `assets/` that we own.**

A `rbxassetid://` is a *pointer*, not an asset. It is fine — necessary, even —
but only when it points at something we uploaded from our own master. An ID
that points at somebody else's upload cannot port to Steam, by definition: the
file lives on Roblox's servers and the licence is theirs.

## What that means in practice

| Kind | Master lives in | Shipped as |
| --- | --- | --- |
| Audio | `assets/audio/*.wav` (or .flac) | Uploaded to Roblox, referenced by id |
| Models | `assets/models/*.blend` | Exported copy imported into Roblox |
| Textures | `assets/textures/` at full resolution | Downscaled copy uploaded |
| Concept art | `assets/<area>/*.png` | Reference only, never shipped |
| UI | `assets/ui/` | Rebuilt in Roblox UI |

Procedural rigs built in Luau — the Sweeper, Glimmer and Window Walker — are
already portable, because the source *is* the code. Keep it that way where it
is cheap to.

## The manifest

`assets/MANIFEST.md` lists every `rbxassetid://` the project references, what
it is, and which source file it came from. If an id is in the code and not in
the manifest, it is an unlicensed dependency and must not ship.

Worth making enforceable: a wiring check that every `rbxassetid://` found in
`src/` appears in the manifest. That belongs to whoever owns `tests/`, which is
the gameplay session, not this one.

## Placeholders

Placeholders are allowed. **Silent placeholders are not.** Mark them in code
and list them in the manifest under "Owed", so nothing borrowed can reach a
build by being forgotten.

## Audio has a second requirement

The handoff makes enemy audio an accessibility rule, not polish: *telegraph
every enemy with sound before sight.* That means each enemy needs a
**distinguishable** cue — a player who cannot see the screen clearly must be
able to tell a Sweeper swarm from a waking Window Walker by ear alone.

Precisely where this stands, corrected 2026-10-20 after the gameplay session
pushed back on an overstatement of mine. The three rigs are *not* playing an
identical cue: they are pitched apart at 1.0, 1.35 and 0.72, now declared as a
named `CUE_PITCH` per rig rather than left incidental, so they are partly
tellable apart already.

But it is one sample wearing three hats, and pitch alone will not carry five
enemies. The Glimmer is the clearest failure: its cue is a single hit, when
what it has to convey is a **2-second window closing**. The player needs to
hear how long they have to go dark, not guess.

So the audio brief is at least:

- Sweeper — many small things moving, low and scuttling
- Stocker — slow, heavy, single source
- Glimmer — a scan tone with a clear 2-second shape, so the window to go dark
  is audible rather than guessed
- Window Walker — stillness breaking. The sound of something that was furniture
  a moment ago
- Warden — unmistakable, and audible from much further than the rest

## Why this matters more here than on a normal project

The whole Roblox-first strategy rests on the portable layer staying portable.
Design, art sources, audio masters, balance data and documents are what survive
the engine change; Studio maps, Luau and Roblox networking get rebuilt anyway.
Borrowed assets are the one category that quietly fails to transfer, and the
cost of unpicking them scales with how long nobody noticed.
