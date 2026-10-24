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
local Workshop = require(Balance:WaitForChild("workshop"))
local Town = require(Balance:WaitForChild("town"))

local Config = {
	StormCharge = StormCharge,
	Loot = Loot,
	Enemies = Enemies,
	Lantern = Lantern,
	Workshop = Workshop,
	Town = Town,
}

-- Session shape, from the handoff's product position: 6-10 minutes door-to-door.
Config.Run = {
	targetMinSeconds = 6 * 60,
	-- A raid may finish early through extraction, but can never run forever.
	-- Twenty minutes is the hard ceiling Chris chose for a deep Galleria push.
	targetMaxSeconds = 20 * 60,
	maxCrewSize = 4,
}

--- How much faster the storm climbs with N lanterns lit.
--
-- Two shapes, chosen by `curve` in storm-charge.json:
--
--   linearCapped (default) - 1 + bonus*n, hard-clamped. Simple, but the clamp
--     flattens crew size: three lanterns (5.5x) and four (7x) both clamp to
--     5.0, so the fourth player's light costs the crew nothing.
--
--   diminishing - a geometric series. Each extra lantern adds less than the
--     last but never nothing, so no player's light is free. It preserves the
--     locked 2.5x one-lantern anchor exactly and self-limits, needing no clamp.
--
-- The default is unchanged; switching the curve is a balance decision.
function Config.lanternMultiplier(activeLanterns: number): number
	local lantern = StormCharge.lantern
	local n = math.max(activeLanterns, 0)
	if n == 0 then
		return 1
	end

	if lantern.curve == "diminishing" then
		local decay = lantern.diminishingDecay
		-- Sum of bonus * decay^(i-1) for i = 1..n.
		local series = if decay == 1 then n else (1 - decay ^ n) / (1 - decay)
		return 1 + lantern.perLanternBonus * series
	end

	return math.min(1 + lantern.perLanternBonus * n, lantern.maxMultiplier)
end

--- Storm Charge rate for a given number of active lanterns.
-- Darkness still climbs; light only accelerates it. The meter never falls.
function Config.stormRate(activeLanterns: number): number
	return StormCharge.baselineRatePerSecond * Config.lanternMultiplier(activeLanterns)
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
