--!strict
-- The Quartermaster's counter, and the four hub fixtures that were silent.
--
-- Chris, a while back: "where is the store going to be? the more you unlock,
-- the more you can buy per floor? flex you progress?" The counter exists in
-- the Lounge and four more fixtures are tagged around the building, and until
-- now CollectionService had nothing listening to any of them:
--
--   Quartermaster   the counter in the Lounge
--   WorkshopBench   the bench in the Workshop
--   WorkshopPlot    the floor plot beside it
--   RelicBench      the workroom bench in the Museum
--   LandmarkSite    the three town landmarks on the arrival floor
--
-- You could walk up to all five and nothing would happen - no prompt, no
-- refusal, nothing. HubService already states the rule this module follows:
-- "a prompt that accepts a press and does nothing is the silent" failure. A
-- fixture that is not finished should say what it is for and that it is not
-- open, which is information; silence is a bug report.
--
-- WHAT THIS DOES AND DELIBERATELY DOES NOT DO
--
-- PlayerDataService already banks `materials` on every completed run through
-- grantPayout, and already carries an `upgrades` table to spend them on.
-- Nothing has ever read `upgrades` and nothing has ever spent `materials`, so
-- a Lightkeeper's haul has been accumulating since the day banking landed and
-- buying nothing.
--
-- This module closes the readable half of that: the counter tells you what
-- you are carrying and how many runs you have finished, so the payout loop is
-- visible and testable end to end. It does NOT invent a shop. Prices, what an
-- upgrade does and how much of it you get are balance, and `data/lantern.json`
-- and `data/loot.json` are full of "Chris's call" notes for exactly that
-- reason - the lantern's fuel gauge was removed by one of them. Those numbers
-- are his, and the decisions needed to open the shop are written down in
-- docs/PROGRESSION-GAP.md rather than guessed at here.
--
-- Like HubService, every part of this is a no-op when the hub is not in the
-- world, so a mall-only test place behaves exactly as it did before.

local CollectionService = game:GetService("CollectionService")

local PlayerDataService = require(script.Parent:WaitForChild("PlayerDataService"))

local TAG_QUARTERMASTER = "Quartermaster"
local REACH = 12

--- The fixtures that are placed, named and not yet wired to anything. Each one
--- says what it is for; none of them pretends to work.
local PENDING: { { tag: string, action: string, object: string } } = {
	{ tag = "WorkshopBench", action = "Workshop - not open yet", object = "The Workshop" },
	{ tag = "WorkshopPlot", action = "Workshop - not open yet", object = "The Workshop" },
	{ tag = "RelicBench", action = "Relic study - not open yet", object = "The Workroom" },
	{ tag = "LandmarkSite", action = "Restoration - not open yet", object = "The Town" },
}

--- The order materials are read out in: biggest decisions last.
local TIER_ORDER = { "Supplies", "Components", "Relics", "RestorationCrates" }

local WorkshopService = {}

local function promptOn(part: BasePart, action: string, object: string): ProximityPrompt
	local existing = part:FindFirstChildOfClass("ProximityPrompt")
	if existing then
		return existing
	end
	local prompt = Instance.new("ProximityPrompt")
	prompt.ActionText, prompt.ObjectText = action, object
	prompt.HoldDuration = 0.3
	prompt.MaxActivationDistance = REACH
	prompt.RequiresLineOfSight = false
	prompt.Parent = part
	return prompt
end

--- One line per tier the player actually holds, so an empty stash reads as
--- empty rather than as four zeroes. Takes the materials table rather than the
--- whole profile: the profile type is exported, but tests/wiring.test.mjs
--- reads any dotted `Module.Name` as a cross-module CALL and fails on a type
--- annotation, so naming one here breaks the suite even inside a comment.
local function stockLines(materials: { [string]: number }): { string }
	local lines = {}
	for _, tier in ipairs(TIER_ORDER) do
		local n = materials[tier]
		if n and n > 0 then
			table.insert(lines, string.format("%s  %d", tier, n))
		end
	end
	return lines
end

--- Read the counter back to whoever pressed it. ActionText is the channel the
--- rest of the hub uses for this, and it needs no client code.
local function readStock(player: Player, prompt: ProximityPrompt)
	local profile = PlayerDataService.load(player)
	if profile.loadFailed then
		-- Never imply a stash is empty when the read failed: that reads as
		-- "my progress is gone" and it is the one lie worth guarding against.
		prompt.ActionText = "stock unavailable - profile did not load"
		task.delay(3, function()
			prompt.ActionText = "Check your stock"
		end)
		return
	end

	local lines = stockLines(profile.materials)
	local summary
	if #lines == 0 then
		summary = if profile.runsCompleted > 0
			then "nothing banked yet"
			else "nothing banked - no runs finished"
	else
		summary = table.concat(lines, "   ")
	end
	prompt.ActionText = summary
	task.delay(4, function()
		prompt.ActionText = "Check your stock"
	end)
end

--- The counter's own sign, so the stash is readable without pressing anything.
local function writeCounter(counter: BasePart)
	local gui = counter:FindFirstChildOfClass("SurfaceGui")
	local label = gui and gui:FindFirstChildOfClass("TextLabel")
	if label then
		label.Text = "QUARTERMASTER\nnothing to buy yet"
	end
end

function WorkshopService.init()
	for _, counter in ipairs(CollectionService:GetTagged(TAG_QUARTERMASTER)) do
		if counter:IsA("BasePart") then
			local prompt = promptOn(counter, "Check your stock", "The Quartermaster")
			prompt.Triggered:Connect(function(player)
				readStock(player, prompt)
			end)
			writeCounter(counter)
		end
	end

	for _, pending in ipairs(PENDING) do
		for _, part in ipairs(CollectionService:GetTagged(pending.tag)) do
			if part:IsA("BasePart") then
				local prompt = promptOn(part, pending.action, pending.object)
				-- Pressing it repeats the refusal rather than doing nothing,
				-- which is the difference between "coming later" and "broken".
				prompt.Triggered:Connect(function()
					prompt.ActionText = pending.action
				end)
			end
		end
	end
end

return WorkshopService
