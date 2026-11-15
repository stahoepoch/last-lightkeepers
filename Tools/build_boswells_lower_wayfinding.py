"""Place lower-floor department landmarks under the new sales-wing ceilings.

Only wayfinding text is exposed. No product, relic, price, or objective text is
emitted, and no new gameplay light source is registered.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_LOWER_WAYFINDING_"


def require(ok, reason):
    if not ok:
        raise RuntimeError("BOSWELLS_LOWER_WAYFINDING " + reason)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in subsystem.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before),
        "already built; refusing duplicate")
require(all(label in before for label in (
    "BSW_LOWER_CEILING_WEST_FIELD", "BSW_LOWER_CEILING_EAST_FIELD",
    "ROUTE_MALL_GRILLE_22", "ROUTE_ESCALATOR_ASCEND")),
    "ceiling, mall exit, or escalator absent")

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
back = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome")
blue = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_NeonBlue")
amber = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_NeonAmber")
text_material = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_EmissiveWayfindingText")
require(isinstance(cube, unreal.StaticMesh), "cube source absent")
require(all(isinstance(m, unreal.MaterialInterface)
            for m in (back, blue, amber, text_material)),
        "wayfinding materials absent")


def panel(label, x, y, z, size, material):
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=0, yaw=0, roll=0))
    require(actor is not None, "panel spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/LowerSales/Wayfinding")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "mesh failed: " + label)
    actor.set_actor_scale3d(unreal.Vector(*(length / 100 for length in size)))
    comp.set_material(0, material)
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_cast_shadow(False)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    return actor


def words(label, copy, x, y, z, yaw, size, color):
    actor = subsystem.spawn_actor_from_class(
        unreal.TextRenderActor, unreal.Vector(x, y, z),
        unreal.Rotator(pitch=0, yaw=yaw, roll=0))
    require(actor is not None, "lettering spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/LowerSales/Wayfinding")
    actor.set_actor_enable_collision(False)
    text = actor.text_render
    text.set_text(copy)
    text.set_world_size(size)
    text.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    text.set_vertical_alignment(unreal.VerticalTextAligment.EVRTA_TEXT_CENTER)
    text.set_text_render_color(unreal.Color(*color, 255))
    text.set_material(0, text_material)
    text.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    if hasattr(text, "set_can_ever_affect_navigation"):
        text.set_can_ever_affect_navigation(False)
    return actor


# Local +X is the text face. Both wing signs face the central customer spine.
# The four labels are ordinary department names, legible from a distance.
departments = (
    ("JUNIORS", "JUNIORS", -930, -1450, 0, blue, (91, 155, 237)),
    ("LADIES", "LADIES", -930, 1000, 0, amber, (238, 166, 95)),
    ("MENS", "MENS", 930, -1450, 180, blue, (91, 155, 237)),
    ("SHOES", "SHOES", 930, 1000, 180, amber, (238, 166, 95)),
)
for label, copy, x, y, yaw, neon, color in departments:
    panel(label + "_BACK", x, y, 448, (3, 255, 54), back)
    front_x = x + (3 if x < 0 else -3)
    panel(label + "_TUBE_TOP", front_x, y, 473, (2, 250, 2), neon)
    panel(label + "_TUBE_BOTTOM", front_x, y, 423, (2, 250, 2), neon)
    words(label + "_WORDS", copy, x + (5 if x < 0 else -5),
          y, 448, yaw, 31, color)

# EXIT is navigation signage, facing into the store from the south approach.
panel("EXIT_BACK", 0, -2460, 425, (190, 3, 52), back)
panel("EXIT_TUBE_TOP", 0, -2457, 449, (185, 2, 2), amber)
panel("EXIT_TUBE_BOTTOM", 0, -2457, 401, (185, 2, 2), amber)
words("EXIT_WORDS", "EXIT", 0, -2455, 425, 90, 27, (247, 169, 89))

after = {actor.get_actor_label(): actor for actor in subsystem.get_all_level_actors()}
require(sum(label.startswith(PREFIX) for label in after) == 20,
        "sign inventory differs")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
unreal.log("BOSWELLS_LOWER_WAYFINDING SAVED: four departments and mall EXIT")
