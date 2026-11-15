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

nav = labels["ANCHOR_NAV_BOUNDS_FULL_PLATE"]
centre, extent = nav.get_actor_bounds(False)
require(centre.y - extent.y <= -8099 and
        centre.y + extent.y >= 2899,
        "nav volume does not cover store and new walk")

message = ("GALLERIA_NEAR_VERIFY PASSED: supported 50 m walk, six shop "
           "pockets, cover, 10 visual route lights and a solid far frontier; "
           f"{len(near)} saved actors")
unreal.log(message)
print(message)
