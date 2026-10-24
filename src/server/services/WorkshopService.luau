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
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local PlayerDataService = require(script.Parent:WaitForChild("PlayerDataService"))
local Config = require(ReplicatedStorage:WaitForChild("Shared"):WaitForChild("Config"))

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

--- What the player is carrying, as one line.
local function stockSummary(profile: any): string
	local lines = stockLines(profile.materials)
	if #lines == 0 then
		return if profile.runsCompleted > 0
			then "nothing banked yet"
			else "nothing banked - no runs finished"
	end
	return table.concat(lines, "   ")
end

-- ------------------------------------------------------------- the shop

--- Catalogue order is the order they are offered in; `requires` chains them.
local function catalogue(): { any }
	local w = Config.Workshop
	return (w and w.upgrades) or {}
end

local function owns(profile: any, id: string): boolean
	return profile.upgrades[id] == true
end

--- The next thing this player could buy: first in catalogue order that they do
--- not own and whose prerequisite they do. nil when the shop is exhausted.
local function nextFor(profile: any): any?
	for _, up in ipairs(catalogue()) do
		if not owns(profile, up.id) then
			if not up.requires or owns(profile, up.requires) then
				return up
			end
			-- a locked upgrade stops the line: it is the next thing, once the
			-- one it needs is bought
			return nil
		end
	end
	return nil
end

local function priceOf(up: any): string
	local parts = {}
	for _, tier in ipairs(Config.Workshop.currency) do
		local n = up.cost[tier]
		if n and n > 0 then
			table.insert(parts, string.format("%d %s", n, tier))
		end
	end
	return table.concat(parts, " + ")
end

local function canAfford(profile: any, up: any): boolean
	for tier, n in pairs(up.cost) do
		if (profile.materials[tier] or 0) < n then
			return false
		end
	end
	return true
end

local IDLE = "Buy / check stock"

local function say(prompt: ProximityPrompt, text: string, seconds: number)
	prompt.ActionText = text
	task.delay(seconds, function()
		prompt.ActionText = IDLE
	end)
end

--- One press, always informative: buy the next upgrade if it is affordable,
--- otherwise say what you are carrying and what the next thing costs.
---
--- The server owns every step. The client never names a price, an upgrade or
--- an amount - it presses a prompt, and everything else is decided here.
---
--- Returns (bought, message) so the purchase path is reachable from a test. A
--- ProximityPrompt's Triggered cannot be fired from the server, so a handler
--- that existed only as a closure could never be exercised without a human
--- standing at the counter.
function WorkshopService.purchase(player: Player): (boolean, string)
	local profile = PlayerDataService.load(player)
	if profile.loadFailed then
		-- Never imply a stash is empty when the read failed: that reads as
		-- "my progress is gone" and it is the one lie worth guarding against.
		--
		-- And never SELL against it either. PlayerDataService refuses to save
		-- a profile that did not load cleanly, so a purchase here would take
		-- materials the player may really have and hand back an upgrade that
		-- evaporates on the next join. In an unpublished place this is every
		-- purchase, because DataStore is unavailable until the place is on
		-- the web - so the message says that rather than leaving Chris to
		-- find out by testing a counter that always refuses.
		return false, "test profile unavailable - cannot sell without a saveable profile"
	end

	local up = nextFor(profile)
	if not up then
		return false, string.format("%s   (nothing left to buy)", stockSummary(profile))
	end

	if not canAfford(profile, up) then
		return false,
			string.format("%s   %s needs %s", stockSummary(profile), up.title, priceOf(up))
	end

	for tier, n in pairs(up.cost) do
		profile.materials[tier] = (profile.materials[tier] or 0) - n
	end
	profile.upgrades[up.id] = true
	PlayerDataService.save(player)
	WorkshopService.refreshSigns()
	return true, string.format("%s - bought. %s", up.title, stockSummary(profile))
end

local function atCounter(player: Player, prompt: ProximityPrompt)
	local bought, message = WorkshopService.purchase(player)
	say(prompt, message, if bought then 5 else 4)
end

--- The best (lowest) relic burden multiplier this player owns. 1 means no
--- upgrade: the full penalty, which is what everyone starts on.
function WorkshopService.relicBurdenMultiplier(player: Player): number
	local profile = PlayerDataService.load(player)
	local best = 1
	for _, up in ipairs(catalogue()) do
		local m = up.effect and up.effect.relicBurdenMultiplier
		if m and owns(profile, up.id) and m < best then
			best = m
		end
	end
	return best
end

--- The counter's own sign: what is for sale and what it costs, readable
--- without pressing anything.
local function writeCounter(counter: BasePart)
	local gui = counter:FindFirstChildOfClass("SurfaceGui")
	local label = gui and gui:FindFirstChildOfClass("TextLabel")
	if not label then
		return
	end
	local lines = { "QUARTERMASTER" }
	local list = catalogue()
	if #list == 0 then
		table.insert(lines, "nothing to buy yet")
	else
		for _, up in ipairs(list) do
			table.insert(lines, string.format("%s   %s", up.title, priceOf(up)))
		end
	end
	label.Text = table.concat(lines, "\n")
end

function WorkshopService.refreshSigns()
	for _, counter in ipairs(CollectionService:GetTagged(TAG_QUARTERMASTER)) do
		if counter:IsA("BasePart") then
			writeCounter(counter)
		end
	end
end

function WorkshopService.init()
	for _, counter in ipairs(CollectionService:GetTagged(TAG_QUARTERMASTER)) do
		if counter:IsA("BasePart") then
			local prompt = promptOn(counter, IDLE, "The Quartermaster")
			prompt.Triggered:Connect(function(player)
				atCounter(player, prompt)
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
