"""Tag and unlock the saved grille hardware for the opening controller.

One-shot authored map migration. It refuses to run in the wrong map or with
missing/duplicated hardware. Run with the editor closed.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
HARDWARE = {
    "BSW_OPENING_SCENE_RELEASE_BAR": "LLK.Opening.GrilleRelease",
    "BSW_OPENING_SCENE_RELEASE_STEM": "LLK.Opening.GrilleStem",
    "BSW_OPENING_SCENE_RELEASE_LINK": "LLK.Opening.GrilleLink",
    "BSW_OPENING_SCENE_RELEASE_HEADER_ROLLER": "LLK.Opening.GrilleRoller",
}


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_GRILLE_HARDWARE " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()
labels = {a.get_actor_label(): a for a in actors}
require(len(labels) == len(actors), "duplicate labels")
require(all(name in labels for name in HARDWARE), "hardware missing")
require("BSW_OPENING_GRILLE_CONTROLLER" in labels, "controller missing")
require(sum("LLK.Opening.GrilleRelease" in
            [str(t) for t in a.get_editor_property("tags")]
            for a in actors) == 1,
        "existing release tag missing or duplicated")
require(all(not any(tag in [str(t) for t in a.get_editor_property("tags")]
                    for a in actors) for tag in list(HARDWARE.values())[1:]),
        "hardware already tagged")
require(all(labels[name].static_mesh_component.get_editor_property("mobility")
            == unreal.ComponentMobility.STATIC for name in HARDWARE),
        "hardware was already made movable")

for name, tag in HARDWARE.items():
    actor = labels[name]
    component = actor.static_mesh_component
    if name != "BSW_OPENING_SCENE_RELEASE_BAR":
        tags = list(actor.get_editor_property("tags"))
        actor.set_editor_property("tags", tags + [unreal.Name(tag)])
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    require(component.get_editor_property("mobility") ==
            unreal.ComponentMobility.MOVABLE,
            "hardware cannot move: " + name)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "save failed")
message = "BOSWELLS_GRILLE_HARDWARE SAVED: handle, stem, link and roller movable"
unreal.log(message)
print(message)
