"""Validate the saved first Galleria shell without changing it."""

import unreal

MAP = "/Game/LLK_Galleria_Shell"
PREFIX = "GALLERIA_SHELL_"


def require(condition, message):
    if not condition:
        raise RuntimeError("GALLERIA_SHELL_VERIFY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None, "map missing")
all_actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()
labels = {actor.get_actor_label(): actor for actor in all_actors}
require(len(labels) == len(all_actors), "duplicate actor labels")
shell = {name: actor for name, actor in labels.items()
         if name.startswith(PREFIX)}
require(len(shell) >= 350, "shell actor count too low")

for name in ("L1_WALK_0", "L1_WALK_1", "L1_WALK_2", "L1_WALK_3",
             "L2_BALCONY_0", "L2_BALCONY_5", "FOOD_COURT_LOWER",
             "CENTER_WELL_LOWER", "FAR_ESCALATOR_WELL_LOWER",
             "CLOCK_ELEVATOR_SHAFT", "BOSWELLS_RETAIL_L1",
             "BOSWELLS_RETAIL_L2", "BOSWELLS_OFFICE_L3",
             "BOSWELLS_DOCK_APRON", "BOSWELLS_SERVICE_CORRIDOR",
             "BOSWELLS_ELEVATOR_SHAFT", "PLAYERSTART_BOSWELLS_FACE",
             "NAV_BOUNDS"):
    require(PREFIX + name in shell, "missing " + name)

pillars = [name for name in shell if "PILLAR_" in name]
shops = [name for name in shell if "SHOP_" in name and name.endswith("_FLOOR")]
fitting = [name for name in shell if "FITTING_" in name and "CUBICLE" in name]
stairs = [name for name in shell if "STAIR_" in name]
public_steps = [name for name in shell if name.startswith(PREFIX + "ESC_")
                and "_STEP_" in name]
store_steps = [name for name in shell if name.startswith(PREFIX + "BOSWELLS_ESC_")
               and "_TREAD_" in name]
require(len(pillars) >= 50, "not enough structural pillars")
require(len(shops) >= 24, "not enough real shop pockets")
require(len(fitting) == 8, "fitting-room count changed")
require(len(stairs) == 7, "stairwell count changed")
require(len(public_steps) == 3 * 2 * 27, "public escalator count changed")
require(len(store_steps) == 2 * 30, "Boswell escalator count changed")

# There must not be an invented third public floor.  The only office level is
# inside the department-store anchor.
upper_public = [name for name in shell if "L2_BALCONY" in name]
third_public = [name for name in shell if "L3" in name and "BOSWELLS" not in name]
require(len(upper_public) == 6 and not third_public,
        "public-floor topology changed")

print("GALLERIA_SHELL_VERIFY PASSED: %d generated actors, %d pillars, %d shop "
      "pockets, two public floors, three escalator connections, one court elevator, "
      "seven stairs, Boswell's two retail floors plus office level, dock, and fitting rooms" %
      (len(shell), len(pillars), len(shops)))
