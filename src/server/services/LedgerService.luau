--!strict
-- The ledger, and the rooms that open.
--
-- This is the thing the hall was missing. Until now a run changed exactly one
-- pixel of Lanternhold - a landmark plate's colour - and docs/REWARDS.md said
-- so plainly: "crates are the only thing that restores the hall, and the hall
-- currently has nothing to restore." Chris, 2026-10-26, chose this over
-- building the first store rung, and it is the right order: a rung's reward has
-- to have somewhere to land before the rung means anything.
--
-- Two fixtures, both no-ops when the hall is not in the world, exactly like
-- HubService's three:
--
--   Ledger board  the order's work book, and the open half of the Good Cheer
--                 Club roll. The jobs not done yet, the trade each one carries,
--                 and a line left blank for the name of whoever does it.
--   Hall rooms    sealed, visibly unfinished parts of the hall. A keeper's
--                 crates pay for one and it opens for good.
--
-- WHOSE CRATES, AND WHY NOTHING IS SUBTRACTED
--
-- `profile.contributions.RestorationCrates` is credit, and the handoff locked
-- it as never removable: you cannot take your name off the roll and nobody can
-- take it over. So opening a room cannot deduct from it. Instead the spend is
-- derived - what you have left is your credited total minus the cost of every
-- room you have already opened - which means the three landmark plates keep
-- reading the gross figure and are untouched by any of this. That is the
-- correct split: the landmarks record what you brought home, the rooms record
-- what you did with it.
--
-- WHOSE HALL
--
-- The host's, per docs/DECISIONS.md O5 and the same rule the landmark plates
-- follow. Only the host's rooms are shown, and only the host can open one. A
-- visitor reads the hall and their own figure, because walking into a friend's
-- Lanternhold to see what they have rebuilt is the point of visiting.
--
-- WHAT IS NOT HERE
--
-- Rung state. Whether the departure generator has been restored is store-side
-- work that is not built (docs/WORKLIST.md A9), so the board prints every rung
-- with its trade and no progress. It is shaped to print progress the day there
-- is some.
--
-- Room geometry and lighting belong to the map lane (COORDINATION.md). This
-- sets `Open` on the seal and puts one light on it; a builder that wants the
-- room's own lamps to come up need only read that attribute.

local CollectionService = game:GetService("CollectionService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Players = game:GetService("Players")

local PlayerDataService = require(script.Parent:WaitForChild("PlayerDataService"))
local TownService = require(script.Parent:WaitForChild("TownService"))
local Config = require(ReplicatedStorage:WaitForChild("Shared"):WaitForChild("Config"))

local TAG_ROOM = "HallRoom"
local TAG_LEDGER = "LedgerBoard"

local REACH = 12

local SEALED = Color3.fromRGB(58, 52, 46)
local OPEN_LIGHT = Color3.fromRGB(255, 214, 160)

-- Opening a room is the most consequential press in the hall, so it is a hold
-- long enough to be a decision rather than a misclick on the way past.
local OPEN_HOLD = 1.2

local LedgerService = {}

-- Rooms opened in a session whose profile would not load. An unpublished place
-- has no DataStore, and a feature nobody can see working in Studio is a feature
-- nobody will finish. Never written to a profile; always labelled as temporary.
local sessionRooms: { [number]: { [string]: boolean } } = {}

-- ------------------------------------------------------------- the catalogue

local function catalogue(): { any }
	return (Config.Ledger and Config.Ledger.rooms) or {}
end

local function rungs(): { any }
	return (Config.Ledger and Config.Ledger.rungs) or {}
end

local function roomById(id: string?)
	if id == nil then
		return nil
	end
	for _, room in ipairs(catalogue()) do
		if room.id == id then
			return room
		end
	end
	return nil
end

-- ------------------------------------------------------------- a keeper's books

--- Where a keeper's opened rooms are kept: their profile, or this session only.
local function roomsOf(player: Player): ({ [string]: boolean }, boolean)
	local profile = PlayerDataService.load(player)
	if profile.loadFailed then
		local held = sessionRooms[player.UserId]
		if not held then
			held = {}
			sessionRooms[player.UserId] = held
		end
		return held, false
	end
	-- A profile saved before this feature existed has no `rooms` table.
	if not profile.rooms then
		profile.rooms = {}
	end
	return profile.rooms, true
end

--- Crates already committed to rooms. Derived, never stored: a stored figure
--- and a list of opened rooms are two truths that can disagree.
local function committedBy(player: Player): number
	local opened = roomsOf(player)
	local spent = 0
	for _, room in ipairs(catalogue()) do
		if opened[room.id] then
			spent += room.crates or 0
		end
	end
	return spent
end

--- Whether one room of this keeper's hall is open, and whether that is
--- persisted rather than session-only. RestorationService asks this: rung one
--- cannot be done until the generator room it is in has been opened.
function LedgerService.isOpen(player: Player, roomId: string): (boolean, boolean)
	local opened, persisted = roomsOf(player)
	return opened[roomId] == true, persisted
end

--- What this keeper has left to commit, and whether the figure is persisted.
function LedgerService.availableFor(player: Player): (number, string)
	local total, scope = TownService.totalFor(player)
	return math.max(0, total - committedBy(player)), scope
end

-- ------------------------------------------------------------- the rooms

local function roomLabel(seal: BasePart): TextLabel?
	local gui = seal:FindFirstChildOfClass("SurfaceGui")
	return gui and gui:FindFirstChildOfClass("TextLabel") or nil
end

local function roomLight(seal: BasePart): PointLight
	local light = seal:FindFirstChildOfClass("PointLight")
	if not light then
		light = Instance.new("PointLight")
		light.Range = 30
		-- Shadows off, always: the same strobing Chris reported in the mall.
		light.Shadows = false
		light.Parent = seal
	end
	return light :: PointLight
end

local function seals(): { BasePart }
	local out: { BasePart } = {}
	for _, inst in ipairs(CollectionService:GetTagged(TAG_ROOM)) do
		if inst:IsA("BasePart") then
			table.insert(out, inst)
		end
	end
	return out
end

--- Dress one seal from the host's books. `open` nil means there is no host yet.
local function dressSeal(seal: BasePart, open: boolean?, available: number, suffix: string)
	local room = roomById(seal:GetAttribute("Room"))
	if not room then
		return
	end

	local isOpen = open == true
	seal:SetAttribute("Open", isOpen)
	-- The boards come off rather than the part being destroyed: the geometry is
	-- the map lane's and this service does not own the right to delete it.
	seal.Transparency = if isOpen then 1 else 0
	seal.CanCollide = not isOpen
	seal.Color = SEALED

	local light = roomLight(seal)
	light.Color = OPEN_LIGHT
	light.Enabled = isOpen
	light.Brightness = 1.2

	local label = roomLabel(seal)
	if not label then
		return
	end
	local line
	if open == nil then
		line = "no keeper has arrived"
	elseif isOpen then
		line = room.restored
	else
		line = string.format(
			"%s\n%d of %d crates%s",
			room.sealed,
			math.min(available, room.crates),
			room.crates,
			suffix
		)
	end
	label.Text = string.format("%s\n%s", room.room, line)
end

-- ------------------------------------------------------------- the board

--- The work book. Rooms first, because they are the thing the reader can act
--- on from where they stand; then the jobs, in the order Chris set them.
local function writeBoard()
	local boards = CollectionService:GetTagged(TAG_LEDGER)
	if #boards == 0 then
		return
	end

	local host = TownService.hostName()
	local scope = TownService.scope()
	local known = scope == "home" or scope == "offline"
	local suffix = if scope == "offline" then "  (this session only)" else ""

	local lines = { "THE LEDGER" }
	if host then
		table.insert(lines, string.format("%s's Lanternhold%s", host, suffix))
	else
		table.insert(lines, "no keeper has arrived")
	end
	table.insert(lines, "")

	-- One source of truth for the host's books, read once here rather than per
	-- room: the figure must not shift halfway down a board somebody is reading.
	local available, openOf = 0, nil :: { [string]: boolean }?
	local hostPlayer = nil
	for _, player in ipairs(Players:GetPlayers()) do
		if TownService.isHost(player) then
			hostPlayer = player
			break
		end
	end
	if hostPlayer and known then
		available = LedgerService.availableFor(hostPlayer)
		openOf = roomsOf(hostPlayer)
	end

	for _, room in ipairs(catalogue()) do
		local state
		if not openOf then
			state = "-"
		elseif openOf[room.id] then
			state = "OPEN"
		else
			state = string.format("%d of %d crates", math.min(available, room.crates), room.crates)
		end
		table.insert(lines, string.format("%s   %s", room.room, state))
	end

	table.insert(lines, "")
	table.insert(lines, "WORK NOT DONE")
	for _, rung in ipairs(rungs()) do
		-- The trade, stated before anybody accepts it. docs/QUESTS.md rule 2:
		-- a trap the player chose is a decision, one sprung on them is a bug
		-- with a story attached.
		table.insert(lines, string.format("%s\n  %s", rung.title, rung.job))
		table.insert(lines, string.format("  opens: %s", rung.opens))
		table.insert(lines, string.format("  wakes: %s", rung.wakes))
		table.insert(lines, "  ______________________")
	end

	for _, board in ipairs(boards) do
		local gui = board:FindFirstChildOfClass("SurfaceGui")
		local label = gui and gui:FindFirstChildOfClass("TextLabel")
		if label then
			label.Text = table.concat(lines, "\n")
		end
	end
end

-- ------------------------------------------------------------- refresh

--- Dress every seal and rewrite every board. Called at init, on a hand-in,
--- when a room opens, and when anyone joins.
function LedgerService.refresh()
	local scope = TownService.scope()
	local known = scope == "home" or scope == "offline"
	local suffix = if scope == "offline" then "  (this session only)" else ""

	local hostPlayer = nil
	for _, player in ipairs(Players:GetPlayers()) do
		if TownService.isHost(player) then
			hostPlayer = player
			break
		end
	end

	local open: { [string]: boolean }? = nil
	local available = 0
	if hostPlayer and known then
		open = roomsOf(hostPlayer)
		available = LedgerService.availableFor(hostPlayer)
	end

	for _, seal in ipairs(seals()) do
		local id = seal:GetAttribute("Room")
		if open and typeof(id) == "string" then
			dressSeal(seal, open[id] == true, available, suffix)
		else
			dressSeal(seal, nil, 0, suffix)
		end
	end
	writeBoard()
end

-- ------------------------------------------------------------- opening a room

local function say(prompt: ProximityPrompt, text: string, back: string)
	prompt.ActionText = text
	task.delay(3.5, function()
		prompt.ActionText = back
	end)
end

--- `ActionText` is one shared string - the same press shows to everyone nearby -
--- so every message below is worded in names rather than "you", which would be
--- wrong for all but one of the people reading it.
local function press(player: Player, seal: BasePart, prompt: ProximityPrompt)
	local room = roomById(seal:GetAttribute("Room"))
	if not room then
		return
	end
	local back = prompt.ActionText

	local scope = TownService.scope()
	if scope == "empty" then
		say(prompt, "no keeper has arrived", back)
		return
	end

	local whose = TownService.hostName() or "this hall"
	if not TownService.isHost(player) then
		-- A visitor cannot open a door in somebody else's hall. They can see
		-- what it would take, and what they have at their own.
		local mine = LedgerService.availableFor(player)
		say(
			prompt,
			string.format(
				"%s - %s's to open. %s has %d crates at home",
				room.room,
				whose,
				player.DisplayName,
				mine
			),
			back
		)
		return
	end

	local opened = roomsOf(player)
	if opened[room.id] then
		say(prompt, string.format("%s - already open", room.room), back)
		return
	end

	local available, hostScope = LedgerService.availableFor(player)
	local cost = room.crates or 0
	if available < cost then
		say(prompt, string.format("%s - %d of %d crates", room.room, available, cost), back)
		return
	end

	opened[room.id] = true
	-- A room that just opened may have been the one a rung was waiting inside,
	-- so whoever cares gets told. A seam rather than a require, because
	-- RestorationService requires THIS module to ask whether its room is open
	-- and the two cannot require each other. Wired in init.server.luau.
	local told, err = pcall(LedgerService.onRoomOpened, player, room.id)
	if not told then
		warn(`[LedgerService] onRoomOpened failed for {room.id}: {err}`)
	end
	if hostScope == "offline" then
		-- Visible, never passed off as saved. Same rule the landmark plates follow.
		warn(`[LedgerService] {room.id} opened for this session only: no profile to save it to`)
	end
	LedgerService.refresh()
	say(prompt, string.format("%s - open", room.room), back)
end

--- Called after a room opens, for anything downstream of a door. Replaced in
--- `init.server.luau`; a no-op by default so this module works alone.
LedgerService.onRoomOpened = function(_player: Player, _roomId: string) end

-- ------------------------------------------------------------- wiring

local function promptOn(part: BasePart, action: string, object: string): ProximityPrompt
	local existing = part:FindFirstChildOfClass("ProximityPrompt")
	if existing then
		return existing
	end
	local prompt = Instance.new("ProximityPrompt")
	prompt.ActionText, prompt.ObjectText = action, object
	prompt.HoldDuration = OPEN_HOLD
	prompt.MaxActivationDistance = REACH
	prompt.RequiresLineOfSight = false
	prompt.Parent = part
	return prompt
end

function LedgerService.init()
	if not Config.Ledger then
		-- The balance module did not sync. Say so once; every fixture below is
		-- then a no-op rather than a crash, which is how the rest of the hall
		-- behaves when its geometry is absent.
		warn("[LedgerService] ledger.json not found under Balance; the hall's rooms are inert")
		return
	end

	for _, seal in ipairs(seals()) do
		local prompt = promptOn(seal, "Open the room", "Lanternhold")
		prompt.Triggered:Connect(function(player)
			press(player, seal, prompt)
		end)
	end

	for _, board in ipairs(CollectionService:GetTagged(TAG_LEDGER)) do
		if board:IsA("BasePart") then
			local prompt = promptOn(board, "Read the work", "The Ledger")
			prompt.HoldDuration = 0.4
			prompt.Triggered:Connect(function(player)
				-- Reading changes nothing. It refreshes, because a board that is
				-- stale when pressed is worse than one that is merely old.
				LedgerService.refresh()
				say(prompt, string.format("%s read it", player.DisplayName), "Read the work")
			end)
		end
	end

	-- The host is whoever arrives first, which in a fresh server is after init,
	-- so without this the seals would read "no keeper has arrived" to the very
	-- person whose hall they are.
	Players.PlayerAdded:Connect(function(player)
		TownService.claim(player)
		LedgerService.refresh()
	end)
	Players.PlayerRemoving:Connect(function(player)
		sessionRooms[player.UserId] = nil
		-- Deferred: the leaver is still in Players:GetPlayers() here, so a
		-- refresh now can pick them as the host they are about to stop being.
		task.defer(LedgerService.refresh)
	end)

	LedgerService.refresh()
end

return LedgerService
