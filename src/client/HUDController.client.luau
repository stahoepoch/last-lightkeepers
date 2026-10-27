--!strict
-- HUD: the Storm Charge meter and the run-end summary.
--
-- The meter is the whole game's tension in one element. It must read instantly,
-- never animate downward, and stay legible for red-green colour-blind players -
-- amber fill against an electric-blue track, never red-on-green.

local Players = game:GetService("Players")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Remotes = require(Shared:WaitForChild("Remotes"))

local player = Players.LocalPlayer
local playerGui = player:WaitForChild("PlayerGui")

local AMBER = Color3.fromRGB(255, 186, 94)
local STORM_BLUE = Color3.fromRGB(64, 110, 200)
local TRACK = Color3.fromRGB(18, 22, 34)

local wardenCharge = Config.wardenCharge()

-- With unlimited light the charge bar is a gauge that never moves, which is
-- worse than no gauge: it reads as a resource right up until you notice it is
-- decorative. Chris's call that the lantern never runs out, so it goes.
local lanternDrains = Config.lanternDrainPerSecond() > 0

local screen = Instance.new("ScreenGui")
screen.Name = "LightkeeperHUD"
screen.ResetOnSpawn = false
screen.IgnoreGuiInset = true
screen.Parent = playerGui

local meterFrame = Instance.new("Frame")
meterFrame.Name = "StormMeter"
meterFrame.AnchorPoint = Vector2.new(0.5, 0)
meterFrame.Position = UDim2.new(0.5, 0, 0, 24)
meterFrame.Size = UDim2.new(0, 340, 0, 18)
meterFrame.BackgroundColor3 = TRACK
meterFrame.BorderSizePixel = 0
meterFrame.Parent = screen

local corner = Instance.new("UICorner")
corner.CornerRadius = UDim.new(0, 9)
corner.Parent = meterFrame

local fill = Instance.new("Frame")
fill.Name = "Fill"
fill.Size = UDim2.new(0, 0, 1, 0)
fill.BackgroundColor3 = AMBER
fill.BorderSizePixel = 0
fill.Parent = meterFrame

local fillCorner = corner:Clone()
fillCorner.Parent = fill

-- Threshold ticks, so a crew can see the next step coming rather than guessing.
for _, threshold in ipairs(Config.StormCharge.thresholds) do
	local tick = Instance.new("Frame")
	tick.Name = threshold.id
	tick.Size = UDim2.new(0, 2, 1, 0)
	tick.Position = UDim2.new(math.clamp(threshold.value / wardenCharge, 0, 1), -1, 0, 0)
	tick.BackgroundColor3 = STORM_BLUE
	tick.BorderSizePixel = 0
	tick.ZIndex = 2
	tick.Parent = meterFrame
end

-- Lantern charge. Separate from the storm meter on purpose: one is the crew's
-- shared clock, this is your personal fuel. Conflating them would hide which
-- resource a player is actually running out of.
local chargeTrack = Instance.new("Frame")
chargeTrack.Visible = lanternDrains
chargeTrack.Name = "LanternCharge"
chargeTrack.AnchorPoint = Vector2.new(0.5, 0)
chargeTrack.Position = UDim2.new(0.5, 0, 1, 4)
chargeTrack.Size = UDim2.new(0, 220, 0, 6)
chargeTrack.BackgroundColor3 = TRACK
chargeTrack.BorderSizePixel = 0
chargeTrack.Parent = meterFrame

local chargeCorner = Instance.new("UICorner")
chargeCorner.CornerRadius = UDim.new(0, 3)
chargeCorner.Parent = chargeTrack

local chargeFill = Instance.new("Frame")
chargeFill.Name = "Fill"
chargeFill.Size = UDim2.new(1, 0, 1, 0)
chargeFill.BackgroundColor3 = AMBER
chargeFill.BorderSizePixel = 0
chargeFill.Parent = chargeTrack

local chargeFillCorner = chargeCorner:Clone()
chargeFillCorner.Parent = chargeFill

local label = Instance.new("TextLabel")
label.Name = "Status"
label.AnchorPoint = Vector2.new(0.5, 0)
label.Position = UDim2.new(0.5, 0, 1, 16)
label.Size = UDim2.new(1, 0, 0, 18)
label.BackgroundTransparency = 1
label.Font = Enum.Font.GothamMedium
label.TextSize = 14
label.TextColor3 = Color3.fromRGB(226, 232, 240)
label.Text = "Storm closing"
label.Parent = meterFrame

-- The choice to keep pushing needs a visible price. This is deliberately a
-- compact readout, not a grid backpack: it answers only "what will I lose if
-- I do not make it out?" and is populated solely from the server snapshot.
local haulFrame = Instance.new("Frame")
haulFrame.Name = "HaulReadout"
haulFrame.AnchorPoint = Vector2.new(1, 0)
haulFrame.Position = UDim2.new(1, -20, 0, 26)
haulFrame.Size = UDim2.fromOffset(190, 116)
haulFrame.BackgroundColor3 = TRACK
haulFrame.BackgroundTransparency = 0.16
haulFrame.BorderColor3 = Color3.fromRGB(92, 113, 142)
haulFrame.BorderSizePixel = 1
haulFrame.Visible = false
haulFrame.Parent = screen

local haulTitle = Instance.new("TextLabel")
haulTitle.Name = "Title"
haulTitle.Position, haulTitle.Size, haulTitle.BackgroundTransparency = UDim2.fromOffset(12, 6), UDim2.fromOffset(166, 20), 1
haulTitle.Font, haulTitle.TextSize, haulTitle.TextXAlignment = Enum.Font.GothamBold, 13, Enum.TextXAlignment.Left
haulTitle.TextColor3, haulTitle.Text = Color3.fromRGB(255, 222, 161), "CARRIED — LOSE ON WIPE"
haulTitle.Parent = haulFrame

local haulLines = Instance.new("TextLabel")
haulLines.Name = "Items"
haulLines.Position, haulLines.Size, haulLines.BackgroundTransparency = UDim2.fromOffset(12, 29), UDim2.fromOffset(166, 78), 1
haulLines.Font, haulLines.TextSize, haulLines.TextXAlignment, haulLines.TextYAlignment = Enum.Font.GothamMedium, 14, Enum.TextXAlignment.Left, Enum.TextYAlignment.Top
haulLines.TextColor3, haulLines.TextWrapped = Color3.fromRGB(221, 230, 239), true
haulLines.Parent = haulFrame

local function updateHaul(haul: { [string]: number }?, burden: number?)
	local order = { "Supplies", "Components", "Relics", "RestorationCrates" }
	local names = { RestorationCrates = "Restoration crates" }
	local lines = {}
	for _, tier in ipairs(order) do
		local amount = haul and haul[tier] or 0
		if amount and amount > 0 then
			table.insert(lines, string.format("%s  x%d", names[tier] or tier, amount))
		end
	end
	haulTitle.Text = if burden and burden > 0 then string.format("CARRIED — %d%% SLOWER", math.floor(math.min(burden * 4.5, 42) + 0.5)) else "CARRIED — LOSE ON WIPE"
	haulFrame.Visible = #lines > 0
	haulLines.Text = table.concat(lines, "\n")
end

-- Once a run has ended, the summary stays on screen. Extraction does not stop
-- the run tick, so the next snapshot - 250ms later - used to overwrite the
-- payout line before anyone could read it, and a player never saw what they
-- came back with.
local runOver = false
local endedRunId: string? = nil
local lastRunId: string? = nil

-- Being downed refuses your lantern, your pickups and your movement, all three
-- silently. A one-line label at the top of a pitch-black screen is not enough to
-- explain that - Chris read it as the game being broken. This says it in the
-- middle of the screen, where someone lying on the floor in the dark will see it.
local downed = Instance.new("TextLabel")
downed.Name = "DownedBanner"
downed.AnchorPoint = Vector2.new(0.5, 0.5)
downed.Position = UDim2.new(0.5, 0, 0.5, 0)
downed.Size = UDim2.new(1, 0, 0, 44)
downed.BackgroundTransparency = 1
downed.Font = Enum.Font.GothamBold
downed.TextSize = 28
downed.TextColor3 = AMBER
downed.TextStrokeTransparency = 0.4
downed.Visible = false
downed.Parent = screen

-- A Sweeper landing a hit used to change nothing a player could perceive: the
-- count lived in run state and the first you knew of it was collapsing. Chris:
-- "i got hit by a sweeper and didn't seem like it did anything."
local hit = Instance.new("TextLabel")
hit.Name = "HitFlash"
hit.AnchorPoint = Vector2.new(0.5, 0.5)
hit.Position = UDim2.new(0.5, 0, 0.5, -70)
hit.Size = UDim2.new(1, 0, 0, 30)
hit.BackgroundTransparency = 1
hit.Font = Enum.Font.GothamBold
hit.TextSize = 22
hit.TextColor3 = Color3.fromRGB(255, 122, 92)
hit.TextStrokeTransparency = 0.4
hit.Visible = false
hit.Parent = screen

local lastHits = 0
local hitShownAt = 0

local downedHint = downed:Clone()
downedHint.Name = "DownedHint"
downedHint.Position = UDim2.new(0.5, 0, 0.5, 30)
downedHint.TextSize = 18
downedHint.Font = Enum.Font.GothamMedium
downedHint.TextColor3 = Color3.fromRGB(226, 232, 240)
downedHint.Parent = screen

local crewAlert = downed:Clone()
crewAlert.Name = "CrewEmergency"
crewAlert.Position = UDim2.new(0.5, 0, 0.72, 0)
crewAlert.TextSize = 19
crewAlert.TextColor3 = Color3.fromRGB(150, 208, 255)
crewAlert.Visible = false
crewAlert.Parent = screen

local extractionAlert = crewAlert:Clone()
extractionAlert.Name = "ExtractionHoldout"
extractionAlert.Position = UDim2.new(0.5, 0, 0.18, 0)
extractionAlert.TextColor3 = Color3.fromRGB(255, 222, 161)
extractionAlert.Parent = screen

Remotes.get("DownedSignal").OnClientEvent:Connect(function(signal)
	crewAlert.Text = string.format("KEEPER DOWN: %s — FIND THEM", tostring(signal.name))
	crewAlert.Visible = true
	task.delay(7, function() crewAlert.Visible = false end)
end)

Remotes.get("ExtractionState").OnClientEvent:Connect(function(state)
	if not state.active then
		extractionAlert.Visible = false
		return
	end
	extractionAlert.Visible = true
	task.spawn(function()
		while extractionAlert.Visible and state.endsAt do
			local left = math.max(0, state.endsAt - workspace:GetServerTimeNow())
			local root = Players.LocalPlayer.Character and Players.LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
			-- Multiple doors begin open. The server tells us which physical door
			-- was called so this message never judges distance to a different exit.
			local activeExit = if typeof(state.exit) == "Instance" and state.exit:IsA("BasePart")
				then state.exit
				else nil
			-- Match the server's forgiving physical exit zone. The visible dock/lift
			-- footprint is larger than its tiny marker part, so the HUD must not say
			-- "GET TO THE EXIT" while the server will successfully extract you.
			local zone = if root and activeExit and activeExit:IsA("BasePart") then (root.Position - activeExit.Position).Magnitude <= 24 else false
			local place = if zone then "YOU ARE IN THE EXIT" else "GET TO THE EXIT"
			extractionAlert.Text = string.format("%s CALLED BY %s — %ds — %s", tostring(state.via), tostring(state.caller), math.ceil(left), place)
			if left <= 0 then break end
			task.wait(0.1)
		end
	end)
end)

Remotes.get("RunSnapshot").OnClientEvent:Connect(function(snapshot)
	lastRunId = snapshot.runId
	local ratio = math.clamp(snapshot.stormCharge / wardenCharge, 0, 1)
	fill.Size = UDim2.new(ratio, 0, 1, 0)

	-- Colour shifts amber -> storm blue as the Warden nears. Brightness and
	-- hue both move, so the change survives colour-blind viewing.
	fill.BackgroundColor3 = AMBER:Lerp(STORM_BLUE, ratio)

	local charge = snapshot.yourCharge or 0
	if lanternDrains then
		chargeFill.Size = UDim2.new(math.clamp(charge, 0, 1), 0, 1, 0)
	end
	-- Dim the charge bar when the lantern is off, so "I have fuel" and "I am
	-- burning it" are never the same reading.
	chargeFill.BackgroundTransparency = if snapshot.yourLanternOn then 0 else 0.55

	local lanterns = snapshot.activeLanterns
	local multiplier = Config.stormRate(lanterns) / Config.StormCharge.baselineRatePerSecond
	local remaining = math.max(0, snapshot.runSecondsRemaining or 0)
	updateHaul(snapshot.yourHaul, snapshot.yourBurden)
	local minutes = math.floor(remaining / 60)
	local seconds = math.floor(remaining % 60)

	-- A new expedition clears the last one's summary.
	if runOver and snapshot.runId ~= endedRunId then
		runOver = false
		endedRunId = nil
	end
	if runOver then
		downed.Visible = false
		downedHint.Visible = false
		return
	end

	-- Only on the rising edge: a hit is an event, not a status.
	local hits = snapshot.yourContactHits or 0
	local limit = snapshot.hitsToKnockDown or 3
	if hits > lastHits and not snapshot.yourKnockedDown then
		local left = math.max(limit - hits, 0)
		hit.Text = if left == 1 then "HIT - one more and you are down" else string.format("HIT - %d more and you are down", left)
		hit.Visible = true
		hitShownAt = os.clock()
	end
	lastHits = hits
	if hit.Visible and os.clock() - hitShownAt > 1.6 then
		hit.Visible = false
	end

	downed.Visible = snapshot.yourKnockedDown == true
	if downed.Visible then
		hit.Visible = false
	end
	downedHint.Visible = downed.Visible
	if downed.Visible then
		downed.Text = "YOU ARE DOWN"
		local crewUp = (snapshot.crewTotal or 1) - (snapshot.crewDown or 0)
		if crewUp > 0 then
			downedHint.Text = string.format("%d still standing - wait for a hand up", crewUp)
		elseif snapshot.yourCanAnchorRevive then
			downedHint.Text = if lanternDrains
				then "Press R to anchor - it burns a full lantern charge"
				else "Press R to anchor - it spikes the storm, once per run"
		elseif (snapshot.yourAnchorRevivesLeft or 0) <= 0 then
			downedHint.Text = "No anchor left this run"
		else
			downedHint.Text = "Anchor needs a full lantern charge"
		end
	end

	-- Most urgent fact first. A downed player does not need the storm multiplier.
	if snapshot.yourKnockedDown then
		local crewUp = (snapshot.crewTotal or 1) - (snapshot.crewDown or 0)
		if crewUp > 0 then
			label.Text = string.format("DOWN - %d still standing", crewUp)
		elseif snapshot.yourCanAnchorRevive then
			label.Text = "DOWN - press R to anchor (burns a full charge)"
		elseif (snapshot.yourAnchorRevivesLeft or 0) <= 0 then
			label.Text = "DOWN - no anchor left this run"
		else
			-- The keybind is deliberately suppressed when the price cannot be
			-- paid; saying so is the difference between a cost and a dead key.
			label.Text = "DOWN - anchor needs a full lantern charge"
		end
	elseif snapshot.wardenActive then
		label.Text = "THE WARDEN IS COMING"
	elseif snapshot.yourTagged then
		label.Text = "TAGGED - they know where you are"
	elseif snapshot.highestThreshold == "PatrolsHunt" then
		label.Text = "HUNT - lit players can be scanned"
	elseif snapshot.highestThreshold == "PatrolsAlert" then
		label.Text = "ALERT - Sweepers are ranging farther"
	elseif lanternDrains and charge <= 0 then
		label.Text = "Lantern dead - storm still rising"
	elseif (snapshot.crewDown or 0) > 0 then
		label.Text = string.format("%d down - go get them", snapshot.crewDown)
	elseif lanterns > 0 then
		label.Text = string.format("%d lantern%s lit - storm rising %.1fx", lanterns, if lanterns == 1 then "" else "s", multiplier)
	else
		label.Text = "Dark - storm still rising"
	end
	label.Text ..= string.format("  |  %02d:%02d left", minutes, seconds)
end)

Remotes.get("RunEnded").OnClientEvent:Connect(function(result)
	runOver = true
	endedRunId = lastRunId
	downed.Visible = false
	downedHint.Visible = false
	updateHaul(nil, nil)
	local lines = {}
	for tier, amount in pairs(result.payout or {}) do
		table.insert(lines, string.format("%s x%d", tier, amount))
	end
	table.sort(lines)

	local headline = if result.outcome == "extracted"
		then string.format("Extracted via %s", tostring(result.via))
		else "The storm took the haul - extract or lose everything"

	label.Text = headline .. (if #lines > 0 then "  |  " .. table.concat(lines, ", ") else "  |  nothing recovered")
end)
