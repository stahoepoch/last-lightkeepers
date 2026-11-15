"""Place the authored children's-floor sign pass in the ApparelPass map.

Run with Tools/run_editor_python.ps1 while Unreal Editor is closed. Text is
real TextRender geometry; sign panels and tube accents are non-colliding.
Nothing here registers with the gameplay light field.
"""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_KIDS_SIGN_"
FIXTURES = "/Game/LLK/Boswells/Fixtures/"
ARCH = "/Game/LLK/Boswells/Architecture/"
CUBE = "/Engine/BasicShapes/Cube.Cube"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_SIGNS " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_ApparelPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
require(not any(name.startswith(PREFIX) for name in before),
        "sign pass already exists; refusing duplicate")
for name in ("BSW_KIDS_CEILING_SALES_COLUMN", "BSW_KIDS_SALES_DESK",
             "ROUTE_ESCALATOR_ASCEND_HEAD", "ROUTE_ESCALATOR_DESCEND_HEAD"):
    require(name in before, "missing landmark " + name)
for y in (-1900, -1650, -900, -500):
    require("BSW_KIDS_ROLLING_RACK_-1250_{}".format(y) in before,
            "east rack missing at y=" + str(y))

cube = unreal.EditorAssetLibrary.load_asset(CUBE)
cream = unreal.EditorAssetLibrary.load_asset(FIXTURES + "cream")
sage = unreal.EditorAssetLibrary.load_asset(FIXTURES + "sage_sign")
burgundy = unreal.EditorAssetLibrary.load_asset(FIXTURES + "burgundy")
neon = unreal.EditorAssetLibrary.load_asset(ARCH + "M_BSW_NeonAmber")
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
require(all(isinstance(m, unreal.MaterialInterface)
            for m in (cream, sage, burgundy, neon)), "sign material missing")


def panel(label, location, size, material):
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(*location), unreal.Rotator(0, 0, 0))
    require(actor is not None, "panel spawn failed " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Children/Signage")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "panel mesh failed " + label)
    actor.set_actor_scale3d(unreal.Vector(*(v / 100.0 for v in size)))
    comp.set_material(0, material)
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_cast_shadow(False)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    return actor


def lettering(label, copy, location, yaw, height, color):
    actor = actors.spawn_actor_from_class(
        unreal.TextRenderActor, unreal.Vector(*location),
        unreal.Rotator(0, yaw, 0))
    require(actor is not None, "text spawn failed " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Boswells/Children/Signage")
    actor.set_actor_enable_collision(False)
    comp = actor.text_render
    comp.set_text(copy)
    comp.set_world_size(height)
    comp.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    comp.set_vertical_alignment(
        unreal.VerticalTextAligment.EVRTA_TEXT_CENTER)
    comp.set_text_render_color(unreal.Color(*color, 255))
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    if hasattr(comp, "set_can_ever_affect_navigation"):
        comp.set_can_ever_affect_navigation(False)
    return actor


# TextRender lies in local YZ and faces local +X. The east-facing words
# therefore use yaw 0; the west-facing directory uses yaw 180.
lettering("BABIES_COLUMN", "S\nE\nI\nB\nA\nB",
          (-2527, -1970, 930), 0, 27, (55, 60, 56))

panel("DESK_PLAQUE", (-2650, -1860, 980), (2, 96, 28), sage)
lettering("DESK_WORDS", "sales desk", (-2648, -1860, 980),
          0, 16, (48, 64, 55))

for label, z in (("KIDS_TUBE_TOP", 1060), ("KIDS_TUBE_BOTTOM", 1018)):
    panel(label, (-1202, -700, z), (2, 110, 2), neon)
lettering("KIDS_WORD", "KIDS", (-1200, -700, 1039),
          0, 33, (255, 113, 100))

for name, y in (("SOUTH", -1700), ("NORTH", -500)):
    panel("EXIT_FACE_" + name, (-1202, y, 1040), (2, 36, 15), burgundy)
    lettering("EXIT_WORD_" + name, "EXIT", (-1200, y, 1040),
              0, 12, (255, 76, 67))

directory = "\n".join((
    "CANDY", "HEARING", "OPTICAL", "GIFT WRAP", "RESTROOMS",
    "KIDS", "BABIES", "TOYS", "SEASONAL", "BEDDING",
    "CURTAINS", "HOME", "FURNITURE"))
panel("DIRECTORY_PANEL", (-800, -400, 1040), (2, 80, 150), cream)
lettering("DIRECTORY_WORDS", directory, (-802, -400, 1040),
          180, 9, (54, 59, 55))

# The card is hidden under the top rail of each east-end rack. Its copy
# carries no product, price, objective or gameplay revelation.
for y in (-1900, -1650, -900, -500):
    suffix = str(abs(y))
    panel("SALE_CARD_" + suffix, (-1180, y, 773),
          (2, 15, 10), cream)
    lettering("SALE_WORD_" + suffix, "SALE", (-1178, y, 773),
              0, 7, (157, 36, 34))

made = [a for a in actors.get_all_level_actors()
        if a.get_actor_label().startswith(PREFIX)]
require(len(made) == 20, "actor count differs: " + str(len(made)))
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
message = "BOSWELLS_SIGNS SAVED {} signs/panels, no new lights".format(len(made))
unreal.log(message)
print(message)
