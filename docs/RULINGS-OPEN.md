# Open rulings — everything waiting on Chris, in one place

Status: **ballot, opened 2026-11-11.** Chris: *"Whatever questions I need to
answer I will."*

**This is not another argument about the pivot.** Every "why" here is a
pointer, not a paragraph. Three documents already carry the reasoning and
`AUDIT-THE-PIVOT-2026-11.md` §2.5 says the right thing to do with a fourth is
not write it. What has been missing is not analysis — it is a single page
where the questions sit together, so answering them is an afternoon rather
than an archaeology.

**How to use it.** Answer in any order; where a one-word answer works, the
option is bolded. Say "all of A–C" and mean it. Nothing here needs a
paragraph from you. Anything you decline to rule stays open and stays
visible, which is the point.

**Who acts after each answer** is named, so you can see what an answer buys.
*Me* means Claude in the `docs/**` and `Source/LLK/**` lanes; **no agent may
edit a locked row**, which is why so much of this is here at all.

---

## A. The gate — one sentence, and it unblocks the most

| # | The question | Options | What it buys |
| --- | --- | --- | --- |
| **RA1** | **Row 3's playtest question has `bank` in it, and the pivot deletes banking.** The gate that has held scope for 22 days cannot be answered as written. Rows 79 and `ROADMAP.md` M4 quote it verbatim | **Restate** as *"do testers kill their lanterns, make camp, and argue for one more shop?"* · restate differently · rule that `bank` already means *secure what you came for*, in which case nothing needs doing | The gate becomes openable. M4 and row 79's instrumentation get a question they can measure. **Me**, same day, in `ROADMAP.md` |

---

## B. The pivot rows — nothing below them can be judged

Rows **84–91** are `PROPOSED` in `DECISIONS.md`. They are the subset of the
pivot that binds mechanism or scope. Full reasoning per row is in that file
and in `AUDIT-THE-PIVOT-2026-11.md`.

| # | Row | The question | Options |
| --- | --- | --- | --- |
| **RB1** | **84** | Version one is a forward-moving campaign; the night loop lives inside it | **Lock** · amend · reject |
| **RB2** | **85** | No extraction. The unit is a night and a camp; the frontier is the progression | **Lock** · amend · reject |
| **RB3** | **86** | Attacking a machine is fatal and alerts every machine | **Lock** · amend · reject |
| **RB4** | **87** | Chapter one is the crossing, Boswell's to Sayers | **Lock** · amend · reject |
| **RB5** | **88** | Motion is one-way within a chapter; returns happen between chapters | **Lock** · amend · reject |
| **RB6** | **89** | No co-op. One player, one lantern | **Lock** · amend · reject |
| **RB7** | **90** | The player is 12 or 13; eye height is measured from a built capsule | **Lock** · amend · reject |
| **RB8** | **91** | The season is the clock on home; no countdown on the family | **Lock** · amend · reject |

**RB9 — or rule the block.** *"Lock 84–91"* is a legitimate answer and the
fastest one. Each still gets its own line in the log.

**What section B unblocks.** RB2 releases the 3,169-line repoint of extraction to
*making camp* (**me**, `Source/LLK`). RB4 releases the next large map pass,
with P2's concourse width (**Codex**). RB6 releases the roadmap and build
cleanup in **P5**. RB8 releases **G3**, the family's elapsed days (**Grok**).

---

## C. The strikes and amendments that follow section B

These are consequences, not new decisions — but four of them are LOCKED rows,
and **no agent may touch a locked row.**

| # | The question | Options | Follows from |
| --- | --- | --- | --- |
| **RC1** | **Rows 36, 50, 51, 52 — strike all four.** Co-op's existence, its no-gating rule, mixed crews, and the NPC lantern ban | **Strike** · strike with 52's *reason* kept as a note (it is why the roster can never perturb the meter) · keep one of them | RB6 / row 89 |
| **RC2** | **Row 28 — amend one clause.** *"Version one is"* one sealed store. Rows 34 and 38 already permit the crossing, so this clause alone is in the way | **Amend** into the build order row 34 already states · retire · keep | RB4 / row 87 |
| **RC3** | **Row 43 — amend.** *"The department store is the tutorial"* survives; its curriculum list teaches extraction | **Amend the list, keep the rule** · rewrite · keep | RB2 / row 85 |
| **RC4** | **Rows 30 and 31 — retire.** Combat. Both only `PROPOSED`, so this is free | **Retire** · keep | RB3 / row 86 |

---

## D. Two locked rows that have been wrong for a while, pivot or no pivot

| # | The question | Options | Open since |
| --- | --- | --- | --- |
| **RD1** | **Row 6 contradicts rows 72 and 73**, both LOCKED. An agent reading top-down finds the dead rule sixth, under "The core rules". `DRIFT.md` §A calls it *"the serious one"* | **Strike through row 6 with a pointer to 73**, as row 8 was handled on 2026-10-26 · rule that row 6 still stands and 72/73 are wrong | **2026-11-01**, eleven days |
| **RD2** | **Rows 68 and 70 are marked `PROPOSED`**, but LOCKED row 73 says it closed them out. Nobody can tell from the file whether they were ratified | **Set both to LOCKED** · name what is still open in each | `DRIFT.md` §A2 |

---

## E. Numbers that are yours under row 55, and are now extraction-only

| # | The question | Options |
| --- | --- | --- |
| **RE1** | **`run.earlyExitSeconds` (360) and `run.hardCeilingSeconds` (780)** in `data/zone-pressure.json`. The ceiling's own note derives it as *720 + 30 (freight lift) + 8 (siren)* — every term is extraction | **Retune to a night's shape** · delete both and let a night end when the player makes camp · leave them inert and annotated, as the lantern block already is |
| **RE2** | **The lantern block in the same file** (`perLanternBonus`, `curve`, `diminishingDecay`, `maxMultiplier`). Already retired from runtime by row 72; with one lantern all four go inert | **Leave annotated** · delete · keep live for a later mode |

---

## F. Open design that is genuinely yours, not mine

| # | The question | Options |
| --- | --- | --- |
| **RF1** | **What losing a camp costs.** The only piece of row 85 that is a real design question rather than a rename. `ELLKExtractionPhase::Failed` carries *"Row 8: the night keeps everything"* and there is no haul to keep | **The frontier recedes to the last camp, dropped salvage left recoverable** (my recommendation; `GAME-THESIS.md` red-teams full loss by name) · full loss · no loss, only time |
| **RF2** | **The rescue time horizon.** Codex is right that containment says nothing about what a body survives | **Days to a few weeks**, seasons after · a long captivity, which then has to show its food, water and sanitation on screen |
| **RF3** | **What the first game's final playable action changes.** Family resolution is the ending; the mechanism is open. Do not assert the airport before it is authored | Yours to name, or **defer until Grok's outline lands under G3** |

---

## S. Standing items, carried across three documents

| # | The question | Options | Note |
| --- | --- | --- | --- |
| **RS1** | **Lamplighters or Lightkeepers** | **Pick one** | Fifth day, three documents, two spellings. Costs nothing to settle and gets quoted wrong until you do |
| **RS2** | **Row 47 against the Shepherd Project** | **Confirm 47 holds** · reopen | All three agents now agree it holds; Codex withdrew his hedge 2026-11-10. This is a confirmation, not a debate |
| **RS3** | **Row 24 for paid licences.** *"Everything we ship, we own"* — a bought asset is owned in letter, and the reason behind the row was something else | **Amend to say what the reason protects** · keep as written · narrow it to generated or pointer assets | `FINDINGS-BUNDLE-TRIAGE-2026-11.md` row 24 table |

---

## What I will do without being asked, once section B is ruled

1. The repoint of extraction to *making camp*, `Source/LLK`, on RB2.
2. The supersession text already drafted for `PITCH.md`, `GAME-THESIS.md` and
   `ROADMAP.md` M3/M4 — `ADJUDICATION-CODEX-PRODUCT-PIVOT-2026-11.md` §3.
3. The cost-of-light surface, so the lantern stops being redundant — the one
   thing in this project I believe is wrong *right now* in the playable build,
   and the only item on this page I would do before any ruling if you said go.

## Keeping this page honest

An answered line is struck through with the date and the row it became, not
deleted — same rule as `DRIFT.md`. A line nobody has answered after a week
gets no reminder and no fourth document; it just stays visible, which is the
only pressure a page like this is allowed to apply.
