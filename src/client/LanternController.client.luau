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

local function ensureLight(): PointLight?
	local character = player.Character
	local root = character and character:FindFirstChild("HumanoidRootPart")
	if not root then
		return nil
	end
	if light and light.Parent == root then
		return light
	end
	local created = Instance.new("PointLight")
	created.Name = "Lantern"
	created.Color = LANTERN_COLOUR
	created.Range = LANTERN_RANGE
	created.Brightness = 2.4
	-- Off by Chris's call, 2026-10-20: "the shadows suck, i hate them."
	--
	-- This was the only thing still casting. The map already runs with
	-- Lighting.GlobalShadows false and its own lights set to Shadows false, so
	-- a lit lantern was the one source of hard shadows in the game - every
	-- railing and shelf throwing a sharp edge across the floor as you moved.
	created.Shadows = false
	created.Enabled = false
	created.Parent = root
	light = created
	return created
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
