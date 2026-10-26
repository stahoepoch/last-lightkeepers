--!strict
-- Lanternhold's fixtures, bound to run state.
--
-- The map (src/map/LanternholdBuilder.luau) places and tags these; nothing in
-- it knows what they mean. This is where they start meaning something:
--
--   Lamp Wall     one lantern per Lightkeeper currently out. Yours burns here
--                 while you are away, is taken down when you extract, and goes
--                 out in front of the whole hall when the crew wipes.
--   Hand-in       the Community Kitchen's counter. Restoration crates stop
--                 being yours and become the Good Cheer Club's, credited by
--                 name on the roll.
--   Relic plinths bring a relic back and it is shelved, with the keeper who
--                 found it named on the plaque.
--
-- All three are no-ops when the hub is not in the world, so a mall-only test
-- place behaves exactly as it did before.
--
-- The ledger board and the hall's rooms live in LedgerService and are started
-- from this file's `init`. They follow the same rule: absent geometry, no-op.

local CollectionService = game:GetService("CollectionService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Players = game:GetService("Players")

local RunStateService = require(script.Parent:WaitForChild("RunStateService"))
local TownService = require(script.Parent:WaitForChild("TownService"))
local LedgerService = require(script.Parent:WaitForChild("LedgerService"))
local Config = require(ReplicatedStorage:WaitForChild("Shared"):WaitForChild("Config"))

local TAG_LAMP = "LampWallSlot"
local TAG_HANDIN = "KitchenContribution"
local TAG_PLINTH = "RelicPlinth"
local TAG_BOARD = "ClubBoard"
local TAG_LANDMARK = "LandmarkSite"

local LIT = Color3.fromRGB(255, 214, 160)
local OUT = Color3.fromRGB(38, 36, 40)
local REACH = 12

-- Landmark plates: unlit stone until the town pays for them, gilt after.
local LANDMARK_DONE = Color3.fromRGB(198, 164, 92)
local LANDMARK_TODO = Color3.fromRGB(116, 112, 104)
local LANDMARK_REFRESH = 20

local HubService = {}

-- Which lamp belongs to whom, and the Club's ledger for this session.
local lampOf: { [number]: BasePart } = {}
local club = { crates = 0, roll = {} :: { [string]: number } }

-- ---------------------------------------------------------------- lamp wall

local function lampLight(lamp: BasePart): PointLight
	local light = lamp:FindFirstChildOfClass("PointLight")
	if not light then
		light = Instance.new("PointLight")
		light.Range = 34
		-- Shadows off, always. A lantern that casts them strobes as a player
		-- walks past the posts, which is the flicker Chris reported in the mall.
		light.Shadows = false
		light.Parent = lamp
	end
	return light :: PointLight
end

--- `name` nil means nobody is out on this lamp: dark, and no nameplate.
local function setLamp(lamp: BasePart, name: string?)
	lamp.Color = if name then LIT else OUT
	local light = lampLight(lamp)
	light.Color = LIT
	light.Enabled = name ~= nil
	light.Brightness = 1.4

	local plate = lamp:FindFirstChild("Keeper") :: BillboardGui?
	if not plate then
		local gui = Instance.new("BillboardGui")
		gui.Name = "Keeper"
		gui.Size = UDim2.fromOffset(170, 34)
		gui.StudsOffset = Vector3.new(0, 4.2, 0)
		gui.MaxDistance = 90
		local text = Instance.new("TextLabel")
		text.Name = "Name"
		text.Size, text.BackgroundTransparency = UDim2.fromScale(1, 1), 1
		text.Font, text.TextScaled = Enum.Font.GothamMedium, true
		text.TextColor3 = Color3.fromRGB(236, 224, 200)
		text.Parent = gui
		gui.Parent = lamp
		plate = gui
	end
	local label = (plate :: BillboardGui):FindFirstChild("Name") :: TextLabel
	label.Text = name or ""
	(plate :: BillboardGui).Enabled = name ~= nil
end

local function lamps(): { BasePart }
	local out: { BasePart } = {}
	for _, inst in ipairs(CollectionService:GetTagged(TAG_LAMP)) do
		if inst:IsA("BasePart") then
			table.insert(out, inst)
		end
	end
	table.sort(out, function(a, b)
		return (a:GetAttribute("Slot") or 0) < (b:GetAttribute("Slot") or 0)
	end)
	return out
end

--- Light a lamp for a Lightkeeper going out. Silent when the hall is not in
--- the world, or when every slot is already spoken for.
function HubService.lightFor(player: Player)
	if lampOf[player.UserId] then
		return
	end
	local taken: { [BasePart]: boolean } = {}
	for _, lamp in pairs(lampOf) do
		taken[lamp] = true
	end
	for _, lamp in ipairs(lamps()) do
		if not taken[lamp] then
			lampOf[player.UserId] = lamp
			setLamp(lamp, player.DisplayName)
			lamp:SetAttribute("State", "out")
			return
		end
	end
end

--- Extraction. The lamp is taken down and carried home, so it leaves the wall
--- rather than going dark: coming back is not the same as not coming back.
function HubService.takeDown(player: Player)
	local lamp = lampOf[player.UserId]
	if not lamp then
		return
	end
	lampOf[player.UserId] = nil
	setLamp(lamp, nil)
	lamp:SetAttribute("State", "home")
end

--- A wipe. Every lamp still burning goes out, in front of the whole hall.
function HubService.putOut()
	for userId, lamp in pairs(lampOf) do
		setLamp(lamp, nil)
		lamp:SetAttribute("State", "lost")
		lampOf[userId] = nil
	end
end

function HubService.release(player: Player)
	local lamp = lampOf[player.UserId]
	if lamp then
		setLamp(lamp, nil)
		lamp:SetAttribute("State", "idle")
		lampOf[player.UserId] = nil
	end
end

-- ---------------------------------------------------------------- the club

local function carriedOf(player: Player, tier: string): number
	local run = RunStateService.current()
	if not run or run.finished then
		return 0
	end
	local state = run.lightkeepers[player.UserId]
	if not state then
		return 0
	end
	return state.haul.carried[tier :: any] or 0
end

local function takeCarried(player: Player, tier: string, amount: number)
	local run = RunStateService.current()
	if not run then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if state then
		state.haul.carried[tier :: any] =
			math.max(0, (state.haul.carried[tier :: any] or 0) - amount)
	end
end

--- The roll, written where everyone can read it. Credited by name and in the
--- order it was given, which is the handoff's requirement that a landmark
--- contribution cannot be undone or monopolised: you cannot take your name off
--- it, and nobody can take it over.
local function writeBoard()
	for _, board in ipairs(CollectionService:GetTagged(TAG_BOARD)) do
		local gui = board:FindFirstChildOfClass("SurfaceGui")
		local label = gui and gui:FindFirstChildOfClass("TextLabel")
		if label then
			local names = {}
			for name, n in pairs(club.roll) do
				table.insert(names, { name = name, n = n })
			end
			table.sort(names, function(a, b)
				return a.n > b.n
			end)
			local lines = {
				"THE GOOD CHEER CLUB",
				string.format("%d crates toward the next room", club.crates),
			}
			for i = 1, math.min(#names, 6) do
				table.insert(lines, string.format("%s  %d", names[i].name, names[i].n))
			end
			label.Text = table.concat(lines, "\n")
		end
	end
end

local function handIn(player: Player, prompt: ProximityPrompt)
	local crates = carriedOf(player, "RestorationCrates")
	if crates <= 0 then
		-- Say so. A prompt that accepts a press and does nothing is the silent
		-- refusal Chris hit all over the first playtest.
		prompt.ActionText = "nothing to hand in"
		task.delay(2, function()
			prompt.ActionText = "Hand in crates"
		end)
		return
	end
	takeCarried(player, "RestorationCrates", crates)
	club.crates += crates
	club.roll[player.DisplayName] = (club.roll[player.DisplayName] or 0) + crates

	-- And to the towns. `club.crates` above is this session's roll and resets
	-- with the server; a keeper's own roll is their Lanternhold's restoration
	-- and persists. TownService owns that write for everyone in the hall now,
	-- including the hander - crediting them here as well would count their
	-- crates twice.
	TownService.contribute(crates)
	-- Through the table, not the local: `writeLandmarks` is defined further
	-- down this file, so calling it by name from up here resolved to a nil
	-- global and threw on every hand-in. HubService is declared at the top and
	-- the field is assigned at load, so this is safe from anywhere.
	HubService.refreshLandmarks()
	-- And the rooms, which is the half of a hand-in a player can actually feel:
	-- the crate that just went over the counter may have paid for a door.
	LedgerService.refresh()

	writeBoard()
	prompt.ActionText = string.format("%d in, thank you", crates)
	task.delay(2.5, function()
		prompt.ActionText = "Hand in crates"
	end)
end

-- ---------------------------------------------------------------- plinths

local function plaqueFor(slot: BasePart): TextLabel?
	local index = slot:GetAttribute("Slot")
	if index == nil or not slot.Parent then
		return nil
	end
	local plaque = slot.Parent:FindFirstChild("Plaque_" .. tostring(index))
	if not plaque then
		return nil
	end
	local gui = plaque:FindFirstChildOfClass("SurfaceGui")
	return gui and gui:FindFirstChildOfClass("TextLabel") or nil
end

local function shelve(player: Player, slot: BasePart, prompt: ProximityPrompt)
	if slot:GetAttribute("Relic") then
		return
	end
	if carriedOf(player, "Relics") <= 0 then
		prompt.ActionText = "no relic to shelve"
		task.delay(2, function()
			prompt.ActionText = "Shelve a relic"
		end)
		return
	end
	takeCarried(player, "Relics", 1)
	slot:SetAttribute("Relic", true)
	slot:SetAttribute("FoundBy", player.DisplayName)

	local relic = Instance.new("Part")
	relic.Name = "ShelvedRelic"
	relic.Size = Vector3.new(5, 6, 5)
	relic.CFrame = slot.CFrame * CFrame.new(0, 3.4, 0)
	relic.Color = Color3.fromRGB(176, 198, 190)
	relic.Material = Enum.Material.Glass
	relic.Transparency = 0.25
	relic.Anchored, relic.CanCollide = true, false
	relic.Parent = slot.Parent

	-- The relic's own name is deliberately not invented here. docs/LANTERNHOLD.md
	-- has it as an open question - archivist or the player who brought it in -
	-- and that is Chris's call. What is certain either way is the credit, so
	-- the plaque carries that now and the name can be written over UNNAMED
	-- whenever the question is settled.
	local label = plaqueFor(slot)
	if label then
		label.Text = "UNNAMED\nbrought in by " .. player.DisplayName
	end
	prompt.Enabled = false
end

-- ------------------------------------------------------------- landmarks
--
-- READ-ONLY, by instruction. Chris, 2026-10-24: "keep it read-only: it should
-- only reflect the ... total and change hub visuals."
--
-- So nothing here writes. It reads a figure and dresses three floor plates
-- with it. A landmark finishing lights the plate and says so, and that is the
-- whole reward - Chris's call: they are the town coming back, and a stat bonus
-- would turn a monument into a grind target.
--
-- WHOSE TOWN, since 2026-10-26
--
-- The figure used to be one global number for the whole experience. It is now
-- the host keeper's own restoration - Chris: "Your personal Lanternhold...
-- the Well, Bell Tower, and Long Bridge are rebuilt by you and the people you
-- play with, which is much more emotionally satisfying than contributing a
-- crate to an invisible global number."
--
-- A plate is one part in the world, so it can only show one town, and the
-- host's is the one the party came to see. That means the plates have to SAY
-- whose hall this is, or a visitor reads a stranger's Bell Tower as their own
-- and the satisfaction Chris is after goes to the wrong person.

--- The plate's own label, made by the map's `label()` helper.
local function landmarkLabel(site: BasePart): TextLabel?
	local gui = site:FindFirstChildOfClass("SurfaceGui")
	return gui and gui:FindFirstChildOfClass("TextLabel") or nil
end

local function landmarkLight(site: BasePart): PointLight
	local light = site:FindFirstChildOfClass("PointLight")
	if not light then
		light = Instance.new("PointLight")
		light.Range = 26
		light.Shadows = false
		light.Parent = site
	end
	return light :: PointLight
end

--- The catalogue entry whose title matches this plate. The map tags each plate
--- with its name in a `Landmark` attribute, so the two meet on that string and
--- neither has to know the other's ordering.
local function landmarkFor(site: BasePart)
	local name = site:GetAttribute("Landmark")
	for _, mark in ipairs((Config.Town and Config.Town.landmarks) or {}) do
		if mark.title == name then
			return mark
		end
	end
	return nil
end

--- Dress every plate from the host's restoration. Called at init, after a
--- hand-in, when the host changes and on a timer.
local function writeLandmarks()
	local scope = TownService.scope()
	local known = scope == "home" or scope == "offline"
	-- An unpublished place has no profile to read, so the figure is this
	-- session's own. Shown, because otherwise none of this can be seen
	-- working in Studio - but never shown as the keeper's saved town.
	local suffix = if scope == "offline" then "  (this session only)" else ""
	local total = TownService.total()
	local whose = TownService.hostName()
	-- Third line on every plate, so nobody mistakes a host's progress for
	-- their own. An empty hall has no town to name.
	local owner = if whose then string.format("%s's Lanternhold", whose) else nil
	for _, site in ipairs(CollectionService:GetTagged(TAG_LANDMARK)) do
		if site:IsA("BasePart") then
			local mark = landmarkFor(site)
			if mark then
				local done = known and total >= mark.crates
				site.Color = if done then LANDMARK_DONE else LANDMARK_TODO
				local light = landmarkLight(site)
				light.Color = LIT
				light.Enabled = done
				light.Brightness = 1.1
				local label = landmarkLabel(site)
				if label then
					local line
					if scope == "empty" then
						line = "no keeper has arrived"
					elseif not known then
						-- Never show 0 of 10 when the profile simply would not
						-- read. "Your town has done nothing" is the worse lie,
						-- and in an unpublished place it is every join.
						line = "restoration unknown"
					elseif done then
						line = mark.restored .. suffix
					else
						line = string.format("%d of %d crates%s", total, mark.crates, suffix)
					end
					label.Text = if owner
						then string.format("%s\n%s\n%s", mark.title, line, owner)
						else string.format("%s\n%s", mark.title, line)
				end
			end
		end
	end
end

--- Pressing a plate reads the restoration back. It changes nothing.
---
--- A visitor gets both figures: the hall's and their own. ActionText is one
--- shared string - the same press shows to everyone nearby - so this is
--- worded in names rather than "you", which would be wrong for all but one of
--- the people reading it.
local function readLandmark(player: Player, site: BasePart, prompt: ProximityPrompt)
	local mark = landmarkFor(site)
	if not mark then
		return
	end

	local function progress(total: number): string
		return if total >= mark.crates
			then "restored"
			else string.format("%d of %d", total, mark.crates)
	end

	local text
	local scope = TownService.scope()
	if scope == "empty" then
		text = string.format("%s - no keeper has arrived", mark.title)
	else
		local suffix = if scope == "offline" then " (this session only)" else ""
		if TownService.isHost(player) then
			text = string.format("%s - %s%s", mark.title, progress(TownService.total()), suffix)
		else
			local mine = TownService.totalFor(player)
			local whose = TownService.hostName() or "this hall"
			text = string.format(
				"%s - %s %s, %s %s%s",
				mark.title,
				whose,
				progress(TownService.total()),
				player.DisplayName,
				progress(mine),
				suffix
			)
		end
	end
	prompt.ActionText = text
	task.delay(4, function()
		prompt.ActionText = "Read the restoration"
	end)
end

function HubService.refreshLandmarks()
	writeLandmarks()
end

-- ---------------------------------------------------------------- wiring

local function promptOn(part: BasePart, action: string, object: string): ProximityPrompt
	local existing = part:FindFirstChildOfClass("ProximityPrompt")
	if existing then
		return existing
	end
	local prompt = Instance.new("ProximityPrompt")
	prompt.ActionText, prompt.ObjectText = action, object
	prompt.HoldDuration = 0.4
	prompt.MaxActivationDistance = REACH
	prompt.RequiresLineOfSight = false
	prompt.Parent = part
	return prompt
end

function HubService.init()
	for _, lamp in ipairs(lamps()) do
		setLamp(lamp, nil)
		lamp:SetAttribute("State", "idle")
	end

	for _, hatch in ipairs(CollectionService:GetTagged(TAG_HANDIN)) do
		if hatch:IsA("BasePart") then
			local prompt = promptOn(hatch, "Hand in crates", "The Good Cheer Club")
			prompt.Triggered:Connect(function(player)
				handIn(player, prompt)
			end)
		end
	end

	for _, slot in ipairs(CollectionService:GetTagged(TAG_PLINTH)) do
		if slot:IsA("BasePart") then
			local prompt = promptOn(slot, "Shelve a relic", "The Keeping Room")
			prompt.Triggered:Connect(function(player)
				shelve(player, slot, prompt)
			end)
		end
	end

	for _, site in ipairs(CollectionService:GetTagged(TAG_LANDMARK)) do
		if site:IsA("BasePart") then
			local prompt = promptOn(site, "Read the restoration", "The Town")
			prompt.Triggered:Connect(function(player)
				readLandmark(player, site, prompt)
			end)
		end
	end
	writeLandmarks()
	-- The host is whoever arrives first, and in a fresh server that is after
	-- init, so the plates would otherwise read "no keeper has arrived" for up
	-- to a full refresh period to the very person whose town they show.
	Players.PlayerAdded:Connect(function(player)
		-- Claim here as well as in TownService. It is idempotent, and relying
		-- on two PlayerAdded handlers firing in connection order to get the
		-- host set before the plates are drawn is the kind of assumption that
		-- holds until it doesn't.
		TownService.claim(player)
		writeLandmarks()
	end)
	task.spawn(function()
		while true do
			task.wait(LANDMARK_REFRESH)
			writeLandmarks()
		end
	end)

	writeBoard()
	Players.PlayerRemoving:Connect(HubService.release)

	-- The ledger and the hall's rooms are the same fixture family as the three
	-- above, so they come up with them. Started from here rather than from
	-- init.server.luau on purpose: that file currently requires an untracked
	-- ExpeditionService (WORKLIST A8), so anything added to it cannot be
	-- committed without handing someone a broken checkout.
	LedgerService.init()
end

return HubService
