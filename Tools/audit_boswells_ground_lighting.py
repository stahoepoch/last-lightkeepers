"""Measure the light that actually lands on the ground floor, at a nine-year-old's eye.

Chris, 2026-11-23, home for the first time in a while: *"hit me some big
shit, make sure the lighting is right, i need this to be kick ass."*

**The reason this script has to exist is a date.** Every light on this floor
was authored before 2026-11-23, and on 2026-11-23 the floor was rebuilt:
25 authored cells replaced inferred department zoning, the aisles narrowed to
165 cm, four mirrored coffers went in at z 513, and sixteen glass cases went
round the atrium well. No light moved. So the lighting is currently correct
for a floor that no longer exists, and nothing in the repository has checked
the two against each other. `audit_boswells_opening_lights.py` prints an
inventory; an inventory is a census, and `docs/FINDINGS-THE-LIGHTS-WERE-IN-
THE-ATTIC.md` is the record of what a census misses - 48 lights, all present,
all counted, all above the ceiling.

This is the opposite test. It ignores what the lights are called and asks one
question at a lot of points on the floor: **standing here, with the lantern
off, is there enough light to see that there is racking in front of me?**

## How the number is got

Illuminance from a point source is `E = I / d^2` with `I` in candelas and `d`
in metres - the inverse-square law, which is exact for a source small against
its distance and these are ceiling fixtures 3.9 m over a child's eye. For a
spot light the sample is only lit if it falls inside the outer cone, with a
linear falloff between the inner and outer angles, because that is what UE's
own spot attenuation does closely enough for a reading of *is there light
here at all*. Attenuation radius is a hard cut: past it UE contributes
nothing, however bright the maths says.

Three things are deliberately **not** modelled, and each one makes this
reading optimistic rather than pessimistic:

- **No occlusion by fixtures.** A 164 cm clothing rail between the light and
  the sample is ignored, so real aisles are darker than this says. On a floor
  where a nine-year-old cannot see over any apparel (`docs/FINDINGS-AT-NINE-
  YOU-CANNOT-SEE-OVER-THE-RACKS.md`) that gap is large.
- **No bounce.** Lumen will add some. The emergency layer is unshadowed and
  has `indirect_lighting_intensity` near zero, so not much.
- **No lantern.** This is the lantern-off reading, which is the one
  `ROADMAP.md` slice 2 makes a requirement: *"the chapter remains navigable
  with the lantern off."* Navigable is a floor-level claim, and until now it
  was an untested one.

So every verdict below is a ceiling on how good it is.

## The thresholds, and where they come from

| lux at the eye | what it is |
| --- | --- |
| 0.0001 | overcast starlight: absolute black, and the eye finds nothing |
| 0.001 | moonless night sky. Shapes against shapes, after ten minutes' adaptation |
| 0.01 | **SHAPES.** You can tell something is hanging there |
| 0.1 | quarter moon. You can tell it is clothing |
| 1.0 | full moon on snow. Colour starts, badly |
| 20.0 | a lit corridor at night. **CONTENTS**: you can read a price tag |

The two that matter are 0.01 and 20. `light_boswells_power_states.py` states
the brief for the two layers in exactly these words - emergency gives
*"shapes, not contents"*, mains give a working store - so the audit scores
them against their own stated brief rather than against a number picked here.

## What it reports

Four things, and the fourth is the one worth reading:

1. **Every light, and whether it can reach the floor at all.** A light above
   the sales ceiling at 514, or above one of the four mirror planes at 513,
   cannot light this floor. That splits in two, and the first run of this
   script got the split wrong: it called all 81 of them faults, and 44 are
   the **mezzanine's** fixtures hung under the roof at 1462, exactly where
   they belong. So a light above the mezzanine deck at 650 is reported as
   `UPSTAIRS` and is not a fault. A light in the 124 cm of plenum between the
   two is `STRANDED` - no floor under it, no room above it - and that is the
   attic defect of `docs/FINDINGS-THE-LIGHTS-WERE-IN-THE-ATTIC.md` in its
   last hiding place, now a standing check.
2. **Coverage of the walkable floor** in each power state, as the share of
   sample points over each threshold.
3. **The four escape routes**, sampled along their length, with the darkest
   stretch on each named and measured. A route with a dark run longer than a
   child can cross on memory is a route that does not work with the lantern
   off, whatever its width.
4. **`BoswellsGroundFloor_Light.svg`**, the floor drawn as a heat map in both
   states, because Chris is remote and a drawing is how he sees this.

Read-only. It places nothing, moves nothing and saves no map.
"""

import math
import os

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"

# The plate, from docs/reference/boswells/make_groundfloor_plan.py.
PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
SALES_CEILING = 514.0
MEZZ_SOFFIT = 638.0
MEZZ_DECK = 650.0            # deck top: above this a light is the upper storey's
COURT = (-1000.0, -1000.0, 1000.0, 1000.0)
VOID = (-600.0, -900.0, 600.0, 900.0)

# Eye height of a nine-year-old at ~134 cm stature, decision row 90.
EYE = 124.0

# Four mirrored coffers, from Tools/build_boswells_mirror_coffers.py. A light
# hung above one of these is behind a mirror, not above a ceiling.
MIRROR_Z = 513.0
MIRROR_AT = ((-2625.0, -1200.0), (2625.0, -1200.0), (-2625.0, 1200.0), (2625.0, 1200.0))
MIRROR_HALF = 831.0            # (1400 + 950) / 2 * cos 45, the diamond's span each way

SAMPLE = 50.0                  # cm between samples; 8700 x 4790 gives ~16.7k points
SHAPES = 0.01
CONTENTS = 20.0

LEVELS = ((0.0001, "black"), (0.001, "sky"), (SHAPES, "shapes"),
          (0.1, "cloth"), (1.0, "colour"), (CONTENTS, "contents"))

# A dark run a child can cross from memory. Three steps of 55 cm is the
# walkway width and the unit this floor is measured in; four of them is about
# the distance you will walk blind towards a light you can see.
BLIND_RUN = 220.0


def say(message):
    unreal.log(message)
    print(message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if world is None or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("GROUND_LIGHT wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()


# --- what blocks a light from the floor -------------------------------------

def over_mirror(x, y):
    for mx, my in MIRROR_AT:
        if abs(x - mx) <= MIRROR_HALF and abs(y - my) <= MIRROR_HALF:
            return True
    return False


def in_court(x, y):
    return COURT[0] <= x <= COURT[2] and COURT[1] <= y <= COURT[3]


def in_void(x, y):
    return VOID[0] <= x <= VOID[2] and VOID[1] <= y <= VOID[3]


def lid_above(x, y):
    """The ceiling between this point and the sky, or None where it is open.

    The court is open to the mezzanine and the atrium void to the roof, so a
    light hung high over either of those does reach the ground floor. Over
    the selling floor the lid is the tile underside at 514, except under a
    mirror, where it is the mirror itself a centimetre lower.
    """
    if in_void(x, y):
        return None
    if in_court(x, y):
        return ("mezzanine soffit", MEZZ_SOFFIT)
    if over_mirror(x, y):
        return ("mirror coffer", MIRROR_Z)
    return ("sales tile", SALES_CEILING)


# --- the lights --------------------------------------------------------------

class Lamp(object):
    def __init__(self, actor, comp):
        self.label = actor.get_actor_label()
        self.cls = actor.get_class().get_name()
        loc = actor.get_actor_location()
        self.x, self.y, self.z = loc.x, loc.y, loc.z
        self.intensity = float(comp.get_editor_property("intensity"))
        self.radius = float(comp.get_editor_property("attenuation_radius"))
        self.units = str(comp.get_editor_property("intensity_units"))
        self.tags = [str(t) for t in actor.tags]
        self.spot = isinstance(actor, unreal.SpotLight)
        self.inner = self.outer = 180.0
        self.dir = unreal.Vector(0, 0, -1)
        if self.spot:
            self.inner = float(comp.get_editor_property("inner_cone_angle"))
            self.outer = float(comp.get_editor_property("outer_cone_angle"))
            self.dir = actor.get_actor_forward_vector()
        lid = lid_above(self.x, self.y)
        self.blocked = lid is not None and self.z > lid[1]
        self.blocker = lid[0] if self.blocked else ""
        self.blocker_z = lid[1] if lid else 0.0
        # **A blocked light is not automatically a defect, and the first run
        # of this script said it was.** It reported 81 lights as faults, 44 of
        # which are the mezzanine's own fixtures hung at 1462 under the roof
        # at 1488 - exactly where `light_boswells_power_states.py` means to
        # put them, "one under the lowest lid for the floor you walk on, one
        # under the highest for the mezzanine". They do not light the ground
        # floor because they are not meant to.
        #
        # So blocked splits in two by height. Above the mezzanine deck at 650
        # a light belongs to the storey above and is reported as `UPSTAIRS`.
        # Between the sales lid and that deck there is 124 cm of plenum with
        # no floor under it and no room above it, and a light in there is
        # serving nobody: that is `STRANDED`, and it is the attic defect of
        # `docs/FINDINGS-THE-LIGHTS-WERE-IN-THE-ATTIC.md` in its only
        # remaining hiding place.
        self.upstairs = self.blocked and self.z > MEZZ_DECK
        self.stranded = self.blocked and not self.upstairs
        # Which power state this lamp belongs to. Tags are the contract -
        # see light_boswells_power_states.py on why not names or a list.
        self.mains = "LLK_Mains" in self.tags or self.label.startswith("BSW_MAINS_")

    def lux_at(self, x, y, z):
        """Illuminance in lux at a point, or 0.

        Candelas over metres squared. Anything not authored in candelas is
        reported separately rather than guessed at - lumens and EV need a
        cone solid angle and an exposure respectively, and a wrong conversion
        here would read as a lighting result.
        """
        if self.blocked or self.intensity <= 0.0:
            return 0.0
        dx, dy, dz = x - self.x, y - self.y, z - self.z
        d2 = dx * dx + dy * dy + dz * dz
        if d2 <= 1.0:
            return self.intensity
        d = math.sqrt(d2)
        if d > self.radius:
            return 0.0
        e = self.intensity / ((d / 100.0) ** 2)
        if self.spot:
            # Angle between the spot's own forward vector and the direction
            # from the lamp to the sample.
            cos = (dx * self.dir.x + dy * self.dir.y + dz * self.dir.z) / d
            ang = math.degrees(math.acos(max(-1.0, min(1.0, cos))))
            if ang > self.outer:
                return 0.0
            if ang > self.inner and self.outer > self.inner:
                e *= 1.0 - (ang - self.inner) / (self.outer - self.inner)
        # Lambert at the sample: a floor-facing eye gets less from a lamp off
        # to the side. Kept, because it is the difference between a pool of
        # light and an evenly lit floor, and that difference is the vibe.
        e *= max(0.0, -dz / d) if dz < 0 else 0.0
        return e


lamps = []
odd_units = []
for actor in actors:
    if not isinstance(actor, (unreal.PointLight, unreal.SpotLight, unreal.RectLight)):
        continue
    comp = actor.get_component_by_class(unreal.LightComponent)
    if comp is None:
        continue
    lamp = Lamp(actor, comp)
    if "CANDELAS" not in lamp.units.upper():
        odd_units.append(lamp)
    lamps.append(lamp)

say("GROUND_LIGHT lamps={} candela={} other_units={}".format(
    len(lamps), len(lamps) - len(odd_units), len(odd_units)))

blocked = [l for l in lamps if l.blocked]
stranded = [l for l in blocked if l.stranded]
upstairs = [l for l in blocked if l.upstairs]
for lamp in sorted(stranded, key=lambda l: -l.z):
    say("  STRANDED {} at z {:.0f} is above the {} at {:.0f} and below the deck "
        "at {:.0f} - it lights nothing ({:.1f} cd)".format(
            lamp.label, lamp.z, lamp.blocker, lamp.blocker_z, MEZZ_DECK, lamp.intensity))
say("GROUND_LIGHT reaching={} stranded={} upstairs={} of {}".format(
    len(lamps) - len(blocked), len(stranded), len(upstairs), len(lamps)))
say("  upstairs lights are the mezzanine's, hung under the roof on purpose; "
    "they are not a fault and are not counted as one")

live = [l for l in lamps if not l.blocked and l.intensity > 0.0]
emergency = [l for l in live if not l.mains]
mains_on = live
say("GROUND_LIGHT emergency_lamps={} mains_adds={} total_with_mains={}".format(
    len(emergency), len(live) - len(emergency), len(mains_on)))
for lamp in odd_units:
    say("  UNITS {} is in {} not candelas - excluded from the reading".format(
        lamp.label, lamp.units))


# --- where the floor is walkable --------------------------------------------
# Reuse the occupancy rule from verify_boswells_ground_cells.py: a point is
# walkable if it is on the plate, not in the atrium void, and not inside the
# footprint of a fixture. Footprints are read from actor bounds rather than
# from a list, so anything placed later is included by being there.

OCCUPY_PREFIXES = ("BSW_COURT_ISLAND_", "BSW_APPAREL_LOWER_", "BSW_LEFTDAY_",
                   "BSW_CASEGOODS_", "BSW_JUNIORS_", "BSW_FIXTURE_")
boxes = []
for actor in actors:
    label = actor.get_actor_label()
    if not label.startswith(OCCUPY_PREFIXES):
        continue
    centre, extent = actor.get_actor_bounds(False)
    if centre.z - extent.z > 300.0:
        continue                      # hanging, not standing: you walk under it
    boxes.append((centre.x - extent.x, centre.y - extent.y,
                  centre.x + extent.x, centre.y + extent.y))
say("GROUND_LIGHT standing_fixture_footprints={}".format(len(boxes)))


def occupied(x, y):
    for x0, y0, x1, y1 in boxes:
        if x0 <= x <= x1 and y0 <= y <= y1:
            return True
    return False


def walkable(x, y):
    return not in_void(x, y) and not occupied(x, y)


def sum_lux(lamp_set, x, y, z):
    total = 0.0
    for lamp in lamp_set:
        total += lamp.lux_at(x, y, z)
    return total


# --- coverage ----------------------------------------------------------------

def band(value):
    name = "black"
    for threshold, label in LEVELS:
        if value >= threshold:
            name = label
    return name


grid = []
x = PLATE[0] + SAMPLE * 0.5
while x < PLATE[2]:
    y = PLATE[1] + SAMPLE * 0.5
    while y < PLATE[3]:
        if walkable(x, y):
            grid.append((x, y))
        y += SAMPLE
    x += SAMPLE

# Deliberately NOT the same number as verify_boswells_ground_cells.py's
# `walkable_m2`. That one inflates every obstacle by the keeper's 68 cm half
# body and flood-fills, so it answers "can a child get there"; this one asks
# only "is this point clear of a fixture", which is the right question for
# where light lands. Clear floor is the larger of the two and always will be.
say("GROUND_LIGHT clear_floor_samples={} at {:.0f} cm spacing = {} m2 "
    "(point-clear, not passable-at-68)".format(
        len(grid), SAMPLE, int(len(grid) * SAMPLE * SAMPLE / 10000.0)))

for state, lamp_set in (("EMERGENCY", emergency), ("MAINS", mains_on)):
    counts = dict((label, 0) for _, label in LEVELS)
    counts["black"] = 0
    readings = []
    for px, py in grid:
        lux = sum_lux(lamp_set, px, py, EYE)
        readings.append(lux)
        counts[band(lux)] += 1
    readings.sort()
    median = readings[len(readings) // 2] if readings else 0.0
    shapes = sum(1 for r in readings if r >= SHAPES)
    contents = sum(1 for r in readings if r >= CONTENTS)
    say("GROUND_LIGHT {} median={:.4f} lux  shapes_or_better={:.0f}%  "
        "contents={:.0f}%  max={:.1f}".format(
            state, median, 100.0 * shapes / max(1, len(readings)),
            100.0 * contents / max(1, len(readings)), readings[-1] if readings else 0.0))
    say("  bands " + "  ".join("{}={:.0f}%".format(
        label, 100.0 * counts[label] / max(1, len(readings))) for _, label in LEVELS))


# --- the four escape routes --------------------------------------------------
# Centre lines from the measured table in docs/plans/BOSWELLS-WALKWAY-AND-
# CEILING-RULES.md. These are the ways out of this floor; a dark run on one
# is worse than a dark department, because a department you can skip.

ROUTES = (
    ("west perimeter", (PLATE[0] + 125.0, PLATE[1] + 125.0), (PLATE[0] + 125.0, PLATE[3] - 125.0)),
    ("east perimeter", (PLATE[2] - 125.0, PLATE[1] + 125.0), (PLATE[2] - 125.0, PLATE[3] - 125.0)),
    ("front walk", (PLATE[0] + 125.0, PLATE[1] + 125.0), (PLATE[2] - 125.0, PLATE[1] + 125.0)),
    ("back-of-house", (PLATE[0] + 125.0, PLATE[3] - 125.0), (PLATE[2] - 125.0, PLATE[3] - 125.0)),
    ("main spine west", (-900.0, PLATE[1] + 125.0), (-900.0, PLATE[3] - 125.0)),
    ("main spine east", (900.0, PLATE[1] + 125.0), (900.0, PLATE[3] - 125.0)),
)

for state, lamp_set in (("EMERGENCY", emergency), ("MAINS", mains_on)):
    say("GROUND_LIGHT routes in {}".format(state))
    for name, a, b in ROUTES:
        length = math.hypot(b[0] - a[0], b[1] - a[1])
        steps = max(2, int(length / SAMPLE))
        dark_run = 0.0
        worst_run = 0.0
        worst_at = 0.0
        lit = 0
        for i in range(steps + 1):
            t = float(i) / steps
            px = a[0] + (b[0] - a[0]) * t
            py = a[1] + (b[1] - a[1]) * t
            lux = sum_lux(lamp_set, px, py, EYE)
            if lux >= SHAPES:
                lit += 1
                dark_run = 0.0
            else:
                dark_run += length / steps
                if dark_run > worst_run:
                    worst_run = dark_run
                    worst_at = t * length
        verdict = "ok" if worst_run <= BLIND_RUN else "DARK RUN"
        say("  {:<18} {:.0f} cm  lit={:.0f}%  darkest_run={:.0f} cm at {:.0f} cm  {}".format(
            name, length, 100.0 * lit / (steps + 1), worst_run, worst_at, verdict))


# --- the drawing -------------------------------------------------------------

OUT = os.path.join(unreal.Paths.project_dir(), "docs", "reference", "boswells",
                   "BoswellsGroundFloor_Light.svg")
SCALE = 0.055
MARGIN = 46.0
HEAD = 64.0                    # a legend band of its own, so nothing overprints
W = (PLATE[2] - PLATE[0]) * SCALE
H = (PLATE[3] - PLATE[1]) * SCALE

# Six steps of blue going to warm white. The two darkest are a deliberate near
# match: the difference between starlight and a moonless sky is not something a
# reader should be asked to tell apart, and on this floor neither is navigable.
HEAT = (("#0a0c12", 0.0), ("#141a28", 0.0001), ("#1f2a3e", 0.001),
        ("#33475f", SHAPES), ("#587086", 0.1), ("#93a4ac", 1.0),
        ("#ecE6d6", CONTENTS))
# **Not the page colour.** The first version of this drawing left every
# unwalkable cell as background cream, which is also the colour of the
# brightest band - so the atrium void and 620 fixture footprints read as the
# most blinding light on the floor. Solid is the floor; this is everything
# that is not floor.
BLOCKED_FILL = "#cfc9bd"


def heat(lux):
    colour = HEAT[0][0]
    for c, threshold in HEAT:
        if lux >= threshold:
            colour = c
    return colour


def px(x):
    return MARGIN + (x - PLATE[0]) * SCALE


CELL = 100.0        # the drawing is coarser than the reading, on purpose
rows = []
for idx, (state, lamp_set) in enumerate((("emergency only", emergency),
                                         ("mains on", mains_on))):
    oy = HEAD + MARGIN + idx * (H + MARGIN * 1.6)
    parts = ['<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
             'font-size="14" fill="#2b2b2b">{}</text>'.format(
                 MARGIN, oy - 10.0, state)]
    x = PLATE[0]
    while x < PLATE[2]:
        y = PLATE[1]
        while y < PLATE[3]:
            cx, cy = x + CELL * 0.5, y + CELL * 0.5
            fill = (heat(sum_lux(lamp_set, cx, cy, EYE)) if walkable(cx, cy)
                    else BLOCKED_FILL)
            parts.append('<rect x="{:.1f}" y="{:.1f}" width="{:.1f}" '
                         'height="{:.1f}" fill="{}"/>'.format(
                             px(x), oy + (y - PLATE[1]) * SCALE,
                             CELL * SCALE + 0.4, CELL * SCALE + 0.4, fill))
            y += CELL
        x += CELL
    # Lights, clipped to the plate. Without the clip the galleria and dock
    # lamps print as a drift of dots below the building, which looks like
    # lights falling out of the floor.
    for lamp in lamp_set:
        if not (PLATE[0] <= lamp.x <= PLATE[2] and PLATE[1] <= lamp.y <= PLATE[3]):
            continue
        parts.append('<circle cx="{:.1f}" cy="{:.1f}" r="1.8" fill="#ffd27f" '
                     'stroke="#8a6a28" stroke-width="0.4"/>'.format(
                         px(lamp.x), oy + (lamp.y - PLATE[1]) * SCALE))
    for lamp in stranded:
        if not (PLATE[0] <= lamp.x <= PLATE[2] and PLATE[1] <= lamp.y <= PLATE[3]):
            continue
        parts.append('<circle cx="{:.1f}" cy="{:.1f}" r="2.6" fill="none" '
                     'stroke="#c0392b" stroke-width="1.2"/>'.format(
                         px(lamp.x), oy + (lamp.y - PLATE[1]) * SCALE))
    parts.append('<rect x="{:.1f}" y="{:.1f}" width="{:.1f}" height="{:.1f}" '
                 'fill="none" stroke="#3a3a3a" stroke-width="0.9"/>'.format(
                     MARGIN, oy, W, H))
    rows.append("\n".join(parts))

# The legend gets its own band and a fixed 58 px pitch, which is wide enough
# for "0.0001" at 8 pt. The first version spaced it at 34 and the three dark
# labels printed through each other.
legend = ['<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
          'font-size="11" fill="#2b2b2b">Boswell\'s ground floor: lux at a '
          '124 cm eye, lantern off</text>'.format(MARGIN, 22.0)]
lx = MARGIN
for c, threshold in HEAT:
    legend.append('<rect x="{:.1f}" y="{:.1f}" width="30" height="10" '
                  'fill="{}" stroke="#9a9a9a" stroke-width="0.4"/>'
                  '<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
                  'font-size="8" fill="#2b2b2b">{}</text>'.format(
                      lx, 32.0, c, lx, 51.0,
                      "0" if threshold == 0 else "{:g}".format(threshold)))
    lx += 58.0
legend.append('<rect x="{:.1f}" y="{:.1f}" width="30" height="10" fill="{}" '
              'stroke="#9a9a9a" stroke-width="0.4"/>'
              '<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
              'font-size="8" fill="#2b2b2b">not floor</text>'.format(
                  lx, 32.0, BLOCKED_FILL, lx, 51.0))
lx += 72.0
legend.append('<circle cx="{:.1f}" cy="{:.1f}" r="1.8" fill="#ffd27f" '
              'stroke="#8a6a28" stroke-width="0.4"/>'
              '<text x="{:.1f}" y="{:.1f}" font-family="Georgia,serif" '
              'font-size="8" fill="#2b2b2b">a light</text>'.format(
                  lx + 4.0, 37.0, lx, 51.0))

total_h = HEAD + MARGIN * 2.0 + 2 * H + MARGIN * 1.6
total_w = max(W + MARGIN * 2.0, lx + 90.0)
svg = ('<svg xmlns="http://www.w3.org/2000/svg" width="{:.0f}" height="{:.0f}" '
       'viewBox="0 0 {:.0f} {:.0f}">\n'
       '<rect width="100%" height="100%" fill="#fbfaf7"/>\n{}\n{}\n</svg>\n').format(
           total_w, total_h, total_w, total_h,
           "\n".join(legend), "\n\n".join(rows))
with open(OUT, "w") as handle:
    handle.write(svg)
say("GROUND_LIGHT drew {}".format(OUT))
say("GROUND_LIGHT done (read-only, nothing saved)")
