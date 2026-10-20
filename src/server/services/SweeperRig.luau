--!strict
-- The Sweeper rig, built in code.
--
-- `EnemyService` clones `ReplicatedStorage.Enemies.Sweeper`, and that template
-- did not exist anywhere in the project, so four tagged spawn points produced
-- nothing and the knockdown rule could not be reached in play.
--
-- Built procedurally rather than shipped as a model file on purpose:
-- `default.project.json` and the `.rbxmx` files are Codex's lane, so a template
-- that needs neither a project mapping nor a map file keeps this inside the
-- gameplay lane. Same pattern as FloodedMallBuilder.
--
-- This is a functional placeholder, not a look. The silhouette is Chris's call;
-- everything here is a grey box that walks, telegraphs and collides correctly.

local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))

local FOLDER_NAME = "Enemies"
local RIG_NAME = "Sweeper"

-- Audio engineering, not balance: the distance at which the telegraph is at
-- full volume before it starts falling off.
local TELEGRAPH_MIN_DISTANCE = 8

local SweeperRig = {}

local function weldTo(root: BasePart, part: BasePart)
	local weld = Instance.new("WeldConstraint")
	weld.Part0 = root
	weld.Part1 = part
	weld.Parent = part
end

--- The sound that has to arrive before the player can be hit.
--
-- The handoff makes this an accessibility requirement rather than polish:
-- "every enemy must telegraph with sound before sight". `SoundId` is left empty
-- deliberately - inventing an `rbxassetid` would be a fabricated asset, and a
-- wrong one is worse than an obvious gap. Chris supplies the audio; the timing,
-- range and falloff around it are wired and correct.
--
-- Audible range is tied to `aggroRadiusLit`, the furthest a Sweeper can notice
-- anybody. If it can see you, you can hear it.
local function addTelegraph(root: BasePart): Sound
	local sweeper = Config.Enemies.Sweepers
	local sound = Instance.new("Sound")
	sound.Name = "Telegraph"
	sound.SoundId = ""
	sound.Looped = false
	sound.Volume = 0.6
	sound.RollOffMode = Enum.RollOffMode.InverseTapered
	sound.RollOffMinDistance = TELEGRAPH_MIN_DISTANCE
	sound.RollOffMaxDistance = sweeper.aggroRadiusLit
	sound.Parent = root
	return sound
end

local function build(): Model
	local sweeper = Config.Enemies.Sweepers

	local model = Instance.new("Model")
	model.Name = RIG_NAME

	-- A squat chassis, because a Sweeper used to clean floors. The root part
	-- carries the name the Humanoid and EnemyService both look for.
	local root = Instance.new("Part")
	root.Name = "HumanoidRootPart"
	root.Size = Vector3.new(2.4, 1.4, 2.8)
	root.Color = Color3.fromRGB(78, 84, 92)
	root.Material = Enum.Material.Metal
	root.TopSurface = Enum.SurfaceType.Smooth
	root.BottomSurface = Enum.SurfaceType.Smooth
	root.CanCollide = true
	root.Parent = model

	-- A Humanoid wants a Head; without one it reports a broken rig. It does not
	-- need a Neck as long as RequiresNeck is off, which is what lets this be a
	-- two-part rig instead of a full R6 joint hierarchy.
	local head = Instance.new("Part")
	head.Name = "Head"
	head.Size = Vector3.new(1.2, 0.8, 1.2)
	head.Color = Color3.fromRGB(96, 104, 112)
	head.Material = Enum.Material.Metal
	head.CanCollide = false
	head.CFrame = root.CFrame * CFrame.new(0, 1.1, -0.6)
	head.Parent = model
	weldTo(root, head)

	-- Amber, matching the lantern palette the readability pillar fixes, so a
	-- Sweeper reads against electric-blue storm light for colour-blind players.
	-- A light on the robot, not from it: this does not touch Storm Charge.
	local eye = Instance.new("Part")
	eye.Name = "Eye"
	eye.Size = Vector3.new(0.5, 0.25, 0.2)
	eye.Color = Color3.fromRGB(255, 154, 48)
	eye.Material = Enum.Material.Neon
	eye.CanCollide = false
	eye.CFrame = head.CFrame * CFrame.new(0, 0, -0.65)
	eye.Parent = model
	weldTo(root, eye)

	local humanoid = Instance.new("Humanoid")
	humanoid.RequiresNeck = false
	humanoid.MaxHealth = sweeper.health
	humanoid.Health = sweeper.health
	humanoid.WalkSpeed = sweeper.moveSpeed
	-- Sits the chassis just clear of the floor. Without this a legless rig
	-- half-sinks and pathfinding fights the ground.
	humanoid.HipHeight = 1.1
	humanoid.DisplayDistanceType = Enum.HumanoidDisplayDistanceType.None
	humanoid.Parent = model

	addTelegraph(root)

	model.PrimaryPart = root
	return model
end

--- Creates the template once, under ReplicatedStorage.Enemies, and returns it.
--- Safe to call repeatedly: a template that already exists is reused.
function SweeperRig.ensure(): Model
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

return SweeperRig
