--!strict
-- Lantern input and local light visuals.
--
-- The client asks; the server decides. This script never changes Storm Charge,
-- never grants loot, and never decides whether the lantern is actually lit for
-- gameplay purposes - it mirrors what the last snapshot implied.

local ContextActionService = game:GetService("ContextActionService")
local Players = game:GetService("Players")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))

local player = Players.LocalPlayer
local lanternRequest = Remotes.get("LanternRequest")

-- Amber, per the readability pillar: amber lamps against electric-blue storms
-- stay distinguishable for red-green colour-blind players. Do not drift to orange.
local LANTERN_COLOUR = Color3.fromRGB(255, 186, 94)
local LANTERN_RANGE = 34

local wantLanternOn = false
local light: PointLight? = nil

-- A lantern that only exists as a light attached to the player's invisible
-- root makes the main choice feel like a keyboard setting. This local prop is
-- cosmetic; the server still owns the actual on/off state and Storm Charge.
local function lanternMount(character: Model): BasePart?
	local hand = character:FindFirstChild("RightHand") or character:FindFirstChild("Right Arm")
	if hand and hand:IsA("BasePart") then
		return hand
	end
	local root = character:FindFirstChild("HumanoidRootPart")
	return if root and root:IsA("BasePart") then root else nil
end

local function ensureLanternProp(): PointLight?
	local character = player.Character
	if not character then return nil end
	local mount = lanternMount(character)
	if not mount then return nil end
	local existing = character:FindFirstChild("KeeperLantern")
	if existing then
		local lens = existing:FindFirstChild("Lens", true)
		local existingLight = lens and lens:FindFirstChildOfClass("PointLight")
		if existingLight then return existingLight end
		existing:Destroy()
	end

	local model = Instance.new("Model")
	model.Name = "KeeperLantern"
	local handle = Instance.new("Part")
	handle.Name = "Handle"
	handle.Size = Vector3.new(0.24, 0.78, 0.24)
	handle.Color, handle.Material = Color3.fromRGB(62, 52, 42), Enum.Material.Metal
	handle.CanCollide, handle.CanTouch, handle.Massless = false, false, true
	handle.CFrame = mount.CFrame * CFrame.new(0, -0.72, -0.22)
	handle.Parent = model
	local housing = Instance.new("Part")
	housing.Name = "Housing"
	housing.Size = Vector3.new(0.62, 0.7, 0.42)
	housing.Color, housing.Material = Color3.fromRGB(54, 48, 43), Enum.Material.Metal
	housing.CanCollide, housing.CanTouch, housing.Massless = false, false, true
	housing.CFrame = handle.CFrame * CFrame.new(0, -0.52, 0)
	housing.Parent = model
	local lens = Instance.new("Part")
	lens.Name = "Lens"
	lens.Shape = Enum.PartType.Ball
	lens.Size = Vector3.new(0.38, 0.38, 0.38)
	lens.Color, lens.Material, lens.Transparency = LANTERN_COLOUR, Enum.Material.Glass, 0.12
	lens.CanCollide, lens.CanTouch, lens.Massless = false, false, true
	lens.CFrame = housing.CFrame * CFrame.new(0, 0, -0.24)
	lens.Parent = model
	for _, part in ipairs({ housing, lens }) do
		local joint = Instance.new("WeldConstraint")
		joint.Part0, joint.Part1, joint.Parent = handle, part, handle
	end
	local joint = Instance.new("WeldConstraint")
	joint.Part0, joint.Part1, joint.Parent = mount, handle, handle
	local created = Instance.new("PointLight")
	created.Name = "Lantern"
	created.Color, created.Range, created.Brightness, created.Shadows = LANTERN_COLOUR, LANTERN_RANGE, 2.4, false
	created.Enabled = false
	created.Parent = lens
	model.Parent = character
	return created
end

local function ensureLight(): PointLight?
	if light and light.Parent then
		return light
	end
	light = ensureLanternProp()
	return light
end

local function applyLocalVisual(on: boolean)
	local pointLight = ensureLight()
	if pointLight then
		pointLight.Enabled = on
	end
end

--- Ask, then wait. The light used to switch on locally the moment F was pressed
--- and switch back off when the server's refusal arrived a frame later, which
--- is what Chris saw as "it flickers and thats it". The server is the authority
--- on whether the lantern is lit, so the visual now follows only the echo.
local function toggle()
	lanternRequest:FireServer(not wantLanternOn)
end

ContextActionService:BindAction("ToggleLantern", function(_, state)
	if state == Enum.UserInputState.Begin then
		toggle()
	end
	return Enum.ContextActionResult.Sink
end, true, Enum.KeyCode.F, Enum.KeyCode.ButtonY)

ContextActionService:SetTitle("ToggleLantern", "Lantern")

player.CharacterAdded:Connect(function()
	light = nil
	-- A fresh character starts dark. Re-asserting costs one remote and avoids a
	-- client that believes it is lit while the server knows better.
	wantLanternOn = false
	lanternRequest:FireServer(false)
	applyLocalVisual(false)
end)

-- The server echoes the authoritative state back to the requester. It can refuse
-- a toggle (charge spent, knocked down), so the local visual follows the echo
-- rather than the keypress. The broadcast snapshot carries a crew-wide lantern
-- count and cannot answer "is *mine* lit", so don't infer it from there.
--- A refusal has to be legible. Pressing a key and getting nothing is the single
--- most common way this game has read as broken.
local notice = Instance.new("TextLabel")
notice.Name = "LanternNotice"
notice.AnchorPoint = Vector2.new(0.5, 0.5)
notice.Position = UDim2.new(0.5, 0, 0.5, 110)
notice.Size = UDim2.new(1, 0, 0, 26)
notice.BackgroundTransparency = 1
notice.Font = Enum.Font.GothamMedium
notice.TextSize = 18
notice.TextColor3 = LANTERN_COLOUR
notice.TextStrokeTransparency = 0.4
notice.Visible = false

local shownAt = 0
local NOTICE_SECONDS = 2.2

task.spawn(function()
	local playerGui = player:WaitForChild("PlayerGui")
	local screen = Instance.new("ScreenGui")
	screen.Name = "LanternNoticeGui"
	screen.ResetOnSpawn = false
	screen.IgnoreGuiInset = true
	screen.Parent = playerGui
	notice.Parent = screen

	while true do
		if notice.Visible and os.clock() - shownAt > NOTICE_SECONDS then
			notice.Visible = false
		end
		task.wait(0.2)
	end
end)

lanternRequest.OnClientEvent:Connect(function(actuallyOn: boolean, refusal: string?)
	wantLanternOn = actuallyOn
	applyLocalVisual(actuallyOn)
	if refusal then
		notice.Text = refusal
		notice.Visible = true
		shownAt = os.clock()
	end
end)
