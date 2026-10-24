--!strict
-- Loot spawning, visibility, pickup, and extraction payout.
--
-- Two rules from the handoff are enforced here and must not be relaxed:
--   1. Baseline loot is always visible. If light revealed everything, darkness
--      would stop being a viable decision.
--   2. Relics are revealed by lantern light only - that is the reason to light up.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Remotes = require(Shared:WaitForChild("Remotes"))
local Types = require(Shared:WaitForChild("Types"))

local RunStateService = require(script.Parent:WaitForChild("RunStateService"))
local EnemyService = require(script.Parent:WaitForChild("EnemyService"))
local Audio = require(Shared:WaitForChild("Audio"))

local TAG_LOOT = "Loot"
local TAG_RELAY = "RelayBank"
local TAG_EXTRACTION = "Extraction"

local PICKUP_RANGE = 10
-- Exit markers are intentionally compact set dressing, not the actual
-- footprint a player reads as "the loading dock" or "the freight lift".
-- Requiring a keeper to balance within fourteen studs of a tiny center pad
-- made a player who had reached the exit wait through the whole window and
-- still lose their haul.  This is a forgiving holdout zone, not an exploit:
-- the caller must still reach the one active route and wait its full timer.
local EXTRACTION_RANGE = 24

local LootService = {}

-- Players with an extraction attempt in flight, so the freight lift's wait
-- cannot be started twice against one haul.
local holdoutActive = false
-- `activeExtraction` is the final door: it is selected at launch so loot can
-- be ranked for the eventual hard route and Final Call always has a known
-- destination. It is not the only usable door at the start of a raid.
local activeExtraction: BasePart? = nil
local openExtractions: { [BasePart]: boolean } = {}
local closureToken = 0

-- Raids begin with choices, then the storm removes them one by one. These are
-- relative to the 20-minute hard cap: a quick extraction is forgiving, while
-- a deep push eventually has to commit to the last route.
local EXIT_CLOSE_SECONDS = { 6 * 60, 10 * 60, 14 * 60 }

-- Loot belongs to the route, not to an architectural coordinate.  An exit is
-- selected fresh each raid, so assigning a "deep" relic while the mall is
-- built can accidentally put it beside tonight's exit.  Re-rank every tagged
-- find after the active extraction is known instead: the easy things are the
-- first things a keeper reaches on the way home, and the valuable things ask
-- for a real return trip through the mall.
local function setLootPresentation(loot: BasePart, tier: string, distance: number)
	local colours = {
		Supplies = Color3.fromRGB(105, 170, 132),
		Components = Color3.fromRGB(116, 164, 201),
		Relics = Color3.fromRGB(196, 146, 242),
		RestorationCrates = Color3.fromRGB(194, 141, 76),
	}
	local labels = {
		Supplies = "SUPPLIES",
		Components = "COMPONENTS",
		Relics = "RELIC CACHE",
		RestorationCrates = "RESTORATION CRATE",
	}

	loot:SetAttribute("Tier", tier)
	loot:SetAttribute("DistanceFromActiveExit", math.floor(distance + 0.5))
	loot.Color = colours[tier] or loot.Color
	loot.Material = if tier == "Relics" then Enum.Material.Glass else Enum.Material.Metal

	local prompt = loot:FindFirstChildOfClass("ProximityPrompt")
	if prompt then
		local isCrate = tier == "RestorationCrates"
		prompt.ActionText = if isCrate then "Lift" else "Take"
		prompt.ObjectText = labels[tier] or tier
		prompt.HoldDuration = Config.Loot.tiers[tier].pickupSeconds
		-- A relic prompt is per-player.  The client visibility controller will
		-- enable it for a lit keeper; do not reveal it to a dark keeper here.
		prompt.Enabled = Config.Loot.tiers[tier].visibility ~= "lanternOnly"
	end
end

local function assignLootForExtraction(exitPart: BasePart)
	local ranked: { { part: BasePart, distance: number } } = {}
	for _, instance in ipairs(CollectionService:GetTagged(TAG_LOOT)) do
		if instance:IsA("BasePart") then
			table.insert(
				ranked,
				{ part = instance, distance = (instance.Position - exitPart.Position).Magnitude }
			)
		end
	end
	table.sort(ranked, function(a, b)
		return a.distance < b.distance
	end)

	local count = #ranked
	if count == 0 then
		warn("[LootService] no tagged loot to rank for the active extraction")
		return
	end
	local suppliesEnd = math.max(1, math.ceil(count * 0.25))
	local componentsEnd = math.max(suppliesEnd + 1, math.ceil(count * 0.65))
	local relicEnd = math.max(componentsEnd + 1, count - 1)
	for index, entry in ipairs(ranked) do
		local tier = if index <= suppliesEnd
			then "Supplies"
			elseif index <= componentsEnd then "Components"
			elseif index <= relicEnd then "Relics"
			else "RestorationCrates"
		setLootPresentation(entry.part, tier, entry.distance)
	end
	print(
		`[LootService] ranked {count} finds from active extraction; supplies <= {math.floor(
			ranked[suppliesEnd].distance + 0.5
		)} studs, deepest cache {math.floor(ranked[count].distance + 0.5)} studs`
	)
end

local function setExitSignal(exitPart: BasePart, mode: "available" | "inbound" | "final" | nil)
	local enabled = mode ~= nil
	local light = exitPart:FindFirstChild("ExtractionSignal") :: PointLight?
	if not light then
		light = Instance.new("PointLight")
		light.Name = "ExtractionSignal"
		light.Range, light.Brightness, light.Shadows = 36, 2, false
		light.Parent = exitPart
	end
	light.Color = if mode == "final"
		then Color3.fromRGB(255, 94, 72)
		elseif mode == "available" then Color3.fromRGB(108, 194, 255)
		else Color3.fromRGB(255, 204, 112)
	light.Enabled = enabled
	local flare = exitPart:FindFirstChild("ExtractionFlare") :: PointLight?
	if not flare then
		flare = Instance.new("PointLight")
		flare.Name = "ExtractionFlare"
		flare.Range, flare.Brightness, flare.Shadows = 52, 0, false
		flare.Parent = exitPart
	end
	flare.Color = if mode == "inbound" then Color3.fromRGB(255, 48, 38) else light.Color
	flare.Enabled = mode == "inbound"
	local marker = exitPart:FindFirstChild("ExtractionStatus") :: BillboardGui?
	if not marker then
		marker = Instance.new("BillboardGui")
		marker.Name = "ExtractionStatus"
		marker.Size, marker.StudsOffset, marker.AlwaysOnTop, marker.MaxDistance =
			UDim2.fromOffset(280, 48), Vector3.new(0, 8, 0), true, 350
		marker.Parent = exitPart
		local label = Instance.new("TextLabel")
		label.Name, label.Size, label.BackgroundTransparency = "Label", UDim2.fromScale(1, 1), 1
		label.Font, label.TextSize, label.TextStrokeTransparency = Enum.Font.GothamBold, 17, 0.35
		label.Parent = marker
	end
	local label = marker:FindFirstChild("Label") :: TextLabel
	label.TextColor3 = light.Color
	label.Text = if mode == "final"
		then "FINAL CALL"
		elseif mode == "inbound" then "EXTRACTION INBOUND — HOLD THE ZONE"
		elseif mode == "available" then "ACTIVE EXIT — HOLD E TO CALL"
		else ""
	marker.Enabled = enabled
	if mode == "inbound" then
		local siren = exitPart:FindFirstChild("ExtractionSiren") :: Sound?
		if not siren then
			siren = Audio.make("ExtractionSiren", exitPart)
			if siren then
				siren.Name = "ExtractionSiren"
				siren.RollOffMinDistance, siren.RollOffMaxDistance = 20, 450
			end
		end
		if siren and not siren.IsPlaying then
			siren:Play()
		end
		local token = os.clock()
		exitPart:SetAttribute("ExtractionFlareToken", token)
		task.spawn(function()
			while
				exitPart.Parent
				and exitPart:GetAttribute("ExtractionFlareToken") == token
				and flare
				and flare.Enabled
			do
				flare.Brightness = if flare.Brightness > 0 then 0 else 5
				task.wait(0.35)
			end
		end)
	else
		exitPart:SetAttribute("ExtractionFlareToken", 0)
		if flare then
			flare.Brightness = 0
		end
		local siren = exitPart:FindFirstChild("ExtractionSiren")
		if siren and siren:IsA("Sound") then
			siren:Stop()
		end
	end
end

local function extractionStartPosition(): Vector3?
	local taggedStarts = CollectionService:GetTagged("RunStart")
	for _, start in ipairs(taggedStarts) do
		if start:IsA("BasePart") then
			return start.Position
		end
	end
	for _, instance in ipairs(workspace:GetDescendants()) do
		if instance:IsA("SpawnLocation") then
			return instance.Position
		end
	end
	return nil
end

local function extractionLabel(id: string): string
	local def = Config.Loot.extraction[id]
	return `Hold extraction ({def.arrivalSeconds}s)`
end

-- The Relay Annex is a physical route board, not a second HUD. One active
-- exit is fair only if a player can learn which door it is without walking to
-- three dead ones first.
local function updateRelayRoute(exit: BasePart?)
	for _, relay in ipairs(CollectionService:GetTagged(TAG_RELAY)) do
		if relay:IsA("BasePart") then
			local board = relay:FindFirstChild("RouteBoard") :: BillboardGui?
			if not board then
				board = Instance.new("BillboardGui")
				board.Name = "RouteBoard"
				board.Size = UDim2.fromOffset(260, 82)
				board.StudsOffset = Vector3.new(0, 5, 0)
				board.AlwaysOnTop = true
				board.MaxDistance = 45
				board.Parent = relay
				local text = Instance.new("TextLabel")
				text.Name = "Readout"
				text.Size = UDim2.fromScale(1, 1)
				text.BackgroundColor3 = Color3.fromRGB(10, 18, 22)
				text.BackgroundTransparency = 0.15
				text.BorderColor3 = Color3.fromRGB(87, 190, 178)
				text.TextColor3 = Color3.fromRGB(194, 255, 244)
				text.TextStrokeTransparency = 0.65
				text.Font = Enum.Font.GothamBold
				text.TextSize = 15
				text.TextWrapped = true
				text.Parent = board
			end
			local readout = board:FindFirstChild("Readout")
			if readout and readout:IsA("TextLabel") then
				if exit then
					local route = exit.Name:gsub("^Exit_", ""):gsub("_", " ")
					local id = exit:GetAttribute("ExtractionId")
					readout.Text = `FINAL ROUTE\n{route} — {id}`
				else
					readout.Text = "NO ACTIVE RAID"
				end
			end
		end
	end
end

--- How far from the run start an exit has to be before it counts as a real
--- one. The rule used to be purely relative - at least 70% as far as the
--- farthest candidate - and that turned out to exclude exactly the exits it
--- was worth having. The Galleria's two mid-arm anchors sit 349 and 398 studs
--- from the court against a 464 stud cutoff, so they were tagged and never
--- once selectable, and pushing them further out to qualify would have made
--- them no nearer than the arm ends they were meant to be an alternative to.
---
--- The intent was never "70%". It was "not a fake random exit beside spawn",
--- and that is an absolute distance, so it is written as one. The ratio stays
--- as a second guard so a much larger map cannot admit a proportionally
--- useless candidate.
local MIN_EXIT_STUDS = 300
local MIN_EXIT_FRACTION = 0.45

--- Selects one eligible exit for this run. Designers can add any number of
--- tagged Extraction candidates; the server chooses randomly among those far
--- enough from the run start to be worth walking to. A map whose candidates
--- are all nearer than that still plays - the farthest one is used - so a map
--- under construction is never left with no way out.
local function setExtractionOpen(candidate: BasePart, open: boolean)
	openExtractions[candidate] = open
	candidate:SetAttribute("ActiveThisRun", open)
	setExitSignal(candidate, if open then "available" else nil)
	local prompt = candidate:FindFirstChild("ExtractionPrompt")
	if prompt and prompt:IsA("ProximityPrompt") then
		prompt.Enabled = open
		if open then
			prompt.ActionText = extractionLabel(candidate:GetAttribute("ExtractionId") :: string)
		end
	end
end

local function closeOneExtraction(candidates: { BasePart })
	-- Never close the final route. Prefer closing the farthest currently-open
	-- alternate first: the mall gets more dangerous over time without stranding
	-- somebody who was already committing to a nearby exit.
	local closing: BasePart? = nil
	local farthest = -1
	local start = extractionStartPosition()
	for _, candidate in ipairs(candidates) do
		if candidate ~= activeExtraction and openExtractions[candidate] then
			local distance = if start then (candidate.Position - start).Magnitude else 0
			if distance > farthest then
				closing, farthest = candidate, distance
			end
		end
	end
	if closing then
		setExtractionOpen(closing, false)
		print(`[LootService] exit closed: {closing:GetAttribute("ExtractionId")}`)
	end
end

function LootService.beginRun()
	local candidates: { BasePart } = {}
	for _, candidate in ipairs(CollectionService:GetTagged(TAG_EXTRACTION)) do
		local id = candidate:GetAttribute("ExtractionId")
		if candidate:IsA("BasePart") and typeof(id) == "string" and Config.Loot.extraction[id] then
			table.insert(candidates, candidate)
		end
	end

	activeExtraction = nil
	local startPosition = extractionStartPosition()
	local farCandidates = candidates
	if startPosition and #candidates > 1 then
		local farthest = 0
		for _, candidate in ipairs(candidates) do
			farthest = math.max(farthest, (candidate.Position - startPosition).Magnitude)
		end
		local minimumDistance = math.max(MIN_EXIT_STUDS, farthest * MIN_EXIT_FRACTION)
		farCandidates = {}
		for _, candidate in ipairs(candidates) do
			if (candidate.Position - startPosition).Magnitude >= minimumDistance then
				table.insert(farCandidates, candidate)
			end
		end
		-- A map whose every exit is inside the absolute minimum is a small or
		-- half-built one, not a broken one. Fall back to the farthest rather
		-- than reporting no escape route.
		if #farCandidates == 0 then
			for _, candidate in ipairs(candidates) do
				if (candidate.Position - startPosition).Magnitude >= farthest - 0.5 then
					table.insert(farCandidates, candidate)
				end
			end
		end
	end
	if #farCandidates > 0 then
		activeExtraction = farCandidates[Random.new():NextInteger(1, #farCandidates)]
	end

	openExtractions = {}
	for _, candidate in ipairs(candidates) do
		setExtractionOpen(candidate, true)
	end

	if activeExtraction then
		updateRelayRoute(activeExtraction)
		assignLootForExtraction(activeExtraction)
		print(
			`[LootService] final extraction: {activeExtraction:GetAttribute("ExtractionId")}; all {#candidates} exits open`
		)
	else
		warn("[LootService] no valid tagged Extraction candidate; the run has no escape route")
	end

	closureToken += 1
	local mine = closureToken
	task.spawn(function()
		for _, atSeconds in ipairs(EXIT_CLOSE_SECONDS) do
			local run = RunStateService.current()
			local startedAt = run and run.startedAt or os.clock()
			local remaining = atSeconds - (os.clock() - startedAt)
			if remaining > 0 then
				task.wait(remaining)
			end
			run = RunStateService.current()
			if mine ~= closureToken or not run or run.finished then
				return
			end
			closeOneExtraction(candidates)
		end
	end)
end

local function tierOf(instance: Instance): Types.LootTier?
	local value = instance:GetAttribute("Tier")
	if typeof(value) ~= "string" then
		return nil
	end
	if Config.Loot.tiers[value] == nil then
		return nil
	end
	return value :: any
end

local function withinRange(player: Player, instance: Instance, range: number?): boolean
	local character = player.Character
	local root = character and character:FindFirstChild("HumanoidRootPart")
	if not root or not instance:IsA("BasePart") then
		return false
	end
	local limit = range or PICKUP_RANGE
	return ((root :: BasePart).Position - (instance :: BasePart).Position).Magnitude <= limit
end

--- Relics only exist for a player whose own lantern is lit.
function LootService.isVisibleTo(instance: Instance, state: Types.LightkeeperState): boolean
	local tier = tierOf(instance)
	if not tier then
		return false
	end
	local def = Config.Loot.tiers[tier]
	if def.visibility == "lanternOnly" then
		return state.lanternOn and state.lanternCharge > 0
	end
	return true
end

function LootService.handlePickup(player: Player, instance: Instance)
	local run = RunStateService.current()
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	if not instance or not instance:IsDescendantOf(workspace) then
		return
	end
	if not CollectionService:HasTag(instance, TAG_LOOT) then
		return
	end
	if not withinRange(player, instance) then
		return
	end

	local tier = tierOf(instance)
	if not tier or not LootService.isVisibleTo(instance, state) then
		return
	end

	if tier == "RestorationCrates" then
		if state.carryingCrate then
			return
		end
		state.carryingCrate = true
	end

	local amount = instance:GetAttribute("Amount")
	amount = if typeof(amount) == "number" then math.clamp(amount, 1, 99) else 1

	state.haul.carried[tier] = (state.haul.carried[tier] or 0) + amount
	RunStateService.refreshMovement(player)
	instance:Destroy()
end

function LootService.handleBank(player: Player)
	RunStateService.bank(player)
end

function LootService.handleExtraction(player: Player, exitPart: Instance)
	local run = RunStateService.current()
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	-- A downed Lightkeeper cannot loot, but can call the exit. Enemies already
	-- ignore downed keepers; this lets the crew crawl one into a rescue holdout
	-- instead of treating a knockdown as a mandatory abandonment.
	if not state then
		return
	end

	if not exitPart:IsA("BasePart") or not CollectionService:HasTag(exitPart, TAG_EXTRACTION) then
		return
	end
	local extractionId = exitPart:GetAttribute("ExtractionId")
	if typeof(extractionId) ~= "string" then
		return
	end
	local def = Config.Loot.extraction[extractionId]
	if not def then
		return
	end
	if not openExtractions[exitPart] then
		return
	end

	-- The client picks the exit it is standing at, not the exit it wants. Resolve
	-- a real tagged instance carrying that id and check the player is actually
	-- there; otherwise anyone could extract from anywhere by naming a string.
	if not withinRange(player, exitPart, EXTRACTION_RANGE) then
		return
	end

	-- One crew holdout at a time. The exit belongs to the whole crew, not the
	-- first player who pressed its prompt.
	if holdoutActive then
		return
	end
	holdoutActive = true
	setExitSignal(exitPart, "inbound")
	local endsAt = workspace:GetServerTimeNow() + def.arrivalSeconds
	EnemyService.beginExtractionPressure(exitPart, os.clock() + def.arrivalSeconds)
	for _, teammate in ipairs(Players:GetPlayers()) do
		Remotes.get("ExtractionState"):FireClient(teammate, {
			active = true,
			via = extractionId,
			exit = exitPart,
			caller = player.DisplayName,
			endsAt = endsAt,
		})
	end

	-- Every exit has a hold now. The active route is never a free escape hatch.
	if def.arrivalSeconds > 0 then
		task.wait(def.arrivalSeconds)
		-- The called vehicle stays on its way. The crew may spread out to rescue
		-- someone, but only keepers inside the zone when it arrives get out.
		if run.finished or not exitPart:IsDescendantOf(workspace) then
			holdoutActive = false
			EnemyService.endExtractionPressure(exitPart)
			setExitSignal(exitPart, if run.finished then nil else "available")
			for _, teammate in ipairs(Players:GetPlayers()) do
				Remotes.get("ExtractionState")
					:FireClient(teammate, { active = false, cancelled = true })
			end
			return
		end
	end

	holdoutActive = false
	EnemyService.endExtractionPressure(exitPart)
	setExitSignal(exitPart, "available")
	for _, teammate in ipairs(Players:GetPlayers()) do
		Remotes.get("ExtractionState"):FireClient(
			teammate,
			{ active = false, arrived = true, via = extractionId, exit = exitPart }
		)
	end

	local evacuees: { Player } = {}
	for userId in pairs(run.lightkeepers) do
		local keeper = Players:GetPlayerByUserId(userId)
		if keeper and withinRange(keeper, exitPart, EXTRACTION_RANGE) then
			table.insert(evacuees, keeper)
		end
	end
	for _, keeper in ipairs(evacuees) do
		local keeperState = run.lightkeepers[keeper.UserId]
		if keeperState then
			local payout = RunStateService.resolveHaul(keeperState, true)
			for tier, amount in pairs(payout) do
				payout[tier] = math.floor(amount * def.payoutModifier)
			end
			Remotes.get("RunEnded")
				:FireClient(keeper, { outcome = "extracted", via = extractionId, payout = payout })
			LootService.onExtracted(keeper, payout)
			RunStateService.finishExtraction(keeper)
		end
	end

	-- The vehicle leaves after each window, but the route remains callable for
	-- whoever stayed behind. A cautious keeper may evacuate early while another
	-- pushes deeper; every call is still loud, timed, and only saves those in
	-- the zone when it arrives. The raid ends only when its final keeper leaves
	-- (or the remaining crew wipes).
end

LootService.onExtracted = function(_player: Player, _payout: any) end

-- The clock-expiry version of extraction: no one has to press E again, but
-- only keepers physically at the already-selected route survive Final Call.
function LootService.finalEvacuate()
	local run, exitPart = RunStateService.current(), activeExtraction
	if not run or run.finished or not exitPart then
		RunStateService.wipe()
		return
	end
	local id = exitPart:GetAttribute("ExtractionId")
	local def = typeof(id) == "string" and Config.Loot.extraction[id] or nil
	if not def then
		RunStateService.wipe()
		return
	end
	for userId in pairs(run.lightkeepers) do
		local player = Players:GetPlayerByUserId(userId)
		if player and withinRange(player, exitPart, EXTRACTION_RANGE) then
			local state = run.lightkeepers[userId]
			if state then
				local payout = RunStateService.resolveHaul(state, true)
				for tier, amount in pairs(payout) do
					payout[tier] = math.floor(amount * def.payoutModifier)
				end
				Remotes.get("RunEnded")
					:FireClient(
						player,
						{ outcome = "extracted", via = "Final Call", payout = payout }
					)
				LootService.onExtracted(player, payout)
				RunStateService.finishExtraction(player)
			end
		end
	end
	if not run.finished then
		RunStateService.wipe()
	end
end

function LootService.finalExit(): BasePart?
	return activeExtraction
end

function LootService.init()
	-- LootRequest and ExtractionRequest USED to be listened to here, and both
	-- listeners are gone deliberately.
	--
	-- Nothing fires them. Picking something up and calling an extraction both
	-- run through the ProximityPrompts that InteractionService builds, and the
	-- prompt's hold is the mechanic: "Taking something is a held action, not a
	-- tap... important finds make you stand exposed longer." A relic cache is
	-- 2.5 seconds of standing still in a dark mall, and that exposure is the
	-- price of the tier.
	--
	-- A remote that reaches handlePickup directly skips all of it. The
	-- function itself validates properly - run state, the loot tag, range,
	-- visibility, the crate limit, a clamped amount - so this was never a way
	-- to conjure loot that is not there. It was a way to collect real loot
	-- INSTANTLY, which costs an exploiter nothing and quietly deletes the one
	-- cost looting has.
	--
	-- The remotes stay declared in the shared remote list: they are the
	-- vocabulary, an unlistened RemoteEvent does nothing when fired, and a
	-- client path may want them later - with the hold enforced server-side
	-- rather than assumed.

	-- Relays are utility points, not insurance: stepping into one can recharge
	-- a lantern if that optional tuning is enabled, but it cannot secure loot.
	--
	-- Watched rather than scanned once, matching Loot and Extraction. A map
	-- built or tagged after this service starts - which is how the generated
	-- Flooded Mall actually arrives - left the relay with no Touched handler at
	-- all, so banking silently did nothing and the loop could not be completed.
	local function attachRelay(relay: Instance)
		if not relay:IsA("BasePart") or relay:GetAttribute("__RelayWired") then
			return
		end
		relay:SetAttribute("__RelayWired", true)
		relay.Touched:Connect(function(hit)
			local player = Players:GetPlayerFromCharacter(hit.Parent)
			if player then
				LootService.handleBank(player)
			end
		end)
	end

	for _, relay in ipairs(CollectionService:GetTagged(TAG_RELAY)) do
		attachRelay(relay)
	end
	CollectionService:GetInstanceAddedSignal(TAG_RELAY):Connect(attachRelay)

	-- Extraction volumes are driven by the client's ExtractionRequest (the player
	-- chooses to leave), so they only need to exist and carry an ExtractionId.
	for _, exit in ipairs(CollectionService:GetTagged(TAG_EXTRACTION)) do
		if exit:GetAttribute("ExtractionId") == nil then
			warn(`[LootService] extraction "{exit:GetFullName()}" has no ExtractionId attribute`)
		end
	end
end

return LootService
