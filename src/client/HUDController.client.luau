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

Remotes.get("RunSnapshot").OnClientEvent:Connect(function(snapshot)
	lastRunId = snapshot.runId
	local ratio = math.clamp(snapshot.stormCharge / wardenCharge, 0, 1)
	fill.Size = UDim2.new(ratio, 0, 1, 0)

	-- Colour shifts amber -> storm blue as the Warden nears. Brightness and
	-- hue both move, so the change survives colour-blind viewing.
	fill.BackgroundColor3 = AMBER:Lerp(STORM_BLUE, ratio)

	local charge = snapshot.yourCharge or 0
	chargeFill.Size = UDim2.new(math.clamp(charge, 0, 1), 0, 1, 0)
	-- Dim the charge bar when the lantern is off, so "I have fuel" and "I am
	-- burning it" are never the same reading.
	chargeFill.BackgroundTransparency = if snapshot.yourLanternOn then 0 else 0.55

	local lanterns = snapshot.activeLanterns
	local multiplier = Config.stormRate(lanterns) / Config.StormCharge.baselineRatePerSecond

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
			downedHint.Text = "Press R to anchor - it burns a full lantern charge"
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
	elseif charge <= 0 then
		label.Text = "Lantern dead - storm still rising"
	elseif (snapshot.crewDown or 0) > 0 then
		label.Text = string.format("%d down - go get them", snapshot.crewDown)
	elseif lanterns > 0 then
		label.Text = string.format("%d lantern%s lit - storm rising %.1fx", lanterns, if lanterns == 1 then "" else "s", multiplier)
	else
		label.Text = "Dark - storm still rising"
	end
end)

Remotes.get("RunEnded").OnClientEvent:Connect(function(result)
	runOver = true
	endedRunId = lastRunId
	downed.Visible = false
	downedHint.Visible = false
	local lines = {}
	for tier, amount in pairs(result.payout or {}) do
		table.insert(lines, string.format("%s x%d", tier, amount))
	end
	table.sort(lines)

	local headline = if result.outcome == "extracted"
		then string.format("Extracted via %s", tostring(result.via))
		else "Lanterns burned out - half of what you carried is gone"

	label.Text = headline .. (if #lines > 0 then "  |  " .. table.concat(lines, ", ") else "  |  nothing recovered")
end)
