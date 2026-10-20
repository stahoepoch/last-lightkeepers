--!strict
-- Single runtime entry point for balance values.
--
-- The numbers themselves live in /data as plain JSON, which Rojo syncs in as
-- ModuleScripts under ReplicatedStorage.Balance. Nothing in this project should
-- hard-code a tunable: change the JSON, not the gameplay code. That keeps the
-- balance data readable by a desktop engine later, per the handoff's Steam path.

local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Balance = ReplicatedStorage:WaitForChild("Balance")

local StormCharge = require(Balance:WaitForChild("storm-charge"))
local Loot = require(Balance:WaitForChild("loot"))
local Enemies = require(Balance:WaitForChild("enemies"))
local Lantern = require(Balance:WaitForChild("lantern"))

local Config = {
	StormCharge = StormCharge,
	Loot = Loot,
	Enemies = Enemies,
	Lantern = Lantern,
}

-- Session shape, from the handoff's product position: 6-10 minutes door-to-door.
Config.Run = {
	targetMinSeconds = 6 * 60,
	targetMaxSeconds = 10 * 60,
	maxCrewSize = 4,
}

--- Storm Charge rate for a given number of active lanterns.
-- Darkness still climbs; light only accelerates it. The meter never falls.
function Config.stormRate(activeLanterns: number): number
	local lantern = StormCharge.lantern
	local multiplier = 1 + lantern.perLanternBonus * math.max(activeLanterns, 0)
	multiplier = math.min(multiplier, lantern.maxMultiplier)
	return StormCharge.baselineRatePerSecond * multiplier
end

--- Highest threshold crossed at a given charge value, or nil.
function Config.thresholdAt(charge: number)
	local crossed = nil
	for _, threshold in ipairs(StormCharge.thresholds) do
		if charge >= threshold.value then
			crossed = threshold
		end
	end
	return crossed
end

--- Fraction of a full lantern charge consumed per second of light.
function Config.lanternDrainPerSecond(): number
	local seconds = Config.Lantern.charge.secondsOfLightPerFullCharge
	if seconds <= 0 then
		return 0
	end
	return 1 / seconds
end

function Config.wardenCharge(): number
	for _, threshold in ipairs(StormCharge.thresholds) do
		if threshold.id == "WardenConverges" then
			return threshold.value
		end
	end
	error("storm-charge.json is missing the WardenConverges threshold")
end

return Config
