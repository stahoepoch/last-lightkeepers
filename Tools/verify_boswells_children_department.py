"""Check saved children\'s slice, emergency lights, and reserved routes."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
actors = {a.get_actor_label(): a for a in
          unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_KIDS_VERIFY " + why)


kids = {name: actor for name, actor in actors.items()
        if name.startswith("BSW_KIDS_") and
        not name.startswith("BSW_KIDS_CEILING_")}
racks = {name: actor for name, actor in kids.items()
         if name.startswith("BSW_KIDS_ROLLING_RACK_")}
lights = {name: actor for name, actor in actors.items()
          if name.startswith("BSW_EGRESS_")}
ceiling = {name: actor for name, actor in actors.items()
           if name.startswith("BSW_KIDS_CEILING_")}
require(len(kids) == 30 and len(racks) == 23 and len(lights) == 31,
        "saved inventory differs")
require(len(ceiling) == 52 and
        "BSW_KIDS_CEILING_TILE_FIELD" in ceiling and
        "BSW_KIDS_CEILING_SALES_COLUMN" in ceiling,
        "ceiling inventory differs")
require(ceiling["BSW_KIDS_CEILING_TILE_FIELD"].static_mesh_component.get_editor_property(
    "static_mesh").get_path_name() == "/Engine/BasicShapes/Cube.Cube",
    "ceiling still depends on the discarded tiled OBJ")
require("BSW_KIDS_SALES_DESK" in kids, "sales desk absent")
require(all(name in actors for name in (
    "ANCHOR_LOADING_DOCK_EXIT", "ROUTE_ESCALATOR_ASCEND",
    "ROUTE_ESCALATOR_DESCEND")), "vertical or dock route absent")

for name, actor in kids.items():
    center, extent = actor.get_actor_bounds(False)
    if "CARPET" in name or "EDGE_" in name:
        require(not actor.get_actor_enable_collision(),
                "floor covering blocks navigation: " + name)
        continue
    require(center.x - extent.x > -3950 and center.x + extent.x < -1060,
            "fixture enters west outer or atrium bypass: " + name)
    require(center.y - extent.y > -2050 and center.y + extent.y < -1450 or
            center.y - extent.y > -1150 and center.y + extent.y < -150,
            "fixture enters a reserved cross aisle: " + name)

for name, actor in ceiling.items():
    if name.endswith("SALES_COLUMN"):
        center, extent = actor.get_actor_bounds(False)
        require(-2600 < center.x < -2500 and center.y + extent.y < -1450,
                "column leaves the desk pocket")
    elif name.startswith("BSW_KIDS_CEILING_FILL_"):
        require(isinstance(actor, unreal.PointLight) and
                abs(actor.get_actor_location().z - 930) < .01 and
                abs(actor.point_light_component.get_editor_property("intensity") - 8) < .001,
                "children's fill moved or retuned: " + name)
    else:
        require(not actor.get_actor_enable_collision(),
                "ceiling panel blocks navigation: " + name)

for name, actor in lights.items():
    require(isinstance(actor, unreal.SpotLight), "non-spot in egress group")
    require(abs(actor.get_actor_rotation().pitch + 90) < .01,
            "light does not aim at floor: " + name)
    light = actor.spot_light_component
    require(abs(light.get_editor_property("intensity") - 8) < .001,
            "light intensity drift: " + name)

volumes = [a for a in actors.values() if isinstance(a, unreal.PostProcessVolume)]
require(len(volumes) == 1 and volumes[0].get_editor_property("unbound"),
        "exposure volume missing or competing")
settings = volumes[0].get_editor_property("settings")
require(abs(settings.get_editor_property("auto_exposure_min_brightness") - .5) < .001,
        "exposure candidate drifted")
require(abs(settings.get_editor_property("auto_exposure_max_brightness") - .5) < .001,
        "exposure lock is not fixed")
message = "BOSWELLS_KIDS_VERIFY PASSED: 30 department parts, 52 ceiling parts, 31 downcast lights, routes clear in actor bounds"
unreal.log(message)
print(message)
