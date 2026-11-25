"""Author night trading-floor landmarks and an interrupted egress circuit.

See docs/THE-STORE.md and docs/FIXTURE-LIGHT-CONTRACT.md. Retail accents
mark places; shielded route spots reveal floor and silhouette edges.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"


def require(ok, message):
    if not ok:
        raise RuntimeError("BOSWELLS_LIGHT_REHAB " + message)


def tune(component, intensity, radius, color, shadows=True, cone=None):
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    component.set_editor_property("intensity", intensity)
    component.set_editor_property("attenuation_radius", radius)
    component.set_editor_property("light_color", color)
    component.set_editor_property("indirect_lighting_intensity", 0.08)
    component.set_editor_property("cast_shadows", shadows)
    if cone is not None:
        component.set_editor_property("inner_cone_angle", cone * 0.45)
        component.set_editor_property("outer_cone_angle", cone)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = subsystem.get_all_level_actors()
by_label = {actor.get_actor_label(): actor for actor in actors}
require(len(by_label) == len(actors), "duplicate labels")

EGRESS = unreal.Color(208, 218, 226, 255)
SERVICE = unreal.Color(226, 214, 194, 255)

changed = []
for label, actor in by_label.items():
    if label.startswith("ANCHOR_EMERGENCY_") and isinstance(actor, unreal.SpotLight):
        # Original trading fixtures are dead. The egress circuit below takes over.
        tune(actor.spot_light_component, 0.0, 800.0, EGRESS)
        changed.append(label)
    elif label.startswith("BSW_EGRESS_"):
        require(isinstance(actor, unreal.SpotLight), "egress fixture type: " + label)
        index = int(label.rsplit("_", 1)[1])
        working = "MAIN" in label or "LINK" in label or index % 2 == 0
        tune(actor.spot_light_component, 34.0 if working else 0.0, 760.0, EGRESS, cone=45.0)
        changed.append(label)
    elif label.startswith("BSW_DEPT_ROUTE_POOL_"):
        require(isinstance(actor, unreal.SpotLight), "department fixture type: " + label)
        index = int(label.split("_")[4])
        tune(actor.spot_light_component, 28.0 if index in (0, 3, 6, 8, 10, 14) else 0.0,
             720.0, SERVICE, cone=40.0)
        changed.append(label)
    elif label.startswith("BSW_OPENING_ENTRY_POOL_") and isinstance(actor, unreal.SpotLight):
        tune(actor.spot_light_component, 55.0, 800.0, SERVICE, cone=48.0)
        changed.append(label)
    elif label.startswith("BSW_ESC_DETAIL_") and "_ROUTE_" in label:
        require(isinstance(actor, unreal.SpotLight), "escalator fixture type: " + label)
        tune(actor.spot_light_component, 23.0, 450.0, EGRESS, cone=44.0)
        changed.append(label)
    elif label.startswith("BSW_KIDS_CEILING_FILL_"):
        require(isinstance(actor, unreal.PointLight), "kids fill fixture type: " + label)
        tune(actor.point_light_component, 0.0, 650.0, EGRESS)
        changed.append(label)
    elif label.startswith("GALLERIA_NEAR_ROUTE_LIGHT_"):
        require(isinstance(actor, unreal.PointLight), "concourse route fixture type: " + label)
        tune(actor.point_light_component, 19.0, 760.0, EGRESS, shadows=False)
        changed.append(label)
    elif label.startswith("GALLERIA_SHOP_") and label.endswith("_POOL"):
        require(isinstance(actor, unreal.PointLight), "shop pool fixture type: " + label)
        tune(actor.point_light_component, 0.0, 500.0, SERVICE)
        changed.append(label)
    elif label.startswith("GALLERIA_SHOP_") and label.endswith("_FRONT_LIGHT"):
        require(isinstance(actor, unreal.SpotLight), "shop front fixture type: " + label)
        index = int(label.split("_")[3])
        working = ("_W_" in label and index == 0) or ("_E_" in label and index == 2)
        tune(actor.spot_light_component, 14.0 if working else 0.0, 680.0, SERVICE, cone=38.0)
        changed.append(label)

# A low mirror island belongs to the retail ceiling, beneath the roof. Reuse
# the canopy's material palette and leave one leg of the old trim energized.
def ceiling_piece(name, center, size, material):
    label = "BSW_NIGHT_MIRROR_" + name
    actor = by_label.get(label)
    if actor is None:
        actor = subsystem.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*center))
        require(actor is not None, "could not place " + label)
        actor.set_actor_label(label)
        actor.set_folder_path("Boswells/Architecture/RetailMirrorIsland")
    component = actor.static_mesh_component
    component.set_static_mesh(cube)
    component.set_material(0, material)
    actor.set_actor_scale3d(unreal.Vector(*(value / 100.0 for value in size)))
    actor.set_actor_enable_collision(False)
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", False)
    component.set_cast_shadow(False)


cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
material_root = "/Game/LLK/Boswells/Architecture/"
mirror, frame, neon = [unreal.EditorAssetLibrary.load_asset(material_root + name) for name in (
    "M_BSW_SmokedMirror", "M_BSW_DarkChrome", "M_BSW_NeonBlue")]
require(cube and mirror and frame and neon, "retail mirror assets missing")
ceiling_piece("BACK", (-1460, -1860, 504), (860, 780, 20), frame)
for row in range(3):
    for column in range(4):
        ceiling_piece(f"PANEL_{row}_{column}", (-1757 + 198 * column, -2100 + 240 * row, 491),
                      (188, 230, 6), mirror)
ceiling_piece("LIVE_TRIM", (-1050, -1980, 485), (4, 485, 4), neon)

require(len(changed) >= 90, "unexpectedly small light inventory: {}".format(len(changed)))
require(by_label["BSW_CAPTURE_STORM_FLASH"].point_light_component.get_editor_property("intensity") == 0.0,
        "storm flash must remain off at rest")
require(by_label["BSW_CAPTURE_WITNESS_POOL"].point_light_component.get_editor_property("intensity") == 6.0,
        "capture witness beat must remain untouched")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "save failed")
print("BOSWELLS_LIGHT_REHAB SAVED: {} visual fixtures now make emergency routes, cover silhouettes, and next choices readable".format(len(changed)))
