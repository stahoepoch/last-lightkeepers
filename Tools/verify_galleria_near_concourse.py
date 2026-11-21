"""Independent saved-map checks for the first mall stretch.

Geometry and light properties are measured here. This does not certify visual
quality, nav pathing, a machine encounter, or the full route to Sayers.
"""

import unreal


def require(ok, why):
    if not ok:
        raise RuntimeError("GALLERIA_NEAR_VERIFY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(
    "/Game/LLK_Boswells_OpeningPass")
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong saved map")
actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()
labels = {a.get_actor_label(): a for a in actors}
require(len(labels) == len(actors), "duplicate editor labels")
near = {label: actor for label, actor in labels.items()
        if label.startswith("GALLERIA_NEAR_")}
require(len(near) >= 90, "the mall stretch is missing fixtures")
for name in ("PUBLIC_FLOOR", "W_SHOP_FLOOR", "E_SHOP_FLOOR",
             "ROOF_GLASS_0", "ROOF_GLASS_9", "FAR_SAFETY_WALL"):
    require("GALLERIA_NEAR_" + name in near, "missing " + name)
require("MALL_ENTRANCE_APPROACH_FLOOR" in labels and
        "BSW_OPENING_GRILLE_CONTROLLER" in labels,
        "Boswell's opening was lost")


def hit_label(start, end):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*start), unreal.Vector(*end),
        unreal.TraceTypeQuery.ECC_VISIBILITY, True, [],
        unreal.DrawDebugTrace.NONE, True)
    actor = hit.to_dict()["hit_actor"] if hit else None
    return actor.get_actor_label() if actor else None


# After the raised grille, the centre of the public walk is supported from
# the existing glass entrance to the last authored tile. A low eye meets the
# frontier wall rather than stepping off the map.
for y in (-3090, -3200, -5000, -7900, -8080):
    expected = ("MALL_ENTRANCE_APPROACH_FLOOR" if y > -3100 else
                "GALLERIA_NEAR_PUBLIC_FLOOR")
    require(hit_label((0, y, 125), (0, y, -100)) == expected,
            f"unsupported centre floor at y={y}")
for x in (-1500, 1500):
    require(hit_label((x, -6000, 125), (x, -6000, -100)) ==
            ("GALLERIA_NEAR_W_SHOP_FLOOR" if x < 0 else
             "GALLERIA_NEAR_E_SHOP_FLOOR"),
            "an open shop has no floor")
require(hit_label((0, -7900, 125), (0, -8200, 125)) ==
        "GALLERIA_NEAR_FAR_SAFETY_WALL",
        "far end can be walked off")
for side, sign in (("W", -1), ("E", 1)):
    for part, y in (("ENTRANCE", -2990),
                    ("SHOP_START", -3200),
                    ("SHOP_END", -8000)):
        expected = f"GALLERIA_NEAR_{side}_{part}_EDGE"
        require(expected in near, "missing fall guard: " + expected)
        require(hit_label((sign * 700, y, 73),
                          (sign * 1100, y, 73)) == expected,
                "side edge can be walked off: " + expected)

for side in ("W", "E"):
    for index in range(3):
        require(f"GALLERIA_NEAR_{side}_BAY_{index}_COUNTER" in near,
                "shop cover missing")
        require(f"GALLERIA_NEAR_{side}_BAY_{index}_HEADER" in near,
                "shop mouth missing")
    for y in (-4050, -5575, -7125):
        # The public walk can actually enter each pocket at the authored
        # five-metre mouth; a facade that looks open but blocks this trace
        # would make six decorative shops, not six alternate routes.
        x = -1 if side == "W" else 1
        require(hit_label((x * 400, y, 125),
                          (x * 1200, y, 125)) is None,
                f"{side} shop mouth is blocked at y={y}")
for index in range(3):
    require(f"GALLERIA_NEAR_KIOSK_{index}_BODY" in near,
            "public cover missing")
require(hit_label((0, -5300, 33), (0, -5600, 33)) ==
        "GALLERIA_NEAR_KIOSK_1_BODY",
        "the centre kiosk does not cover a crouched child")
require(hit_label((0, -5300, 125), (0, -5600, 125)) is None,
        "the centre kiosk hides standing orientation as well")

lights = [a for name, a in near.items()
          if name.startswith("GALLERIA_NEAR_ROUTE_LIGHT_")]
require(len(lights) == 10 and all(isinstance(a, unreal.PointLight)
                                  for a in lights),
        "emergency light inventory changed")
for actor in lights:
    light = actor.point_light_component
    require(light.get_editor_property("intensity_units") ==
            unreal.LightUnits.CANDELAS and
            abs(light.get_editor_property("intensity") - 8) < 0.01 and
            abs(light.get_editor_property("indirect_lighting_intensity") - .25)
            < .01, "route light left its authored band")
    require(light.get_editor_property("mobility") ==
            unreal.ComponentMobility.MOVABLE,
            "route light still requires an unavailable bake")

# The six public-facing shops are now furnished, with physical fixtures
# rather than art the keeper can walk through. Their ceiling meets the
# existing storefront header at 5.5 m instead of hanging 6.5 m above it.
for side, sign in (("W", -1), ("E", 1)):
    ceiling = near[f"GALLERIA_NEAR_{side}_SHOP_CEILING"]
    require(abs(ceiling.get_actor_location().z - 560) < 1,
            "shop ceiling still has a warehouse-scale void")
    for index, y in enumerate((-4050, -5575, -7125)):
        stem = f"GALLERIA_SHOP_{side}_{index}_"
        for suffix in ("DEEP", "N", "S"):
            visual = labels.get(stem + suffix)
            proxy = labels.get(stem + suffix + "_COLLISION")
            require(isinstance(visual, unreal.StaticMeshActor) and
                    isinstance(proxy, unreal.StaticMeshActor),
                    "furnished shop or collision missing: " + stem + suffix)
            require(visual.static_mesh_component.get_collision_enabled() ==
                    unreal.CollisionEnabled.NO_COLLISION and
                    proxy.static_mesh_component.get_collision_enabled() ==
                    unreal.CollisionEnabled.QUERY_AND_PHYSICS and
                    proxy.static_mesh_component.get_editor_property(
                        "can_ever_affect_navigation"),
                    "render and physics disagree: " + stem + suffix)
        sign_actor = labels.get(stem + "SIGN")
        pool = labels.get(stem + "POOL")
        front = labels.get(stem + "FRONT_LIGHT")
        require(isinstance(sign_actor, unreal.TextRenderActor) and
                isinstance(pool, unreal.PointLight) and
                pool.point_light_component.get_editor_property("mobility") ==
                unreal.ComponentMobility.MOVABLE and
                pool.point_light_component.get_editor_property("intensity_units") ==
                unreal.LightUnits.CANDELAS and
                abs(pool.point_light_component.get_editor_property("intensity") - 2) < .01,
                "shop identity or dynamic light missing: " + stem)
        require(abs(sign_actor.get_actor_location().z - 480) < 1 and
                abs(sign_actor.get_actor_rotation().yaw - (0 if sign < 0 else 180)) < .1,
                "department sign is above the facade or facing away: " + stem)
        require(isinstance(front, unreal.SpotLight) and
                front.spot_light_component.get_editor_property("mobility") ==
                unreal.ComponentMobility.MOVABLE and
                front.spot_light_component.get_editor_property("intensity_units") ==
                unreal.LightUnits.CANDELAS and
                abs(front.spot_light_component.get_editor_property("intensity") - 16) < .01 and
                abs(front.spot_light_component.get_editor_property("outer_cone_angle") - 55) < .01,
                "inward shop light missing: " + stem)
        require(abs(pool.get_actor_location().z - 350) < 1 and
                abs(front.get_actor_location().z - 480) < 1 and
                abs(front.get_actor_rotation().pitch + 30) < .1 and
                abs(front.get_actor_rotation().roll) < .1 and
                abs(front.get_actor_rotation().yaw - (180 if sign < 0 else 0)) < .1,
                "shop lighting reverted to ceiling glare: " + stem)
        require(hit_label((sign * 1800, y, 80),
                          (sign * 2050, y, 80)) == stem + "DEEP_COLLISION",
                "deep stock has no blocking body: " + stem)
        for offset in (-200, 200):
            require(hit_label((sign * 1200, y + offset, 80),
                              (sign * 1800, y + offset, 80)) is None,
                    "counter has no flank route: " + stem)

nav = labels["ANCHOR_NAV_BOUNDS_FULL_PLATE"]
centre, extent = nav.get_actor_bounds(False)
require(centre.y - extent.y <= -8099 and
        centre.y + extent.y >= 2899,
        "nav volume does not cover store and new walk")

message = ("GALLERIA_NEAR_VERIFY PASSED: supported 50 m walk, six furnished "
           "pockets and inward lights, cover, 10 visual route lights, solid frontier; "
           f"{len(near)} saved actors")
unreal.log(message)
print(message)
