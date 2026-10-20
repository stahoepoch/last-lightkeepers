--!strict
-- Per-player loot readability: what you can see, and what you are told you can
-- take. Both are per-viewer, so both live on the client.
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
local marks: { [BasePart]: Highlight } = {}

-- "Baseline loot is always visible" is a rule from the handoff, and it was only
-- true on paper: an unlit grey box in an unlit room is invisible at any range.
-- Chris walked past loot repeatedly and reported E as broken - he was pressing
-- it fifteen studs from the nearest pickup, outside the prompt's ten.
--
-- A thin outline, drawn only where you could actually see the object, is the
-- smallest thing that makes the rule real without lighting the room up. It is
-- not a light: it adds nothing to Storm Charge, which only ever counts lanterns.
local MARK_COLOUR = Color3.fromRGB(214, 226, 240)
local MARK_TRANSPARENCY = 0.35

local function markFor(part: BasePart): Highlight
	local existing = marks[part]
	if existing and existing.Parent then
		return existing
	end
	local mark = Instance.new("Highlight")
	mark.Name = "LootMark"
	mark.FillTransparency = 1
	mark.OutlineColor = MARK_COLOUR
	mark.OutlineTransparency = MARK_TRANSPARENCY
	-- Occluded, so loot does not glow through walls and give away a room you
	-- have not entered. Darkness still hides things; it just stops hiding the
	-- thing you are standing next to.
	mark.DepthMode = Enum.HighlightDepthMode.Occluded
	mark.Adornee = part
	mark.Parent = part
	marks[part] = mark
	return mark
end

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

local function markLoot(instance: Instance)
	if not instance:IsA("BasePart") then
		return
	end
	local part = instance :: BasePart
	local mark = markFor(part)
	-- A relic you cannot take should not be outlined either, or the outline
	-- becomes the lantern-only reveal that the lantern is supposed to be.
	mark.Enabled = not isLanternOnly(part) or lanternOn
end

local function onLoot(instance: Instance)
	track(instance)
	markLoot(instance)
end

for _, instance in ipairs(CollectionService:GetTagged(TAG_LOOT)) do
	onLoot(instance)
end
CollectionService:GetInstanceAddedSignal(TAG_LOOT):Connect(onLoot)
CollectionService:GetInstanceRemovedSignal(TAG_LOOT):Connect(function(instance)
	if instance:IsA("BasePart") then
		prompts[instance :: BasePart] = nil
		marks[instance :: BasePart] = nil
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

	for part, mark in pairs(marks) do
		if mark.Parent and part:IsDescendantOf(workspace) then
			mark.Enabled = not isLanternOnly(part) or lit
		else
			marks[part] = nil
		end
	end
end)
