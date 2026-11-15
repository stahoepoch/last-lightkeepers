# The fixture collision result does not reproduce, and my own fix's proof was a coin flip

**Status: open. 2026-11-15.** This corrects `962d506`, which is mine and which
claims more than it can support.

## What `962d506` says

> Both now block at exactly the heights they were authored to block at,
> proven by trace rather than by counting box elements.

That was true in the process that applied the fix, and in one later process.
It is **not reliably true**, and I have since watched the same mesh at the
same coordinates give different answers in different runs.

## The evidence that it does not reproduce

`SM_BSW_FoldedGoodsTable`, placed at `(-3400, -1150, 662.5)`, traced across
its short axis at 40 cm — the height it must block — on the children's deck:

| run | process | result |
|---|---|---|
| fix applied + verified | full editor | **BLOCK** (in open space) |
| persistence check | full editor | **BLOCK** on the deck |
| trace-geometry sweep, six geometries | full editor | **clear** on all six |
| `place_fixtures.py` probes | commandlet | clear |
| `place_fixtures.py` probes | full editor | clear |

The geometry sweep is the decisive one: six different start and end distances,
including the exact line that reported BLOCK in the persistence check, all
clear in a single process. `SM_BSW_ServiceCounter` blocked on all six in that
same process, as it has in every run all day.

So the variable is not position, not height, not trace length, not the start
distance, and not `set_collision_profile_name` — that call was tested before
and after and changes nothing. It is not the commandlet either: a full editor
gives both answers on different runs.

## What is actually established

- `SM_BSW_ServiceCounter` has blocked correctly in **every** run, in both
  process types, at every geometry. It is the one asset that is sound.
- The other two block **sometimes**. Never before the Nanite change, and
  inconsistently after it.
- Turning Nanite off did change something real — they went from blocking
  nowhere to blocking sometimes — but it did not make them reliable.
- Nanite settings are identical on all three meshes, the sound one included,
  so configuration never explained the difference and still does not.

## The likely story, and it is a hypothesis

The two meshes were imported and built with Nanite on. `962d506` flips the
Nanite flag on **already-built** assets; it does not rebuild them. Whether a
given process ends up with usable simple collision then depends on what is in
the derived data cache when it loads, which is exactly the kind of thing that
differs between runs and explains a result that will not sit still.

If that is right, the fix is not a flag on built data. It is a **clean
re-import of the two OBJs with Nanite off**, so the physics data is built
correctly from the start, followed by the same trace verification run **more
than once, in both process types**, before anything claims they are cover.
The source OBJs are not suspected: they are the same sources the sound
counter came from.

## What this means for the placement

`place_fixtures.py` is still correct to refuse. Its probes have now caught
two separate real defects that asset-level asserts missed, and a flaky pass
would be worse than the current honest failure — it would put fixtures in the
map that are cover in the process that placed them and scenery in the game.

**Nothing should be placed until a fixture blocks in several consecutive runs
in both a commandlet and a full editor.** One green run is what produced
`1ee88b8`'s wrong claim and then mine.

## The pattern, three times in one day

`1ee88b8` counted box elements and called it blocking. A guard matched its own
function definition and called it a test. `962d506` measured once and called
it proven. Each one consulted something real and still proved nothing, because
what it consulted was not the thing being claimed. A single passing
observation of a non-deterministic system is not evidence, and it is most
persuasive exactly when it agrees with what you already believe.
