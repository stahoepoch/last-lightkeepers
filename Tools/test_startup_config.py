#!/usr/bin/env python3
"""Fast, engine-free checks for the temporary playable harness configuration."""
from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ENGINE_CONFIG = ROOT / "Config" / "DefaultEngine.ini"
GAME_CONFIG = ROOT / "Config" / "DefaultGame.ini"
GAME_MODE_SECTION = "[/Script/EngineSettings.GameMapsSettings]"
GAME_MODE_LINE = "GlobalDefaultGameMode=/Script/LLK.LLKTestGameMode"


def fail(message: str) -> None:
    raise AssertionError(message)


def main() -> int:
    engine = ENGINE_CONFIG.read_text(encoding="utf-8-sig")
    game = GAME_CONFIG.read_text(encoding="utf-8-sig")

    if GAME_MODE_SECTION not in engine or GAME_MODE_LINE not in engine:
        fail(
            "the playable harness GameMode must live in Config/DefaultEngine.ini; "
            "UGameMapsSettings does not read DefaultGame.ini"
        )
    if GAME_MODE_SECTION in game or "GlobalDefaultGameMode=" in game:
        fail("DefaultGame.ini must not contain a GameMapsSettings GameMode override")
    if "SecurityToken=" in engine:
        fail("DefaultEngine.ini contains a machine-generated Android File Server token")

    print("PASS playable harness startup configuration")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
