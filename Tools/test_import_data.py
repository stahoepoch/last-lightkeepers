#!/usr/bin/env python3
"""Black-box checks proving the data importer goes red for contract drift."""
from __future__ import annotations

import json
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
    must_fail("unknown gameplay keys are errors", "lantern.json", lambda value: value["movement"].__setitem__("sprintSped", 99), "unrecognised key(s): sprintSped")
    must_fail("missing gameplay keys are errors", "storm-charge.json", lambda value: value["warden"].pop("effect"), "missing required key(s): effect")
    must_fail("unknown enums are errors", "storm-charge.json", lambda value: value["lantern"].__setitem__("curve", "freeLight"), "unknown enum value")
    must_fail("enemy family shapes are errors", "enemies.json", lambda value: value["Warden"].__setitem__("health", 1), "unrecognised key(s): health")
    print("All importer contract tests passed.")
    return 0

if __name__ == "__main__": raise SystemExit(main())
