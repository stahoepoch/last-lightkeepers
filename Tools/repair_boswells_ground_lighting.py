"""Three lighting repairs on Boswell's ground floor, each found by looking.

Chris, 2026-11-23: *"hit me some big shit, make sure the lighting is right, i
need this to be kick ass."*

`Tools/audit_boswells_ground_lighting.py` measured the floor and
`Tools/shoot_map.ps1` photographed it, and the two agreed on three faults.
**The order matters: the pictures corrected the measurements twice**, and
both corrections are recorded here because they are the reason this file does
what it does rather than what the arithmetic first said.

---

## What the measurement said, and what the pictures said back

**Correction one: the mains are not too dim.** The audit computed that mains
on gives a median 0.84 lux at a child's eye and a maximum of 6.1, against the
300-500 lux a working department store runs, and concluded the payoff was two
orders of magnitude short. That arithmetic is right and the conclusion is
wrong, because `Config/DefaultEngine.ini` pins
`r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=0` and the opening
map's `BSW_EXPOSURE_LOCK` pins min and max brightness together at 0.5. Screen
brightness is therefore a fixed mapping of scene luminance, not a physical
one, and the photograph of the mains state is a **fully, evenly lit room**.
Raising the candelas would have blown it out. Nothing here changes a mains
intensity.

**Correction two: the mirrors are not broken.** The mirrored coffers read as
black slabs in the dark, which looked like a missing material. With the mains
on they reflect the ceiling correctly. They are dark because a mirror shows
the room, and the room is dark. That is right, and it is left alone.

## The three faults that survived both

**1. The east perimeter is black for its whole 45 m, and it is the way in.**
`verify_boswells_ground_cells.py` enters at (3200, 2000), inside the east
service door. The audit sampled the four escape routes at a 124 cm eye and the
east perimeter came back `lit=0%` with a 45 m dark run, against the west
perimeter's `100%`. The photograph from (4200, 1700) is a lit ceiling over an
invisible floor.

**The first repair here was wrong, and it is left in this history because the
way it was wrong is the useful part.** It mirrored the live lights near the
west wall across to the east, on the reasoning that two walls of one building
should agree. Thirteen lights and thirteen fittings went in, the new troffers
are visible in the re-render - and the floor was still black. Treating the
asymmetry as the fault meant copying a symptom.

The cause is one line of arithmetic in the grid. `BSW_EMERGENCY_FILL_*` sits
on columns at x = **-3600, -2100, -600, 900, 2400**: a step of 1500, started
from the west edge, run five times and stopped. The plate runs to **4350**, so
the westernmost column stands 750 cm off its wall and the easternmost stands
**1950 cm** off its own. The grid did not fail; it ran out. Nobody chose the
dark east wall, and nobody chose the lit west one either - they are both just
where the loop happened to begin and end.

So the repair continues the grid's own step until the next column would leave
the plate: one more at **x = 3900**, 450 cm off the east wall. Each new light
copies the one in its own row, because the fill is authored to fade with
distance from the mall entrance glass and a column that ignored that would be
a brighter back wall than the rest of the store has. Nothing is invented: the
step, the rows, the intensities and the heights are all read from the lights
already there.

**2. The mains are an even wash with no shadows, and they read as a greybox.**
`light_boswells_power_states.py` authored both layers unshadowed, and said why:
*"the mains do not [fall off], because mains lighting is designed to be even."*
That is true of a real store and it is true because a real store hangs
troffers at a spacing near its mounting height. **Ours are 1200 cm apart at a
390 cm drop to the eye - about three times the mounting height**, far past the
1.0-1.5x that produces even light. So the evenness was never coming from the
grid; it was coming from the lights not casting shadows, which is a different
thing wearing the same word.

The photograph is the argument: the mains state looks like an untextured model
viewer, and it is supposed to be the reward for crossing the store. A lit room
has shadows. So the mains cast them. This **overrides a written decision**, so
it is written down here rather than done quietly, and `docs/FINDINGS-EMERGENCY-
POWER-IS-THE-VIBE.md` is amended with it.

**3. Fittings glow over dead lights.** `ULLKPowerSubsystem::MainsFixtureTag`
carries Chris's rule in his own words, locked on 2026-11-22: *"a fixture that
glows while its light is off is worse than no fixture at all, because it tells
the player the power is on and the room disagrees with it."* The runtime obeys
it for the mains. Nothing obeys it for the emergency circuit, and
`rehabilitate_boswells_emergency_lighting.py` deliberately kills about half of
those lights - *"original trading fixtures are dead"* - while
`fixture_boswells_lights.py` gives every non-mains light the same
`MI_LLK_Fixture_Emergency` housing glowing at 2.5 whether its lamp is lit or
not. So the ceiling advertises a working emergency circuit over a floor that
has none.

A dead fluorescent is not a black hole either: it is a cold grey diffuser that
catches what light reaches it. That is a third material, not the absence of
one.

Idempotent on all three: the new fill column is `BSW_EASTFILL_*` and is
removed before placing, along with the `BSW_EASTWALK_*` actors the first,
wrong version of this repair left in the map, and the other two set properties rather than adding
actors. Saves the map, so it needs `-AllowUnsafeMapSave`.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
EAST_PREFIX = "BSW_EASTFILL_"
STALE_PREFIX = "BSW_EASTWALK_"   # the first, wrong attempt; removed on every run
FIXTURE_PREFIX = "BSW_FIXTURE_"
MATERIAL_DIR = "/Game/LLK/Boswells/Architecture"

MAINS_TAG = "LLK_Mains"
MAINS_FIXTURE_TAG = "LLK_MainsFixture"

PLATE = (-4350.0, -2700.0, 4350.0, 2090.0)
EMERGENCY_TAG = "LLK_Emergency"
FILL_PREFIX = "BSW_EMERGENCY_FILL_"
SALES_CEILING = 514.0      # only lights under this serve the ground floor
EYE = 124.0

TROFFER_SCALE = unreal.Vector(1.2, 0.3, 0.08)
FIXTURE_RISE_CM = 9.0
NEAR_LAMP_CM = 60.0        # a housing belongs to the lamp it sits on, not one nearby


def say(message):
    unreal.log("GROUND_FIX " + message)
    print("GROUND_FIX " + message)


def require(ok, message):
    if not ok:
        raise RuntimeError("GROUND_FIX " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()


def lights():
    out = []
    for actor in subsystem.get_all_level_actors():
        if isinstance(actor, (unreal.SpotLight, unreal.PointLight, unreal.RectLight)):
            comp = actor.get_component_by_class(unreal.LightComponent)
            if comp is not None:
                out.append((actor, comp))
    return out


# =====================================================================
# 1. The east perimeter, derived from the west
# =====================================================================

removed = 0
for actor in subsystem.get_all_level_actors():
    # STALE_PREFIX is the mirrored-west-wall attempt described above. It is
    # cleaned up here rather than by hand so that a checkout of this tool at
    # any revision leaves the map in one state.
    if actor.get_actor_label().startswith((EAST_PREFIX, STALE_PREFIX)):
        subsystem.destroy_actor(actor)
        removed += 1

# Read the fill grid off the map rather than retyping it.
fill = []
for actor, comp in lights():
    loc = actor.get_actor_location()
    if loc.z >= SALES_CEILING:
        continue                                    # the mezzanine's, not ours
    if not actor.actor_has_tag(EMERGENCY_TAG):
        continue
    if not actor.get_actor_label().startswith(FILL_PREFIX):
        continue
    fill.append((actor, comp, loc))

require(fill, "no %s lights under the sales ceiling - this repair extends that "
              "grid and there is none to extend" % FILL_PREFIX)

columns = sorted(set(round(loc.x) for _, _, loc in fill))
rows = sorted(set(round(loc.y) for _, _, loc in fill))
require(len(columns) >= 2, "the fill grid has one column; its step cannot be read")
steps = sorted(set(columns[i + 1] - columns[i] for i in range(len(columns) - 1)))
require(len(steps) == 1,
        "the fill grid's x step is not uniform (%s) - it is no longer a grid and "
        "this repair's premise is gone" % steps)
step = steps[0]
say("fill grid: columns at x %s, rows at y %s, step %d" % (columns, rows, step))

# Continue the step east until the next column would leave the plate.
wanted = []
x = columns[-1] + step
while x <= PLATE[2]:
    wanted.append(x)
    x += step
require(wanted, "the grid already reaches the east edge; nothing to extend")
say("fill grid: adding %d column(s) at x %s - the east wall is %d cm from the "
    "last one and %d cm from the first new one"
    % (len(wanted), wanted, PLATE[2] - columns[-1], PLATE[2] - wanted[0]))

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
require(cube is not None, "/Engine/BasicShapes/Cube missing")
emergency_mat = unreal.EditorAssetLibrary.load_asset(
    "%s/MI_LLK_Fixture_Emergency" % MATERIAL_DIR)
require(emergency_mat is not None,
        "MI_LLK_Fixture_Emergency missing - run fixture_boswells_lights.py first")

placed = 0
for nx in wanted:
    for row in rows:
        # The light in this row nearest the new column is the one to copy, so
        # the fade with distance from the mall glass survives the extension.
        source = min((f for f in fill if round(f[2].y) == row),
                     key=lambda f: abs(f[2].x - nx))
        actor, comp, loc = source
        new = subsystem.spawn_actor_from_class(
            unreal.PointLight, unreal.Vector(float(nx), loc.y, loc.z),
            unreal.Rotator(0.0, 0.0, 0.0))
        require(new is not None, "spawn failed at (%d, %d)" % (nx, row))
        new.set_actor_label("%s%d_%d" % (EAST_PREFIX, int(nx), int(loc.y)))
        new.set_folder_path("Boswells/LowerSales/Lighting")
        target = new.get_component_by_class(unreal.LightComponent)
        target.set_mobility(unreal.ComponentMobility.MOVABLE)
        for prop in ("intensity_units", "intensity", "attenuation_radius",
                     "light_color", "indirect_lighting_intensity", "cast_shadows"):
            try:
                target.set_editor_property(prop, comp.get_editor_property(prop))
            except Exception:
                pass
        # Tags carried across, so a new light joins the same circuit as the one
        # it was copied from. That is the contract in ULLKPowerSubsystem: a
        # light belongs to a circuit by being tagged, not by being listed.
        new.set_editor_property("tags", list(actor.tags))

        # And a fitting, because this is the circuit Chris said must be visible.
        housing = subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(float(nx), loc.y, loc.z + FIXTURE_RISE_CM))
        require(housing is not None, "housing spawn failed at (%d, %d)" % (nx, row))
        housing.set_actor_label("%sFIXTURE_%d_%d" % (EAST_PREFIX, int(nx), int(loc.y)))
        housing.set_folder_path("Boswells/LowerSales/Lighting")
        hcomp = housing.static_mesh_component
        hcomp.set_mobility(unreal.ComponentMobility.MOVABLE)
        hcomp.set_static_mesh(cube)
        housing.set_actor_scale3d(TROFFER_SCALE)
        hcomp.set_material(0, emergency_mat)
        hcomp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        placed += 1

say("fill grid: removed=%d placed=%d lights, each with a fitting"
    % (removed, placed))

# =====================================================================
# 2. The mains cast shadows
# =====================================================================

mains_lit = mains_total = 0
for actor, comp in lights():
    if not actor.actor_has_tag(MAINS_TAG):
        continue
    mains_total += 1
    if actor.get_actor_location().z >= SALES_CEILING:
        continue                                    # the mezzanine's set
    comp.set_editor_property("cast_shadows", True)
    # Shadow casting on a movable light is the cost here, so the radius is
    # trimmed to the drop it actually has to cover rather than left wide. The
    # eye is at 124 and the lamp near 487, so 363 cm of drop; 1400 reaches the
    # floor and about 6 m sideways, which is half the 12 m spacing - the two
    # neighbours meet in the middle instead of every lamp lighting the whole
    # wing and nothing having an edge.
    comp.set_editor_property("attenuation_radius", 1400.0)
    mains_lit += 1
say("mains: %d of %d now cast shadows and reach 1400 cm (the rest are the "
    "mezzanine's)" % (mains_lit, mains_total))


# =====================================================================
# 3. No fitting glows over a dead lamp
# =====================================================================

master = unreal.EditorAssetLibrary.load_asset("%s/M_LLK_LightFixture" % MATERIAL_DIR)
require(master is not None, "M_LLK_LightFixture missing")

dead_path = "%s/MI_LLK_Fixture_Dead" % MATERIAL_DIR
dead_mat = unreal.EditorAssetLibrary.load_asset(dead_path)
if dead_mat is None:
    dead_mat = tools.create_asset("MI_LLK_Fixture_Dead", MATERIAL_DIR,
                                  unreal.MaterialInstanceConstant,
                                  unreal.MaterialInstanceConstantFactoryNew())
    unreal.MaterialEditingLibrary.set_material_instance_parent(dead_mat, master)
    # Emissive 0 is the whole point. The base colour is a cold grey diffuser
    # rather than the housing's near-black, so a dead tube still catches what
    # light reaches it and reads as a fitting that is off - not as a hole in
    # the ceiling, which is the failure the mirrored coffers already show.
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        dead_mat, "Emissive", 0.0)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        dead_mat, "BaseColour", unreal.LinearColor(0.42, 0.44, 0.46, 1.0))
    unreal.EditorAssetLibrary.save_asset(dead_path)
    say("created %s" % dead_path)

lamp_at = []
for actor, comp in lights():
    loc = actor.get_actor_location()
    lamp_at.append((loc, float(comp.get_editor_property("intensity")),
                    actor.actor_has_tag(MAINS_TAG)))

darkened = kept = orphan = 0
for actor in subsystem.get_all_level_actors():
    label = actor.get_actor_label()
    if not label.startswith(FIXTURE_PREFIX) and not label.startswith(EAST_PREFIX + "FIXTURE"):
        continue
    if actor.actor_has_tag(MAINS_FIXTURE_TAG):
        kept += 1
        continue                                    # the runtime owns these
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    if comp is None:
        continue
    here = actor.get_actor_location()
    # The lamp this housing was placed over: the nearest one, and it has to be
    # genuinely near. A housing with no lamp under it is reported rather than
    # guessed about.
    best = None
    for loc, intensity, is_mains in lamp_at:
        d = (here - loc).length()
        if d <= NEAR_LAMP_CM and (best is None or d < best[0]):
            best = (d, intensity)
    if best is None:
        orphan += 1
        continue
    if best[1] <= 0.0:
        comp.set_material(0, dead_mat)
        darkened += 1
    else:
        kept += 1
say("fittings: darkened=%d over dead lamps, left_lit=%d, no_lamp_under=%d"
    % (darkened, kept, orphan))


saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
say("SAVED=%s" % bool(saved))
