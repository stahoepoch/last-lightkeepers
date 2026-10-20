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
	-- Sweeper contact accumulates; decays if they break away. See lantern.json.
	contactHits: number,
	lastContactAt: number?,
	-- Set while a teammate is holding a revive on this player.
	reviveProgress: number,
	reviveBy: number?,
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
	-- Lets the HUD tell "the run you were watching" from "a fresh one", so a
	-- frozen run-end summary clears itself when the next expedition starts.
	runId: string,
	stormCharge: number,
	stormRate: number,
	activeLanterns: number,
	highestThreshold: ThresholdId?,
	wardenActive: boolean,
	-- Per-viewer fields. Sent with FireClient, not broadcast, so one player's
	-- charge and state never leak into another's HUD.
	yourCharge: number,
	yourLanternOn: boolean,
	yourKnockedDown: boolean,
	yourTagged: boolean,
	-- Why the Anchor Revive is or is not offered. The client used to suppress
	-- the keybind with no way to say why, which read as a dead key.
	yourAnchorRevivesLeft: number,
	yourCanAnchorRevive: boolean,
	crewDown: number,
	crewTotal: number,
}

return nil
