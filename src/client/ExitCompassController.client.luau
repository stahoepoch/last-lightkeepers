--!strict
-- Where the exits are, shown in the world rather than on the edge of the
-- screen.
--
-- Two versions of this have now been wrong in the same way. The first
-- rotated a small triangle in a box; the second added a sliding cardinal
-- compass strip, and Chris walked it: "that mini map with the square, its
-- worse, i had no clue where the extract was, at least arc raiders has stuff
-- to show you."
--
-- He is right and the lesson is the same both times. A compass is ABSTRACT -
-- it tells you a bearing and leaves you to convert that into "which way do I
-- walk", in a building you do not know, in the dark. A marker over the thing
-- itself is CONCRETE. ARC Raiders does not give you a compass rose, it hangs
-- an icon on the extraction and lets you walk at it. So:
--
--   1. A billboard ON the exit, drawn through walls, with its distance. You
--      look up, you see where to go, you go there.
--   2. The OTHER exits marked too, dimmer. Four known doors in a mall you are
--      learning beats one known door, and they double as landmarks even when
--      they are shut.
--   3. The relay bank, in its own colour, because banking is the other
--      decision you make under pressure.
--   4. An edge arrow for the active exit when it is behind you, since a
--      billboard cannot be seen through the back of your own head.
--
-- No minimap. The mall staying something you learn is the point, and the
-- complaint was never that the map was unknown - it was that the thing you
-- are required to reach was unmarked.
--
-- Frames and text only: no asset ids, nothing to upload, nothing to licence.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")
local RunService = game:GetService("RunService")

local player = Players.LocalPlayer

local ACTIVE = Color3.fromRGB(126, 232, 143)
local CLOSED = Color3.fromRGB(150, 172, 196)
local RELAY = Color3.fromRGB(255, 196, 120)

local gui = Instance.new("ScreenGui")
gui.Name = "ExitMarkers"
gui.ResetOnSpawn, gui.IgnoreGuiInset = false, true
gui.DisplayOrder = 4
gui.Parent = player:WaitForChild("PlayerGui")

-- One billboard per marked thing, kept in a map so parts coming and going
-- between raids do not leak guis.
type Marker = { gui: BillboardGui, title: TextLabel, dist: TextLabel, pip: TextLabel }
local markers: { [BasePart]: Marker } = {}

local function build(part: BasePart): Marker
	local billboard = Instance.new("BillboardGui")
	billboard.Name = "Marker"
	billboard.Adornee = part
	billboard.AlwaysOnTop = true -- the whole point: readable through the mall
	billboard.Size = UDim2.fromOffset(150, 58)
	billboard.StudsOffsetWorldSpace = Vector3.new(0, 11, 0)
	billboard.MaxDistance = 4096
	billboard.Parent = gui

	local pip = Instance.new("TextLabel")
	pip.Name = "Pip"
	pip.Size = UDim2.new(1, 0, 0, 24)
	pip.BackgroundTransparency = 1
	pip.Font = Enum.Font.GothamBlack
	pip.Text = "◆"
	pip.TextSize = 22
	pip.TextStrokeTransparency = 0.35
	pip.Parent = billboard

	local title = Instance.new("TextLabel")
	title.Name = "Title"
	title.Position = UDim2.fromOffset(0, 22)
	title.Size = UDim2.new(1, 0, 0, 18)
	title.BackgroundTransparency = 1
	title.Font = Enum.Font.GothamBold
	title.TextSize = 14
	title.TextStrokeTransparency = 0.4
	title.Parent = billboard

	local dist = Instance.new("TextLabel")
	dist.Name = "Distance"
	dist.Position = UDim2.fromOffset(0, 38)
	dist.Size = UDim2.new(1, 0, 0, 16)
	dist.BackgroundTransparency = 1
	dist.Font = Enum.Font.GothamBold
	dist.TextSize = 12
	dist.TextStrokeTransparency = 0.5
	dist.Parent = billboard

	return { gui = billboard, title = title, dist = dist, pip = pip }
end

local function marker(part: BasePart): Marker
	local existing = markers[part]
	if existing and existing.gui.Parent then
		return existing
	end
	local made = build(part)
	markers[part] = made
	return made
end

-- The off-screen cue for the active exit. A billboard behind you draws
-- nothing, which is exactly when you most need telling.
local edge = Instance.new("TextLabel")
edge.Name = "EdgeArrow"
edge.AnchorPoint = Vector2.new(0.5, 0.5)
edge.Size = UDim2.fromOffset(72, 72)
edge.BackgroundTransparency = 1
edge.Font = Enum.Font.GothamBlack
edge.Text = "➤"
edge.TextSize = 52
edge.TextColor3 = ACTIVE
edge.TextStrokeTransparency = 0.25
edge.Visible = false
edge.Parent = gui

local edgeDist = Instance.new("TextLabel")
edgeDist.Name = "EdgeDistance"
edgeDist.AnchorPoint = Vector2.new(0.5, 0)
edgeDist.Position = UDim2.fromOffset(0, 38)
edgeDist.Size = UDim2.fromOffset(120, 18)
edgeDist.BackgroundTransparency = 1
edgeDist.Font = Enum.Font.GothamBold
edgeDist.TextSize = 13
edgeDist.TextColor3 = ACTIVE
edgeDist.TextStrokeTransparency = 0.3
edgeDist.Parent = edge

local function wrap(a: number): number
	return (a + math.pi) % (2 * math.pi) - math.pi
end

RunService.RenderStepped:Connect(function()
	local character = player.Character
	local root = character and character:FindFirstChild("HumanoidRootPart")
	local camera = workspace.CurrentCamera
	if not root or not root:IsA("BasePart") or not camera then
		for _, m in pairs(markers) do
			m.gui.Enabled = false
		end
		edge.Visible = false
		return
	end

	local seen: { [BasePart]: boolean } = {}
	local active: BasePart? = nil

	for _, instance in ipairs(CollectionService:GetTagged("Extraction")) do
		if instance:IsA("BasePart") then
			seen[instance] = true
			local isActive = instance:GetAttribute("ActiveThisRun") == true
			if isActive then
				active = instance
			end
			local m = marker(instance)
			local away = (instance.Position - root.Position).Magnitude
			local colour = if isActive then ACTIVE else CLOSED
			m.pip.Text = if isActive then "◆" else "◇"
			m.pip.TextColor3 = colour
			m.title.TextColor3 = colour
			m.dist.TextColor3 = colour
			m.title.Text = if isActive then "EXTRACT" else "exit — shut"
			m.dist.Text = `{math.floor(away + 0.5)} studs`
			-- A shut door is a landmark, not a destination: keep it quiet and
			-- do not let three of them clutter the far end of the mall.
			m.gui.Enabled = isActive or away < 420
			m.pip.TextTransparency = if isActive then 0 else 0.35
			m.title.TextTransparency = if isActive then 0 else 0.4
			m.dist.TextTransparency = if isActive then 0 else 0.45
		end
	end

	for _, instance in ipairs(CollectionService:GetTagged("RelayBank")) do
		if instance:IsA("BasePart") then
			seen[instance] = true
			local m = marker(instance)
			local away = (instance.Position - root.Position).Magnitude
			m.pip.Text = "■"
			m.pip.TextColor3 = RELAY
			m.title.TextColor3 = RELAY
			m.dist.TextColor3 = RELAY
			m.title.Text = "RELAY — BANK HAUL"
			m.dist.Text = `{math.floor(away + 0.5)} studs`
			m.gui.Enabled = away < 520
			m.pip.TextTransparency = 0.1
			m.title.TextTransparency = 0.15
			m.dist.TextTransparency = 0.25
		end
	end

	-- Anything that went away with the last mall.
	for part, m in pairs(markers) do
		if not seen[part] or not part.Parent then
			m.gui:Destroy()
			markers[part] = nil
		end
	end

	if not active then
		edge.Visible = false
		return
	end

	local _, onScreen = camera:WorldToViewportPoint(active.Position)
	if onScreen then
		edge.Visible = false
		return
	end

	local delta = active.Position - root.Position
	local horizontal = Vector3.new(delta.X, 0, delta.Z)
	local look = camera.CFrame.LookVector
	local relative = wrap(math.atan2(horizontal.X, horizontal.Z) - math.atan2(look.X, look.Z))

	local viewport = camera.ViewportSize
	local dir = Vector2.new(math.sin(relative), -math.cos(relative))
	local half = Vector2.new(viewport.X / 2 - 60, viewport.Y / 2 - 60)
	local scale = math.min(
		if math.abs(dir.X) > 0.001 then half.X / math.abs(dir.X) else math.huge,
		if math.abs(dir.Y) > 0.001 then half.Y / math.abs(dir.Y) else math.huge
	)
	local at = Vector2.new(viewport.X / 2, viewport.Y / 2) + dir * scale
	edge.Position = UDim2.fromOffset(at.X, at.Y)
	edge.Rotation = math.deg(relative) - 90
	-- The label stays upright while the arrow turns.
	edgeDist.Rotation = -edge.Rotation
	edgeDist.Text = `{math.floor(horizontal.Magnitude + 0.5)} studs`
	edge.Visible = true
end)
