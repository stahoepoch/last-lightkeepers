--!strict
-- Enemy spawning and behaviour.
--
-- Day 4 of the roadmap is Sweepers only. Stockers, Glimmers, and the Warden
-- have their config and their hooks here but are deliberately not implemented -
-- the handoff says build one enemy before the full roster.

local CollectionService = game:GetService("CollectionService")
local PathfindingService = game:GetService("PathfindingService")
local Players = game:GetService("Players")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Types = require(Shared:WaitForChild("Types"))

local RunStateService = require(script.Parent:WaitForChild("RunStateService"))

local EnemyService = {}

local REPATH_INTERVAL = 0.6

--- Who a robot should chase. A tagged player is known to every robot regardless
--- of light or line of sight, for the 20 seconds the tag lasts.
function EnemyService.pickTarget(fromPosition: Vector3, kind: Types.EnemyKind): Player?
	local run = RunStateService.current()
	if not run or run.finished then
		return nil
	end

	local def = Config.Enemies[kind]
	local best: Player? = nil
	local bestDistance = math.huge

	for userId, state in pairs(run.lightkeepers) do
		if state.knockedDown then
			continue
		end
		local player = Players:GetPlayerByUserId(userId)
		local character = player and player.Character
		local root = character and character:FindFirstChild("HumanoidRootPart")
		if not root then
			continue
		end

		local distance = (((root :: BasePart).Position) - fromPosition).Magnitude

		-- A completed Glimmer tag overrides range and darkness entirely.
		if state.taggedUntil then
			return player
		end

		local reach = if kind == "Sweepers"
			then (if state.lanternOn then def.aggroRadiusLit else def.aggroRadiusDark)
			else def.sightRange or def.aggroRadiusLit or 40

		if distance <= reach and distance < bestDistance then
			best = player
			bestDistance = distance
		end
	end

	return best
end

--- Sweeper loop: weak alone, swarms lit players and dropped loot.
function EnemyService.driveSweeper(model: Model)
	local humanoid = model:FindFirstChildOfClass("Humanoid")
	local root = model:FindFirstChild("HumanoidRootPart")
	if not humanoid or not root then
		return
	end

	local def = Config.Enemies.Sweepers
	humanoid.WalkSpeed = def.moveSpeed
	humanoid.MaxHealth = def.health
	humanoid.Health = def.health

	-- Contact watcher. Separate from the path loop so a Sweeper still lands hits
	-- while it is mid-waypoint rather than only when it finishes moving.
	task.spawn(function()
		local contact = Config.Lantern.sweeperContact
		while model.Parent and humanoid.Health > 0 do
			for _, player in ipairs(Players:GetPlayers()) do
				local character = player.Character
				local targetRoot = character and character:FindFirstChild("HumanoidRootPart")
				if targetRoot and targetRoot:IsA("BasePart") then
					local distance = ((root :: BasePart).Position - targetRoot.Position).Magnitude
					if distance <= contact.contactRange then
						RunStateService.registerContact(player)
					end
				end
			end
			task.wait(0.2)
		end
	end)

	task.spawn(function()
		while model.Parent and humanoid.Health > 0 do
			local target = EnemyService.pickTarget((root :: BasePart).Position, "Sweepers")
			if target and target.Character then
				local targetRoot = target.Character:FindFirstChild("HumanoidRootPart")
				if targetRoot then
					local path = PathfindingService:CreatePath({
						AgentRadius = 2,
						AgentHeight = 4,
						AgentCanJump = false,
					})
					local ok = pcall(function()
						path:ComputeAsync((root :: BasePart).Position, (targetRoot :: BasePart).Position)
					end)
					if ok and path.Status == Enum.PathStatus.Success then
						for _, waypoint in ipairs(path:GetWaypoints()) do
							if not model.Parent or humanoid.Health <= 0 then
								return
							end
							humanoid:MoveTo(waypoint.Position)
							humanoid.MoveToFinished:Wait()
						end
					end
				end
			end
			task.wait(REPATH_INTERVAL)
		end
	end)
end

function EnemyService.spawnSweepersAtTaggedPoints()
	for _, point in ipairs(CollectionService:GetTagged("SweeperSpawn")) do
		local template = ReplicatedStorage:FindFirstChild("Enemies")
		local model = template and template:FindFirstChild("Sweeper")
		if model and point:IsA("BasePart") then
			local clone = (model :: Model):Clone()
			clone:PivotTo(CFrame.new((point :: BasePart).Position + Vector3.new(0, 3, 0)))
			clone.Parent = workspace
			EnemyService.driveSweeper(clone)
		end
	end
end

--- Storm Charge thresholds are the only pacing dial for enemy pressure.
function EnemyService.onThresholdCrossed(threshold: any)
	if threshold.id == "PatrolsAlert" then
		-- TODO Day 4+: widen Sweeper investigate radius.
	elseif threshold.id == "PatrolsHunt" then
		-- TODO: all patrols hunt; raise Glimmer scan frequency.
	elseif threshold.id == "WardenConverges" then
		EnemyService.spawnWarden()
	end
end

--- The Warden is unkillable and drops nothing. It is the meter with a body.
function EnemyService.spawnWarden()
	-- TODO Day 6+: spawn at the rooftop stage, retarget the brightest position
	-- every Config.Enemies.Warden.retargetIntervalSeconds.
end

function EnemyService.init()
	RunStateService.onThresholdCrossed = EnemyService.onThresholdCrossed
end

return EnemyService
