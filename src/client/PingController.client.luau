--!strict
-- G places a short, bright crew marker where the player is looking. The server
-- validates the point; this controller only supplies a camera ray and renders
-- the received signal.

local ContextActionService = game:GetService("ContextActionService")
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Remotes = require(ReplicatedStorage:WaitForChild("Shared"):WaitForChild("Remotes"))
local player = Players.LocalPlayer

local function ping()
	local camera = workspace.CurrentCamera
	if not camera then return end
	local params = RaycastParams.new()
	params.FilterType = Enum.RaycastFilterType.Exclude
	params.FilterDescendantsInstances = { player.Character }
	local hit = workspace:Raycast(camera.CFrame.Position, camera.CFrame.LookVector * 80, params)
	Remotes.get("PingRequest"):FireServer(if hit then hit.Position else camera.CFrame.Position + camera.CFrame.LookVector * 60)
end

ContextActionService:BindAction("CrewPing", function(_, state)
	if state == Enum.UserInputState.Begin then ping() end
	return Enum.ContextActionResult.Sink
end, true, Enum.KeyCode.G, Enum.KeyCode.DPadUp)
ContextActionService:SetTitle("CrewPing", "Ping")

Remotes.get("PingSignal").OnClientEvent:Connect(function(signal)
	if typeof(signal) ~= "table" or typeof(signal.position) ~= "Vector3" then return end
	local beacon = Instance.new("Part")
	beacon.Name = "CrewPing"
	beacon.Shape, beacon.Size = Enum.PartType.Ball, Vector3.new(0.8, 0.8, 0.8)
	beacon.Color, beacon.Material = Color3.fromRGB(108, 194, 255), Enum.Material.Neon
	beacon.Anchored, beacon.CanCollide, beacon.CanQuery = true, false, false
	beacon.Position = signal.position + Vector3.new(0, 1.4, 0)
	beacon.Parent = workspace
	local light = Instance.new("PointLight")
	light.Color, light.Range, light.Brightness, light.Shadows = beacon.Color, 18, 1.2, false
	light.Parent = beacon
	local gui = Instance.new("BillboardGui")
	gui.Size, gui.StudsOffset, gui.AlwaysOnTop, gui.MaxDistance = UDim2.fromOffset(170, 32), Vector3.new(0, 2.5, 0), true, 250
	gui.Parent = beacon
	local label = Instance.new("TextLabel")
	label.Size, label.BackgroundTransparency, label.Font, label.TextSize = UDim2.fromScale(1, 1), 1, Enum.Font.GothamBold, 15
	label.TextColor3, label.TextStrokeTransparency = beacon.Color, 0.35
	label.Text = tostring(signal.caller) .. " — PING"
	label.Parent = gui
	task.delay(7, function() if beacon.Parent then beacon:Destroy() end end)
end)
