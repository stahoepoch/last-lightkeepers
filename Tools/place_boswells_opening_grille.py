"""Place the compiled opening release and make its existing bars movable.

The controller resolves map pieces by tags. Run once with the editor closed;
this never duplicates a controller or changes another map.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
CONTROLLER_LABEL = "BSW_OPENING_GRILLE_CONTROLLER"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_OPENING_GRILLE_PLACE " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()
labels = {a.get_actor_label(): a for a in actors}
require(len(labels) == len(actors), "duplicate actor labels")
require(CONTROLLER_LABEL not in labels, "controller already placed")


def tagged(tag):
    return [a for a in actors if tag in [str(t) for t in a.get_editor_property("tags")]]


witness = tagged("LLK.Opening.LanternRead")
release = tagged("LLK.Opening.GrilleRelease")
segments = tagged("LLK.Opening.GrilleSegment")
require(len(witness) == 1 and len(release) == 1 and len(segments) == 45,
        "opening tags must resolve 1 witness, 1 release, 45 bars")
require(all(a.get_actor_label().startswith("ROUTE_MALL_GRILLE_") for a in segments),
        "grille tag points at an unrelated actor")

cls = unreal.load_class(None, "/Script/LLK.LLKOpeningGrille")
require(cls is not None, "compiled LLKOpeningGrille class is unavailable")
controller = subsystem.spawn_actor_from_class(
    cls, unreal.Vector(780, -2608, 0), unreal.Rotator())
require(controller is not None, "controller spawn failed")
controller.set_actor_label(CONTROLLER_LABEL)
controller.set_folder_path("Boswells/Opening/Grille")

for actor in segments:
    component = actor.static_mesh_component
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    require(component.get_editor_property("mobility") ==
            unreal.ComponentMobility.MOVABLE,
            "bar is still static: " + actor.get_actor_label())

require(unreal.EditorLoadingAndSavingUtils.save_dirty_packages(
    True, True), "map save failed")
message = "BOSWELLS_OPENING_GRILLE_PLACE SAVED: 1 controller, 45 movable bars"
unreal.log(message)
print(message)
