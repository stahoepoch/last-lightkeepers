--!strict
-- Shared type definitions. Kept free of Roblox instance types where possible so
-- the shapes survive a future port to a desktop engine.

export type LootTier = "Supplies" | "Components" | "Relics" | "RestorationCrates"

export type ExtractionId = "LoadingDock" | "FreightLift"

export type ThresholdId = "PatrolsAlert" | "PatrolsHunt" | "WardenConverges"

export type EnemyKind = "Sweepers" | "Stockers" | "Glimmers" | "Warden"

-- A player's loot for the current run, split by whether it survives a wipe.
export type Haul = {
	carried: { [LootTier]: number },
	banked: { [LootTier]: number },
}

export type LightkeeperState = {
	userId: number,
	lanternOn: boolean,
	lanternCharge: number, -- 0..1
	knockedDown: boolean,
	anchorRevivesLeft: number,
	taggedUntil: number?, -- os.clock() deadline set by a completed Glimmer scan
	carryingCrate: boolean,
	haul: Haul,
}

export type RunState = {
	runId: string,
	startedAt: number,
	stormCharge: number,
	crossedThresholds: { [ThresholdId]: boolean },
	lightkeepers: { [number]: LightkeeperState },
	finished: boolean,
}

-- Snapshot pushed to clients for the HUD. Deliberately small: this replicates often.
export type RunSnapshot = {
	stormCharge: number,
	stormRate: number,
	activeLanterns: number,
	highestThreshold: ThresholdId?,
	wardenActive: boolean,
}

return nil
