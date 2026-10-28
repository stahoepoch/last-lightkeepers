# Function Loops — licence record

**Retrieved** 2026-10-28 from <https://www.functionloops.com/license.html>
(reached via `functionloops.com/license`, which redirects there), titled
"Terms & Conditions (License)". Footer: "Function Loops LTD 2011 - 2026".

**What this file is.** A compliance record: the obligations, stated precisely,
with short quotes where the exact wording carries the legal weight. **It is not
the licence and does not replace it.** Re-fetch the URL above for the
authoritative text, and see "Still to do" at the bottom — one thing here needs
Chris, not me.

**What it covers.** The **Ultimate SFX for Games** bundle bought from Humble
Bundle on 2026-10-28 ($25 tier, 38 packs). Every pack in that bundle lists
Function Loops as developer, so this is the governing grant for all of them.

## What we are allowed to do

**Commercial use in a game is explicitly permitted**, which was the open
question. Clause 1 grants sounds that are "100% royalty free, once purchased"
for use in commercial productions. Clause 4 permits editing and re-working
them, and names the medium directly — the enumerated uses include
"video games".

That is the clearance the build needed. Shipping these sounds inside Last
Lightkeepers, sold on Steam, is within the grant. No royalty, no revenue share,
and the page states no attribution requirement anywhere.

## What we are not allowed to do

**1. Do not redistribute the sounds as sounds.** Clause 2 forbids reselling,
re-licensing, trading, uploading, copying, sharing and distributing them;
ownership stays with Function Loops, and clause 3 is explicit that they are
"licensed to you, not sold."

This is the ordinary sample-library bargain — you may ship a *work* containing
the sounds, not the *library*. It has one real consequence for how we package:

> **Ship cooked audio, never loose `.wav` files in the pak.** Unreal cooks
> audio into compressed in-engine assets by default, which is the correct
> posture. A build that puts 180 extractable, named `.wav` files on disk is
> much closer to "distributing the sounds" than one that bakes them into a
> derivative work, and the difference is free — it is the default behaviour.
> Do not add a loose-file or uncooked-audio packaging step for convenience.

**2. No AI use whatsoever, and this is the clause that matters most to this
project.** The page carries a separate "AI Training & Machine Learning
Prohibition": the content may not be used for AI training, machine learning,
model fine-tuning, dataset creation, voice cloning, music generation systems,
or any other generative AI application, without prior written permission.

**This project is built by AI agents, so the boundary has to be explicit.**
Nothing we currently do breaches it, and one thing we might casually try would:

| | Status |
| --- | --- |
| Agents reading, cataloguing and placing these files | Fine. That is use, not training |
| `assets/audio/make_cues.py` | Fine, and unaffected. It is procedural synthesis — `wave`, `math`, `struct` — with no model and no training data |
| Feeding a pack to an AI tool to generate variations, extend a loop, or make a "similar" cue | **Prohibited.** Needs prior written permission |
| Using them as reference material to train or fine-tune anything | **Prohibited** |

So the rule for this repo: **these wavs are raw material for humans and
scripts, never input to a model.** If we want more variants than the library
has, we synthesise them or record them.

**3. The licence is to one licensee.** Clause 2's "licensed only to you" is
singular. Fine while Chris is the buyer and the team is one person plus agents.
If a second human ever needs the raw library on their own machine, that is a
second licence, not a copy.

## Governing law

Clause 5 invokes "the Law of Israel and other countries". Noted, not analysed.

## Still to do — Chris's, not mine

**Confirm the Humble receipt does not carry different terms.** The bundle's
Humble page carried no licence text and the archives contain no licence file,
so the Function Loops page above is the only grant I could find. Humble
occasionally attaches its own terms to a bundle. Worth thirty seconds in the
purchase receipt, and if it says anything different, that supersedes this file.

**Keep the authoritative copy.** Print the URL above to PDF into this folder,
so the terms we relied on are preserved as they stood today. A licence you
cannot produce later is a licence you do not have — and a web page can change
without telling anyone.
