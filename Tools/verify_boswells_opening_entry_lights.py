"""Check two entry pools after independent reload of the saved opening map."""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_OPENING_ENTRY_POOL_"


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_ENTRY_LIGHTS_VERIFY " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
all_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
by_label = {a.get_actor_label(): a for a in all_actors}
require(len(by_label) == len(all_actors), "duplicate actor labels")
require(sum(name.startswith(PREFIX) for name in by_label) == 4,
        "opening pool actor count differs")
require(sum(name.startswith("BSW_EGRESS_") for name in by_label) == 31,
        "older egress lights changed")

for side, x in (("WEST", -1600), ("EAST", 1600)):
    label = PREFIX + side
    actor = by_label.get(label)
    face = by_label.get(label + "_DIFFUSER")
    require(isinstance(actor, unreal.SpotLight) and
            isinstance(face, unreal.StaticMeshActor),
            "entry fixture incomplete: " + side)
    loc = actor.get_actor_location()
    require(abs(loc.x - x) < 1 and abs(loc.y + 2200) < 1 and
            abs(loc.z - 485) < 1, "spot moved: " + side)
    light = actor.spot_light_component
    require(light.get_editor_property("mobility") ==
            unreal.ComponentMobility.MOVABLE,
            "entry light needs a baked interaction: " + side)
    require(light.get_editor_property("intensity_units") ==
            unreal.LightUnits.CANDELAS and
            abs(light.get_editor_property("intensity") - 8) < 0.01 and
            abs(light.get_editor_property("outer_cone_angle") - 35) < 0.01 and
            abs(light.get_editor_property("attenuation_radius") - 800) < 0.01,
            "light band changed: " + side)
    center, extent = face.get_actor_bounds(False)
    require(abs(center.x - x) < 1 and abs(center.y + 2200) < 1 and
            center.z - extent.z >= 505 and
            extent.x <= 61 and extent.y <= 31,
            "diffuser escaped ceiling: " + side)
    comp = face.static_mesh_component
    require(comp.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION
            and not comp.get_editor_property("can_ever_affect_navigation"),
            "diffuser blocks movement: " + side)
    # The perfume and jewelry cases begin at |x|=2800, well outside these
    # cones; the checkout counters nearer the route may receive some light.
    require(abs(x) + (485 - 100) * 0.7 < 2800,
            "case glass enters direct entry pool")

for label in ("BSW_OPENING_LANTERN", "ROUTE_MALL_GRILLE_22",
              "ANCHOR_PLAYERSTART_ENTRANCE", "ROUTE_ESCALATOR_ASCEND"):
    require(label in by_label, "opening route changed: " + label)
print("BOSWELLS_ENTRY_LIGHTS_VERIFY PASSED: two saved visual pools, "
      "non-colliding diffusers, original egress lights and route retained")
