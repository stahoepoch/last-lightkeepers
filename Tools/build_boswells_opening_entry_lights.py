"""Fill the two dark entry-route gaps in the saved opening map.

These are visual-only emergency ceiling pools. They illuminate the reserved
front promenade, leave the cases unpowered, and do not join LLKLightField.
The exposure lock, existing lights, and lantern are untouched. Judge the
result in a normal rendered play session; geometry checks cannot prove feel.
"""

import unreal

MAP = "/Game/LLK_Boswells_OpeningPass"
PREFIX = "BSW_OPENING_ENTRY_POOL_"
POS = (("WEST", -1600), ("EAST", 1600))


def require(ok, why):
    if not ok:
        raise RuntimeError("BOSWELLS_ENTRY_LIGHTS " + why)


world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
require(world and world.get_name() == "LLK_Boswells_OpeningPass", "wrong map")
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = sub.get_all_level_actors()
by_label = {a.get_actor_label(): a for a in actors}
require(len(by_label) == len(actors), "duplicate actor labels")
require(not any(name.startswith(PREFIX) for name in by_label),
        "pool already present")
require(sum(name.startswith("BSW_EGRESS_") for name in by_label) == 31,
        "original egress light inventory changed")
for label in ("BSW_LOWER_CEILING_WEST_FIELD",
              "BSW_LOWER_CEILING_EAST_FIELD",
              "ANCHOR_PLAYERSTART_ENTRANCE", "BSW_OPENING_LANTERN",
              "ROUTE_MALL_GRILLE_22"):
    require(label in by_label, "opening geometry differs: " + label)
start = by_label["ANCHOR_PLAYERSTART_ENTRANCE"].get_actor_location()
require(abs(start.x - 1600) < 1 and abs(start.y + 2370) < 1,
        "spawn moved; re-plan entry pools")
cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
panel = unreal.EditorAssetLibrary.load_asset(
    "/Game/LLK/Boswells/Architecture/M_BSW_LowerCeilingTile")
require(isinstance(cube, unreal.StaticMesh) and
        isinstance(panel, unreal.MaterialInterface), "ceiling panel assets missing")

for side, x in POS:
    y = -2200
    label = PREFIX + side
    actor = sub.spawn_actor_from_class(
        unreal.SpotLight, unreal.Vector(x, y, 485),
        unreal.Rotator(pitch=-90, yaw=0, roll=0))
    require(actor is not None, "spot spawn failed: " + side)
    actor.set_actor_label(label)
    actor.set_folder_path("Boswells/Lighting/OpeningEntry")
    light = actor.spot_light_component
    # This prototype map has no cooked light build. Stationary lights report
    # unbuilt interactions in -game; keep these two route pools dynamic so
    # their contribution survives a fresh package load.
    light.set_mobility(unreal.ComponentMobility.MOVABLE)
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", 8.0)
    light.set_editor_property("attenuation_radius", 800.0)
    light.set_editor_property("inner_cone_angle", 20.0)
    light.set_editor_property("outer_cone_angle", 35.0)
    light.set_editor_property("source_radius", 40.0)
    light.set_editor_property("indirect_lighting_intensity", 0.25)
    light.set_editor_property("cast_shadows", False)

    # A dim fixture face in the suspended grid, with its light source 23 cm
    # below it. The panel is above headroom, non-colliding and visual-only.
    face = sub.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, 508), unreal.Rotator())
    require(face is not None, "panel spawn failed: " + side)
    face.set_actor_label(label + "_DIFFUSER")
    face.set_folder_path("Boswells/Lighting/OpeningEntry")
    comp = face.static_mesh_component
    require(comp.set_static_mesh(cube), "panel mesh failed: " + side)
    face.set_actor_scale3d(unreal.Vector(1.2, 0.6, 0.035))
    comp.set_material(0, panel)
    face.set_actor_enable_collision(False)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("can_ever_affect_navigation", False)

require(unreal.EditorLoadingAndSavingUtils.save_map(world, MAP), "map save failed")
print("BOSWELLS_ENTRY_LIGHTS SAVED: 2 visual-only 8 cd entry pools and "
      "2 suspended diffusers; old lighting and exposure untouched")
