"""Furnish the six already-built concourse shops without extending the map.

Each shop gets three measured retail fixtures and matching hidden collision
proxies, a category sign, and two emergency lights. The public walk,
five-metre mouths, counter cover, and far safety wall stay where they are.
The original 12 m pocket ceilings were a scale error: lower them to the
existing 5.5 m storefront header before placing stock.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "GALLERIA_SHOP_"
ROOT = "/Game/LLK/Boswells/Fixtures/"


def require(ok, why):
    if not ok:
        raise RuntimeError("GALLERIA_SHOP_DRESS " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = sub.get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
require(len(actors) == len(by_label), "duplicate labels")
require(not any(name.startswith(PREFIX) for name in by_label),
        "shop dressing already placed")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
require(isinstance(cube, unreal.StaticMesh), "collision cube missing")

shop_types = (
    ("W", 0, "APPAREL", "SM_BSW_ClothingRun", "SM_BSW_RoundRack"),
    ("E", 0, "SHOES", "SM_BSW_Department_ShoeDisplay_Short_A",
     "SM_BSW_Department_FeatureTable_A"),
    ("W", 1, "HOUSEWARES", "SM_BSW_Department_HomeDisplay_A",
     "SM_BSW_Department_FeatureTable_A"),
    ("E", 1, "TOYS", "SM_BSW_Department_ToyGondola_A",
     "SM_BSW_Department_FeatureTable_A"),
    ("W", 2, "FURNITURE", "SM_BSW_Department_SofaVignette_A",
     "SM_BSW_Department_FeatureTable_A"),
    ("E", 2, "OUTFITTERS", "SM_BSW_ClothingRun", "SM_BSW_RoundRack"),
)
centres_y = (-4050, -5575, -7125)
mesh_names = {item for _, _, _, deep, front in shop_types for item in (deep, front)}
meshes = {name: unreal.EditorAssetLibrary.load_asset(ROOT + name)
          for name in mesh_names}
require(all(isinstance(mesh, unreal.StaticMesh) for mesh in meshes.values()),
        "a fixture mesh is missing")
for name, mesh in meshes.items():
    bounds = mesh.get_bounds()
    require(bounds.box_extent.x < 110 and bounds.box_extent.y < 290 and
            bounds.origin.z - bounds.box_extent.z > -1 and
            bounds.origin.z + bounds.box_extent.z < 180,
            "fixture footprint or height changed: " + name)

for side in ("W", "E"):
    ceiling = by_label.get(f"GALLERIA_NEAR_{side}_SHOP_CEILING")
    require(isinstance(ceiling, unreal.StaticMeshActor), "pocket ceiling missing")
    origin = ceiling.get_actor_location()
    require(abs(origin.z - 1190) < 1, "pocket ceiling already changed")
for side, index, _, _, _ in shop_types:
    require(f"GALLERIA_NEAR_{side}_BAY_{index}_COUNTER" in by_label and
            f"GALLERIA_NEAR_{side}_BAY_{index}_HEADER" in by_label,
            "parent shop changed")


def fixture(label, mesh, x, y):
    bounds = mesh.get_bounds()
    actor = sub.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, 0), unreal.Rotator())
    require(actor is not None, "visual spawn failed: " + label)
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path("Galleria/NearConcourse/ShopDressing")
    component = actor.static_mesh_component
    require(component.set_static_mesh(mesh), "mesh assignment failed: " + label)
    actor.set_actor_enable_collision(False)
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", False)

    # The authored fixture meshes are visible, while a simple conservative
    # cuboid gives player and nav the same occupied footprint. No invisible
    # armature is allowed to protrude beyond the model's measured bounds.
    proxy = sub.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(x + bounds.origin.x, y + bounds.origin.y, bounds.origin.z),
        unreal.Rotator())
    require(proxy is not None, "proxy spawn failed: " + label)
    proxy.set_actor_label(PREFIX + label + "_COLLISION")
    proxy.set_folder_path("Galleria/NearConcourse/ShopCollision")
    body = proxy.static_mesh_component
    require(body.set_static_mesh(cube), "proxy mesh failed: " + label)
    proxy.set_actor_scale3d(unreal.Vector(
        bounds.box_extent.x * .02, bounds.box_extent.y * .02,
        bounds.box_extent.z * .02))
    body.set_visibility(False, True)
    body.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    body.set_editor_property("can_ever_affect_navigation", True)


for side, index, category, deep_name, front_name in shop_types:
    sign = -1 if side == "W" else 1
    y = centres_y[index]
    stem = f"{side}_{index}_"
    fixture(stem + "DEEP", meshes[deep_name], sign * 1930, y)
    for suffix, offset in (("N", 410), ("S", -410)):
        fixture(stem + suffix, meshes[front_name], sign * 1170, y + offset)

    # Category signage is wayfinding, not inspectable relic information.
    text = sub.spawn_actor_from_class(
        unreal.TextRenderActor, unreal.Vector(sign * 900, y, 480),
        unreal.Rotator(pitch=0, yaw=0 if side == "W" else 180, roll=0))
    require(text is not None, "sign spawn failed: " + stem)
    text.set_actor_label(PREFIX + stem + "SIGN")
    text.set_folder_path("Galleria/NearConcourse/ShopSigns")
    text.set_actor_enable_collision(False)
    words = text.text_render
    words.set_text(category)
    words.set_world_size(44)
    words.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    words.set_vertical_alignment(unreal.VerticalTextAligment.EVRTA_TEXT_CENTER)
    words.set_text_render_color(unreal.Color(201, 225, 213, 255))
    words.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    words.set_editor_property("can_ever_affect_navigation", False)

    pool = sub.spawn_actor_from_class(
        unreal.PointLight, unreal.Vector(sign * 1500, y, 350), unreal.Rotator())
    require(pool is not None, "pool spawn failed: " + stem)
    pool.set_actor_label(PREFIX + stem + "POOL")
    pool.set_folder_path("Galleria/NearConcourse/ShopLights")
    light = pool.point_light_component
    light.set_mobility(unreal.ComponentMobility.MOVABLE)
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", 2.0)
    light.set_editor_property("attenuation_radius", 660.0)
    light.set_editor_property("source_radius", 40.0)
    light.set_editor_property("indirect_lighting_intensity", .25)

    # The overhead pool lights the floor and tops, but leaves the counter's
    # vertical face black. A track head just inside the mouth looks into the
    # shop and gives its fixtures a silhouette from the public walk.
    front = sub.spawn_actor_from_class(
        unreal.SpotLight, unreal.Vector(sign * 980, y, 480),
        unreal.Rotator(pitch=-30, yaw=180 if side == "W" else 0, roll=0))
    require(front is not None, "front light spawn failed: " + stem)
    front.set_actor_label(PREFIX + stem + "FRONT_LIGHT")
    front.set_folder_path("Galleria/NearConcourse/ShopLights")
    beam = front.spot_light_component
    beam.set_mobility(unreal.ComponentMobility.MOVABLE)
    beam.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    beam.set_editor_property("intensity", 16.0)
    beam.set_editor_property("attenuation_radius", 900.0)
    beam.set_editor_property("inner_cone_angle", 25.0)
    beam.set_editor_property("outer_cone_angle", 55.0)
    beam.set_editor_property("source_radius", 40.0)
    beam.set_editor_property("indirect_lighting_intensity", .25)
    beam.set_editor_property("cast_shadows", True)

for side in ("W", "E"):
    ceiling = by_label[f"GALLERIA_NEAR_{side}_SHOP_CEILING"]
    point = ceiling.get_actor_location()
    ceiling.set_actor_location(unreal.Vector(point.x, point.y, 560), False, False)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("GALLERIA_SHOP_DRESS SAVED: six 5.5 m shops, 18 fixtures with collision, "
      "six category signs and 12 emergency lights")
