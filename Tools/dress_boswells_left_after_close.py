"""Populate Boswell's as a busy department store abandoned after a normal day.

Adds only visible, non-colliding merchandise/detail around the existing
collision-backed department fixtures.  It never changes lanes, the opening
story, or the verified escalator/dock collision surfaces.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_LEFTDAY_"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_LEFTDAY " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "opening map unavailable")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in sub.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before), "pass already exists")
for label in ("BSW_REALITY_FITTING_COUNTER", "BSW_REALITY_DOCK_FLOOR",
              "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND",
              "BSW_OPENING_LANTERN"):
    require(label in before, "required store architecture missing: " + label)

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
root = "/Game/LLK/Boswells/Fixtures/"
mesh_paths = {
    "rack": root + "SM_BSW_ClothingRun",
    "round": root + "SM_BSW_RoundRack",
    "fourway": root + "SM_BSW_FourWay",
    "toy": root + "SM_BSW_Department_ToyGondola_A",
    "home": root + "SM_BSW_Department_HomeDisplay_A",
    "sofa": root + "SM_BSW_Department_SofaVignette_A",
}
meshes = {name: unreal.EditorAssetLibrary.load_asset(path)
          for name, path in mesh_paths.items()}
require(all(isinstance(mesh, unreal.StaticMesh) for mesh in meshes.values()),
        "fixture mesh unavailable")
mat_paths = {
    "cream": root + "cream", "denim": root + "denim", "rose": root + "rose",
    "oat": root + "oat", "navy": root + "navy", "steel": root + "steel",
    "wood": root + "dark_wood", "cardboard": "/Game/Props/Boswells/Materials/oat",
}
mats = {name: unreal.EditorAssetLibrary.load_asset(path) for name, path in mat_paths.items()}
require(all(isinstance(mat, unreal.MaterialInterface) for mat in mats.values()),
        "detail material missing")


def detail_box(name, x, y, z, width, length, height, mat, folder):
    actor = sub.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator())
    require(actor is not None, "detail spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/LeftAfterClose/" + folder)
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "detail mesh failed " + name)
    actor.set_actor_scale3d(unreal.Vector(width / 100, length / 100, height / 100))
    comp.set_material(0, mats[mat])
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)
    return actor


def fixture(name, kind, x, y, z, folder):
    actor = sub.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator())
    require(actor is not None, "fixture spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/LeftAfterClose/" + folder)
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(meshes[kind]), "fixture mesh failed " + name)
    actor.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)
    return actor


# Apparel needs a rhythm of full rails, four-ways and folded stock, not islands
# in an empty hall.  Fixtures sit at the outer display edge; the existing
# planned central routes retain their collision and width.
apparel_rows = [(-3800, -1500), (-3300, -1500), (-2800, -1500), (-2300, -1500),
                (-1800, -1500), (-1300, -1500), (1200, -1500), (1700, -1500),
                (2200, -1500), (2700, -1500), (3200, -1500), (3700, -1500)]
for index, (x, y) in enumerate(apparel_rows):
    fixture("APPAREL_RACK_%02d" % index, "rack" if index % 3 else "fourway", x, y, 5, "Apparel")
    for layer in range(4):
        detail_box("APPAREL_FOLD_%02d_%d" % (index, layer), x + (layer - 1.5) * 34,
                   y + 170, 84 + layer * 8, 46, 34, 7,
                   ("denim", "rose", "oat", "navy")[layer], "Apparel")

# Department-specific in-store texture: toys spill across gondola faces, home
# has boxed small goods, and furniture has accessory piles.  The original
# structural fixtures remain the only collision, preventing fake blocked aisles.
for index, (kind, x, y, z) in enumerate((
        ("toy", 1250, -500, 667), ("toy", 2150, -500, 667),
        ("home", -3550, 650, 667), ("home", -2650, 650, 667),
        ("sofa", 1450, 650, 667), ("sofa", 2450, 650, 667))):
    fixture("DEPARTMENT_%02d" % index, kind, x, y, z, "Departments")
    for pile in range(6):
        detail_box("DEPARTMENT_PILE_%02d_%02d" % (index, pile),
                   x - 90 + (pile % 3) * 90, y - 130 + (pile // 3) * 260,
                   z + 88 + (pile % 2) * 26, 54, 44, 22,
                   ("cream", "rose", "denim", "oat", "navy", "cardboard")[pile],
                   "Departments")

# The official store's service offering inspires a dense but fictional service
# cluster: counters, glasses trays, hearing-brochure drawers, gift-wrap rolls,
# and returns bags.  No Boscov's marks or real department signage are copied.
for index, (x, y) in enumerate(((-3250, 2350), (-2050, 2350), (1350, 2350), (2350, 2350))):
    detail_box("SERVICE_COUNTER_%02d" % index, x, y, 720, 420, 115, 105, "wood", "Services")
    for item in range(5):
        detail_box("SERVICE_STOCK_%02d_%02d" % (index, item), x - 145 + item * 72,
                   y - 10, 832, 48, 42, 16,
                   ("steel", "cream", "rose", "navy", "oat")[item], "Services")

# Fitting rooms look recently left: returns rail, numbered-door tags, shoes
# and bags outside bays. These low details deliberately do not block the lobby.
fixture("FITTING_RETURNS_RACK", "rack", 2980, 2100, 667, "FittingRooms")
for cubicle in range(4):
    detail_box("FITTING_BAG_%d" % cubicle, 3260 + cubicle * 320, 2110, 676,
               34, 28, 38, "cardboard", "FittingRooms")
    detail_box("FITTING_SHOES_%d" % cubicle, 3280 + cubicle * 320, 2175, 671,
               54, 28, 9, "navy", "FittingRooms")

# Receiving remains full of a normal shift's unfinished work: rolling rails,
# cartons and six hand-pallet lanes under the covered dock.
for index, (x, y) in enumerate(((-1250, 3000), (-850, 3600), (-450, 4150),
                                 (450, 4150), (850, 3600), (1250, 3000))):
    fixture("DOCK_ROLLING_RACK_%02d" % index, "rack", x, y, 28, "Dock")
    for carton in range(4):
        detail_box("DOCK_CARTON_DETAIL_%02d_%d" % (index, carton),
                   x - 85 + carton * 55, y + 150, 126 + carton * 32,
                   44, 38, 38, "cardboard", "Dock")

# Sales-floor clutter is sparse enough to preserve flight routes, but appears
# where a last-minute closing crew would leave it: price cards, bags, hangers,
# and folded merchandise at rack ends.
for index, (x, y) in enumerate(((-4050, -450), (-3550, -450), (-3050, -450),
                                 (-2550, -450), (2550, -450), (3050, -450),
                                 (3550, -450), (4050, -450), (-4050, 1250),
                                 (4050, 1250))):
    detail_box("FLOOR_GO_BACK_%02d" % index, x, y, 7, 75, 55, 11,
               ("oat", "denim", "rose", "cream")[index % 4], "GoBacks")

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
new = [actor for actor in sub.get_all_level_actors()
       if actor.get_actor_label().startswith(PREFIX)]
require(len(new) >= 150, "store density unexpectedly low")
print("BOSWELLS_LEFTDAY SAVED: %d non-blocking closing-shift details across apparel, "
      "departments, services, fitting rooms, dock, and floor go-backs" % len(new))
