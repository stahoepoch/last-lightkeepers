"""Extend the opening map past Boswell's grille into a safe mall first stretch.

This is a provisional spatial slice, not a ruling on the whole concourse's
width or length. It gives the player real floor, shop pockets and cover beyond
the release, and a solid far boundary until the next mall section is built.
Run once with the editor closed. The parent Boswell's geometry is untouched.
"""

import unreal


MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "GALLERIA_NEAR_"


def require(ok, why):
    if not ok:
        raise RuntimeError("GALLERIA_NEAR_BUILD " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world is not None and world.get_name() == "LLK_Boswells_OpeningPass",
        "wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {a.get_actor_label(): a for a in subsystem.get_all_level_actors()}
require(not any(n.startswith(PREFIX) for n in before),
        "near concourse already exists")
for name in ("MALL_ENTRANCE_APPROACH_FLOOR", "MALL_GLASS_ENTRANCE",
             "BSW_OPENING_GRILLE_CONTROLLER", "ANCHOR_NAV_BOUNDS_FULL_PLATE"):
    require(name in before, "missing parent: " + name)

cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
materials = {
    "stone": "/Game/LLK/Boswells/Architecture/M_BSW_StoneTile",
    "glass": "/Game/LLK/Boswells/Architecture/M_BSW_MallEntranceGlass",
    "chrome": "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome",
    "teal": "/Game/LLK/Boswells/Architecture/M_BSW_MallMullionTeal",
    "carpet": "/Game/LLK/Boswells/Architecture/M_BSW_KidsSageCarpet",
    "wood": "/Game/Props/Boswells/Materials/dark_wood",
    "steel": "/Game/Props/Boswells/Materials/steel",
    "cloth": "/Game/Props/Boswells/Materials/denim",
}
materials = {key: unreal.EditorAssetLibrary.load_asset(path)
             for key, path in materials.items()}
require(isinstance(cube, unreal.StaticMesh), "engine cube missing")
require(all(isinstance(value, unreal.MaterialInterface)
            for value in materials.values()), "material missing")


def span(name, xmin, xmax, ymin, ymax, bottom, top, mat, solid=True):
    require(xmax > xmin and ymax > ymin and top > bottom,
            "bad dimensions: " + name)
    actor = subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector((xmin + xmax) / 2, (ymin + ymax) / 2,
                      (bottom + top) / 2), unreal.Rotator())
    require(actor is not None, "spawn failed: " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Galleria/NearConcourse")
    comp = actor.static_mesh_component
    require(comp.set_static_mesh(cube), "mesh failed: " + name)
    actor.set_actor_scale3d(unreal.Vector((xmax - xmin) / 100,
                                          (ymax - ymin) / 100,
                                          (top - bottom) / 100))
    comp.set_material(0, materials[mat])
    actor.set_actor_enable_collision(solid)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                               if solid else unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", solid)
    return actor


# A 50 m public first stretch at the entrance's 18 m width. The side shop
# pockets add depth and alternate cover while leaving a continuous broad walk.
# The terminal wall is explicitly a temporary safe boundary, not a chapter
# gate: the next mall slice replaces it after its own route is authored.
span("PUBLIC_FLOOR", -900, 900, -8100, -3100, -30, 0, "stone")
for side, x in (("W", -900), ("E", 900)):
    # The old glass return ends at -2880. Its apron continues to -3100,
    # and our shops start at -3300. These two spans close that entire side
    # edge without narrowing the 18 m public walk.
    span(side + "_ENTRANCE_EDGE", x - 9, x + 9,
         -3100, -2880, 0, 135, "teal")
    span(side + "_SHOP_START_EDGE", x - 9, x + 9,
         -3300, -3100, 0, 135, "teal")
    span(side + "_SHOP_END_EDGE", x - 9, x + 9,
         -8100, -7900, 0, 135, "teal")
for side, xmin, xmax in (("W", -2200, -900), ("E", 900, 2200)):
    span(side + "_SHOP_FLOOR", xmin, xmax, -7900, -3300,
         -30, 0, "carpet")
    outer_x = xmin if side == "W" else xmax
    span(side + "_OUTER_WALL", outer_x - 12, outer_x + 12,
         -8100, -3100, 0, 1200, "wood")
    for index, (north, south) in enumerate(((-3300, -4800),
                                             (-4800, -6350),
                                             (-6350, -7900))):
        # Five metres of opening into each shop. Flanking panels stop sight,
        # while the counter inside is cover; there is no forced narrow throat.
        mouth_a = south + 500
        mouth_b = north - 500
        front_x = -900 if side == "W" else 900
        for suffix, y0, y1 in (("N", mouth_b, north),
                               ("S", south, mouth_a)):
            span(f"{side}_BAY_{index}_{suffix}_GLASS",
                 front_x - 8, front_x + 8, y0, y1,
                 0, 500, "glass")
        span(f"{side}_BAY_{index}_HEADER", front_x - 12,
             front_x + 12, south, north, 500, 550, "teal")
        # A low island and folded stock make the pocket usable as cover.
        shop_centre_x = (xmin + xmax) / 2
        centre_y = (north + south) / 2
        span(f"{side}_BAY_{index}_COUNTER", shop_centre_x - 165,
             shop_centre_x + 165, centre_y - 75, centre_y + 75,
             0, 105, "wood")
        for piece, offset in enumerate((-95, 0, 95)):
            span(f"{side}_BAY_{index}_FOLD_{piece}",
                 shop_centre_x + offset - 42, shop_centre_x + offset + 42,
                 centre_y - 45, centre_y + 45, 105, 120, "cloth", False)
    for partition_y in (-4800, -6350):
        span(f"{side}_PARTITION_{abs(partition_y)}", xmin, xmax,
             partition_y - 9, partition_y + 9, 0, 600, "wood")
    span(side + "_NEAR_END", xmin, xmax, -3312, -3300,
         0, 800, "wood")
    span(side + "_FAR_END", xmin, xmax, -7900, -7888,
         0, 800, "wood")
    # Retail ceiling over the pockets. The high central roof remains glass.
    span(side + "_SHOP_CEILING", xmin, xmax, -8100, -3100,
         1180, 1200, "wood")

# Kiosks and planters break the long view without making a pinch point.
for index, y in enumerate((-3950, -5450, -7050)):
    span(f"KIOSK_{index}_BODY", -180, 180, y - 125, y + 125,
         0, 105, "wood")
    span(f"KIOSK_{index}_GLASS", -170, 170, y - 120, y + 120,
         105, 165, "glass", False)
    for side, x in (("W", -690), ("E", 690)):
        span(f"PLANTER_{index}_{side}", x - 95, x + 95,
             y - 470, y - 310, 0, 95, "chrome")

# Roof: opaque retail shoulders with a glazed storm strip down the public
# walk. The panels do not carve navigation or block a lighting sightline.
span("ROOF_SHOULDER_W", -2200, -520, -8100, -3100,
     1480, 1500, "wood")
span("ROOF_SHOULDER_E", 520, 2200, -8100, -3100,
     1480, 1500, "wood")
for index, y in enumerate(range(-8100, -3100, 500)):
    span(f"ROOF_GLASS_{index}", -520, 520, y, y + 492,
         1480, 1492, "glass", False)
    span(f"ROOF_RIB_{index}", -530, 530, y + 492, y + 500,
         1470, 1500, "teal", False)

# The next segment is not built. A real collision boundary closes every
# floor edge so opening the grille never invites another fall off the map.
span("FAR_SAFETY_WALL", -2200, 2200, -8100, -8075,
     0, 500, "chrome")

# Route lights guide feet and shop mouths, without making the merchandise
# labels readable. They are plain visual lights, outside the gameplay field.
for index, y in enumerate((-3650, -4650, -5650, -6650, -7650)):
    for side, x in (("W", -850), ("E", 850)):
        actor = subsystem.spawn_actor_from_class(
            unreal.PointLight, unreal.Vector(x, y, 480), unreal.Rotator())
        require(actor is not None, "light spawn failed")
        actor.set_actor_label(PREFIX + f"ROUTE_LIGHT_{index}_{side}")
        actor.set_folder_path("Galleria/NearConcourse/Lights")
        light = actor.point_light_component
        light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
        light.set_editor_property("intensity", 8.0)
        light.set_editor_property("attenuation_radius", 720.0)
        light.set_editor_property("source_radius", 40.0)
        light.set_editor_property("indirect_lighting_intensity", 0.25)

# Keep the original store fully inside the volume while covering the new
# 50 m walk. Recast data still needs independent verification after save.
nav = before["ANCHOR_NAV_BOUNDS_FULL_PLATE"]
old_center, old_extent = nav.get_actor_bounds(False)
require(abs(old_center.y) < 1 and abs(old_extent.y - 2900) < 1,
        "navigation volume changed before this pass")
scale = nav.get_actor_scale3d()
nav.set_actor_scale3d(unreal.Vector(scale.x, scale.y * (5500 / 2900), scale.z))
old_location = nav.get_actor_location()
nav.set_actor_location(unreal.Vector(old_location.x, -2600, old_location.z),
                       False, False)
new_center, new_extent = nav.get_actor_bounds(False)
require(new_center.y - new_extent.y <= -8099 and
        new_center.y + new_extent.y >= 2899,
        "nav bounds no longer contain store and new walk")

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP),
        "map save failed")
message = ("GALLERIA_NEAR_BUILD SAVED: 50 m supported walk, six open shop "
           "pockets, cover, storm glass, ten route lights and a safe frontier")
unreal.log(message)
print(message)
