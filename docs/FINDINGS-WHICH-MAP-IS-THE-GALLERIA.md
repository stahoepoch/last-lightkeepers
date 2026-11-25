# Which map is the Galleria, and the storey its own glass was asking for

**2026-11-25.** Chris, asked which of the two maps that model this mall is the
real one:

> *"The Galleria"*

---

## 1. The two maps, measured

| | `LLK_Boswells_OpeningPass` concourse | `LLK_Galleria_Shell` |
| --- | --- | --- |
| actors | 170 | **578** |
| length modelled | 48 m | **680 m** |
| long axis | **Y** | **X** |
| lights | 16 point, 6 spot | **none at all** |
| Boswell's | the playable store, 87 × 54 m, two floors | a 45 × 48 m box, three floors |
| dressing | shopfronts, neon fascia, kiosks, planters, signage, narrative lines | untextured shells |
| escalators, food court, centre court, clock | no | yes |
| connected to the store | yes, through `MALL_GLASS_ENTRANCE` | no |

Two facts settle it before taste gets involved.

**They disagree about which way the mall runs.** The shell's long axis is X;
the playable concourse's is Y. One is rotated ninety degrees from the other,
about different origins.

**They disagree about Boswell's.** The shell's anchor is
`GALLERIA_SHELL_BOSWELLS_RETAIL_L1/L2/OFFICE_L3` — a 4500 × 4800 box with
three floor plates at z 11, 671 and 1331, centred at (−17250, −4100). The
store Chris has been walking is 8700 × 5400, two floors, centred at the
origin. The shell's Boswell's is **roughly half the area of the real one**,
with a storey the real one does not have, somewhere else.

So the shell cannot be imported into the playable map, and the playable map
cannot be moved into the shell's frame without abandoning the store, its
lighting circuits, its departments and everything built on top of them.

## 2. The decision

**`LLK_Boswells_OpeningPass` is the Galleria. `LLK_Galleria_Shell` is a
plan.**

It is the map Chris plays, it is where `ROADMAP.md`'s Boswell's-to-Sayers
target lives, and it is the only one of the two with a single light in it.
Importing 575 unlit actors into a level whose whole lighting discipline is
circuit tags would recreate, at a stroke, the exact defect
`docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md` spent a day removing.

**`LLK_Galleria_Shell.umap` is another writer's uncommitted work and nothing
here opens, modifies or deletes it.** `docs/LANES.md` gives each binary asset
one active writer, and it is not mine. It is also still untracked — 935 KB of
somebody's afternoon that a `git clean` would take. **That is Chris's to
resolve with Codex, not mine.**

**This is a recommendation, not a ratification.** `docs/DECISIONS.md` rows are
Chris's. If he wants the shell to be canon instead, the cost is named above:
the store gets rebuilt at half size with three floors somewhere else.

## 3. What the shell is for, and the first thing it told us

A plan's value is what it knows that the level does not, and the first such
thing is structural: **the Galleria has two levels.** The shell has
`L2_BALCONY_*`, escalators and a centre court well. The playable concourse had
one floor.

That was not only the shell's opinion. The playable map's own geometry had
been saying it for weeks:

- **`MALL_GLASS_ENTRANCE` is 1468 cm tall with a horizontal mullion at 660.**
  A two-storey mall screen, divided at the floor level of a storey nobody
  built.
- And `docs/FINDINGS-THE-STORE-HAD-NO-OUTSIDE.md` had just put an upper mall
  entrance in the store's south wall at deck level, because Chris said the
  mall entrance has glass "on both top and bottom". **It opened onto nothing** —
  a sheet of glass over a 650 cm drop.

## 4. What was built

`Tools/build_galleria_upper_level.py`, 152 actors:

- **Balcony decks** each side at z 650, over the lower shop bays, with the
  middle left open as a well so the two levels can see each other. 830 cm of
  headroom under the glass roof.
- **A crossing deck** at the near end, which is what the store's upper doorway
  now opens onto. The tool asserts that specifically, by tracing down from the
  doorway and failing unless it lands on this level.
- **A feature stair** from the lower concourse up to the crossing, in the
  centre of the court in front of the mall screen.
- **Six shuttered upper tenancies** — frontage only. An empty upper level with
  open units would be a lie about what is behind them.
- **Twelve egress lights**, all tagged `LLK_Emergency`, at 2.8 cd, which is
  the top of this project's own emergency band.
- **A lit edge band** along each balcony's well edge at `Emissive = 0.40`, the
  value `docs/FINDINGS-THE-FITTINGS-WERE-NOT-THE-PROBLEM.md` settled on.

## 5. Two defects found on the way

**The concourse roof had no collision.** A visibility trace straight up the
middle reported `OPEN SKY` for all 48 m, through ten glass panels and ten
ribs that are plainly there in every render. They render and they occlude;
nothing could measure them. That is the kids-ceiling defect from
`docs/FINDINGS-FIFTY-LIGHTS-ON-NO-CIRCUIT.md` again — *a ceiling nobody can
measure is a ceiling nobody can prove is there* — and it had gone unnoticed
because it only started to matter when there was a sky above it. **20 roof
pieces** were given collision.

**The outer walls stop at 1200 and the roof is at 1490.** A 290 cm strip of
open sky down both sides of the mall, invisible while the sky was black,
infilled now.

And one of mine: **the stair first rose up the middle of the well** and topped
out 650 cm in the air with no floor near it. Every assert in the file was
about the balcony and none about the stair. There is one now, and it traces
the landing.

## What is measured and deliberately not fixed

**The upper level is a shell too.** Six shuttered units, no interiors, no
dressing, nothing on the floor. It is a place to walk and look down from, not
a place with anything in it.

**It has no second way down.** One stair, at one end. For a stealth floor that
is a dead end with a single exit, and whether that is a trap or a defect is a
design question nobody has answered.

**The far end is a bulkhead.** The mall continues south in the shell's plan
and in the dock chapter's corridor; on the upper level it stops at a wall.

**`LLK_Galleria_Shell.umap` is still untracked**, and the sight line south
from the mall entrance is still **193 m** of unobstructed corridor, which is
its own problem for a game about not being seen.

**Nobody has played it.** Every number here is a measurement against a saved
map and every picture is a render.
