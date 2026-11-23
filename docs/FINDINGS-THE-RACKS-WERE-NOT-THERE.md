# The racks were not there

**2026-11-23.** Chris, after the lighting pass: re-check the Sweeper against
the new aisles and the new light.

The light turned out to be the easy half. `LLKLightField::DetectabilityAt`
takes a list of lit **lanterns** and nothing else, so the radius a Sweeper
notices within — `Lerp(12, 38, detectability)` studs, 336 to 1,064 cm — moves
with the player's lamp and not with the building's. **Lighting the east wall
changed nothing about how far any machine can see**, which is the obvious
worry and the answer is no.

It also means something stranger, which is left open at the bottom of this
page: throwing the breaker and flooding the store with light leaves every
machine exactly as blind as before.

The aisles were the other half, and they were a different story.

---

## The floor the player sees and the floor the Sweeper saw

`Tools/audit_boswells_sweeper_sight.py` reads collision off every static mesh
on the ground floor and asks which block `ECC_Visibility`, the channel
`FLLKWorldVisibility` traces. Before today:

| | blocked sight | seen through |
| --- | --- | --- |
| `BSW_APPAREL_LOWER_*` socket bodies | **36** | 0 |
| `BSW_JUNIORS_*` dressing | 0 | **465** |
| `BSW_CIRCULATION_LOWER_STOCK_*` | 0 | **72** |
| `BSW_LEFTDAY_APPAREL_*` | 0 | **60** |
| `BSW_ARRIVAL_PERFUME_*`, `BSW_ARRIVAL_JEWELRY_*` | 0 | **30** |

**Nothing here was a bug in the tool that placed it.**
`dress_boswells_juniors.py` says so in its own header — *"The existing socket
bodies carry collision"* — and its `visual_only()` helper sets `NO_COLLISION`
and `can_ever_affect_navigation = False` deliberately. Dressing should not each
carry a collider; that is how a floor gets six hundred navmesh obstacles and
becomes impassable. Every tool was right on its own.

The gap is what they added up to. **The player walked into a store packed with
clothing and the Sweeper walked through an open hall with thirty-six objects in
it.** A nine-year-old crouching behind a rail of coats was, to the machine,
standing in an empty aisle — and nothing on screen would ever have said so.
That is the worst shape a stealth defect can take, because the player's own
eyes tell them they are hidden.

`docs/FINDINGS-AT-NINE-YOU-CANNOT-SEE-OVER-THE-RACKS.md` argued this floor's
entire stealth proposition from the height of its fixtures. The heights are
real and measured. **The fixtures were not there.**

## And the machine was looking out of its waist

`ALLKSweeper::Senses` built its sight query with `GetActorLocation()` at both
ends. That is the capsule centre, and the class takes `ACharacter`'s default
capsule on purpose — the constructor says *"No capsule size... the map owns the
Sweeper's physical scale."* So the eye sat at **88 cm** on a machine **176 cm**
tall.

Measured before the dressing changed, that was worth almost nothing: an eye at
head height saw **6%** more floor than an eye at the waist, because there was
hardly anything for a tall machine to see over. The asymmetry the design rests
on did not exist, and could not have.

**Fixing the racks is what made the eye height matter.** With 1,114 occluders
instead of 36, the same measurement said the waist was costing everything:

| from the patrol, within the lit radius | eye at 88 (waist) | eye at 168 (head) |
| --- | --- | --- |
| `ANCHOR_PATROL_1` | **0 of 358 clear** | 151 of 358 |
| `ANCHOR_PATROL_2` | 90 of 206 | 90 of 206 |
| `ANCHOR_PATROL_3` | **0 of 357 clear** | 166 of 357 |
| floor in range it can see | **10%** | **44%** |
| worth of the height asymmetry | — | **+352%** |

Two of three patrol points could see **nothing at all**. Had the racks been
fixed and the eye left alone, this floor would have shipped with a blind
enemy — and it would have looked like good stealth design right up until
somebody wondered why they were never caught.

## What changed

**`FLLKPerceiver` gained `EyePosition`,** an optional the visibility trace
reads and every range ignores. The split is the point: `aggroRadiusLit` 38 and
`aggroRadiusDark` 12 are authored as distances between two bodies on a floor,
and raising the measuring point to a machine's head would have quietly
re-scaled every radius in the enemy table. Unset means "looks from where it
stands", so every other caller and every existing test keeps its behaviour by
saying nothing.

**The Sweeper sets it** from its own capsule — half-height less an 8 cm inset,
so the trace starts just under the crown rather than at it, and so a Sweeper
rescaled by a level author keeps looking out of its own head. Read from the
body, never typed.

**458 dressing fixtures became cover**: `QUERY_ONLY`, every channel ignored
except `ECC_Visibility`, which they block. `can_ever_affect_navigation` stays
false, so the navmesh is untouched and needs no rebuild — this does not go near
the scar in `LLKSweeper.cpp` where a decorative mesh carved a nav hole the
machine then carried around with it. 24 fixtures were skipped because they are
hidden in game, and 175 because they do not stand in the band a child's body
occupies.

**`ANCHOR_PATROL_2` was ten centimetres inside the back wall**, at y 2100
against a sales plate ending at 2090. It saw zero of the 179 samples in its
range, because it was looking at the inside of masonry. It now stands at
y 1965, the centre line of the back-of-house walk — the same line the escape
routes are measured along.

Build clean, **41 automation tests pass**, including every perception and
Sweeper test.

## What is measured and deliberately not fixed

**Three patrol points cover 23% of the floor.** 921 of 3,942 samples are within
1,064 cm of any patrol point, so **more than three-quarters of this floor is
somewhere no Sweeper can notice anything, ever.** The loop is 125 m of a
4,167 m² plate. That is a design decision about what the machine is for, not a
defect to patch, and it is left with the number attached.

**The player still walks through the racks.** The Pawn channel was left
unblocked, so the dressing hides you without stopping you. That is wrong, and
it was wrong before today — the difference is that the floor used to be wrong
in two ways at once and is now wrong in one. Making the racks solid needs real
bodies, a navmesh rebuild and somebody to walk the result.

**The mains still change nothing for the machines.** Flooding the store with
light leaves detectability exactly where it was, because the light field is
lanterns only. Fiction and mechanics disagree: the building wakes up and
nothing in it sees better. Raised, not answered.

**Nobody has played any of it.** Every number here is a measurement against a
saved map. The Sweeper has not walked its own patrol since the floor was
re-laid, and the keeper capsule is still not nine years old.
