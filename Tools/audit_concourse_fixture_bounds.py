"""Read only: measure fixture asset bounds before placing concourse stock."""

import unreal

ROOT = "/Game/LLK/Boswells/Fixtures/"
NAMES = ("SM_BSW_ClothingRun", "SM_BSW_RoundRack", "SM_BSW_FourWay",
         "SM_BSW_Department_ToyGondola_A", "SM_BSW_Department_FeatureTable_A",
         "SM_BSW_Department_ShoeDisplay_Short_A",
         "SM_BSW_Department_HomeDisplay_A",
         "SM_BSW_Department_SofaVignette_A")
for name in NAMES:
    mesh = unreal.EditorAssetLibrary.load_asset(ROOT + name)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError("missing mesh " + name)
    box = mesh.get_bounds()
    print(f"CONCOURSE_BOUND {name} origin={box.origin} extent={box.box_extent}")
