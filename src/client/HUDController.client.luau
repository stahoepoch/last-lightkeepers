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

Remotes.get("RunSnapshot").OnClientEvent:Connect(function(snapshot)
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

	-- Most urgent fact first. A downed player does not need the storm multiplier.
	if snapshot.yourKnockedDown then
		local crewUp = (snapshot.crewTotal or 1) - (snapshot.crewDown or 0)
		if crewUp > 0 then
			label.Text = string.format("DOWN - %d still standing", crewUp)
		else
			label.Text = "DOWN - no one left standing"
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
