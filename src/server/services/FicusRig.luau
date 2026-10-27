--!strict
-- A potted-mall-plant stalker. It looks like ordinary dead decor until light
-- wakes it, then the pot walks and the leaves reach for the keeper.

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))

local FicusRig = {}
local FOLDER_NAME = "Enemies"
local RIG_NAME = "Ficus"
-- PLACEHOLDER. This sound is not owned by the project and must be replaced
-- with the ficus rustle / pot-drag master before release; it is retained only
-- so the wake-up telegraph is audible during playtests.
local TELEGRAPH_SOUND_ID = "rbxassetid://9118959688"
local CUE_PITCH = 0.58

local function weld(root: BasePart, item: BasePart)
	local joint = Instance.new("WeldConstraint")
	joint.Part0, joint.Part1, joint.Parent = root, item, item
end

local function leaf(model: Model, root: BasePart, offset: Vector3, yaw: number)
	local item = Instance.new("Part")
	item.Name = "Leaf"
	item.Shape = Enum.PartType.Ball
	item.Size = Vector3.new(1.7, 0.38, 0.9)
	item.Color = Color3.fromRGB(48, 88, 48)
	item.Material = Enum.Material.SmoothPlastic
	item.CanCollide = false
	item.CFrame = root.CFrame * CFrame.new(offset) * CFrame.Angles(0, math.rad(yaw), math.rad(28))
	item.Parent = model
	weld(root, item)
end

local function build(): Model
	local def = Config.Enemies.Ficus
	local model = Instance.new("Model")
	model.Name = RIG_NAME
	local root = Instance.new("Part")
	root.Name = "HumanoidRootPart"
	root.Shape = Enum.PartType.Cylinder
	root.Size = Vector3.new(2.4, 2.2, 2.4)
	root.Color = Color3.fromRGB(86, 60, 43)
	root.Material = Enum.Material.CeramicTiles
	root.CanCollide = true
	root.Parent = model

	local trunk = Instance.new("Part")
	trunk.Name = "Trunk"
	trunk.Shape = Enum.PartType.Cylinder
	trunk.Size = Vector3.new(0.42, 4.2, 0.42)
	trunk.Color = Color3.fromRGB(72, 52, 34)
	trunk.Material = Enum.Material.Wood
	trunk.CanCollide = false
	trunk.CFrame = root.CFrame * CFrame.new(0, 2.4, 0)
	trunk.Parent = model
	weld(root, trunk)
	for i = 0, 5 do
		local angle = i * 60
		local r = math.rad(angle)
		leaf(
			model,
			root,
			Vector3.new(math.cos(r) * 1.05, 3.55 + (i % 2) * 0.35, math.sin(r) * 1.05),
			angle
		)
	end

	local sound = Instance.new("Sound")
	sound.Name = "Telegraph"
	sound.SoundId = TELEGRAPH_SOUND_ID
	sound.Volume = 0.36
	sound.PlaybackSpeed = CUE_PITCH
	sound.RollOffMode = Enum.RollOffMode.InverseTapered
	sound.RollOffMinDistance, sound.RollOffMaxDistance = 7, def.sightRange
	sound.Parent = root

	local humanoid = Instance.new("Humanoid")
	humanoid.RequiresNeck = false
	humanoid.MaxHealth, humanoid.Health = def.health, def.health
	humanoid.WalkSpeed, humanoid.HipHeight = def.moveSpeed, 1.1
	humanoid.DisplayDistanceType = Enum.HumanoidDisplayDistanceType.None
	humanoid.Parent = model
	model.PrimaryPart = root
	return model
end

function FicusRig.ensure(): Model
	local folder = ReplicatedStorage:FindFirstChild(FOLDER_NAME)
	if not folder then
		folder = Instance.new("Folder")
		folder.Name, folder.Parent = FOLDER_NAME, ReplicatedStorage
	end
	local existing = folder:FindFirstChild(RIG_NAME)
	if existing and existing:IsA("Model") then
		return existing
	end
	local model = build()
	model.Parent = folder
	return model
end

return FicusRig
