#!/usr/bin/env python3
"""Black-box checks proving the data importer goes red for contract drift."""
from __future__ import annotations

import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "data"
IMPORTER = ROOT / "Tools" / "import_data.py"

def copy_data() -> Path:
    root = Path(tempfile.mkdtemp()) / "data"
    shutil.copytree(SOURCE, root)
    return root

def invoke(directory: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run([sys.executable, str(IMPORTER), "--data-dir", str(directory)], text=True, capture_output=True, check=False)

def modify(directory: Path, filename: str, change) -> None:
    path = directory / filename
    value = json.loads(path.read_text(encoding="utf-8"))
    change(value)
    path.write_text(json.dumps(value), encoding="utf-8")

def must_fail(label: str, filename: str, change, excerpt: str) -> None:
    directory = copy_data()
    modify(directory, filename, change)
    result = invoke(directory)
    if result.returncode == 0 or excerpt not in result.stderr:
        raise AssertionError(f"{label}: wanted {excerpt!r}; stdout={result.stdout!r}, stderr={result.stderr!r}")
    print(f"PASS {label}")

def main() -> int:
    result = invoke(SOURCE)
    if result.returncode:
        raise AssertionError(result.stderr)
    print("PASS valid data")
    manifest_dir = Path(tempfile.mkdtemp()) / "payloads"
    emitted = subprocess.run([sys.executable, str(IMPORTER), "--data-dir", str(SOURCE), "--emit", "--output", str(manifest_dir)], text=True, capture_output=True, check=False)
    if emitted.returncode:
        raise AssertionError(emitted.stderr)
    manifest = json.loads((manifest_dir / "asset-manifest.json").read_text(encoding="utf-8"))
    headers = "\n".join(path.read_text(encoding="utf-8") for path in (ROOT / "Source" / "LLK" / "Public" / "Data").glob("*.h"))
    expected_tables = {"DT_Enemies", "DT_LootTiers", "DT_LedgerRooms", "DT_LedgerRungs", "DT_Relics", "DT_WorkshopUpgrades", "DT_TownLandmarks"}
    expected_assets = {"DA_StormCharge", "DA_Lantern", "DA_LootRules", "DA_WorkshopRules"}
    if {item["asset"] for item in manifest["DataTables"]} != expected_tables or {item["asset"] for item in manifest["DataAssets"]} != expected_assets:
        raise AssertionError("asset manifest no longer names the eleven generated outputs")
    for item in manifest["DataTables"]:
        pattern = rf"USTRUCT\([^)]*\)\s*struct\s+{re.escape(item['rowStruct'])}\s*:"
        if not re.search(pattern, headers):
            raise AssertionError(f"manifest row struct is not a reflected DataTable declaration: {item['rowStruct']}")
    for item in manifest["DataAssets"]:
        pattern = rf"UCLASS\([^)]*\)\s*class\s+LLK_API\s+{re.escape(item['class'])}\s*:"
        if not re.search(pattern, headers):
            raise AssertionError(f"manifest DataAsset class is not a reflected declaration: {item['class']}")
    loot_asset = next(item for item in manifest["DataAssets"] if item["asset"] == "DA_LootRules")
    if loot_asset.get("keyedObjectFields", {}).get("extraction") != "key-to-id-array":
        raise AssertionError("loot extraction must promote object keys into FLLKExtractionPoint.Id")
    storm_asset = next(item for item in manifest["DataAssets"] if item["asset"] == "DA_StormCharge")
    if storm_asset.get("derivedFields", {}).get("thresholds") != ["darkSeconds", "regularUseSeconds", "oneLanternSeconds"]:
        raise AssertionError("storm threshold timing fields must remain writer-derived from value")
    unreal_enemy = json.loads((manifest_dir / manifest["unrealPayloadDirectory"] / "enemies.json").read_text(encoding="utf-8"))
    if "bKillable" not in unreal_enemy["Warden"] or "killable" in unreal_enemy["Warden"]:
        raise AssertionError("Unreal payload must use reflected bKillable, not human-facing killable")
    unreal_lantern = json.loads((manifest_dir / manifest["unrealPayloadDirectory"] / "lantern.json").read_text(encoding="utf-8"))
    if "bDropsCarriedCrate" not in unreal_lantern["knockdown"]:
        raise AssertionError("Unreal payload must use reflected boolean property names")
    print("PASS manifest matches the reflected Unreal contract")
    must_fail("unknown gameplay keys are errors", "lantern.json", lambda value: value["movement"].__setitem__("sprintSped", 99), "unrecognised key(s): sprintSped")
    directory = copy_data()
    directory.joinpath("keepers.json").write_text("{}", encoding="utf-8")
    result = invoke(directory)
    if result.returncode == 0 or "unrecognised contract file(s): keepers.json" not in result.stderr:
        raise AssertionError(f"unknown data file: stdout={result.stdout!r}, stderr={result.stderr!r}")
    print("PASS unknown data files are errors")
    must_fail("missing gameplay keys are errors", "storm-charge.json", lambda value: value["warden"].pop("effect"), "missing required key(s): effect")
    must_fail("unknown enums are errors", "storm-charge.json", lambda value: value["lantern"].__setitem__("curve", "freeLight"), "unknown enum value")
    must_fail("enemy family shapes are errors", "enemies.json", lambda value: value["Warden"].__setitem__("health", 1), "unrecognised key(s): health")
    print("All importer contract tests passed.")
    return 0

if __name__ == "__main__": raise SystemExit(main())
