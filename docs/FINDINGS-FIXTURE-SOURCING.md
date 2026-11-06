# Findings — sourcing the five fixtures from the open web

**Written 2026-11-06**, adjudicating a research pass Chris relayed, against
`docs/reference/boswells/BoswellsSalesFloor_Fixtures.csv` (the block-in),
the sightline table in `PROPOSAL-BOSWELLS-RETAIL-FLOOR.md`, and
`DECISIONS.md` row 24.

Two of the research pass's judgements invert on the project's own numbers, and
its one real worry — *"pages say public domain while leaving the maker
unproven"* — turns out to have a named mechanism, which changes what the
licence is worth rather than confirming or clearing it.

The five fixtures, as blocked in, are the authority for every size below:

| Fixture | Blocked at |
| --- | --- |
| Clothing gondola island | 110 wide × 300 long × **140 high** |
| Glass case | 140 long × 60 deep × **100 high** |
| Shoe table | 200 long × 80 deep × **45 high** |
| Round rack | **90 across**, 160 high |
| Checkout | 340 long × 80 deep × **90 high** |

## 1. The "closest free match" is the one to reject

The research ranks Innerscene's **round clothing rack** as the closest free
match to one of the five shapes. It is 146.5 × 128.5 × 155 cm against a blocked
rack of **90 across** and 160 high — **63% wider** than blocked, and a
146.5 × 128.5 footprint is not round, which means it is an oval or an armed
four-way rather than the shape named. Dropped on the blocked centres
(`RACK_0` at x −1025…−935, y −525…−435) it narrows the gap between each rack
and the clothing island from 180 cm to **148 cm**, which is a patrol-fit
question for the Sweeper rather than a dressing one.

## 2. The piece it lists in passing is the find

Innerscene's **jewelry case** is 139 × 66 × 101 cm. `GLASS_0`–`GLASS_3` are
140 long × 60 deep × 100 high. That is **within 6 cm on every axis** — a
drop-in for the west wall run, and the strongest dimensional match in the whole
search. The research does not rank it.

## 3. Both gondolas fail a decision already made

The Innerscene single-sided gondola is **185 cm** tall and the paid Fab shelf
is 1.85 m. A keeper's eye is authored at **152 cm**
(`LLKLanternFeel.cpp:7`), and the proposal's sightline table runs out at
183 = *blocked*, with 152 named as the one height to avoid. The block-in is
140 precisely so the floor reads: a keeper sees across by 12 cm, so heads and
lantern glow carry over the fixtures while bodies do not.

At 185 nobody sees anything, standing or crouched — and crouched is worse.
Nothing in `Source/` sets `CrouchedHalfHeight`, so it is UE's default 40, which
puts a crouched eye at 104 cm and a fully-pitched lantern reaching 146.

So those two are not cheaper versions of the blocked gondola. They are a
different floor, and taking one would decide a gameplay question that
`PROPOSAL-BOSWELLS-RETAIL-FLOOR.md` deliberately leaves to Chris. The research's
closing line — that a low gondola you can see across is still ours to make — is
right, and this is the reason rather than a consolation.

## 4. Provenance: a curated chain of title versus a checkbox

This is the part the research could not weigh, because it read the Innerscene
terms page and reported the conclusion rather than the mechanism. Both pages
were fetched and quoted below.

**Poly Haven** states: *"All assets on this site are the original work of Poly
Haven staff, or artists who willingly and directly donate/sell their work to
Poly Haven."* Every asset carries a named author in its metadata —
`Television_01` is `{"Gabriel Radić": "All"}` from the site's own API. That is a
curated chain of title with a person attached.

**Innerscene's Free Parts Library** is a **contribution** library: *"3D models,
CAD parts and 2D drawings contributed by people using the Innerscene 3D
Viewer, plus parts we add from public-domain collections."* Its CC0 rests on a
contributor ticking a box — *"You created this file, or you otherwise hold every
right needed to release it"* — and the terms add *"Parts are provided as-is,
with no warranty of any kind"*. Three further clauses matter here:

- **Files are kept 1 to 5 years depending on size**, then deleted. The research
  reported "available until 2031"; the terms say 1–5 years. Either way the file
  must be kept locally, not linked.
- Some parts are public domain **as a work of the US federal government
  (17 U.S.C. §105)**, which the page says *"applies in the United States and may
  differ elsewhere"* — a live caveat for a global Steam release.
- *"CC0 does not grant rights in trademarks or patents that a part may
  depict"*, which is the clause to remember for a cash-wrap or any fixture
  modelled from a real product.

**Row 24 is what decides it:** *"Everything we ship, we own"*, recorded with
Chris's own words, *"we make our own assets and sounds then to carry"*, and
`LANES.md:108` adds that on Steam this is a legal exposure rather than a
principle. CC0 does not make us an owner — it makes the work unowned. So every
candidate here is at best **temporary**, in the same bucket as the Game
Animation Sample row in `assets/MANIFEST.md`, which is marked *"temporary /
review before shipping"*. The difference between Poly Haven and Innerscene is
not whether they can ship — neither can, as the rule stands — but how much
confidence a greybox placeholder carries while it is in the project.

## 5. The AI-generated sets, and generating our own

The 3dassets.dev pack's 49 models are marked AI-generated, **model name Claude
Opus 5**, which is the model that wrote this file. Declining it is right, and
the reason matters: decline on **provenance**, not on licence. If the output is
not ownable, the CC0 dedication conveys nothing, so it is neither reassuring
nor alarming — a no-op. Row 24 is the clause it fails.

The same answer covers generating fixtures ourselves with an AI generator,
including Innerscene's. It would be the same category as the pack we just
declined, with better records rather than better standing: still nothing we
own, still row 24. And separately, the free credits in hand are Anthropic API
credits, which do not buy 3D generation at all — Anthropic models do not output
meshes, and Innerscene's generator is their own service on their own terms. The
premise does not connect before the licence question even arrives.

The Fab paid shelf ($11.99, Glamora Studio) is the wrong height anyway, and its
*"use with AI is not allowed"* clause reads more broadly than no-AI-training in
a repository where every writer is an agent. Not a licence opinion — just not
worth having to interpret a clause in our own favour for $11.99.

## 6. What was taken

**Poly Haven `Television_01`** only, by Gabriel Radić, CC0, measured 60.0 ×
47.1 × 45.7 cm from the site's own dimensions field — a real CRT for the teens
area, and the only candidate in the search with a named author behind a curated
licence.

Downloaded outside the repository, per `ASSET-INTAKE.md`'s intake convention,
to `C:/Users/chris/Documents/LLK_AssetIntake_5_8/_downloads/polyhaven-television-01/`:
the 2k FBX plus diffuse, DirectX normal and packed ARM at 2k JPG, 1.5 MB in
total. Licence record and checksums are in
`assets/props/licences/polyhaven-television-01.md`.

**Nothing was imported.** `Content/` is not mine, the editor's MCP bridge is
refusing connections, and `ASSET-INTAKE.md` step 4 requires Unreal's
**Asset Actions → Migrate** rather than a file copy. No row was added to
`assets/MANIFEST.md`: step 2 says a row is owed only *"when game content starts
referencing the pack"*, asset provenance is Chris's lane per `LANES.md:108`,
and the file is currently dirty with another writer's work.

**One trap for whoever imports it.** The FBX's own material references point at
`nor_gl` — the OpenGL normal — plus separate roughness and metallic EXRs. What
was downloaded is `nor_dx`, which is the one Unreal wants. Pointing UE at the
GL normal inverts the green channel, which does not fail an import and does not
look obviously wrong on a dark prop; it just lights backwards.

## 7. Still ours to make

A 1992 clothing rail, a low gondola at 140 that a keeper can see across, a
four-way, and the Boswell's door alarm. The research found no clean unbranded
1990s EAS pedestal, which matches `EAS_W`/`EAS_E` still being 30 × 40 × 150 cm
boxes in the block-in.
