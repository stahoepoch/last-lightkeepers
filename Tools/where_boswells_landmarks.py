"""Read only: print camera-ready positions for the opening map's landmarks.

So a `-game` boot can be sent to a named place with BugItGo and photographed,
instead of only ever being seen from the PlayerStart. Nobody had looked at
this store until 2026-11-22; this is the list of places worth looking at.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("WHERE map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {a.get_actor_label(): a for a in actors}

WANTED = (
    "PlayerStart",
    "ANCHOR_FULL_FLOOR_LOWER",
    "BSW_REALITY_CARPET_ZONE_00",
    "BSW_REALITY_CARPET_ZONE_02",
    "ROUTE_ESCALATOR_ASCEND",
    "ROUTE_ESCALATOR_DESCEND",
    "ANCHOR_LOADING_DOCK_EXIT",
    "GALLERIA_NEAR_PUBLIC_FLOOR",
    "BSW_KIDS_CEILING_TILE_FIELD",
)

for name in WANTED:
    actor = by_label.get(name)
    if actor is None:
        # Prefix match, because several of these are generated names.
        actor = next((a for a in actors
                      if a.get_actor_label().startswith(name)), None)
    if actor is None:
        print("WHERE %s=missing" % name)
        continue
    center, extent = actor.get_actor_bounds(False)
    print("WHERE %s loc=(%.0f,%.0f,%.0f) extent=(%.0f,%.0f,%.0f)" % (
        actor.get_actor_label(), center.x, center.y, center.z,
        extent.x, extent.y, extent.z))

# Every actor class the player would call furniture, with a position, so a
# camera can be put where the store is densest rather than where it is empty.
fixtures = [a for a in actors
            if a.get_actor_label().startswith("BSW_")
            and a.get_components_by_class(unreal.StaticMeshComponent)]
if fixtures:
    xs = [a.get_actor_location().x for a in fixtures]
    ys = [a.get_actor_location().y for a in fixtures]
    zs = [a.get_actor_location().z for a in fixtures]
    print("WHERE fixtures=%d mean=(%.0f,%.0f,%.0f) x=[%.0f,%.0f] y=[%.0f,%.0f]" % (
        len(fixtures), sum(xs) / len(xs), sum(ys) / len(ys), sum(zs) / len(zs),
        min(xs), max(xs), min(ys), max(ys)))

# The upper floor, if there is one worth photographing.
upper = [a for a in actors if a.get_actor_location().z > 400.0]
print("WHERE actors_above_400z=%d" % len(upper))
if upper:
    print("WHERE upper_sample=%s" % [a.get_actor_label() for a in upper[:10]])
