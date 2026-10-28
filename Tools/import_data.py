#!/usr/bin/env python3
"""Strict, deterministic front end for the portable balance data.

This validates the JSON-to-Unreal contract without reimplementing balance
rules (the Node suite owns those).  It deliberately strips underscore-prefixed
analysis keys, reports them, and treats every other unknown or missing key as
a hard error.  ``--emit`` writes canonical payloads for the editor-side asset
writer; runtime USTRUCTs remain owned by Source/LLK.
"""
from __future__ import annotations

import argparse
import json
import shutil
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DATA = ROOT / "data"
DEFAULT_OUTPUT = ROOT / "Intermediate" / "LLKDataImport"

# The editor writer consumes these payloads, never data/*.json directly.  Keep
# this manifest text-only and deterministic: it is the seam between the
# portable validator and Source/LLKEditor, not a second source of balance.
ASSET_MANIFEST = {
    "DataTables": [
        {"asset": "DT_Enemies", "rowStruct": "FLLKEnemyDef", "source": "enemies.json", "field": None, "rows": "object"},
        {"asset": "DT_LootTiers", "rowStruct": "FLLKLootTierDef", "source": "loot.json", "field": "tiers", "rows": "object"},
        {"asset": "DT_LedgerRooms", "rowStruct": "FLLKRoomDef", "source": "ledger.json", "field": "rooms", "rows": "id-array"},
        {"asset": "DT_LedgerRungs", "rowStruct": "FLLKRungDef", "source": "ledger.json", "field": "rungs", "rows": "id-array"},
        {"asset": "DT_Relics", "rowStruct": "FLLKRelicDef", "source": "relics.json", "field": "relics", "rows": "id-array"},
        {"asset": "DT_WorkshopUpgrades", "rowStruct": "FLLKUpgradeDef", "source": "workshop.json", "field": "upgrades", "rows": "id-array"},
        {"asset": "DT_TownLandmarks", "rowStruct": "FLLKLandmarkDef", "source": "town.json", "field": "landmarks", "rows": "id-array"},
    ],
    "DataAssets": [
        {"asset": "DA_StormCharge", "class": "ULLKStormChargeData", "source": "storm-charge.json", "exclude": [], "derivedFields": {"thresholds": ["darkSeconds", "regularUseSeconds", "oneLanternSeconds"]}},
        {"asset": "DA_Lantern", "class": "ULLKLanternData", "source": "lantern.json", "exclude": []},
        {"asset": "DA_LootRules", "class": "ULLKLootRulesData", "source": "loot.json", "exclude": ["tiers"], "keyedObjectFields": {"extraction": "key-to-id-array"}},
        {"asset": "DA_WorkshopRules", "class": "ULLKWorkshopRulesData", "source": "workshop.json", "exclude": ["upgrades"]},
    ],
}

class ContractError(Exception): pass

def error(path: str, message: str) -> None: raise ContractError(f"{path}: {message}")
def obj(value: Any, path: str) -> dict[str, Any]:
    if not isinstance(value, dict): error(path, f"expected object, got {type(value).__name__}")
    return value
def array(value: Any, path: str) -> list[Any]:
    if not isinstance(value, list): error(path, f"expected array, got {type(value).__name__}")
    return value
def scalar(value: Any, expected: type | tuple[type, ...], path: str) -> None:
    if isinstance(value, bool) and expected in (int, float, (int, float)): error(path, "expected number, got bool")
    if not isinstance(value, expected):
        label = "/".join(t.__name__ for t in expected) if isinstance(expected, tuple) else expected.__name__
        error(path, f"expected {label}, got {type(value).__name__}")
def exact(row: dict[str, Any], required: set[str], path: str, optional: set[str] | None = None) -> None:
    optional = optional or set()
    missing, unknown = sorted(required - row.keys()), sorted(row.keys() - required - optional)
    if missing: error(path, f"missing required key(s): {', '.join(missing)}")
    if unknown: error(path, f"unrecognised key(s): {', '.join(unknown)}")
def fields(row: dict[str, Any], spec: dict[str, type | tuple[type, ...]], path: str) -> None:
    for key, expected in spec.items(): scalar(row[key], expected, f"{path}.{key}")
def strip(value: Any, stripped: list[int]) -> Any:
    if isinstance(value, dict):
        result = {}
        for key, child in value.items():
            if key.startswith("_"): stripped[0] += 1
            else: result[key] = strip(child, stripped)
        return result
    if isinstance(value, list): return [strip(child, stripped) for child in value]
    return value

ENEMY_COMMON = {"formerJob": str, "behaviour": str, "counterplay": str, "moveSpeed": (int, float), "audioTelegraphSeconds": (int, float)}
ENEMY_FAMILIES = {
    "Sweepers": {"health": (int, float), "aggroRadiusDark": (int, float), "aggroRadiusLit": (int, float), "swarmCallRadius": (int, float), "attractedToDroppedLoot": bool},
    "Stockers": {"health": (int, float), "sightRange": (int, float), "sightConeDegrees": (int, float), "losesTargetAfterSeconds": (int, float)},
    "Glimmers": {"health": (int, float), "scanRange": (int, float), "scanSeconds": (int, float), "tagSeconds": (int, float), "tagRevealsPositionToAllRobots": bool, "tagAddsStormCharge": (int, float), "tagGivesWardenPermanentWaypoint": bool, "scanRequiresActiveLantern": bool},
    "WindowWalkers": {"health": (int, float), "wakeRange": (int, float), "sightRange": (int, float), "wakeRequiresActiveLantern": bool},
    "Ficus": {"health": (int, float), "wakeRange": (int, float), "sightRange": (int, float), "wakeRequiresActiveLantern": bool},
    "Warden": {"killable": bool, "dropsLoot": bool, "spawnStage": str, "retargetIntervalSeconds": (int, float)},
}

def validate_enemies(data: Any) -> None:
    root = obj(data, "enemies.json"); exact(root, set(ENEMY_FAMILIES), "enemies.json")
    for name, special in ENEMY_FAMILIES.items():
        row = obj(root[name], f"enemies.json.{name}"); spec = ENEMY_COMMON | special
        exact(row, set(spec), f"enemies.json.{name}"); fields(row, spec, f"enemies.json.{name}")

def pair(value: Any, path: str) -> None:
    items = array(value, path)
    if len(items) != 2: error(path, "must contain exactly [low, high]")
    for index, item in enumerate(items): scalar(item, (int, float), f"{path}[{index}]")

def validate_storm(data: Any) -> None:
    root = obj(data, "storm-charge.json")
    exact(root, {"baselineRatePerSecond", "lantern", "run", "extractionCall", "thresholds", "warden", "anchorRevive"}, "storm-charge.json")
    scalar(root["baselineRatePerSecond"], (int, float), "storm-charge.json.baselineRatePerSecond")
    lantern = obj(root["lantern"], "storm-charge.json.lantern"); lantern_spec = {"perLanternBonus": (int, float), "maxMultiplier": (int, float), "curve": str, "diminishingDecay": (int, float)}
    exact(lantern, set(lantern_spec), "storm-charge.json.lantern"); fields(lantern, lantern_spec, "storm-charge.json.lantern")
    if lantern["curve"] not in {"linearCapped", "diminishing"}: error("storm-charge.json.lantern.curve", f"unknown enum value {lantern['curve']!r}")
    run = obj(root["run"], "storm-charge.json.run"); run_spec = {"earlyExitSeconds": (int, float), "targetSeconds": (int, float), "hardCeilingSeconds": (int, float)}
    exact(run, set(run_spec) | {"targetBandSeconds"}, "storm-charge.json.run"); fields(run, run_spec, "storm-charge.json.run"); pair(run["targetBandSeconds"], "storm-charge.json.run.targetBandSeconds")
    call = obj(root["extractionCall"], "storm-charge.json.extractionCall"); call_spec = {"stormChargeSpike": (int, float), "convergeAllPatrols": bool, "sirenSeconds": (int, float)}
    exact(call, set(call_spec), "storm-charge.json.extractionCall"); fields(call, call_spec, "storm-charge.json.extractionCall")
    threshold_spec = {"id": str, "value": (int, float), "darkSeconds": (int, float), "regularUseSeconds": (int, float), "oneLanternSeconds": (int, float), "effect": str}
    for index, row in enumerate(array(root["thresholds"], "storm-charge.json.thresholds")):
        path = f"storm-charge.json.thresholds[{index}]"; row = obj(row, path); exact(row, set(threshold_spec), path); fields(row, threshold_spec, path)
    warden = obj(root["warden"], "storm-charge.json.warden"); warden_spec = {"chargeValue": (int, float), "lightSensitivity": (int, float), "effect": str}
    exact(warden, set(warden_spec) | {"arrivalBandSeconds"}, "storm-charge.json.warden"); fields(warden, warden_spec, "storm-charge.json.warden"); pair(warden["arrivalBandSeconds"], "storm-charge.json.warden.arrivalBandSeconds")
    revive = obj(root["anchorRevive"], "storm-charge.json.anchorRevive"); revive_spec = {"chargesPerRun": (int, float), "reviveSeconds": (int, float), "consumesFullLanternCharge": bool, "stormChargeSpike": (int, float), "soloOnly": bool}
    exact(revive, set(revive_spec), "storm-charge.json.anchorRevive"); fields(revive, revive_spec, "storm-charge.json.anchorRevive")

def validate_lantern(data: Any) -> None:
    root = obj(data, "lantern.json")
    specs = {
        "charge": {"secondsOfLightPerFullCharge": (int, float), "autoOffAtEmpty": bool, "rechargeAtRelay": bool},
        "knockdown": {"downedWalkSpeed": (int, float), "lanternForcedOffWhenDowned": bool, "dropsCarriedCrate": bool, "bleedOutSeconds": (int, float)},
        "revive": {"teammateHoldSeconds": (int, float), "maxReviveDistance": (int, float)},
        "sweeperContact": {"knockdownOnContact": bool, "contactRange": (int, float), "contactCooldownSeconds": (int, float), "hitsToKnockDown": (int, float), "hitDecaySeconds": (int, float)},
        "movement": {"walkSpeed": (int, float), "sprintSpeed": (int, float), "crouchSpeed": (int, float)},
    }
    exact(root, set(specs), "lantern.json")
    for name, spec in specs.items():
        block = obj(root[name], f"lantern.json.{name}"); exact(block, set(spec), f"lantern.json.{name}"); fields(block, spec, f"lantern.json.{name}")

def validate_loot(data: Any) -> set[str]:
    root = obj(data, "loot.json"); exact(root, {"rules", "tiers", "burden", "banking", "extraction"}, "loot.json")
    rules = obj(root["rules"], "loot.json.rules"); rule_spec = {"baselineLootAlwaysVisible": bool, "relicsRequireLanternLight": bool, "restorationCratesArePocketable": bool}; exact(rules, set(rule_spec), "loot.json.rules"); fields(rules, rule_spec, "loot.json.rules")
    tiers = obj(root["tiers"], "loot.json.tiers")
    if not tiers: error("loot.json.tiers", "must not be empty")
    for name, row in tiers.items():
        path = f"loot.json.tiers.{name}"; row = obj(row, path); required = {"examples", "visibility", "feeds", "carrySlots", "pickupSeconds", "burden"}; optional = {"sourceHint", "blocksCombatWhileCarried"}
        exact(row, required, path, optional); array(row["examples"], f"{path}.examples")
        for index, value in enumerate(row["examples"]): scalar(value, str, f"{path}.examples[{index}]")
        if not isinstance(row["feeds"], (str, list)): error(f"{path}.feeds", "expected string or string array")
        if isinstance(row["feeds"], list):
            for index, value in enumerate(row["feeds"]): scalar(value, str, f"{path}.feeds[{index}]")
        fields(row, {"visibility": str, "carrySlots": (int, float), "pickupSeconds": (int, float), "burden": (int, float)}, path)
        if "sourceHint" in row: scalar(row["sourceHint"], str, f"{path}.sourceHint")
        if "blocksCombatWhileCarried" in row: scalar(row["blocksCombatWhileCarried"], bool, f"{path}.blocksCombatWhileCarried")
    for name, spec in {"burden": {"speedPenaltyPerPoint": (int, float), "maxSpeedPenalty": (int, float)}, "banking": {"relayBanksPerRun": (int, float), "bankedKeptOnWipe": (int, float), "unbankedKeptOnWipe": (int, float), "disconnectUsesSameRule": bool}}.items():
        block = obj(root[name], f"loot.json.{name}"); exact(block, set(spec), f"loot.json.{name}"); fields(block, spec, f"loot.json.{name}")
    exits = obj(root["extraction"], "loot.json.extraction")
    for name, row in exits.items():
        path = f"loot.json.extraction.{name}"; row = obj(row, path); spec = {"reliability": str, "payoutModifier": (int, float), "arrivalSeconds": (int, float), "note": str}; exact(row, set(spec), path, {"requires"}); fields(row, spec, path)
        if "requires" in row: scalar(row["requires"], str, f"{path}.requires")
    return set(tiers)

def validate_relics(data: Any) -> None:
    root = obj(data, "relics.json"); exact(root, {"relics"}, "relics.json")
    spec = {"id": str, "name": str, "found": str, "text": str}
    for index, row in enumerate(array(root["relics"], "relics.json.relics")):
        path = f"relics.json.relics[{index}]"; row = obj(row, path); exact(row, set(spec), path); fields(row, spec, path)

def validate_workshop(data: Any, tiers: set[str]) -> None:
    root = obj(data, "workshop.json"); exact(root, {"currency", "upgrades"}, "workshop.json")
    for index, tier in enumerate(array(root["currency"], "workshop.json.currency")):
        scalar(tier, str, f"workshop.json.currency[{index}]")
        if tier not in tiers: error(f"workshop.json.currency[{index}]", f"unknown loot tier {tier!r}")
    for index, row in enumerate(array(root["upgrades"], "workshop.json.upgrades")):
        path = f"workshop.json.upgrades[{index}]"; row = obj(row, path); exact(row, {"id", "title", "blurb", "cost", "effect"}, path, {"requires"}); fields(row, {"id": str, "title": str, "blurb": str}, path)
        if "requires" in row: scalar(row["requires"], str, f"{path}.requires")
        cost = obj(row["cost"], f"{path}.cost")
        if not cost: error(f"{path}.cost", "must not be empty")
        for name, value in cost.items():
            if name not in root["currency"]: error(f"{path}.cost.{name}", "must name a workshop currency")
            scalar(value, (int, float), f"{path}.cost.{name}")
        effect = obj(row["effect"], f"{path}.effect")
        exact(effect, {"relicBurdenMultiplier"}, f"{path}.effect")
        scalar(effect["relicBurdenMultiplier"], (int, float), f"{path}.effect.relicBurdenMultiplier")

def validate_town(data: Any) -> None:
    root = obj(data, "town.json"); exact(root, {"landmarks"}, "town.json"); spec = {"id": str, "title": str, "crates": (int, float), "restored": str}
    for index, row in enumerate(array(root["landmarks"], "town.json.landmarks")):
        path = f"town.json.landmarks[{index}]"; row = obj(row, path); exact(row, set(spec), path); fields(row, spec, path)

def validate_ledger(data: Any) -> None:
    root = obj(data, "ledger.json"); exact(root, {"rooms", "rungs"}, "ledger.json")
    room_spec = {"id": str, "room": str, "crates": (int, float), "sealed": str, "restored": str, "rung": str}
    for index, row in enumerate(array(root["rooms"], "ledger.json.rooms")):
        path = f"ledger.json.rooms[{index}]"; row = obj(row, path); exact(row, set(room_spec), path); fields(row, room_spec, path)
    rung_spec = {"id": str, "title": str, "job": str, "opens": str, "wakes": str}
    for index, row in enumerate(array(root["rungs"], "ledger.json.rungs")):
        path = f"ledger.json.rungs[{index}]"; row = obj(row, path); exact(row, set(rung_spec), path, {"restoration"}); fields(row, rung_spec, path)
        if "restoration" not in row: continue
        restoration = obj(row["restoration"], f"{path}.restoration"); exact(restoration, {"requiresRoom", "salvage", "wakesWithoutLantern", "departure"}, f"{path}.restoration")
        scalar(restoration["requiresRoom"], str, f"{path}.restoration.requiresRoom")
        for key, value in obj(restoration["salvage"], f"{path}.restoration.salvage").items(): scalar(value, (int, float), f"{path}.restoration.salvage.{key}")
        for child_index, child in enumerate(array(restoration["wakesWithoutLantern"], f"{path}.restoration.wakesWithoutLantern")): scalar(child, str, f"{path}.restoration.wakesWithoutLantern[{child_index}]")
        departure = obj(restoration["departure"], f"{path}.restoration.departure"); dep_spec = {"unpoweredSeconds": (int, float), "poweredSeconds": (int, float), "unpoweredFalseStartChance": (int, float)}; exact(departure, set(dep_spec), f"{path}.restoration.departure"); fields(departure, dep_spec, f"{path}.restoration.departure")

VALIDATORS = {"enemies.json": validate_enemies, "storm-charge.json": validate_storm, "lantern.json": validate_lantern, "loot.json": validate_loot, "relics.json": validate_relics, "town.json": validate_town, "ledger.json": validate_ledger}
CONTRACT_FILES = ("enemies.json", "storm-charge.json", "lantern.json", "loot.json", "relics.json", "workshop.json", "town.json", "ledger.json")

def load(data_dir: Path) -> tuple[dict[str, Any], dict[str, int]]:
    clean, reports = {}, {}
    # Rule 7: the file set is closed, just as rule 2 closes each object's key
    # set. A new JSON source must arrive with an explicit importer contract;
    # otherwise it is more dangerous to ignore than to stop the build.
    actual_files = {path.name for path in data_dir.glob("*.json")}
    expected_files = set(CONTRACT_FILES)
    unexpected_files = sorted(actual_files - expected_files)
    if unexpected_files:
        error("data", f"unrecognised contract file(s): {', '.join(unexpected_files)}")
    missing_files = sorted(expected_files - actual_files)
    if missing_files:
        error("data", f"missing contract file(s): {', '.join(missing_files)}")
    for filename in CONTRACT_FILES:
        path = data_dir / filename
        try: raw = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc: error(filename, f"invalid JSON: {exc.msg} at line {exc.lineno}, column {exc.colno}")
        count = [0]; clean[filename] = strip(raw, count); reports[filename] = count[0]
    tiers = validate_loot(clean["loot.json"])
    for filename, validator in VALIDATORS.items():
        if filename != "loot.json": validator(clean[filename])
    validate_workshop(clean["workshop.json"], tiers)
    return clean, reports

def emit(files: dict[str, Any], output: Path) -> None:
    staging = output.with_name(output.name + ".staging")
    if staging.exists(): shutil.rmtree(staging)
    staging.mkdir(parents=True)
    for filename, value in files.items(): (staging / filename).write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    (staging / "asset-manifest.json").write_text(json.dumps(ASSET_MANIFEST, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if output.exists(): shutil.rmtree(output)
    staging.replace(output)

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument("--data-dir", type=Path, default=DEFAULT_DATA); parser.add_argument("--emit", action="store_true"); parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT); args = parser.parse_args()
    try: files, reports = load(args.data_dir)
    except ContractError as exc:
        print(f"DATA CONTRACT ERROR: {exc}", file=sys.stderr); return 1
    for filename, count in reports.items(): print(f"{filename}: stripped {count} underscore-prefixed analysis key(s)")
    if args.emit: emit(files, args.output); print(f"Wrote {len(files)} canonical payloads to {args.output}")
    print("Data contract passed."); return 0
if __name__ == "__main__": raise SystemExit(main())
