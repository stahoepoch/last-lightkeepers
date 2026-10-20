--!strict
-- Per-player relic visibility.
--
-- Relics are revealed by *your* lantern, not the crew's - that is the whole
-- reason to turn light on, and a locked decision.
--
-- `ProximityPrompt.Enabled` is a property of the instance, so the server cannot
-- show one prompt to one player and hide it from another. It therefore creates
-- relic prompts disabled and never touches them again; this script enables them
-- for this viewer alone. A property written on the client does not replicate, so
-- one player lighting up never reveals a relic for anybody else.
--
-- This only decides what is *advertised*. The server still re-checks the acting
-- player's own lantern in `LootService.isVisibleTo` before granting anything, so
-- a client that force-enables a prompt gains nothing by it.

local CollectionService = game:GetService("CollectionService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Remotes = require(Shared:WaitForChild("Remotes"))

local TAG_LOOT = "Loot"

-- How long to wait for a server-created prompt to replicate in before giving up
-- on a part. Generous: a slow join should not permanently hide a relic.
local PROMPT_WAIT_SECONDS = 15

local lanternOn = false
local prompts: { [BasePart]: ProximityPrompt } = {}

local function isLanternOnly(instance: BasePart): boolean
	local tier = instance:GetAttribute("Tier")
	if typeof(tier) ~= "string" then
		return false
	end
	local def = Config.Loot.tiers[tier]
	return def ~= nil and def.visibility == "lanternOnly"
end

local function track(instance: Instance)
	if not instance:IsA("BasePart") then
		return
	end
	local part = instance :: BasePart
	if prompts[part] or not isLanternOnly(part) then
		return
	end

	task.spawn(function()
		-- The prompt is made on the server, so it can arrive after the part does.
		local waited = 0
		local prompt = part:FindFirstChildOfClass("ProximityPrompt")
		while not prompt and waited < PROMPT_WAIT_SECONDS and part:IsDescendantOf(workspace) do
			task.wait(0.2)
			waited += 0.2
			prompt = part:FindFirstChildOfClass("ProximityPrompt")
		end
		if not prompt or not part:IsDescendantOf(workspace) then
			return
		end
		prompts[part] = prompt
		prompt.Enabled = lanternOn
	end)
end

for _, instance in ipairs(CollectionService:GetTagged(TAG_LOOT)) do
	track(instance)
end
CollectionService:GetInstanceAddedSignal(TAG_LOOT):Connect(track)
CollectionService:GetInstanceRemovedSignal(TAG_LOOT):Connect(function(instance)
	if instance:IsA("BasePart") then
		prompts[instance :: BasePart] = nil
	end
end)

-- `yourLanternOn` is already per-viewer: it is sent with FireClient and answers
-- "is *mine* lit", which the crew-wide lantern count cannot.
Remotes.get("RunSnapshot").OnClientEvent:Connect(function(snapshot)
	local lit = snapshot.yourLanternOn == true
	if lit == lanternOn then
		return
	end
	lanternOn = lit

	for part, prompt in pairs(prompts) do
		if prompt.Parent and part:IsDescendantOf(workspace) then
			prompt.Enabled = lit
		else
			prompts[part] = nil
		end
	end
end)
