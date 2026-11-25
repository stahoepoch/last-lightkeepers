"""Repair the opening map's landings, receiving bay and near-mall bypasses.

Uses the authored B5 service-door and atrium dimensions, and the existing
near-concourse footprint. Dock beats are markers for the runtime handoff.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_ROUTE_REPAIR_"
VOICE = "/Game/Audio/Opening/opening-sayers-machine-v1"


def require(condition, message):
    if not condition:
        raise RuntimeError("BOSWELLS_OPENING_ROUTES " + message)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
before = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
require(not any(label.startswith(PREFIX) for label in before), "pass already installed")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
materials = {key: unreal.EditorAssetLibrary.load_asset(path) for key, path in {
    "steel": "/Game/Props/Boswells/Materials/steel",
    "dark": "/Game/LLK/Boswells/Architecture/M_BSW_DarkChrome",
    "tile": "/Game/LLK/Boswells/Architecture/M_BSW_StoneTile",
    "wood": "/Game/Props/Boswells/Materials/dark_wood",
}.items()}
voice = unreal.EditorAssetLibrary.load_asset(VOICE)
require(cube and all(materials.values()), "route materials unavailable")
require(isinstance(voice, unreal.SoundWave), "opening call is not a sound wave")


def box(name, center, size, material, roll=0, solid=True):
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(*center), unreal.Rotator(roll=roll))
    require(actor is not None, "spawn failed " + name)
    actor.set_actor_label(PREFIX + name)
    actor.set_folder_path("Boswells/Opening/RouteRepair")
    component = actor.static_mesh_component
    require(component.set_static_mesh(cube), "mesh assignment failed " + name)
    actor.set_actor_scale3d(unreal.Vector(*(axis / 100 for axis in size)))
    component.set_material(0, materials[material])
    actor.set_actor_enable_collision(solid)
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS
                                    if solid else unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", solid)
    return actor


def span(name, x0, x1, y0, y1, z0, z1, material, solid=True):
    return box(name, ((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
               (x1 - x0, y1 - y0, z1 - z0), material, solid=solid)


def retire(label):
    actor = before[label]
    actor.set_actor_hidden_in_game(True)
    actor.set_actor_enable_collision(False)
    component = actor.static_mesh_component
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property("can_ever_affect_navigation", False)


for name, x, sign in (("ASCEND", -250, 1), ("DESCEND", 250, -1)):
    base = before["ROUTE_ESCALATOR_" + name]
    require(abs(base.get_actor_location().x - x) < 1, "escalator moved")
    # The original 220 cm deck ended at the slab edge, inside a 300 cm
    # guard opening. Bridge that full mouth, overlap the slab and the ramp,
    # and retain a solid fascia under both side rails.
    retire("ROUTE_ESCALATOR_" + name + "_HEAD")
    for side in (-1, 1):
        old_fascia = f"BSW_REALITY_ESC_{name}_STRINGER_{side}"
        if old_fascia in before:
            retire(old_fascia)
        box(f"{name}_FASCIA_{side}", (x + side * 95, 0, 296),
            (23, 1335, 76), "dark", roll=-30 * sign, solid=False)
        rail = x + side * 144
        y0, y1 = sorted((sign * 565, sign * 925))
        span(f"{name}_LANDING_GUARD_{side}", rail - 6, rail + 6,
             y0, y1, 662.5, 777.5, "dark")
        # The ramp is 180 cm wide. Close the drop between its rails and the
        # narrower track, retaining that 180 cm clear travel surface.
        box(f"{name}_RAMP_SHOULDER_{side}", (x + side * 94, 0, 331.25),
            (12, 1325, 22), "steel", roll=-30 * sign)
    y0, y1 = sorted((sign * 550, sign * 950))
    span(name + "_UPPER_DECK", x - 150, x + 150, y0, y1, 625, 662.5, "tile")
    c0, c1 = sorted((sign * 576, sign * 620))
    span(name + "_UPPER_COMB", x - 91, x + 91, c0, c1, 662.6, 664,
         "steel", solid=False)

# The receiving pad guards were a boundary before a real dock was built.
# Remove the interior dead end and keep a safety barrier at the unbuilt
# truck approach outside the roll-up door.
for label in ("ROUTE_DOCK_GUARD_W", "ROUTE_DOCK_GUARD_E", "ROUTE_DOCK_GUARD_N"):
    retire(label)
require("BSW_REALITY_DOCK_FLOOR" in before, "extended dock floor missing")
span("DOCK_TRUCK_GATE", -520, 520, 4500, 4516, 22, 144, "dark")

# Each row of shuttered shops has back-of-shop openings. Players can leave
# the public corridor and return at a different storefront.
for side, x0, x1 in (("W", -2200, -900), ("E", 900, 2200)):
    for y in (-4800, -6350):
        retire(f"GALLERIA_NEAR_{side}_PARTITION_{abs(y)}")
        left, right = (-1940, -900) if side == "W" else (900, 1940)
        span(f"MALL_{side}_{abs(y)}_PARTITION", left, right, y - 9, y + 9, 0, 600, "wood")
        header_left, header_right = (x0, left) if side == "W" else (right, x1)
        span(f"MALL_{side}_{abs(y)}_HEADER", header_left, header_right,
             y - 9, y + 9, 230, 600, "wood")

# Dock-first arrival and west stock corridor. The existing east service door
# gives the player a separate way onto the floor. These are authoring
# markers, so the current spawn and capture state machine remain coherent.
for name, tag, position in (
    ("ARRIVAL", "Arrival", (400, 3340, 106)),
    ("GRAB", "Grab", (0, 2980, 22)),
    ("DOCK_MOUTH", "Haul.01", (0, 2550, 0)),
    ("SERVICE_TURN", "Haul.02", (-3200, 2320, 0)),
    ("WEST_DOOR", "Haul.03", (-3200, 1900, 0)),
    ("WEST_OUTER_AISLE", "Haul.04", (-4090, 1510, 0)),
    ("WEST_SALES_FLOOR", "Haul.05", (-4090, -1810, 0)),
    ("FRONT_PROMENADE", "Haul.06", (-3550, -2200, 0)),
    ("EAST_PLAYER_DOOR", "Alternate", (3200, 2100, 0)),
):
    marker = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*position))
    require(marker is not None, "could not place dock marker " + name)
    marker.set_actor_label(PREFIX + "DOCK_" + name)
    marker.set_folder_path("Boswells/Opening/DockSequence")
    marker.set_editor_property("tags", [unreal.Name("LLK.DockOpening." + tag)])


def trace(start, end):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*start), unreal.Vector(*end),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], unreal.DrawDebugTrace.NONE, True)
    return hit.to_dict() if hit is not None else None


for x, sign in ((-250, 1), (250, -1)):
    for offset in (-125, -55, 0, 55, 125):
        for y in (565, 615, 850, 905, 935):
            hit = trace((x + offset, sign * y, 740), (x + offset, sign * y, 640))
            require(hit and abs(hit["impact_point"].z - 662.5) <= 15,
                    f"upper landing surface missing near {x + offset}, {sign * y}")
for x in (-3200, 3200):
    require(not trace((x, 1950, 110), (x, 2220, 110)), f"service door closed: {x}")
for x in (-2070, 2070):
    for y in (-4800, -6350):
        require(not trace((x, y - 110, 95), (x, y + 110, 95)), f"shop bypass blocked at {x}, {y}")
require(not trace((0, 3150, 95), (0, 3410, 95)), "receiving bay still has an interior barrier")
require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")

# This wave is the single-word machine cue. Keeping the asset address lets
# the legacy capture complete until the dock runtime takes over.
voice.set_editor_property("volume", 0.0)
require(unreal.EditorAssetLibrary.save_loaded_asset(voice), "voice asset save failed")
require(voice.get_editor_property("volume") == 0.0, "voice volume not muted")
unreal.log_warning("BOSWELLS_ROUTES_SAVED: two solid landings, dock clear, four shop openings, voice muted")
