--!strict
-- A real, procedural Window Walker mannequin. It is deliberately simple and
-- non-human: a shop-display silhouette that gameplay can spawn today, while
-- the art source remains a reference for a future custom model.

local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))

local WindowWalkerRig = {}

local FOLDER_NAME = "Enemies"
local RIG_NAME = "WindowWalker"
-- PLACEHOLDER. This id is not ours: it points at somebody else's upload, so it
-- cannot port to the Steam build and must be replaced with a master we own.
-- Listed in assets/MANIFEST.md under "Owed"; the rule is in docs/ASSETS.md.
--
-- CUE_PITCH is what currently tells this enemy apart from the others by ear.
-- All three rigs play the same sample at different pitches, which is a stopgap:
-- "telegraph every enemy with sound before sight" is an accessibility rule, and
-- it is properly met by three distinct recordings, not one sample pitched three
-- ways. Keep the pitches distinct until those exist.
local TELEGRAPH_SOUND_ID = "rbxassetid://9118959688"
local CUE_PITCH = 0.72

local function weld(root: BasePart, part: BasePart)
	local constraint = Instance.new("WeldConstraint")
	constraint.Part0 = root
	constraint.Part1 = part
	constraint.Parent = part
end

local function part(
	model: Model,
	root: BasePart,
	name: string,
	size: Vector3,
	offset: Vector3
): BasePart
	local item = Instance.new("Part")
	item.Name = name
	item.Size = size
	item.Color = Color3.fromRGB(202, 199, 184)
	item.Material = Enum.Material.SmoothPlastic
	item.CanCollide = false
	item.CFrame = root.CFrame * CFrame.new(offset)
	item.Parent = model
	weld(root, item)
	return item
end

local function build(): Model
	local def = Config.Enemies.WindowWalkers
	local model = Instance.new("Model")
	model.Name = RIG_NAME

	local root = Instance.new("Part")
	root.Name = "HumanoidRootPart"
	root.Size = Vector3.new(1.8, 2.4, 1.0)
	root.Color = Color3.fromRGB(202, 199, 184)
	root.Material = Enum.Material.SmoothPlastic
	root.CanCollide = true
	root.Parent = model

	-- The chest is a clean display form, not an anatomical body. A tilted blank
	-- head and too-low arms carry the unsettling silhouette without turning it
	-- into a human NPC.
	part(model, root, "Torso", Vector3.new(2.0, 2.7, 0.8), Vector3.new(0, 0.35, 0))
	local head = part(model, root, "Head", Vector3.new(1.25, 1.45, 1.1), Vector3.new(0.16, 2.35, 0))
	head.Orientation = Vector3.new(0, 0, -10)
	part(model, root, "LeftArm", Vector3.new(0.42, 2.5, 0.42), Vector3.new(-1.16, 0.05, 0))
	part(model, root, "RightArm", Vector3.new(0.42, 2.5, 0.42), Vector3.new(1.16, -0.22, 0))
	part(model, root, "LeftLeg", Vector3.new(0.52, 2.9, 0.55), Vector3.new(-0.48, -2.05, 0))
	part(model, root, "RightLeg", Vector3.new(0.52, 2.9, 0.55), Vector3.new(0.48, -2.05, 0))

	local crack = Instance.new("Part")
	crack.Name = "HeadCrack"
	crack.Size = Vector3.new(0.03, 0.72, 0.04)
	crack.Color = Color3.fromRGB(72, 76, 84)
	crack.Material = Enum.Material.Metal
	crack.CanCollide = false
	crack.CFrame = head.CFrame * CFrame.new(0.16, 0, -0.57) * CFrame.Angles(0, 0, math.rad(18))
	crack.Parent = model
	weld(root, crack)

	local sound = Instance.new("Sound")
	sound.Name = "Telegraph"
	sound.SoundId = TELEGRAPH_SOUND_ID
	sound.Volume = 0.45
	sound.PlaybackSpeed = CUE_PITCH
	sound.RollOffMode = Enum.RollOffMode.InverseTapered
	sound.RollOffMinDistance = 7
	sound.RollOffMaxDistance = def.sightRange
	sound.Parent = root

	local humanoid = Instance.new("Humanoid")
	humanoid.RequiresNeck = false
	humanoid.MaxHealth = def.health
	humanoid.Health = def.health
	humanoid.WalkSpeed = def.moveSpeed
	humanoid.HipHeight = 2.2
	humanoid.DisplayDistanceType = Enum.HumanoidDisplayDistanceType.None
	humanoid.Parent = model

	model.PrimaryPart = root
	return model
end

function WindowWalkerRig.ensure(): Model
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

return WindowWalkerRig
