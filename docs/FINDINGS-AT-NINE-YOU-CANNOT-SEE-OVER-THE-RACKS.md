# At nine you cannot see over the racks

**2026-11-23.** Chris, about the Boswell's ground floor:

> *"It should be pretty big. You're a nine-year-old kid, so getting lost
> should be pretty easy. You're tiny."*

That reverses Codex's narrowing of decision row 90 from Chris's own *"the
player is actually a kid, young adulting. Like 9-13"* down to 12 or 13. Nine
is the low end, stated directly, and row 90 is PROPOSED rather than locked.

It is not a flavour note. It is the difference between a floor you can read
and a floor you cannot, and the merchandise to prove it is already in the
map.

## The measurement

A nine-year-old is about 134 cm; eye height runs about 0.93 of stature, so
**124 cm**. Twelve is about 153 cm, so 142 cm. Everything below was measured
off the actors in `LLK_Boswells_OpeningPass` — bounds, not catalogue numbers.

| Fixture family | n | median top | tallest | over a 9-year-old's eye | over a 12-year-old's |
| --- | --- | --- | --- | --- | --- |
| `BSW_APPAREL_LOWER_*` | 36 | 164 cm | 164 cm | **100%** | 75% |
| `BSW_LEFTDAY_*` dressing | 12 | 169 cm | 171 cm | **100%** | 100% |
| Juniors fixtures | 116 | 137 cm | 145 cm | **93%** | 17% |
| Department case goods | 12 | 124 cm | 124 cm | 0% | 0% |
| `BSW_COURT_ISLAND_*` glass cases | 16 | **96 cm** | 96 cm | 0% | 0% |

**At twelve you can see across most of juniors and read where you are. At
nine you cannot see over anything on this floor** except the counters — and
the counters are all in two places, the front hall by the mall entrance and
the islands ringing the escalator well, where they were put on the same day
to answer *"what are we doing with that giant empty space in front of the
escalators?"*

That turns out to matter more than it was meant to. The glass cases are 96 cm
and the apparel is 164 cm, so **the only two parts of the ground floor a
nine-year-old can see across are the way in and the way up.** Everything
between them is over their head. Nobody designed that; it fell out of
measuring real fixture heights against a real child, and it is worth keeping.

## Why that is worth more than a fog volume

Getting lost stops being something to script. It becomes a property of the
geometry, and it is already built:

- **The department signs are useless to you.** They hang over the racks, at
  adult reading height, and you are under the rail line. A nine-year-old
  navigates by what is at 124 cm, which is merchandise and aisle mouths.
- **The 180 cm walkway is a corridor, not an aisle.** Two 164 cm rails either
  side of a 180 cm gap is a trench. You cannot cut the corner, see round it,
  or check whether the next run is a dead end without walking it.
- **The Sweeper gets a free advantage nobody authored.** It is 176 cm. It can
  see over everything you cannot. Whether its perception already assumes that
  has not been checked.
- **The mirrored coffers become the only wayfinding on the floor.** 532 m² of
  mirror at 5.14 m is the one thing a short player can see from anywhere, and
  it shows them the aisle behind rather than the one ahead. That was built as
  a horror instrument on 2026-11-23; at nine it is also the map.

## What this does not settle

The walkway. It is three child steps, and a step is about 0.41 of stature: at
145 cm that gave 60 cm and **180**, at 134 cm it gives 55 and **165**. The
rule is Chris's and the age under it moved, so the number is his to re-say
rather than mine to quietly re-derive — decision row 81 licenses deriving,
not substituting.

Unmeasured: the keeper capsule in the build is not nine yet, nobody has
walked any of this, and the Sweeper's sightlines have not been re-checked
against a player who cannot see over a clothing rail.


---

## Amended 2026-11-23: the racks were not there

This document argued the floor's stealth from the measured height of its
fixtures, and the heights are right. **What it did not check is whether any of
those fixtures existed for the machine that hunts on the floor**, and for all
but thirty-six of them, they did not: the dressing is placed `NO_COLLISION` by
`dress_boswells_juniors.py` and the others, deliberately and correctly, and
nothing blocked `ECC_Visibility`. A child hiding behind a rail of coats was
standing in an open hall.

This document's own sentence — *"the Sweeper at 176 cm sees over everything the
player cannot"* — was also untrue in code twice over. The Sweeper traced sight
from `GetActorLocation()`, its capsule centre at 88 cm, and on the floor as it
stood an eye at head height was worth **6%** more visible floor, because there
was nothing to see over.

Both are fixed, and the fixes turn out to depend on each other: making the
racks block sight is what made the eye height worth **352%**.
`docs/FINDINGS-THE-RACKS-WERE-NOT-THERE.md` is the measurement and the repair.
