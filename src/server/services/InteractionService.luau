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

local function makePrompt(
	parent: Instance,
	actionText: string,
	objectText: string,
	holdSeconds: number
): ProximityPrompt
	local prompt = Instance.new("ProximityPrompt")
	prompt.ActionText = actionText
	prompt.ObjectText = objectText
	prompt.HoldDuration = holdSeconds
	prompt.MaxActivationDistance = 10
	prompt.RequiresLineOfSight = false
	prompt.Parent = parent
	return prompt
end

--- Loot prompts.
---
--- Relic prompts are created disabled and deliberately left that way here.
--- `ProximityPrompt.Enabled` is a property of the instance, not a per-player
--- view, so nothing on the server can show one prompt to one player and hide it
--- from another; the old crew-wide toggle meant a dark player standing next to a
--- lit teammate saw relics they should not.
---
--- `RelicVisibilityController` enables them per viewer instead, from the
--- per-player `yourLanternOn` in the run snapshot. Client property writes do not
--- replicate, so one player's light cannot reveal a relic for anyone else.
---
--- The server keeps the authority that matters: `Triggered` still fires here
--- with the player who pressed, and `LootService.isVisibleTo` re-checks that
--- player's own lantern before granting anything. A client that force-enables a
--- relic prompt gains a prompt and nothing else.
local function attachLootPrompt(instance: Instance)
	if not instance:IsA("BasePart") or instance:FindFirstChildOfClass("ProximityPrompt") then
		return
	end

	local tier = instance:GetAttribute("Tier")
	if typeof(tier) ~= "string" or Config.Loot.tiers[tier] == nil then
		warn(`[InteractionService] loot "{instance:GetFullName()}" has an invalid Tier attribute`)
		return
	end

	-- Taking something is a held action, not a tap. Chris, after playing: "it
	-- just feels like i should hold E for a second or so to see that I've picked
	-- it up." An instant pickup gives you nothing to feel, and the prompt's own
	-- fill ring is the only progress the player ever sees.
	--
	-- Important finds make you stand exposed longer. The timings live with the
	-- loot tier, so a relic cache cannot accidentally collect as fast as scrap.
	local isCrate = tier == "RestorationCrates"
	local pickupSeconds = Config.Loot.tiers[tier].pickupSeconds
	local prompt = makePrompt(instance, if isCrate then "Lift" else "Take", tier, pickupSeconds)

	-- A lantern-only tier should not advertise itself in the dark. Off here, and
	-- turned on per viewer by RelicVisibilityController; see above.
	if Config.Loot.tiers[tier].visibility == "lanternOnly" then
		prompt.Enabled = false
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
		warn(
			`[InteractionService] extraction "{instance:GetFullName()}" has an invalid ExtractionId`
		)
		return
	end

	local def = Config.Loot.extraction[id]
	local label = `Hold extraction ({def.arrivalSeconds}s)`
	local prompt = makePrompt(instance, label, "Extraction", 1)
	prompt.Name = "ExtractionPrompt"
	-- Every route begins open. LootService closes alternate exits as the storm
	-- escalates, leaving one final route for Final Call.
	prompt.Enabled = instance:GetAttribute("ActiveThisRun") == true

	prompt.Triggered:Connect(function(player: Player)
		-- handleExtraction re-resolves the tagged instance and re-checks range,
		-- so the prompt is convenience, not authority.
		task.spawn(LootService.handleExtraction, player, instance)
	end)
end

--- Revive prompts live on the downed player's own character, created when they
--- go down and removed when they stand. Teammates walk up and hold.
local function watchForDowns()
	local hold = Config.Lantern.revive.teammateHoldSeconds
	local function teammateCanRevive(target: Player, root: BasePart): boolean
		-- A default ProximityPrompt is visible to its own parent character too.
		-- Creating one while solo therefore offered "Revive yourself", then the
		-- server correctly rejected it. That prompt sat on top of the exit prompt
		-- and made a crawling player unable to call extraction. Only create the
		-- teammate interaction when a teammate is actually close enough to use it.
		for _, rescuer in ipairs(Players:GetPlayers()) do
			if rescuer ~= target then
				local rescuerRoot = rescuer.Character
					and rescuer.Character:FindFirstChild("HumanoidRootPart")
				if rescuerRoot and rescuerRoot:IsA("BasePart") then
					if
						(rescuerRoot.Position - root.Position).Magnitude
						<= Config.Lantern.revive.maxReviveDistance
					then
						return true
					end
				end
			end
		end
		return false
	end

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

						local teammateNearby = state.knockedDown and teammateCanRevive(player, root)
						if teammateNearby and not existing then
							local prompt = makePrompt(root, "Revive", player.DisplayName, hold)
							prompt.Name = "RevivePrompt"
							prompt.MaxActivationDistance = Config.Lantern.revive.maxReviveDistance
							prompt.Triggered:Connect(function(rescuer: Player)
								-- Triggered arrives only after this prompt's configured
								-- hold completes. The server validates the finish, rather
								-- than adding a second invisible countdown after it.
								RunStateService.completeRevive(rescuer, player)
							end)
						elseif existing and (not state.knockedDown or not teammateNearby) then
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
