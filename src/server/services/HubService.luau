--!strict
-- Lanternhold's three fixtures, bound to run state.
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

local CollectionService = game:GetService("CollectionService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Players = game:GetService("Players")

local RunStateService = require(script.Parent:WaitForChild("RunStateService"))
local PlayerDataService = require(script.Parent:WaitForChild("PlayerDataService"))
local TownService = require(script.Parent:WaitForChild("TownService"))
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

	local profile = PlayerDataService.load(player)
	profile.contributions = profile.contributions or {}
	profile.contributions.RestorationCrates = (profile.contributions.RestorationCrates or 0)
		+ crates
	PlayerDataService.save(player)

	-- And to the town itself, which is one figure shared by every server.
	-- `club.crates` above is this session's roll and resets with the server;
	-- this is the number the landmarks read.
	TownService.contribute(crates)
	writeLandmarks()

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
-- only reflect the global total and change hub visuals. The crate store needs
-- a live published-place test before we trust actual contributions, but
-- thresholds don't need to wait."
--
-- So nothing here writes. It reads TownService's one global figure and dresses
-- three floor plates with it. A landmark finishing lights the plate and says
-- so, and that is the whole reward - Chris's call: they are the town coming
-- back, and a stat bonus would turn a monument into a grind target.

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

--- Dress every plate from the town total. Called at init, after a hand-in, and
--- on a timer - the total refreshes from other servers in the background, so a
--- plate that only updated on local hand-ins would lag the town.
local function writeLandmarks()
	local scope = TownService.scope()
	local known = scope ~= "unknown"
	-- An unpublished place has no global town, so its figure is this server's
	-- own and is labelled as such. Never shown as the town's.
	local suffix = if scope == "local" then " (local)" else ""
	local total = TownService.total()
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
					if not known then
						-- Never show 0 of 10 when we simply have not read the
						-- store. "The town has done nothing" is the worse lie.
						line = "restoration unknown"
					elseif done then
						line = mark.restored .. suffix
					else
						line = string.format("%d of %d crates%s", total, mark.crates, suffix)
					end
					label.Text = string.format("%s\n%s", mark.title, line)
				end
			end
		end
	end
end

--- Pressing a plate reads the town back to you. It changes nothing.
local function readLandmark(site: BasePart, prompt: ProximityPrompt)
	local mark = landmarkFor(site)
	if not mark then
		return
	end
	local text
	local scope = TownService.scope()
	if scope == "unknown" then
		text = "restoration unknown - no town total"
	else
		local suffix = if scope == "local" then " (this server only)" else ""
		local total = TownService.total()
		text = if total >= mark.crates
			then string.format("%s - restored%s", mark.title, suffix)
			else string.format(
				"%s - %d of %d crates%s",
				mark.title,
				total,
				mark.crates,
				suffix
			)
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
			prompt.Triggered:Connect(function()
				readLandmark(site, prompt)
			end)
		end
	end
	writeLandmarks()
	task.spawn(function()
		while true do
			task.wait(LANDMARK_REFRESH)
			writeLandmarks()
		end
	end)

	writeBoard()
	Players.PlayerRemoving:Connect(HubService.release)
end

return HubService
