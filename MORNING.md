# Morning, Chris

Written 2026-10-20, overnight. Read this first; it's the whole state in one page.

## Do this first (5 minutes)

1. **Add Rojo to PATH** so both agents can run it:
   `C:\Users\chris\AppData\Local\Microsoft\WinGet\Packages\Rojo.Rojo_Microsoft.Winget.Source_8wekyb3d8bbwe`
2. **Get typechecking**, because nothing here has been compiled:
   ```bash
   rokit add JohnnyMorganz/luau-lsp && rokit install && luau-lsp analyze src
   ```
3. **Connect Rojo in Studio** (plugin panel → Connect), then press Play.
4. **Watch the Output window.** Startup prints tagged instance counts. If
   `Loot`, `Extraction`, or `RelayBank` is 0, the map isn't tagged and a run
   cannot be completed — that's the first thing to fix, and it's Codex's lane.

## What works now, in theory

Four commits, 33 files, `rojo build` green, balance tests 48/48.

- **Storm Charge** rises always; lanterns accelerate it 2.5× each, capped at 5×.
  Thresholds at 8/12/16 minutes dark, 3.2/4.8/6.4 lit.
- **Lantern charge** drains — 150 seconds of light per full charge, then it cuts
  out. This is new: charge previously never depleted, which made the solo Anchor
  Revive free despite the handoff pricing it at "a full lantern charge."
- **Knockdown** instead of death, enforced by intercepting lethal damage. Downed
  players are movement-locked so teammates must come to them.
- **Revive** is a 4-second proximity hold on a prompt over the downed player.
  Walking away cancels it. Solo players press **R** for the Anchor Revive, which
  costs a full charge plus a permanent Storm Charge spike.
- **Sweepers** chase (lit players from 38 studs, dark from 12) and knock you down
  after 3 contacts, which decay if you break away. One is survivable; four are not.
- **Loot** via prompts: **E** to take. Relic prompts don't appear unless a lantern
  is lit. Crates take a 0.6s lift and stop you fighting.
- **Extraction** via prompt. Loading dock is instant; freight lift makes you wait
  20 seconds and cancels if you step off the pad.
- **HUD**: storm meter with threshold ticks, a separate amber lantern-charge bar,
  and a status line that leads with the most urgent fact.

## The honest caveat

**None of this has ever executed.** There is no Luau toolchain on the machine, and
`rojo build` embeds scripts without parsing them. I verified that the project
builds, that every new script lands in the generated place, and that the balance
numbers are self-consistent. I could not verify that a single line runs. Expect
to spend the first 20 minutes on runtime errors, not on balance.

The most likely failure points, in order: the `data/*.json` files loading as Rojo
JSON modules (needs Rojo 7+, you have 7.7.1, should be fine); `Config.luau`'s
`WaitForChild("lantern")` if the JSON module name differs; and
`InteractionService` requiring `LootService` at init order.

## The number to argue with

`maxMultiplier: 5.0` in `data/storm-charge.json`. A fully-lit four-person crew
hits the Warden in about 3.2 minutes. I picked that cap; the handoff only gave a
one-lantern figure. It's the most consequential untested value in the project.

## Your calls waiting

- `rechargeAtRelay` in `data/lantern.json` is **off**. Turning it on would let
  banking refill your lantern, making the relay do double duty and strengthening
  the bank-or-push decision. It also softens the charge economy. Yours to decide.
- `bleedOutSeconds: 0` — downed players wait indefinitely. The forgiving tone
  argues for that; set a timer only if playtests show downs being ignored.
- Branch is `master`, not `main`. Rename if you care.
- The loot tier→feeds mapping in `data/loot.json` is still my guess from the
  collapsed table in the handoff PDF. Worth 60 seconds of your eyes.

## Lane status

Codex fixed the `GalleriaDetails.rbxmx` build blocker and owns the map, tags, and
Studio integration. I answered its full systems audit in `NOTES-FOR-CODEX.md` —
all four findings are addressed. The ownership split held; neither of us wrote
into the other's files.

One process note: no agent ran while you slept. Everything above was done in a
single stretch after your last message, then both agents stopped.
