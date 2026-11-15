"""Read-only inventory for the lower public entrance and sales desks."""

import json
from pathlib import Path

import unreal


world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/LLK_Boswells_ApparelPass")
if not world or world.get_name() != "LLK_Boswells_ApparelPass":
    raise RuntimeError("wrong map")
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
records = []
for actor in subsystem.get_all_level_actors():
    center, extent = actor.get_actor_bounds(False)
    if (center.y - extent.y > -2050 or center.y + extent.y < -3100
            or center.z - extent.z > 350 or center.z + extent.z < 0):
        continue
    if extent.x > 4500 or extent.y > 3000:
        continue
    records.append({
        "label": actor.get_actor_label(),
        "class": actor.get_class().get_name(),
        "bounds": [round(center.x - extent.x, 1), round(center.x + extent.x, 1),
                   round(center.y - extent.y, 1), round(center.y + extent.y, 1),
                   round(center.z - extent.z, 1), round(center.z + extent.z, 1)],
    })
meshes = {}
for name in ("SM_BSW_ServiceCounter", "SM_BSW_WallMerchBay", "SM_BSW_FoldedGoodsTable"):
    mesh = unreal.EditorAssetLibrary.load_asset("/Game/Props/Boswells/" + name)
    if isinstance(mesh, unreal.StaticMesh):
        extent = mesh.get_bounds().box_extent
        meshes[name] = {
            "extent": [extent.x, extent.y, extent.z],
            "materials": [str(item.material_interface.get_path_name())
                          if item.material_interface else None
                          for item in mesh.get_editor_property("static_materials")],
        }
report = {"actors": sorted(records, key=lambda r: r["label"]), "meshes": meshes}
path = Path(unreal.Paths.project_saved_dir()) / "Temp/audit_boswells_arrival.json"
path.parent.mkdir(parents=True, exist_ok=True)
path.write_text(json.dumps(report, indent=2), encoding="utf-8")
print("BOSWELLS_ARRIVAL_AUDIT wrote {} actors to {}".format(len(records), path))
