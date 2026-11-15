"""Verify the saved entrance dressing and walkable front promenade."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
PREFIX = "BSW_ARRIVAL_"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("BOSWELLS_ARRIVAL_VERIFY wrong map")
actors = {a.get_actor_label(): a for a in unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()}
new = {label: actor for label, actor in actors.items() if label.startswith(PREFIX)}
if len(new) != 76:
    raise RuntimeError("expected 76 arrival parts, got {}".format(len(new)))
for old in ("DEPT_CHECKOUT_WEST", "DEPT_CHECKOUT_EAST",
            "DEPT_COSMETICS_COUNTER", "DEPT_JEWELRY_CASE"):
    if old in actors:
        raise RuntimeError("old block remains: " + old)

for label, actor in new.items():
    centre, extent = actor.get_actor_bounds(False)
    xmin, xmax = centre.x - extent.x, centre.x + extent.x
    ymin, ymax = centre.y - extent.y, centre.y + extent.y
    zmin = centre.z - extent.z
    if label.startswith(PREFIX + "CHECKOUT_WEST"):
        socket = (-2100, -1500, -2600, -2400)
    elif label.startswith(PREFIX + "CHECKOUT_EAST"):
        socket = (1500, 2100, -2600, -2400)
    elif label.startswith(PREFIX + "PERFUME"):
        socket = (-3800, -2800, -2600, -2400)
    else:
        socket = (2800, 3800, -2600, -2400)
    if (xmin < socket[0] - 1 or xmax > socket[1] + 1
            or ymin < socket[2] - 1 or ymax > socket[3] + 1):
        raise RuntimeError("part outside its socket: " + label)
    if label.endswith("_PLINTH") or "CHECKOUT" in label:
        if abs(zmin) > 1:
            raise RuntimeError("fixture floats above floor: {} at z {}".format(label, zmin))


def hit_label(start, end):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(*start), unreal.Vector(*end),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [],
        unreal.DrawDebugTrace.NONE, True)
    result = hit.to_dict() if hit else None
    actor = result["hit_actor"] if result else None
    return actor.get_actor_label() if actor else None


# The human route is y=-2350..-2050. Check the middle at eye/chest height
# and the central line from the entry toward the escalator.
for z in (88, 152):
    label = hit_label((-3900, -2250, z), (3900, -2250, z))
    if label:
        raise RuntimeError("front promenade blocked at z {} by {}".format(z, label))
label = hit_label((0, -2600, 110), (0, -1500, 110))
if label:
    raise RuntimeError("central arrival blocked by " + label)
for name, x in (("checkout", -1800), ("glass case", -3310)):
    label = hit_label((x, -2650, 80), (x, -2350, 80))
    if not label or not label.startswith(PREFIX):
        raise RuntimeError("{} lacks blocking collision: {}".format(name, label))

if sum(label.startswith("BSW_EGRESS_") for label in actors) != 31:
    raise RuntimeError("egress light inventory changed")
if sum(label.startswith("BSW_CANOPY_GLASS_") for label in actors) != 24:
    raise RuntimeError("atrium mirror inventory changed")
for label in ("ROUTE_MALL_GRILLE_22", "ROUTE_ESCALATOR_ASCEND",
              "ANCHOR_LOADING_DOCK_EXIT"):
    if label not in actors:
        raise RuntimeError("route actor missing: " + label)
print("BOSWELLS_ARRIVAL_VERIFY PASSED: 76 placed parts; grounded cases and counters; clear 300cm promenade")
