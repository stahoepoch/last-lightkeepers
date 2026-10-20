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
	created.Shadows = true
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

local function toggle()
	wantLanternOn = not wantLanternOn
	applyLocalVisual(wantLanternOn)
	lanternRequest:FireServer(wantLanternOn)
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
lanternRequest.OnClientEvent:Connect(function(actuallyOn: boolean)
	wantLanternOn = actuallyOn
	applyLocalVisual(actuallyOn)
end)
