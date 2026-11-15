"""Check saved apparel placement, old-block overlap, and collision."""

import unreal


MAP = "/Game/LLK_Boswells_ApparelPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong saved map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
apparel = [a for a in actors if a.get_actor_label().startswith("BSW_APPAREL_")]
old = [a for a in actors if a.get_actor_label().startswith("DEPT_")]
if len(apparel) != 36:
    raise RuntimeError("expected 36 apparel fixtures, got {}".format(len(apparel)))


def xy_bounds(actor):
    centre, extent = actor.get_actor_bounds(False)
    return centre.x - extent.x, centre.x + extent.x, centre.y - extent.y, centre.y + extent.y


overlaps = []
for fixture in apparel:
    a = xy_bounds(fixture)
    for block in old:
        b = xy_bounds(block)
        xoverlap = min(a[1], b[1]) - max(a[0], b[0])
        yoverlap = min(a[3], b[3]) - max(a[2], b[2])
        if xoverlap > 1 and yoverlap > 1:
            overlaps.append((fixture.get_actor_label(), block.get_actor_label(),
                             round(xoverlap), round(yoverlap)))

if overlaps:
    for record in overlaps:
        unreal.log_warning("BOSWELLS_APPAREL_VERIFY OVERLAP {} {} {}x{}cm".format(*record))
    raise RuntimeError("{} apparel fixtures overlap legacy department blocks".format(
        len(overlaps)))

for fixture in apparel:
    loc = fixture.get_actor_location()
    result = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(loc.x - 25, loc.y, 110),
        unreal.Vector(loc.x + 25, loc.y, 110),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [],
        unreal.DrawDebugTrace.NONE, True)
    hit = result.to_dict() if result else None
    actor = hit["hit_actor"] if hit else None
    label = actor.get_actor_label() if actor else "None"
    if label != fixture.get_actor_label():
        raise RuntimeError("fixture {} lacks center collision; hit {}".format(
            fixture.get_actor_label(), label))

unreal.log("BOSWELLS_APPAREL_VERIFY PASSED: 36 fixtures, no old-block overlap, 36 collision traces")
