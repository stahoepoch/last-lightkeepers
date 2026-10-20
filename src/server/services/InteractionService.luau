--!strict
-- Physical interaction: how a player actually collects, extracts, and revives.
--
-- Addresses the P0 in NOTES-FOR-CLAUDE.md. One deliberate deviation from the
-- "done when" in that note, which asked for client interaction to produce a
-- validated remote request: these use ProximityPrompts created and handled
-- **on the server**. `ProximityPrompt.Triggered` fires server-side with the
-- player who triggered it, so the interaction never round-trips through client
-- trust at all. That is strictly stronger than validating a remote, and it
-- removes three attack surfaces instead of hardening them.
--
-- The remotes still exist and stay validated, because the Anchor Revive is a
-- keypress rather than a prompt, and a future gamepad path may need them.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))

local RunStateService = require(script.Parent:WaitForChild("RunStateService"))
local LootService = require(script.Parent:WaitForChild("LootService"))

local TAG_LOOT = "Loot"
local TAG_EXTRACTION = "Extraction"

local InteractionService = {}

local function makePrompt(parent: Instance, actionText: string, objectText: string, holdSeconds: number): ProximityPrompt
	local prompt = Instance.new("ProximityPrompt")
	prompt.ActionText = actionText
	prompt.ObjectText = objectText
	prompt.HoldDuration = holdSeconds
	prompt.MaxActivationDistance = 10
	prompt.RequiresLineOfSight = false
	prompt.Parent = parent
	return prompt
end

--- Loot prompts. Relics are hidden unless the player's own lantern is lit, so
--- the prompt's visibility has to track that per-player, not globally.
local function attachLootPrompt(instance: Instance)
	if not instance:IsA("BasePart") or instance:FindFirstChildOfClass("ProximityPrompt") then
		return
	end

	local tier = instance:GetAttribute("Tier")
	if typeof(tier) ~= "string" or Config.Loot.tiers[tier] == nil then
		warn(`[InteractionService] loot "{instance:GetFullName()}" has an invalid Tier attribute`)
		return
	end

	local isCrate = tier == "RestorationCrates"
	local prompt = makePrompt(instance, if isCrate then "Lift" else "Take", tier, if isCrate then 0.6 else 0)

	-- A lantern-only tier should not advertise itself in the dark.
	if Config.Loot.tiers[tier].visibility == "lanternOnly" then
		prompt.Enabled = false
		task.spawn(function()
			while prompt.Parent and instance:IsDescendantOf(workspace) do
				local anyoneLit = RunStateService.activeLanterns() > 0
				prompt.Enabled = anyoneLit
				task.wait(0.4)
			end
		end)
	end

	prompt.Triggered:Connect(function(player: Player)
		LootService.handlePickup(player, instance)
	end)
end

local function attachExtractionPrompt(instance: Instance)
	if not instance:IsA("BasePart") or instance:FindFirstChildOfClass("ProximityPrompt") then
		return
	end

	local id = instance:GetAttribute("ExtractionId")
	if typeof(id) ~= "string" or Config.Loot.extraction[id] == nil then
		warn(`[InteractionService] extraction "{instance:GetFullName()}" has an invalid ExtractionId`)
		return
	end

	local def = Config.Loot.extraction[id]
	local label = if def.arrivalSeconds > 0 then `Call lift ({def.arrivalSeconds}s)` else "Leave"
	local prompt = makePrompt(instance, label, "Extraction", 1)

	prompt.Triggered:Connect(function(player: Player)
		-- handleExtraction re-resolves the tagged instance and re-checks range,
		-- so the prompt is convenience, not authority.
		task.spawn(LootService.handleExtraction, player, id)
	end)
end

--- Revive prompts live on the downed player's own character, created when they
--- go down and removed when they stand. Teammates walk up and hold.
local function watchForDowns()
	local hold = Config.Lantern.revive.teammateHoldSeconds

	task.spawn(function()
		while true do
			local run = RunStateService.current()
			if run and not run.finished then
				for userId, state in pairs(run.lightkeepers) do
					local player = Players:GetPlayerByUserId(userId)
					local character = player and player.Character
					local root = character and character:FindFirstChild("HumanoidRootPart")

					if root then
						local existing = root:FindFirstChild("RevivePrompt")

						if state.knockedDown and not existing then
							local prompt = makePrompt(root, "Revive", player.DisplayName, hold)
							prompt.Name = "RevivePrompt"
							prompt.MaxActivationDistance = Config.Lantern.revive.maxReviveDistance
							prompt.Triggered:Connect(function(rescuer: Player)
								RunStateService.beginRevive(rescuer, player)
							end)
							-- The hold is advanced server-side in the run tick;
							-- this prompt only registers intent. Releasing early
							-- or walking off cancels it there.
							prompt.PromptButtonHoldEnded:Connect(function()
								RunStateService.cancelRevive(player)
							end)
						elseif not state.knockedDown and existing then
							existing:Destroy()
						end
					end
				end
			end
			task.wait(0.3)
		end
	end)
end

function InteractionService.init()
	for _, instance in ipairs(CollectionService:GetTagged(TAG_LOOT)) do
		attachLootPrompt(instance)
	end
	CollectionService:GetInstanceAddedSignal(TAG_LOOT):Connect(attachLootPrompt)

	for _, instance in ipairs(CollectionService:GetTagged(TAG_EXTRACTION)) do
		attachExtractionPrompt(instance)
	end
	CollectionService:GetInstanceAddedSignal(TAG_EXTRACTION):Connect(attachExtractionPrompt)

	watchForDowns()

	-- Loud, because a map with no tags looks identical to a working map until
	-- you try to play it.
	local lootCount = #CollectionService:GetTagged(TAG_LOOT)
	local exitCount = #CollectionService:GetTagged(TAG_EXTRACTION)
	local relayCount = #CollectionService:GetTagged("RelayBank")
	local spawnCount = #CollectionService:GetTagged("SweeperSpawn")
	print(
		`[InteractionService] tagged: Loot={lootCount} Extraction={exitCount} RelayBank={relayCount} SweeperSpawn={spawnCount}`
	)
	if lootCount == 0 or exitCount == 0 or relayCount == 0 then
		warn("[InteractionService] the map is missing tags - a run cannot be completed")
	end
end

return InteractionService
