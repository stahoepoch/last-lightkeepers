--!strict
-- A direction, not a minimap. The mall should remain something players learn
-- to navigate, but an active extraction cannot be a secret that a crew only
-- discovers by accident after surviving the whole run.
--
-- The first version of this was correct and unreadable. It rotated a 28 pixel
-- "▲" glyph inside a small box at the top of the screen, and Chris walked it:
-- "the UI up here is pretty crappy, the arrow is impossible to know where its
-- pointing, very awkward. a better navigation/mini map would be nice."
--
-- He is right about the glyph. A small rotating triangle has no frame of
-- reference - you cannot tell 20 degrees from 40, and when the exit is behind
-- you it points down and reads as nothing at all. So there are three things
-- here instead, and none of them is a minimap:
--
--   1. A COMPASS STRIP. Cardinal marks sliding past a fixed centre line, the
--      way every first-person game does it, so a bearing has something to be
--      measured against. The exit rides the strip as a labelled tick.
--   2. The READOUT, with a much larger arrow, kept for distance.
--   3. An EDGE ARROW that pins to the side of the screen the exit is on when
--      it is out of view - which is the case the old arrow handled worst.
--
-- Built out of frames and text only. No asset ids: nothing here can break
-- because an upload was never done, and nothing needs a licence for a future
-- Steam build.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")
local RunService = game:GetService("RunService")

local player = Players.LocalPlayer

-- How much of the horizon the strip shows at once. 180 means a mark at the
-- edge of the strip is 90 degrees off your heading, which keeps the scale
-- readable without the marks crawling.
local STRIP_ARC = math.rad(180)
local STRIP_W, STRIP_H = 460, 26
local ACCENT = Color3.fromRGB(108, 194, 255)
local INK = Color3.fromRGB(220, 237, 255)
local PANEL = Color3.fromRGB(13, 20, 31)

local gui = Instance.new("ScreenGui")
gui.Name = "ExitCompass"
gui.ResetOnSpawn, gui.IgnoreGuiInset = false, true
gui.DisplayOrder = 4
gui.Parent = player:WaitForChild("PlayerGui")

-- ---------------------------------------------------------------- the strip
local strip = Instance.new("Frame")
strip.Name = "CompassStrip"
strip.AnchorPoint = Vector2.new(0.5, 0)
strip.Position = UDim2.new(0.5, 0, 0, 54)
strip.Size = UDim2.fromOffset(STRIP_W, STRIP_H)
strip.BackgroundColor3 = PANEL
strip.BackgroundTransparency = 0.35
strip.BorderSizePixel = 0
strip.ClipsDescendants = true
strip.Visible = false
strip.Parent = gui

-- The centre line is what everything else is read against.
local centre = Instance.new("Frame")
centre.Name = "Centre"
centre.AnchorPoint = Vector2.new(0.5, 0)
centre.Position = UDim2.new(0.5, 0, 0, 0)
centre.Size = UDim2.fromOffset(2, STRIP_H)
centre.BackgroundColor3 = ACCENT
centre.BorderSizePixel = 0
centre.Parent = strip

local CARDINALS = {
	{ "N", 0 },
	{ "NE", 45 },
	{ "E", 90 },
	{ "SE", 135 },
	{ "S", 180 },
	{ "SW", 225 },
	{ "W", 270 },
	{ "NW", 315 },
}

local marks: { { label: TextLabel, bearing: number } } = {}
for _, spec in ipairs(CARDINALS) do
	local text = Instance.new("TextLabel")
	text.Name = `Mark_{spec[1]}`
	text.AnchorPoint = Vector2.new(0.5, 0.5)
	text.Size = UDim2.fromOffset(34, STRIP_H)
	text.BackgroundTransparency = 1
	text.Font = Enum.Font.GothamBold
	text.TextSize = if #(spec[1] :: string) == 1 then 15 else 11
	text.Text = spec[1] :: string
	text.TextColor3 = if #(spec[1] :: string) == 1 then INK else Color3.fromRGB(150, 172, 196)
	text.Parent = strip
	table.insert(marks, { label = text, bearing = math.rad(spec[2] :: number) })
end

-- The exit's own tick, which rides the same strip as the cardinals so the two
-- can be read against each other.
local exitTick = Instance.new("Frame")
exitTick.Name = "ExitTick"
exitTick.AnchorPoint = Vector2.new(0.5, 1)
exitTick.Position = UDim2.new(0.5, 0, 1, 0)
exitTick.Size = UDim2.fromOffset(3, STRIP_H)
exitTick.BackgroundColor3 = Color3.fromRGB(255, 196, 120)
exitTick.BorderSizePixel = 0
exitTick.Parent = strip

-- -------------------------------------------------------------- the readout
local frame = Instance.new("Frame")
frame.Name = "Readout"
frame.AnchorPoint = Vector2.new(0.5, 0)
frame.Position = UDim2.new(0.5, 0, 0, 54 + STRIP_H + 6)
frame.Size = UDim2.fromOffset(244, 54)
frame.BackgroundColor3 = PANEL
frame.BackgroundTransparency = 0.18
frame.BorderColor3 = ACCENT
frame.BorderSizePixel = 1
frame.Visible = false
frame.Parent = gui

local arrow = Instance.new("TextLabel")
arrow.Name = "Arrow"
arrow.AnchorPoint = Vector2.new(0.5, 0.5)
arrow.Position = UDim2.fromOffset(33, 27)
arrow.Size = UDim2.fromOffset(52, 52)
arrow.BackgroundTransparency = 1
arrow.Font = Enum.Font.GothamBlack
arrow.Text = "➤"
arrow.TextSize = 40
arrow.TextColor3 = ACCENT
arrow.TextStrokeTransparency = 0.4
arrow.Parent = frame

local label = Instance.new("TextLabel")
label.Name = "Label"
label.Position = UDim2.fromOffset(66, 0)
label.Size = UDim2.fromOffset(172, 54)
label.BackgroundTransparency = 1
label.Font = Enum.Font.GothamBold
label.TextSize = 13
label.TextXAlignment = Enum.TextXAlignment.Left
label.TextColor3 = INK
label.Parent = frame

-- ------------------------------------------------------------- the edge cue
-- When the exit is off screen the readout alone does not say which way to
-- turn, which is exactly the case the old arrow handled worst. This pins to
-- the edge of the screen the exit is actually on.
local edge = Instance.new("TextLabel")
edge.Name = "EdgeArrow"
edge.AnchorPoint = Vector2.new(0.5, 0.5)
edge.Size = UDim2.fromOffset(64, 64)
edge.BackgroundTransparency = 1
edge.Font = Enum.Font.GothamBlack
edge.Text = "➤"
edge.TextSize = 46
edge.TextColor3 = Color3.fromRGB(255, 196, 120)
edge.TextStrokeTransparency = 0.3
edge.Visible = false
edge.Parent = gui

local function activeExit(): BasePart?
	for _, instance in ipairs(CollectionService:GetTagged("Extraction")) do
		if instance:IsA("BasePart") and instance:GetAttribute("ActiveThisRun") == true then
			return instance
		end
	end
	return nil
end

-- Shortest signed angle from a to b, in radians.
local function wrap(a: number): number
	return (a + math.pi) % (2 * math.pi) - math.pi
end

RunService.RenderStepped:Connect(function()
	local target = activeExit()
	local character = player.Character
	local root = character and character:FindFirstChild("HumanoidRootPart")
	local camera = workspace.CurrentCamera
	if not target or not root or not root:IsA("BasePart") or not camera then
		frame.Visible, strip.Visible, edge.Visible = false, false, false
		return
	end

	local look = camera.CFrame.LookVector
	local heading = math.atan2(look.X, look.Z)

	-- The cardinals slide against the heading. Anything more than half the
	-- shown arc away is behind you and is simply parked off the clipped edge.
	for _, mark in ipairs(marks) do
		local offset = wrap(mark.bearing - heading)
		mark.label.Position = UDim2.new(0.5 + offset / STRIP_ARC, 0, 0.5, 0)
	end

	local delta = target.Position - root.Position
	local horizontal = Vector3.new(delta.X, 0, delta.Z)
	local distance = math.floor(horizontal.Magnitude + 0.5)

	local relative = 0
	if horizontal.Magnitude > 0.01 then
		relative = wrap(math.atan2(horizontal.X, horizontal.Z) - heading)
		-- The readout arrow: 0 is straight ahead, 180 is behind you.
		arrow.Rotation = math.deg(relative) - 90
	end
	exitTick.Position = UDim2.new(0.5 + relative / STRIP_ARC, 0, 1, 0)
	-- Past the edge of the shown arc the tick would sit outside the strip and
	-- be clipped, which would read as "no exit" rather than "behind you".
	exitTick.Visible = math.abs(relative) < STRIP_ARC / 2

	-- Is the exit actually on screen? If not, pin the cue to the edge.
	local _, onScreen = camera:WorldToViewportPoint(target.Position)
	if onScreen then
		edge.Visible = false
	else
		local viewport = camera.ViewportSize
		-- Push out from the centre along the bearing, then clamp to a margin
		-- inside the screen so the glyph is never half off.
		local dir = Vector2.new(math.sin(relative), -math.cos(relative))
		local half = Vector2.new(viewport.X / 2 - 52, viewport.Y / 2 - 52)
		local scale = math.min(
			if math.abs(dir.X) > 0.001 then half.X / math.abs(dir.X) else math.huge,
			if math.abs(dir.Y) > 0.001 then half.Y / math.abs(dir.Y) else math.huge
		)
		local at = Vector2.new(viewport.X / 2, viewport.Y / 2) + dir * scale
		edge.Position = UDim2.fromOffset(at.X, at.Y)
		edge.Rotation = math.deg(relative) - 90
		edge.Visible = true
	end

	local bearing = math.floor(math.deg(math.atan2(horizontal.X, horizontal.Z)) + 0.5) % 360
	label.Text = `ACTIVE EXIT  ·  {bearing}°\n{distance} studs`
	frame.Visible, strip.Visible = true, true
end)
