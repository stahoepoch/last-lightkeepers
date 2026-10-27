--!strict
-- The ladder's rungs: what a keeper has restored, and what it cost them.
--
-- Rung one is the only one built (docs/WORKLIST.md A9). The point of doing it
-- small was to prove the shape without a quest state machine, and the shape is
-- four fields in `data/ledger.json`: a room that must be open, salvage that is
-- spent, something that changes, something that wakes. A later rung adds a
-- block like it and needs nothing added here but its id.
--
-- A restoration is one boolean per keeper, kept on their profile, and it never
-- reverses - docs/QUESTS.md locks that. It is not a run-scoped buff and it
-- cannot be bought: the only way to set it is to open the room with crates and
-- then spend the salvage.
--
-- Like every other hall fixture, this is a no-op until the map lane tags a
-- part. The tag contract is in COORDINATION.md.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))

local LedgerService = require(script.Parent:WaitForChild("LedgerService"))
local PlayerDataService = require(script.Parent:WaitForChild("PlayerDataService"))
local TownService = require(script.Parent:WaitForChild("TownService"))

local PANEL_TAG = "RestorationPanel"
local IDLE = "Fit the salvage"

-- Long enough to be a decision, like opening a room. Restoring the generator
-- spends salvage that does not come back.
local FIT_HOLD = 1.2

local RestorationService = {}

-- Restorations done in a session whose profile would not load, for the same
-- reason LedgerService keeps `sessionRooms`: an unpublished place has no
-- DataStore, and a feature nobody can see working in Studio is a feature
-- nobody finishes. Never written to a profile, always labelled as temporary.
local sessionDone: { [number]: { [string]: boolean } } = {}

-- ------------------------------------------------------------- the catalogue

local function rungs(): { any }
	return (Config.Ledger and Config.Ledger.rungs) or {}
end

--- The rung with this id, or nil. Rungs without a `restoration` block are
--- still read-only text and are not restorable.
function RestorationService.rungById(id: string?)
	if id == nil then
		return nil
	end
	for _, rung in ipairs(rungs()) do
		if rung.id == id then
			return rung
		end
	end
	return nil
end

--- Every rung that has been given a `restoration` block, in ledger order.
local function buildable(): { any }
	local found = {}
	for _, rung in ipairs(rungs()) do
		if type(rung.restoration) == "table" then
			table.insert(found, rung)
		end
	end
	return found
end

-- ------------------------------------------------------------- a keeper's books

--- Where a keeper's restorations are kept: their profile, or this session only.
local function doneOf(player: Player): ({ [string]: boolean }, boolean)
	local profile = PlayerDataService.load(player)
	if profile.loadFailed then
		local held = sessionDone[player.UserId]
		if not held then
			held = {}
			sessionDone[player.UserId] = held
		end
		return held, false
	end
	-- A profile saved before this feature existed has no `restorations` table.
	if not profile.restorations then
		profile.restorations = {}
	end
	return profile.restorations, true
end

--- Whether this keeper has restored a rung, and whether that is persisted.
function RestorationService.isRestored(player: Player, rungId: string): (boolean, boolean)
	local done, persisted = doneOf(player)
	return done[rungId] == true, persisted
end

-- ------------------------------------------------------------- the host's hall

--- The host keeper, or nil. The hall in the world is theirs, per
--- docs/DECISIONS.md O5, so the generator that powers the departure board is
--- theirs too: a visitor's own restorations do not light somebody else's board.
local function host(): Player?
	for _, player in ipairs(Players:GetPlayers()) do
		if TownService.isHost(player) then
			return player
		end
	end
	return nil
end

--- Whether the hall everyone is standing in has this rung restored. This is
--- what the departure board and the enemies read, and it is deliberately false
--- when there is no host yet: an empty hall is an unpowered one.
function RestorationService.hallHas(rungId: string): boolean
	local keeper = host()
	if not keeper then
		return false
	end
	return (RestorationService.isRestored(keeper, rungId))
end

--- The enemy names that stop needing a carried lantern to wake, given what the
--- hall has restored. Returns a set so a caller can ask about one enemy
--- cheaply, which `pickWalkerTarget` does on every tick.
function RestorationService.wokenWithoutLantern(): { [string]: boolean }
	local woken = {}
	for _, rung in ipairs(buildable()) do
		if RestorationService.hallHas(rung.id) then
			for _, name in ipairs(rung.restoration.wakesWithoutLantern or {}) do
				woken[name] = true
			end
		end
	end
	return woken
end

-- Every walker asks about its own wake rule on its own tick, and the answer is
-- a world fact that changes at most once in a session. Rebuilding the set per
-- walker per tick would be pure garbage, so it is cached for half a second -
-- invisible for a rule about whether the lights are on, and `refresh` drops it
-- immediately so the moment a keeper restores the generator is not delayed.
local wokenCache: { [string]: boolean } = {}
local wokenAt = -1

--- Whether this enemy no longer needs a carried lantern to wake, because the
--- hall has restored something that powers the store's own lights.
function RestorationService.wakesWithoutLantern(name: string): boolean
	local now = os.clock()
	if wokenAt < 0 or now - wokenAt > 0.5 then
		wokenCache = RestorationService.wokenWithoutLantern()
		wokenAt = now
	end
	return wokenCache[name] == true
end

-- ------------------------------------------------------------- the departure board

--- How long the road takes to open, and how likely the board is to fail first.
--- Before rung one the generator is not running and the board is working off
--- whatever is left in the room; after it, the wait is short and it holds.
---
--- Returns (seconds, falseStartChance) so ExpeditionService never has to know
--- which rung powers the board.
function RestorationService.departureTiming(): (number, number)
	local rung = RestorationService.rungById("DepartureGenerator")
	local timing = rung and rung.restoration and rung.restoration.departure
	if not timing then
		-- No data, no invented behaviour: the board keeps the five seconds it
		-- had before this feature existed.
		return 5, 0
	end
	if RestorationService.hallHas("DepartureGenerator") then
		return timing.poweredSeconds, 0
	end
	return timing.unpoweredSeconds, timing.unpoweredFalseStartChance or 0
end

--- The two or three lines the board prints about its own power, for the state
--- the client renders. The trade has to be readable BEFORE a keeper presses
--- launch: a departure that fails without warning is a bug, and the same
--- departure announced in advance is the reason restoring the thing is a
--- reward. docs/WORKLIST.md A9: "the trade was stated before the player
--- accepted it."
function RestorationService.departureLines(): { restored: boolean, line: string, exits: { string } }
	local seconds, chance = RestorationService.departureTiming()
	local restored = RestorationService.hallHas("DepartureGenerator")
	if restored then
		local exits = {}
		-- Basic markers: which ways out exist and what each one costs. Rung
		-- four is the one that gives live word on the way out, so this stays
		-- the standing facts and says nothing about the run in progress.
		for id, exit in pairs((Config.Loot and Config.Loot.extraction) or {}) do
			if type(exit) == "table" and exit.reliability then
				table.insert(
					exits,
					string.format(
						"%s - %s, %ss hold, %sx",
						id,
						tostring(exit.reliability),
						tostring(exit.arrivalSeconds or 0),
						tostring(exit.payoutModifier or 1)
					)
				)
			end
		end
		table.sort(exits)
		return {
			restored = true,
			line = string.format("Generator running. The road opens in %ds.", seconds),
			exits = exits,
		}
	end
	return {
		restored = false,
		line = string.format(
			"Generator dark. %ds to wind up, and roughly %d%% of departures fail.",
			seconds,
			math.floor(chance * 100 + 0.5)
		),
		exits = {},
	}
end

-- ------------------------------------------------------------- restoring one

local function priceOf(restoration: any): string
	local parts = {}
	for tier, n in pairs(restoration.salvage or {}) do
		if type(n) == "number" and n > 0 then
			table.insert(parts, string.format("%d %s", n, tier))
		end
	end
	table.sort(parts)
	return if #parts > 0 then table.concat(parts, " + ") else "nothing"
end

local function canAfford(profile: any, restoration: any): boolean
	for tier, n in pairs(restoration.salvage or {}) do
		if (profile.materials[tier] or 0) < n then
			return false
		end
	end
	return true
end

local function carriedSummary(profile: any, restoration: any): string
	local parts = {}
	for tier, n in pairs(restoration.salvage or {}) do
		table.insert(parts, string.format("%d/%d %s", profile.materials[tier] or 0, n, tier))
	end
	table.sort(parts)
	return table.concat(parts, ", ")
end

--- Restore one rung for one keeper, or say why not.
---
--- Returns (restored, message) rather than only acting, for the same reason
--- WorkshopService.purchase does: a ProximityPrompt's Triggered cannot be
--- fired from the server, so a handler that existed only as a closure could
--- never be exercised without a human standing in the room.
function RestorationService.restore(player: Player, rungId: string): (boolean, string)
	local rung = RestorationService.rungById(rungId)
	if not rung or type(rung.restoration) ~= "table" then
		return false, "this one is not a job yet"
	end
	local restoration = rung.restoration

	local profile = PlayerDataService.load(player)
	if profile.loadFailed then
		-- Never spend against a profile that did not load. WorkshopService
		-- refuses to sell for exactly this reason: the salvage would really
		-- leave the stash and the restoration would evaporate on the next
		-- join. In an unpublished place this is every press, so say which.
		return false, "test profile unavailable - cannot spend salvage without a saveable profile"
	end

	local done = doneOf(player)
	if done[rungId] then
		return false, "already done. It does not come undone"
	end

	local roomId = restoration.requiresRoom
	if roomId then
		local open = LedgerService.isOpen(player, roomId)
		if not open then
			return false, string.format("%s is still sealed - crates open it first", roomId)
		end
	end

	if not canAfford(profile, restoration) then
		return false,
			string.format(
				"needs %s - carrying %s",
				priceOf(restoration),
				carriedSummary(profile, restoration)
			)
	end

	for tier, n in pairs(restoration.salvage or {}) do
		profile.materials[tier] = (profile.materials[tier] or 0) - n
	end
	done[rungId] = true
	PlayerDataService.save(player)
	RestorationService.refresh()
	-- The departure board is deliberately NOT pushed at here. Its client turns
	-- the panel on whenever a state arrives, so an unsolicited refresh would
	-- pop the board open in front of everyone standing in the hall. It reads
	-- the new power state the next time somebody presses it, which is the only
	-- time anybody is looking at it.
	return true, string.format("%s - done", rung.title or rungId)
end

-- ------------------------------------------------------------- the panel

local function say(prompt: ProximityPrompt, text: string, seconds: number)
	prompt.ActionText = text
	task.delay(seconds, function()
		prompt.ActionText = IDLE
	end)
end

local function panelLabel(panel: BasePart): TextLabel?
	local gui = panel:FindFirstChildOfClass("SurfaceGui")
	return gui and gui:FindFirstChildOfClass("TextLabel") or nil
end

--- Which rung a panel is for. The attribute is the map lane's; a panel without
--- one is assumed to be the generator, because rung one is the only rung with
--- a restoration block and guessing wrong is better than a dead prompt.
local function rungOf(panel: BasePart): string
	local named = panel:GetAttribute("Rung")
	return if type(named) == "string" and named ~= "" then named else "DepartureGenerator"
end

--- Write every panel's face from the host's books. The wording is in names
--- rather than "you", because ActionText is one shared value and not
--- per-player: whatever this says, every keeper in reach reads it.
function RestorationService.refresh()
	-- Anything that calls refresh has changed the books, so the cached wake
	-- set is stale by definition.
	wokenAt = -1
	for _, panel in ipairs(CollectionService:GetTagged(PANEL_TAG)) do
		if panel:IsA("BasePart") then
			local rung = RestorationService.rungById(rungOf(panel))
			local label = panelLabel(panel)
			if label then
				if not rung or type(rung.restoration) ~= "table" then
					label.Text = "NOT A JOB YET"
				else
					local whose = TownService.hostName() or "this hall"
					local done = RestorationService.hallHas(rung.id)
					label.Text = string.format(
						"%s\n\n%s\n\n%s\n\n%s",
						rung.title or rung.id,
						rung.job or "",
						if done
							then string.format("DONE, in %s's Lanternhold.", whose)
							else string.format("NEEDS %s", priceOf(rung.restoration)),
						if done
							then rung.wakes or ""
							else string.format("WAKES: %s", rung.wakes or "")
					)
				end
			end
			panel:SetAttribute("Restored", RestorationService.hallHas(rungOf(panel)))
		end
	end
end

function RestorationService.init()
	for _, panel in ipairs(CollectionService:GetTagged(PANEL_TAG)) do
		if panel:IsA("BasePart") then
			local prompt = panel:FindFirstChild("RestorePrompt") :: ProximityPrompt?
			if not prompt then
				prompt = Instance.new("ProximityPrompt")
				prompt.Name = "RestorePrompt"
				prompt.ActionText = IDLE
				prompt.ObjectText = "The generator"
				prompt.HoldDuration = FIT_HOLD
				prompt.MaxActivationDistance = 12
				prompt.RequiresLineOfSight = false
				prompt.Parent = panel
			end
			local id = rungOf(panel)
			prompt.Triggered:Connect(function(player: Player)
				local ok, message = RestorationService.restore(player, id)
				say(prompt :: ProximityPrompt, message, if ok then 6 else 4)
			end)
		end
	end
	RestorationService.refresh()
end

return RestorationService
