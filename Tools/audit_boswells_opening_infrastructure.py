"""Read only: inventory the opening map's store infrastructure."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
if not world or world.get_name() != "LLK_Boswells_OpeningPass":
    raise RuntimeError("BOSWELLS_OPENING_AUDIT map unavailable")

actors = unreal.get_editor_subsystem(
    unreal.EditorActorSubsystem).get_all_level_actors()
labels = [actor.get_actor_label() for actor in actors]
terms = ("ELEV", "FITT", "DOCK", "SERVICE", "PILLAR", "COLUMN", "ESCAL", "CARPET")
print("BOSWELLS_OPENING_AUDIT actors=%d" % len(actors))
for term in terms:
    found = [label for label in labels if term in label.upper()]
    print("BOSWELLS_OPENING_AUDIT %s=%d %s" % (term, len(found), found[:20]))
for label in ("ANCHOR_FULL_ROOF", "ANCHOR_LOADING_DOCK_EXIT",
              "ROUTE_ESCALATOR_ASCEND", "ROUTE_ESCALATOR_DESCEND"):
    actor = next((actor for actor in actors if actor.get_actor_label() == label), None)
    if actor:
        center, extent = actor.get_actor_bounds(False)
        print("BOSWELLS_OPENING_AUDIT %s center=(%.0f,%.0f,%.0f) extent=(%.0f,%.0f,%.0f)" %
              (label, center.x, center.y, center.z, extent.x, extent.y, extent.z))
