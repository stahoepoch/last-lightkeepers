--!strict
-- Loot spawning, visibility, pickup, and extraction payout.
--
-- Two rules from the handoff are enforced here and must not be relaxed:
--   1. Baseline loot is always visible. If light revealed everything, darkness
--      would stop being a viable decision.
--   2. Relics are revealed by lantern light only - that is the reason to light up.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Remotes = require(Shared:WaitForChild("Remotes"))
local Types = require(Shared:WaitForChild("Types"))

local RunStateService = require(script.Parent:WaitForChild("RunStateService"))

local TAG_LOOT = "Loot"
local TAG_RELAY = "RelayBank"
local TAG_EXTRACTION = "Extraction"

local PICKUP_RANGE = 10

local LootService = {}

local function tierOf(instance: Instance): Types.LootTier?
	local value = instance:GetAttribute("Tier")
	if typeof(value) ~= "string" then
		return nil
	end
	if Config.Loot.tiers[value] == nil then
		return nil
	end
	return value :: any
end

local function withinRange(player: Player, instance: Instance): boolean
	local character = player.Character
	local root = character and character:FindFirstChild("HumanoidRootPart")
	if not root or not instance:IsA("BasePart") then
		return false
	end
	return ((root :: BasePart).Position - (instance :: BasePart).Position).Magnitude <= PICKUP_RANGE
end

--- Relics only exist for a player whose own lantern is lit.
function LootService.isVisibleTo(instance: Instance, state: Types.LightkeeperState): boolean
	local tier = tierOf(instance)
	if not tier then
		return false
	end
	local def = Config.Loot.tiers[tier]
	if def.visibility == "lanternOnly" then
		return state.lanternOn and state.lanternCharge > 0
	end
	return true
end

function LootService.handlePickup(player: Player, instance: Instance)
	local run = RunStateService.current()
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	if not instance or not instance:IsDescendantOf(workspace) then
		return
	end
	if not CollectionService:HasTag(instance, TAG_LOOT) then
		return
	end
	if not withinRange(player, instance) then
		return
	end

	local tier = tierOf(instance)
	if not tier or not LootService.isVisibleTo(instance, state) then
		return
	end

	if tier == "RestorationCrates" then
		if state.carryingCrate then
			return
		end
		state.carryingCrate = true
	end

	local amount = instance:GetAttribute("Amount")
	amount = if typeof(amount) == "number" then math.clamp(amount, 1, 99) else 1

	state.haul.carried[tier] = (state.haul.carried[tier] or 0) + amount
	instance:Destroy()
end

function LootService.handleBank(player: Player)
	RunStateService.bank(player)
end

function LootService.handleExtraction(player: Player, extractionId: string)
	local run = RunStateService.current()
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end

	local def = Config.Loot.extraction[extractionId]
	if not def then
		return
	end

	-- The freight lift's 20-second arrival is the cost of the deeper exit.
	if def.arrivalSeconds > 0 then
		task.wait(def.arrivalSeconds)
		if run.finished or state.knockedDown then
			return
		end
	end

	local payout = RunStateService.resolveHaul(state, true)
	for tier, amount in pairs(payout) do
		payout[tier] = math.floor(amount * def.payoutModifier)
	end

	Remotes.get("RunEnded"):FireClient(player, {
		outcome = "extracted",
		via = extractionId,
		payout = payout,
	})

	LootService.onExtracted(player, payout)
end

LootService.onExtracted = function(_player: Player, _payout: any) end

function LootService.init()
	Remotes.get("LootRequest").OnServerEvent:Connect(function(player, instance)
		if typeof(instance) == "Instance" then
			LootService.handlePickup(player, instance)
		end
	end)

	Remotes.get("ExtractionRequest").OnServerEvent:Connect(function(player, extractionId)
		if type(extractionId) == "string" then
			LootService.handleExtraction(player, extractionId)
		end
	end)

	-- Relay banks: stepping into a tagged zone banks whatever you carry.
	for _, relay in ipairs(CollectionService:GetTagged(TAG_RELAY)) do
		if relay:IsA("BasePart") then
			relay.Touched:Connect(function(hit)
				local player = Players:GetPlayerFromCharacter(hit.Parent)
				if player then
					LootService.handleBank(player)
				end
			end)
		end
	end

	-- Extraction volumes are driven by the client's ExtractionRequest (the player
	-- chooses to leave), so they only need to exist and carry an ExtractionId.
	for _, exit in ipairs(CollectionService:GetTagged(TAG_EXTRACTION)) do
		if exit:GetAttribute("ExtractionId") == nil then
			warn(`[LootService] extraction "{exit:GetFullName()}" has no ExtractionId attribute`)
		end
	end
end

return LootService
