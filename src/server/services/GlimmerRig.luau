--!strict
-- A compact maintenance drone template for the Glimmer encounter.
-- It is built in code so the gameplay system does not depend on a map/model file.

local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))

local GlimmerRig = {}

local FOLDER_NAME = "Enemies"
local RIG_NAME = "Glimmer"
-- Reuses the already play-tested, creator-licensed mechanical telegraph while
-- sound design is still intentionally limited to a single approved asset.
-- PLACEHOLDER. This id is not ours: it points at somebody else's upload, so it
-- cannot port to the Steam build and must be replaced with a master we own.
-- Listed in assets/MANIFEST.md under "Owed"; the rule is in docs/ASSETS.md.
--
-- CUE_PITCH is what currently tells this enemy apart from the others by ear.
-- All three rigs play the same sample at different pitches, which is a stopgap:
-- "telegraph every enemy with sound before sight" is an accessibility rule, and
-- it is properly met by three distinct recordings, not one sample pitched three
-- ways. Keep the pitches distinct until those exist.
local SCAN_SOUND_ID = "rbxassetid://9118959688"
local CUE_PITCH = 1.35

local function weld(root: BasePart, child: BasePart)
	local constraint = Instance.new("WeldConstraint")
	constraint.Part0 = root
	constraint.Part1 = child
	constraint.Parent = child
end

local function build(): Model
	local def = Config.Enemies.Glimmers
	local model = Instance.new("Model")
	model.Name = RIG_NAME

	local root = Instance.new("Part")
	root.Name = "HumanoidRootPart"
	root.Size = Vector3.new(1.6, 0.8, 1.6)
	root.Color = Color3.fromRGB(54, 62, 76)
	root.Material = Enum.Material.Metal
	root.CanCollide = false
	root.Parent = model

	local head = Instance.new("Part")
	head.Name = "Head"
	head.Size = Vector3.new(1.1, 0.7, 1.1)
	head.Color = Color3.fromRGB(74, 88, 108)
	head.Material = Enum.Material.Metal
	head.CanCollide = false
	head.CFrame = root.CFrame * CFrame.new(0, 0.7, 0)
	head.Parent = model
	weld(root, head)

	local lens = Instance.new("Part")
	lens.Name = "ScanLens"
	lens.Size = Vector3.new(0.45, 0.45, 0.16)
	lens.Color = Color3.fromRGB(108, 224, 255)
	lens.Material = Enum.Material.Neon
	lens.CanCollide = false
	lens.CFrame = head.CFrame * CFrame.new(0, 0, -0.62)
	lens.Parent = model
	weld(root, lens)

	local glow = Instance.new("PointLight")
	glow.Color = lens.Color
	glow.Brightness = 1.1
	glow.Range = 10
	glow.Shadows = false
	glow.Parent = lens

	local sound = Instance.new("Sound")
	sound.Name = "ScanTelegraph"
	sound.SoundId = SCAN_SOUND_ID
	sound.Volume = 0.4
	sound.PlaybackSpeed = CUE_PITCH
	sound.RollOffMode = Enum.RollOffMode.InverseTapered
	sound.RollOffMinDistance = 6
	sound.RollOffMaxDistance = def.scanRange
	sound.Parent = root

	local humanoid = Instance.new("Humanoid")
	humanoid.RequiresNeck = false
	humanoid.MaxHealth = def.health
	humanoid.Health = def.health
	humanoid.WalkSpeed = def.moveSpeed
	humanoid.HipHeight = 1.25
	humanoid.DisplayDistanceType = Enum.HumanoidDisplayDistanceType.None
	humanoid.Parent = model

	model.PrimaryPart = root
	return model
end

function GlimmerRig.ensure(): Model
	local folder = ReplicatedStorage:FindFirstChild(FOLDER_NAME)
	if not folder then
		folder = Instance.new("Folder")
		folder.Name = FOLDER_NAME
		folder.Parent = ReplicatedStorage
	end
	local existing = folder:FindFirstChild(RIG_NAME)
	if existing and existing:IsA("Model") then
		return existing
	end
	local model = build()
	model.Parent = folder
	return model
end

return GlimmerRig
