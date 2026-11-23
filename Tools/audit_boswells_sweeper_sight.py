"""What the Sweeper can actually see from its patrol, on the floor as rebuilt.

Chris, 2026-11-23, after the lighting pass: re-check the Sweeper against the
new aisles and the new light.

Two things changed under it today and neither asked its permission. The floor
was re-laid at 165 cm aisles with sixteen glass cases round the well, and the
lighting was repaired so the east half of the store is no longer black. This
asks what either did to the one machine that hunts on it.

---

## What the code actually does, read before measuring

**`ALLKSweeper::Senses` builds its sight query from `GetActorLocation()` at
both ends.** That is the capsule *centre*, and the Sweeper takes `ACharacter`'s
default capsule - 34 cm radius, 88 cm half-height - because the constructor
says so in as many words: *"No capsule size... The map owns the Sweeper's
physical scale."* So the machine's eye is at **88 cm**, not at the 176 cm top
of its head, and the keeper is seen at their centre rather than their head.

`docs/FINDINGS-AT-NINE-YOU-CANNOT-SEE-OVER-THE-RACKS.md` says *"the Sweeper at
176 cm sees over everything the player cannot"*, and the whole argument for
this floor's stealth rests on that asymmetry. **It is not in the code.** This
script measures the floor at both heights so the size of the gap is a number
rather than an argument.

**The trace is `ECC_Visibility`, via `FLLKWorldVisibility`**, one ray, no
capsule sweep. So an actor conceals only if its collision is enabled *and* its
visibility response is Block. A decorative rack of clothes with collision off
hides nobody, and the ceiling tells you nothing about that.

**Detectability is lantern-only.** `LLKLightField::DetectabilityAt` takes a
list of lit lanterns and nothing else, so the radius the Sweeper notices
within - `Lerp(12, 38, detectability)` studs, 336 to 1064 cm - moves with the
player's lamp and not with the building's. **Today's lighting repair therefore
changed nothing about how far it can see**, which is worth stating plainly
because it is the obvious worry and the answer is no. It also means throwing
the breaker and flooding the store with light leaves every machine exactly as
blind as before, which is a design question and not a defect, and is raised
rather than fixed here.

## How this measures, and why not with engine traces

Occlusion is computed analytically - a 2D segment against the footprint of
every occluding actor, at a given height - rather than by firing thousands of
`line_trace` calls. Three reasons, and the third is the real one:

1. It is fast enough to sample the whole plate from every patrol point.
2. It does not depend on this script guessing which `TraceTypeQuery` maps to
   `ECC_Visibility` in this project, which would be a silent wrong answer.
3. **It can say which actors conceal and which do not**, by reading collision
   enablement and the visibility response off each component. A trace returns
   a boolean; the question worth answering is *what is hiding the child*.

The cost is that it treats every occluder as a vertical box spanning its
bounds, so a rack of hanging clothes conceals across its whole footprint
rather than between its garments. That makes this reading **optimistic for the
player**: the real floor hides less than this says.

Read-only. Draws `BoswellsGroundFloor_Sweeper.svg`.
"""

import math
import os

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"

PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
VOID = (-600.0, -900.0, 600.0, 900.0)
SALES_CEILING = 514.0

STUDS_TO_CM = 28.0                      # LLKUnits::StudsToCm
AGGRO_DARK_CM = 12.0 * STUDS_TO_CM      # 336: lantern off
AGGRO_LIT_CM = 38.0 * STUDS_TO_CM       # 1064: lantern up

# The three heights that matter, all measured from the floor.
# **Both of these are real heights the Sweeper has looked from.** 88 is the
# ACharacter capsule centre, which `ALLKSweeper::Senses` used for the sight
# trace until 2026-11-23 - the machine looked out of its waist. 168 is what it
# uses now: the capsule half-height less an 8 cm inset, read off the body
# rather than typed, so the trace starts just under the crown of a 176 cm
# machine. The pair is kept because the difference is the whole measurement.
EYE_WAIST = 88.0
EYE_HEAD = 168.0
KEEPER_NINE = 67.0        # a 134 cm child's capsule centre, once row 90 is built

PATROL_TAGS = ("LLK.Patrol", "LLK.Patrol.Lower")
SAMPLE = 100.0


def say(message):
    unreal.log("SWEEP " + message)
    print("SWEEP " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if world is None or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("SWEEP wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()


# --- who actually conceals ---------------------------------------------------

# CollisionResponseType, not CollisionResponse - the latter is a struct.
BLOCK = unreal.CollisionResponseType.ECR_BLOCK
ENABLED = (unreal.CollisionEnabled.QUERY_ONLY,
           unreal.CollisionEnabled.QUERY_AND_PHYSICS)

occluders = []        # (x0, y0, x1, y1, base_z, top_z, label)
ignored = {}
for actor in actors:
    label = actor.get_actor_label()
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not comps:
        continue
    blocks = False
    for comp in comps:
        if comp.get_editor_property("static_mesh") is None:
            continue
        try:
            enabled = comp.get_collision_enabled()
            response = comp.get_collision_response_to_channel(
                unreal.CollisionChannel.ECC_VISIBILITY)
        except Exception:
            continue
        if enabled in ENABLED and response == BLOCK:
            blocks = True
            break
    centre, extent = actor.get_actor_bounds(False)
    base, top = centre.z - extent.z, centre.z + extent.z
    if base > SALES_CEILING:
        continue                      # the storey above; nothing to do with this
    family = "_".join(label.split("_")[:3])
    if not blocks:
        ignored[family] = ignored.get(family, 0) + 1
        continue
    occluders.append((centre.x - extent.x, centre.y - extent.y,
                      centre.x + extent.x, centre.y + extent.y, base, top, family))

say("occluders={} on the ground floor".format(len(occluders)))
by_family = {}
for box in occluders:
    by_family[box[6]] = by_family.get(box[6], 0) + 1
for family in sorted(by_family, key=lambda f: -by_family[f])[:12]:
    tops = [b[5] for b in occluders if b[6] == family]
    say("  conceals {:<34} n={:<4} top {:.0f}..{:.0f} cm".format(
        family, by_family[family], min(tops), max(tops)))
say("--- present but concealing nobody (collision off, or not blocking "
    "ECC_Visibility) ---")
for family in sorted(ignored, key=lambda f: -ignored[f])[:12]:
    say("  SEES THROUGH {:<30} n={}".format(family, ignored[family]))


# --- the patrol --------------------------------------------------------------

patrol = []
for actor in actors:
    tags = set(str(t) for t in actor.tags)
    if tags & set(PATROL_TAGS):
        loc = actor.get_actor_location()
        if loc.z < SALES_CEILING:
            patrol.append((actor.get_actor_label(), loc.x, loc.y, loc.z))
patrol.sort(key=lambda p: p[0])
say("patrol points on the ground floor: {}".format(len(patrol)))
for name, x, y, z in patrol:
    say("  {:<34} ({:.0f}, {:.0f}, {:.0f})".format(name, x, y, z))

if len(patrol) >= 2:
    legs = []
    for i in range(len(patrol)):
        a, b = patrol[i], patrol[(i + 1) % len(patrol)]
        legs.append(math.hypot(b[1] - a[1], b[2] - a[2]))
    say("patrol loop: {} legs, total {:.0f} cm, longest {:.0f} cm, "
        "shortest {:.0f} cm".format(len(legs), sum(legs), max(legs), min(legs)))


# --- occlusion ---------------------------------------------------------------

def blocked(x0, y0, h0, x1, y1, h1, boxes):
    """Does the segment from (x0,y0,h0) to (x1,y1,h1) cross any box.

    **The first version of this took one height for the whole ray**, and it
    collapsed the comparison this script exists to make: an eye at 88 and an
    eye at 176 both reduced to the keeper's 67 and returned identical numbers.
    A sightline is a sloping line, and whether a 164 cm rack blocks it depends
    on how high the line is *where it crosses that rack*.

    So the slab test keeps its entry and exit parameters, the ray's height is
    interpolated at each, and the box blocks only if its own vertical span
    overlaps the height band the ray occupies while inside it.
    """
    dx, dy = x1 - x0, y1 - y0
    for bx0, by0, bx1, by1, base, top, _ in boxes:
        t0, t1 = 0.0, 1.0
        ok = True
        for p, q0, lo, hi in ((dx, x0, bx0, bx1), (dy, y0, by0, by1)):
            if abs(p) < 1e-6:
                if q0 < lo or q0 > hi:
                    ok = False
                    break
                continue
            a, b = (lo - q0) / p, (hi - q0) / p
            if a > b:
                a, b = b, a
            t0, t1 = max(t0, a), min(t1, b)
            if t0 > t1:
                ok = False
                break
        if not ok:
            continue
        ray_lo = h0 + (h1 - h0) * t0
        ray_hi = h0 + (h1 - h0) * t1
        if ray_lo > ray_hi:
            ray_lo, ray_hi = ray_hi, ray_lo
        if top >= ray_lo and base <= ray_hi:
            return True
    return False


def in_void(x, y):
    return VOID[0] <= x <= VOID[2] and VOID[1] <= y <= VOID[3]


grid = []
x = PLATE[0] + SAMPLE * 0.5
while x < PLATE[2]:
    y = PLATE[1] + SAMPLE * 0.5
    while y < PLATE[3]:
        if not in_void(x, y):
            grid.append((x, y))
        y += SAMPLE
    x += SAMPLE
say("sampling {} points at {:.0f} cm".format(len(grid), SAMPLE))

results = {}
in_range = set()
for eye_name, eye in (("waist, 88 cm (before)", EYE_WAIST),
                      ("head, 168 cm (now)", EYE_HEAD)):
    seen = set()
    per_point = []
    for name, ppx, ppy, ppz in patrol:
        visible = 0
        reachable = 0
        for gx, gy in grid:
            d = math.hypot(gx - ppx, gy - ppy)
            if d > AGGRO_LIT_CM:
                continue
            reachable += 1
            in_range.add((gx, gy))
            if not blocked(ppx, ppy, eye, gx, gy, KEEPER_NINE, occluders):
                visible += 1
                seen.add((gx, gy))
        per_point.append((name, reachable, visible))
    results[eye_name] = (seen, per_point)
    say("eye {:<18} sees {} of the {} samples in range ({:.0f}%), "
        "{:.0f}% of the whole plate".format(
            eye_name, len(seen), len(in_range),
            100.0 * len(seen) / max(1, len(in_range)),
            100.0 * len(seen) / max(1, len(grid))))
    for name, reachable, visible in per_point:
        say("    {:<32} in range: {:<5} clear: {:<5} ({:.0f}% concealed)".format(
            name, reachable, visible,
            100.0 * (reachable - visible) / max(1, reachable)))

say("coverage: {} of {} floor samples ({:.0f}%) are within 1064 cm of any "
    "patrol point - the rest of the floor no Sweeper can reach even with the "
    "keeper's lantern up".format(
        len(in_range), len(grid), 100.0 * len(in_range) / max(1, len(grid))))

low, high = results["waist, 88 cm (before)"][0], results["head, 168 cm (now)"][0]
only_tall = high - low
say("the height asymmetry: an eye at 168 sees {} samples an eye at 88 cannot "
    "({:.0f}% more floor). This is what ALLKSweeper::Senses now implements, and "
    "what the findings doc always assumed.".format(
        len(only_tall), 100.0 * len(only_tall) / max(1, len(low))))

# --- the drawing -------------------------------------------------------------

OUT = os.path.join(unreal.Paths.project_dir(), "docs", "reference", "boswells",
                   "BoswellsGroundFloor_Sweeper.svg")
SCALE = 0.055
MARGIN = 46.0
HEAD = 58.0
W = (PLATE[2] - PLATE[0]) * SCALE
H = (PLATE[3] - PLATE[1]) * SCALE


def px(x):
    return MARGIN + (x - PLATE[0]) * SCALE


rows = []
for idx, (eye_name, eye) in enumerate((("waist, 88 cm (before)", EYE_WAIST),
                                       ("head, 168 cm (now)", EYE_HEAD))):
    oy = HEAD + MARGIN + idx * (H + MARGIN * 1.6)
    parts = ['<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
             'font-size="14" fill="#2b2b2b">the Sweeper\'s eye at {}</text>'.format(
                 MARGIN, oy - 10.0, eye_name)]
    for gx, gy in grid:
        shade = None
        for name, ppx, ppy, _ in patrol:
            d = math.hypot(gx - ppx, gy - ppy)
            if d > AGGRO_LIT_CM:
                continue
            if not blocked(ppx, ppy, eye, gx, gy, KEEPER_NINE, occluders):
                shade = "#b4342a" if d <= AGGRO_DARK_CM else "#d8a24a"
                break
            if shade is None:
                shade = "#3f6b52"
        fill = shade if shade else "#e8e4d9"
        parts.append('<rect x="{:.1f}" y="{:.1f}" width="{:.1f}" '
                     'height="{:.1f}" fill="{}"/>'.format(
                         px(gx - SAMPLE * 0.5),
                         oy + (gy - SAMPLE * 0.5 - PLATE[1]) * SCALE,
                         SAMPLE * SCALE + 0.4, SAMPLE * SCALE + 0.4, fill))
    for bx0, by0, bx1, by1, base, top, _ in occluders:
        if top < KEEPER_NINE:
            continue
        # Clipped to the plate. The dock, the galleria and the mall grille are
        # all occluders and all outside this building, and drawn unclipped they
        # printed over the legend and into the gap between the two panels,
        # which looked like the store having walls where it has none.
        bx0, bx1 = max(bx0, PLATE[0]), min(bx1, PLATE[2])
        by0, by1 = max(by0, PLATE[1]), min(by1, PLATE[3])
        if bx1 <= bx0 or by1 <= by0:
            continue
        parts.append('<rect x="{:.1f}" y="{:.1f}" width="{:.1f}" height="{:.1f}" '
                     'fill="#2b2b2b" fill-opacity="0.30"/>'.format(
                         px(bx0), oy + (by0 - PLATE[1]) * SCALE,
                         max(0.6, (bx1 - bx0) * SCALE), max(0.6, (by1 - by0) * SCALE)))
    for name, ppx, ppy, _ in patrol:
        parts.append('<circle cx="{:.1f}" cy="{:.1f}" r="4" fill="none" '
                     'stroke="#1d1d1b" stroke-width="1.6"/>'.format(
                         px(ppx), oy + (ppy - PLATE[1]) * SCALE))
    if len(patrol) >= 2:
        pts = " ".join("{:.1f},{:.1f}".format(
            px(p[1]), oy + (p[2] - PLATE[1]) * SCALE) for p in patrol)
        parts.append('<polygon points="{}" fill="none" stroke="#1d1d1b" '
                     'stroke-width="1.1" stroke-dasharray="4 3"/>'.format(pts))
    parts.append('<rect x="{:.1f}" y="{:.1f}" width="{:.1f}" height="{:.1f}" '
                 'fill="none" stroke="#3a3a3a" stroke-width="0.9"/>'.format(
                     MARGIN, oy, W, H))
    rows.append("\n".join(parts))

KEY = (("#b4342a", "seen, lantern off (12 studs)"),
       ("#d8a24a", "seen only with the lantern up (38)"),
       ("#3f6b52", "concealed by something"),
       ("#e8e4d9", "out of range of every patrol point"))
legend = ['<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
          'font-size="11" fill="#2b2b2b">Boswell\'s ground floor: what the '
          'Sweeper can see from its patrol</text>'.format(MARGIN, 20.0)]
lx = MARGIN
for colour, text in KEY:
    legend.append('<rect x="{:.1f}" y="{:.1f}" width="22" height="10" fill="{}" '
                  'stroke="#9a9a9a" stroke-width="0.4"/>'
                  '<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
                  'font-size="8" fill="#2b2b2b">{}</text>'.format(
                      lx, 30.0, colour, lx, 48.0, text))
    lx += 220.0

total_h = HEAD + MARGIN * 2.0 + 2 * H + MARGIN * 1.6
total_w = max(W + MARGIN * 2.0, lx + 20.0)
svg = ('<svg xmlns="http://www.w3.org/2000/svg" width="{:.0f}" height="{:.0f}" '
       'viewBox="0 0 {:.0f} {:.0f}">\n'
       '<rect width="100%" height="100%" fill="#fbfaf7"/>\n{}\n{}\n</svg>\n').format(
           total_w, total_h, total_w, total_h,
           "\n".join(legend), "\n\n".join(rows))
with open(OUT, "w") as handle:
    handle.write(svg)
say("drew {}".format(OUT))
say("done (read-only, nothing saved)")
