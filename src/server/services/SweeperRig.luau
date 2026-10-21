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

-- The telegraph sound.
--
-- "Servo Motor 7 (SFX)", asset 9118959688, from the Pro Sound Effects library
-- Roblox licenses for creator use - free, and usable in an unpublished place.
-- Short servo bursts, which is what a former floor cleaner should sound like
-- noticing you, and distinct from the storm's own noise.
--
-- Chosen rather than invented: searched the Creator Store, then loaded it in a
-- running playtest and read back `IsLoaded = true` and `TimeLength = 1.11s`.
-- That 1.11s covers `audioTelegraphSeconds` of 1.0, so the warning is still
-- sounding when the first hit becomes legal rather than having finished before.
--
-- Swap the id if Chris wants a different voice for the Sweepers; everything
-- around it - timing, falloff, range - is independent of the choice.
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
local CUE_PITCH = 1
local TELEGRAPH_SOUND_NAME = "Servo Motor 7 (SFX), Pro Sound Effects"
local TELEGRAPH_MEASURED_SECONDS = 1.11

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
-- "every enemy must telegraph with sound before sight".
--
-- Audible range is tied to `aggroRadiusLit`, the furthest a Sweeper can notice
-- anybody. If it can see you, you can hear it.
--
-- A telegraph shorter than `audioTelegraphSeconds` would finish before the hit
-- it is warning about, so the mismatch is reported rather than left to be
-- noticed in play.
local function addTelegraph(root: BasePart): Sound
	local sweeper = Config.Enemies.Sweepers
	local sound = Instance.new("Sound")
	sound.Name = "Telegraph"
	sound.SoundId = TELEGRAPH_SOUND_ID
	sound.Looped = false
	sound.Volume = 0.6
	sound.PlaybackSpeed = CUE_PITCH
	sound.RollOffMode = Enum.RollOffMode.InverseTapered
	sound.RollOffMinDistance = TELEGRAPH_MIN_DISTANCE
	sound.RollOffMaxDistance = sweeper.aggroRadiusLit
	sound.Parent = root

	if TELEGRAPH_MEASURED_SECONDS < sweeper.audioTelegraphSeconds then
		warn(
			`[SweeperRig] telegraph "{TELEGRAPH_SOUND_NAME}" is {TELEGRAPH_MEASURED_SECONDS}s but audioTelegraphSeconds is {sweeper.audioTelegraphSeconds}s - the warning would end before the hit it announces`
		)
	end

	return sound
end

--- The sound a Sweeper makes while it is actually coming for you.
--
-- The telegraph is a single 1.1 second burst when it first notices you, which
-- Chris correctly read as not being able to hear them at all: "i can't like hear
-- them, hear them approaching". One noise at acquisition tells you something
-- happened; it does not tell you something is getting closer.
--
-- Same asset as the telegraph, looped and quieter, so a chase has a continuous
-- mechanical source you can place in the room. Reused rather than sourcing a
-- second clip so there is still only one audio asset in the project to replace
-- when real sound design happens - this is a placeholder with correct wiring,
-- not a finished mix.
local function addChaseLoop(root: BasePart): Sound
	local sweeper = Config.Enemies.Sweepers
	local sound = Instance.new("Sound")
	sound.Name = "Chase"
	sound.SoundId = TELEGRAPH_SOUND_ID
	sound.Looped = true
	sound.Volume = 0.35
	sound.PlaybackSpeed = 0.8
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
	addChaseLoop(root)

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
