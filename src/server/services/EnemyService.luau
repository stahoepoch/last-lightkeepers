--!strict
-- Enemy spawning and behaviour.
--
-- Sweepers, Glimmers, and Window Walkers are implemented here. Stockers and
-- the Warden still have their config and hooks but are deliberately held.

local CollectionService = game:GetService("CollectionService")
local PathfindingService = game:GetService("PathfindingService")
local Players = game:GetService("Players")
local Workspace = game:GetService("Workspace")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Types = require(Shared:WaitForChild("Types"))

local RunStateService = require(script.Parent:WaitForChild("RunStateService"))
local SweeperRig = require(script.Parent:WaitForChild("SweeperRig"))
local GlimmerRig = require(script.Parent:WaitForChild("GlimmerRig"))
local WindowWalkerRig = require(script.Parent:WaitForChild("WindowWalkerRig"))
local FicusRig = require(script.Parent:WaitForChild("FicusRig"))

local EnemyService = {}
local watchingSpawnerTags = false
local watchingStockerTags = false
local extractionBeacon: BasePart? = nil
local extractionBeaconEndsAt = 0

local REPATH_INTERVAL = 0.6
-- PatrolsAlert is a run-wide state, not a one-frame spawn event. It resets
-- when clearSweepers runs between expeditions.
local patrolsAlerted = false

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

		local distance = ((root :: BasePart).Position - fromPosition).Magnitude

		-- A completed Glimmer tag overrides range and darkness entirely.
		if state.taggedUntil then
			return player
		end

		local reach = if kind == "Sweepers"
			then (if state.lanternOn then def.aggroRadiusLit else def.aggroRadiusDark)
			else def.sightRange or def.aggroRadiusLit or 40

		-- The first Storm threshold promised that Sweepers investigate farther
		-- from their routes. Until this switch existed, a dark crew could get to
		-- minute eight and have the announced escalation do literally nothing.
		-- `swarmCallRadius` is already the authored, nearby-not-mapwide distance.
		if kind == "Sweepers" and patrolsAlerted then
			reach = math.max(reach, def.swarmCallRadius)
		end

		if distance <= reach and distance < bestDistance then
			best = player
			bestDistance = distance
		end
	end

	return best
end

local function extractionDestination(): Vector3?
	if extractionBeacon and extractionBeacon.Parent and os.clock() < extractionBeaconEndsAt then
		return extractionBeacon.Position
	end
	return nil
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

	-- The telegraph. "Every enemy must telegraph with sound before sight" is an
	-- accessibility requirement from the handoff, so a Sweeper may not land its
	-- first hit until `audioTelegraphSeconds` after it has announced itself.
	-- Tracked per Sweeper, because the warning belongs to the robot that is
	-- about to hit you, not to the species.
	local telegraphedAt: number? = nil
	local sound = (root :: BasePart):FindFirstChild("Telegraph") :: Sound?
	local chase = (root :: BasePart):FindFirstChild("Chase") :: Sound?

	local function telegraph()
		telegraphedAt = os.clock()
		-- An empty SoundId is the known gap, not a bug: playing it would only
		-- log noise. EnemyService.init says so once, at startup.
		if sound and sound.SoundId ~= "" then
			sound:Play()
		end
	end

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
						-- Walking into an idle Sweeper must still be announced,
						-- so the telegraph starts here if the chase never did.
						if not telegraphedAt then
							telegraph()
						elseif os.clock() - telegraphedAt >= def.audioTelegraphSeconds then
							RunStateService.registerContact(player)
						end
					end
				end
			end
			task.wait(0.2)
		end
	end)

	task.spawn(function()
		local hadTarget = false
		while model.Parent and humanoid.Health > 0 do
			local target = EnemyService.pickTarget((root :: BasePart).Position, "Sweepers")
			-- Announce on acquisition, which is what puts the sound before sight.
			if target and not hadTarget then
				telegraph()
			end
			-- Audible for as long as it is actually hunting, so a player can
			-- hear one coming and hear it give up.
			if chase then
				if target and not chase.IsPlaying then
					chase:Play()
				elseif not target and chase.IsPlaying then
					chase:Stop()
				end
			end
			hadTarget = target ~= nil
			local destination: Vector3? = nil
			if target and target.Character then
				local targetRoot = target.Character:FindFirstChild("HumanoidRootPart")
				if targetRoot and targetRoot:IsA("BasePart") then
					destination = targetRoot.Position
				end
			end
			-- An extraction call is a beacon, not merely a countdown. Sweepers
			-- that cannot currently see a keeper investigate the siren itself,
			-- which makes the holdout a location the mall converges on.
			destination = destination or extractionDestination()
			if destination then
				-- AgentCanJump is a STOPGAP, not a design choice. A floor
				-- cleaner should not hop.
				--
				-- Codex's two-level mall has grand stairs the navmesh will
				-- not climb: measured tread to tread, the east stair returns
				-- NoPath with jumping off and Success with it on, while the
				-- upper deck paths fine once you are on it. The geometry
				-- looks right - 1.35 rise, 3.2 going, edge to edge, 5 studs
				-- of headroom - but a stack of discrete step boxes is what
				-- the navmesh refuses without a sloped surface over it.
				--
				-- Left off, the upper gallery is a room no enemy can reach,
				-- and the relic - the rarest tier, the whole reason to light
				-- up - sits in it. A safe room holding the best loot is worse
				-- than a robot that hops a step. Set this back to false once
				-- the stairs are walkable; NOTES-FOR-CODEX.md has the
				-- measurements.
				local path = PathfindingService:CreatePath({
					AgentRadius = 2,
					AgentHeight = 4,
					AgentCanJump = true,
				})
				local ok = pcall(function()
					path:ComputeAsync((root :: BasePart).Position, destination)
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
			task.wait(REPATH_INTERVAL)
		end
	end)
end

local function spawnExtractionWave(exitPart: BasePart, count: number)
	local points: { BasePart } = {}
	for _, point in ipairs(CollectionService:GetTagged("SweeperSpawn")) do
		if point:IsA("BasePart") then
			table.insert(points, point)
		end
	end
	table.sort(points, function(a, b)
		return (a.Position - exitPart.Position).Magnitude
			< (b.Position - exitPart.Position).Magnitude
	end)
	-- Do not materialise a robot in the holdout circle. Starting from the
	-- nearest legal sweeper post past the first two means the response arrives
	-- with a few seconds of audible approach, not an unfair instant hit.
	for index = 3, math.min(#points, count + 2) do
		EnemyService.spawnSweeperAt(points[index])
	end
end

--- Makes an extraction call loud in gameplay terms: nearby robots investigate
--- immediately and a second group arrives mid-hold. The exit service owns the
--- timer; this module only remembers the beacon target for pathing.
function EnemyService.beginExtractionPressure(exitPart: BasePart, endsAt: number)
	extractionBeacon = exitPart
	extractionBeaconEndsAt = endsAt
	spawnExtractionWave(exitPart, 3)
	task.delay(8, function()
		if extractionBeacon == exitPart and os.clock() < extractionBeaconEndsAt then
			spawnExtractionWave(exitPart, 2)
		end
	end)
end

function EnemyService.endExtractionPressure(exitPart: BasePart)
	if extractionBeacon == exitPart then
		extractionBeacon = nil
		extractionBeaconEndsAt = 0
	end
end

--- Glimmers only acquire a player actively broadcasting lantern light. Their
--- scan is deliberately local: a player gets pressure in the same court, not
--- an unavoidable tag from the far side of the mall.
local function pickLitGlimmerTarget(fromPosition: Vector3): Player?
	local run = RunStateService.current()
	if not run or run.finished then
		return nil
	end
	local nearest: Player? = nil
	local nearestDistance = math.huge
	for userId, state in pairs(run.lightkeepers) do
		if not state.lanternOn or state.knockedDown then
			continue
		end
		local player = Players:GetPlayerByUserId(userId)
		local character = player and player.Character
		local root = character and character:FindFirstChild("HumanoidRootPart")
		if root and root:IsA("BasePart") then
			local distance = (root.Position - fromPosition).Magnitude
			if distance <= Config.Enemies.Glimmers.scanRange and distance < nearestDistance then
				nearest, nearestDistance = player, distance
			end
		end
	end
	return nearest
end

-- Spawning is allowed to choose any lit player. The range rule belongs to the
-- scan itself; applying it here would make the first Glimmer silently fail
-- unless a player happened to be standing near world origin.
local function findAnyLitGlimmerTarget(): Player?
	local run = RunStateService.current()
	if not run or run.finished then
		return nil
	end
	for userId, state in pairs(run.lightkeepers) do
		if not state.lanternOn or state.knockedDown then
			continue
		end
		local player = Players:GetPlayerByUserId(userId)
		local character = player and player.Character
		local root = character and character:FindFirstChild("HumanoidRootPart")
		if root and root:IsA("BasePart") then
			return player
		end
	end
	return nil
end

local function hasLineOfSight(
	from: BasePart,
	target: BasePart,
	glimmer: Model,
	character: Model
): boolean
	local parameters = RaycastParams.new()
	parameters.FilterType = Enum.RaycastFilterType.Exclude
	parameters.FilterDescendantsInstances = { glimmer, character }
	local hit = Workspace:Raycast(from.Position, target.Position - from.Position, parameters)
	return hit == nil
end

--- A Glimmer announces a scan, keeps clear sight on a lit target for the locked
--- two seconds, then applies the locked twenty-second shared-robot tag.
function EnemyService.driveGlimmer(model: Model)
	local humanoid = model:FindFirstChildOfClass("Humanoid")
	local root = model:FindFirstChild("HumanoidRootPart")
	if not humanoid or not root or not root:IsA("BasePart") then
		return
	end
	local def = Config.Enemies.Glimmers
	humanoid.WalkSpeed = def.moveSpeed
	humanoid.MaxHealth = def.health
	humanoid.Health = def.health
	local scanSound = root:FindFirstChild("ScanTelegraph") :: Sound?
	local scanningPlayer: Player? = nil
	local scanStartedAt: number? = nil

	task.spawn(function()
		while model.Parent and humanoid.Health > 0 do
			local target = pickLitGlimmerTarget(root.Position)
			local character = target and target.Character
			local targetRoot = character and character:FindFirstChild("HumanoidRootPart")
			if target and character and targetRoot and targetRoot:IsA("BasePart") then
				humanoid:MoveTo(targetRoot.Position)
				if hasLineOfSight(root, targetRoot, model, character) then
					if scanningPlayer ~= target then
						scanningPlayer = target
						scanStartedAt = os.clock()
						if scanSound and scanSound.SoundId ~= "" then
							scanSound:Play()
						end
					elseif scanStartedAt and os.clock() - scanStartedAt >= def.scanSeconds then
						local run = RunStateService.current()
						local state = run and run.lightkeepers[target.UserId]
						-- Re-check the lantern at completion: going dark during the scan
						-- is the counterplay, not merely a visual effect.
						if state and state.lanternOn and not state.knockedDown then
							state.taggedUntil = os.clock() + def.tagSeconds
						end
						scanningPlayer = nil
						scanStartedAt = nil
					end
				else
					scanningPlayer = nil
					scanStartedAt = nil
				end
			else
				scanningPlayer = nil
				scanStartedAt = nil
			end
			task.wait(0.15)
		end
	end)
end

--- A Window Walker is inert scenery until active lantern light catches it in
--- the same nearby space. Once awake it walks at a normal, deliberately slow
--- pace, but never pathfinds or magically knows a player through a wall.
local function pickWalkerTarget(from: BasePart, walker: Model, range: number, def: any): Player?
	local run = RunStateService.current()
	if not run or run.finished then
		return nil
	end
	local closest: Player? = nil
	local bestDistance = math.huge
	for userId, state in pairs(run.lightkeepers) do
		if state.knockedDown or (def.wakeRequiresActiveLantern and not state.lanternOn) then
			continue
		end
		local player = Players:GetPlayerByUserId(userId)
		local character = player and player.Character
		local targetRoot = character and character:FindFirstChild("HumanoidRootPart")
		if character and targetRoot and targetRoot:IsA("BasePart") then
			local distance = (targetRoot.Position - from.Position).Magnitude
			if
				distance <= range
				and distance < bestDistance
				and hasLineOfSight(from, targetRoot, walker, character)
			then
				closest, bestDistance = player, distance
			end
		end
	end
	return closest
end

function EnemyService.driveWindowWalker(model: Model)
	local humanoid = model:FindFirstChildOfClass("Humanoid")
	local root = model:FindFirstChild("HumanoidRootPart")
	if not humanoid or not root or not root:IsA("BasePart") then
		return
	end
	local def = Config.Enemies.WindowWalkers
	humanoid.WalkSpeed = def.moveSpeed
	humanoid.MaxHealth = def.health
	humanoid.Health = def.health
	local sound = root:FindFirstChild("Telegraph") :: Sound?
	local awake = false
	local telegraphedAt: number? = nil

	local function wake()
		awake = true
		telegraphedAt = os.clock()
		if sound and sound.SoundId ~= "" then
			sound:Play()
		end
	end

	-- Contact is intentionally gated behind the same audible warning as a
	-- Sweeper. A player who lights up the mannequin gets a chance to hear and
	-- see it begin walking before it can contribute a knockdown hit.
	task.spawn(function()
		local contact = Config.Lantern.sweeperContact
		while model.Parent and humanoid.Health > 0 do
			if
				awake
				and telegraphedAt
				and os.clock() - telegraphedAt >= def.audioTelegraphSeconds
			then
				for _, player in ipairs(Players:GetPlayers()) do
					local character = player.Character
					local targetRoot = character and character:FindFirstChild("HumanoidRootPart")
					if targetRoot and targetRoot:IsA("BasePart") then
						if
							(root.Position - targetRoot.Position).Magnitude <= contact.contactRange
						then
							RunStateService.registerContact(player)
						end
					end
				end
			end
			task.wait(0.2)
		end
	end)

	task.spawn(function()
		while model.Parent and humanoid.Health > 0 do
			local target =
				pickWalkerTarget(root, model, if awake then def.sightRange else def.wakeRange, def)
			if target and not awake then
				wake()
			end
			if awake and target and target.Character then
				local targetRoot = target.Character:FindFirstChild("HumanoidRootPart")
				if targetRoot and targetRoot:IsA("BasePart") then
					humanoid:MoveTo(targetRoot.Position)
				end
			else
				-- It does not shuffle around when the light or line of sight is gone.
				humanoid:MoveTo(root.Position)
			end
			task.wait(0.25)
		end
	end)
end

-- Ficus stalkers use the same readable lantern-wake rule as mannequins, but
-- move more slowly and begin as ordinary department-store greenery.
function EnemyService.driveFicus(model: Model)
	local humanoid = model:FindFirstChildOfClass("Humanoid")
	local root = model:FindFirstChild("HumanoidRootPart")
	if not humanoid or not root or not root:IsA("BasePart") then
		return
	end
	local def = Config.Enemies.Ficus
	humanoid.WalkSpeed, humanoid.MaxHealth, humanoid.Health = def.moveSpeed, def.health, def.health
	local sound = root:FindFirstChild("Telegraph") :: Sound?
	local awake, telegraphedAt = false, nil :: number?
	task.spawn(function()
		local contact = Config.Lantern.sweeperContact
		while model.Parent and humanoid.Health > 0 do
			if
				awake
				and telegraphedAt
				and os.clock() - telegraphedAt >= def.audioTelegraphSeconds
			then
				for _, player in ipairs(Players:GetPlayers()) do
					local character = player.Character
					local targetRoot = character and character:FindFirstChild("HumanoidRootPart")
					if
						targetRoot
						and targetRoot:IsA("BasePart")
						and (root.Position - targetRoot.Position).Magnitude
							<= contact.contactRange
					then
						RunStateService.registerContact(player)
					end
				end
			end
			task.wait(0.2)
		end
	end)
	task.spawn(function()
		while model.Parent and humanoid.Health > 0 do
			local target =
				pickWalkerTarget(root, model, if awake then def.sightRange else def.wakeRange, def)
			if target and not awake then
				awake, telegraphedAt = true, os.clock()
				if sound and sound.SoundId ~= "" then
					sound:Play()
				end
			end
			local targetRoot = target
				and target.Character
				and target.Character:FindFirstChild("HumanoidRootPart")
			if awake and targetRoot and targetRoot:IsA("BasePart") then
				humanoid:MoveTo(targetRoot.Position)
			else
				humanoid:MoveTo(root.Position)
			end
			task.wait(0.25)
		end
	end)
end

--- The Sweeper rig. Absent from the project today, so this reports the gap
--- loudly rather than spawning nothing in silence - a map with four spawn
--- points and no robots looks identical to a working one until you go looking
--- for something to be chased by.
local warnedMissingTemplate = false

local function sweeperTemplate(): Model?
	local folder = ReplicatedStorage:FindFirstChild("Enemies")
	local model = folder and folder:FindFirstChild("Sweeper")
	if model and model:IsA("Model") then
		return model
	end
	if not warnedMissingTemplate then
		warnedMissingTemplate = true
		warn(
			"[EnemyService] no ReplicatedStorage.Enemies.Sweeper template - no Sweepers will spawn, so nothing can knock a Lightkeeper down"
		)
	end
	return nil
end

local function glimmerTemplate(): Model?
	local folder = ReplicatedStorage:FindFirstChild("Enemies")
	local model = folder and folder:FindFirstChild("Glimmer")
	return if model and model:IsA("Model") then model else nil
end

local function windowWalkerTemplate(): Model?
	local folder = ReplicatedStorage:FindFirstChild("Enemies")
	local model = folder and folder:FindFirstChild("WindowWalker")
	return if model and model:IsA("Model") then model else nil
end

local function ficusTemplate(): Model?
	local folder = ReplicatedStorage:FindFirstChild("Enemies")
	local model = folder and folder:FindFirstChild("Ficus")
	return if model and model:IsA("Model") then model else nil
end

function EnemyService.spawnSweeperAt(point: Instance)
	if not point:IsA("BasePart") then
		return
	end
	local model = sweeperTemplate()
	if not model then
		return
	end
	local clone = model:Clone()
	clone:PivotTo(CFrame.new((point :: BasePart).Position + Vector3.new(0, 3, 0)))
	clone.Parent = workspace
	EnemyService.driveSweeper(clone)
end

function EnemyService.spawnGlimmerForLitTarget()
	local target = findAnyLitGlimmerTarget()
	local character = target and target.Character
	local targetRoot = character and character:FindFirstChild("HumanoidRootPart")
	local template = glimmerTemplate()
	if not targetRoot or not targetRoot:IsA("BasePart") or not template then
		return
	end
	local clone = template:Clone()
	-- Spawn inside the same nearby court, never directly on top of a player.
	clone:PivotTo(
		CFrame.new(targetRoot.Position + Vector3.new(Config.Enemies.Glimmers.scanRange - 5, 3, 0))
	)
	clone.Parent = workspace
	EnemyService.driveGlimmer(clone)
end

function EnemyService.spawnWindowWalkerForPlayer(spawnPoint: BasePart?)
	local run = RunStateService.current()
	if not run or run.finished then
		return
	end
	local target: Player? = nil
	for userId, state in pairs(run.lightkeepers) do
		if not state.knockedDown then
			local candidate = Players:GetPlayerByUserId(userId)
			if
				candidate
				and candidate.Character
				and candidate.Character:FindFirstChild("HumanoidRootPart")
			then
				target = candidate
				break
			end
		end
	end
	local character = target and target.Character
	local targetRoot = character and character:FindFirstChild("HumanoidRootPart")
	local template = windowWalkerTemplate()
	if not targetRoot or not targetRoot:IsA("BasePart") or not template then
		return
	end

	-- Prefer a known enemy marker to an arbitrary point in the new mall. This
	-- keeps the map authoritative about safe spawnable floor while still letting
	-- the Walker exist without its own map tag.
	local spawnPosition = if spawnPoint
		then spawnPoint.Position + Vector3.new(0, 3, 0)
		else targetRoot.Position + Vector3.new(24, 3, 0)
	if not spawnPoint then
		local nearestDistance = math.huge
		for _, point in ipairs(CollectionService:GetTagged("SweeperSpawn")) do
			if point:IsA("BasePart") then
				local distance = (point.Position - targetRoot.Position).Magnitude
				if distance < nearestDistance then
					nearestDistance = distance
					spawnPosition = point.Position + Vector3.new(0, 3, 0)
				end
			end
		end
	end

	local clone = template:Clone()
	clone:PivotTo(CFrame.new(spawnPosition))
	clone.Parent = Workspace
	EnemyService.driveWindowWalker(clone)
end

function EnemyService.spawnWindowWalkersAtStoreMarkers()
	local spawned = 0
	for index, point in ipairs(CollectionService:GetTagged("StockerSpawn")) do
		-- A mannequin belongs in a department-store aisle, not all clustered by
		-- the player at the court. Four distributed sleepers make each store a
		-- possible "uh oh" moment when a lantern comes on.
		if index % 2 == 0 and point:IsA("BasePart") then
			EnemyService.spawnWindowWalkerForPlayer(point)
			spawned += 1
		end
	end
	if spawned == 0 then
		EnemyService.spawnWindowWalkerForPlayer()
	end
end

function EnemyService.spawnFicusAt(point: Instance, offset: Vector3?)
	if not point:IsA("BasePart") then
		return
	end
	local template = ficusTemplate()
	if not template then
		return
	end
	local clone = template:Clone()
	clone:PivotTo(point.CFrame * CFrame.new(offset or Vector3.new(0, 3, 0)))
	clone.Parent = Workspace
	EnemyService.driveFicus(clone)
end

function EnemyService.spawnFicusAtTaggedPoints()
	local points = CollectionService:GetTagged("FicusSpawn")
	-- Map safety validation may reject a planter marker when it intersects a
	-- shelf or column. The department-store Stocker markers are already proven
	-- walkable, so use every other one rather than silently shipping no ficus.
	if #points == 0 then
		local stockers = CollectionService:GetTagged("StockerSpawn")
		for index, point in ipairs(stockers) do
			if index % 2 == 1 then
				EnemyService.spawnFicusAt(point, Vector3.new(6, 3, 0))
			end
		end
		return
	end
	for _, point in ipairs(points) do
		EnemyService.spawnFicusAt(point)
	end
end

--- Removes every live enemy. Called between expeditions: the spawn points are
--- re-tagged when the map rebuilds, and without this the new ones pile on top of
--- the old and the second run is four times as dangerous as the first.
function EnemyService.clearSweepers()
	patrolsAlerted = false
	extractionBeacon = nil
	extractionBeaconEndsAt = 0
	for _, model in ipairs(workspace:GetChildren()) do
		if
			model:IsA("Model")
			and (
				model.Name == "Sweeper"
				or model.Name == "Stocker"
				or model.Name == "Glimmer"
				or model.Name == "WindowWalker"
				or model.Name == "Ficus"
				or model.Name == "Warden"
			)
		then
			model:Destroy()
		end
	end
end

function EnemyService.spawnSweepersAtTaggedPoints()
	for _, point in ipairs(CollectionService:GetTagged("SweeperSpawn")) do
		EnemyService.spawnSweeperAt(point)
	end
end

function EnemyService.spawnStockerAt(point: Instance)
	if not point:IsA("BasePart") then
		return
	end
	local enemies = ReplicatedStorage:FindFirstChild("Enemies")
	local template = enemies and enemies:FindFirstChild("Sweeper")
	if not template or not template:IsA("Model") then
		return
	end
	local model = template:Clone()
	model.Name = "Stocker"
	model:PivotTo(point.CFrame * CFrame.new(0, 3, 0))
	model.Parent = Workspace
	local humanoid, root =
		model:FindFirstChildOfClass("Humanoid"), model:FindFirstChild("HumanoidRootPart")
	if not humanoid or not root or not root:IsA("BasePart") then
		model:Destroy()
		return
	end
	local def = Config.Enemies.Stockers
	humanoid.WalkSpeed, humanoid.MaxHealth, humanoid.Health = def.moveSpeed, def.health, def.health
	local cue = root:FindFirstChild("Telegraph")
	if cue and cue:IsA("Sound") then
		cue:Play()
	end
	task.spawn(function()
		while model.Parent and humanoid.Health > 0 do
			local target = EnemyService.pickTarget(root.Position, "Stockers")
			local targetRoot = target
				and target.Character
				and target.Character:FindFirstChild("HumanoidRootPart")
			if targetRoot and targetRoot:IsA("BasePart") then
				humanoid:MoveTo(targetRoot.Position)
				if (root.Position - targetRoot.Position).Magnitude <= 7 then
					RunStateService.knockDown(target :: Player)
				end
			end
			task.wait(REPATH_INTERVAL)
		end
	end)
end

function EnemyService.spawnStockersAtTaggedPoints()
	for _, point in ipairs(CollectionService:GetTagged("StockerSpawn")) do
		EnemyService.spawnStockerAt(point)
	end
end

--- Storm Charge thresholds are the only pacing dial for enemy pressure.
function EnemyService.onThresholdCrossed(threshold: any)
	if threshold.id == "PatrolsAlert" then
		patrolsAlerted = true
		EnemyService.spawnWindowWalkerForPlayer()
	elseif threshold.id == "PatrolsHunt" then
		EnemyService.spawnGlimmerForLitTarget()
	elseif threshold.id == "WardenConverges" then
		EnemyService.spawnWarden()
	end
end

--- The Warden is unkillable and drops nothing. It is the meter with a body.
function EnemyService.spawnWarden()
	local enemies = ReplicatedStorage:FindFirstChild("Enemies")
	local template = enemies and enemies:FindFirstChild("Sweeper")
	local run = RunStateService.current()
	if not template or not template:IsA("Model") or not run then
		return
	end
	local target: BasePart? = nil
	for userId, state in pairs(run.lightkeepers) do
		if not state.knockedDown then
			local player = Players:GetPlayerByUserId(userId)
			local root = player
				and player.Character
				and player.Character:FindFirstChild("HumanoidRootPart")
			if root and root:IsA("BasePart") then
				target = root
				break
			end
		end
	end
	if not target then
		return
	end
	local warden = template:Clone()
	warden.Name = "Warden"
	warden:PivotTo(CFrame.new(target.Position + Vector3.new(70, 4, 70)))
	warden.Parent = Workspace
	local humanoid = warden:FindFirstChildOfClass("Humanoid")
	local root = warden:FindFirstChild("HumanoidRootPart")
	if not humanoid or not root or not root:IsA("BasePart") then
		warden:Destroy()
		return
	end
	humanoid.WalkSpeed = Config.Enemies.Warden.moveSpeed
	humanoid.MaxHealth, humanoid.Health = math.huge, math.huge
	local sound = root:FindFirstChild("Telegraph")
	if sound and sound:IsA("Sound") then
		sound:Play()
	end
	task.spawn(function()
		while warden.Parent and RunStateService.current() == run and not run.finished do
			local nearest: BasePart? = nil
			local distance = math.huge
			for userId, state in pairs(run.lightkeepers) do
				if not state.knockedDown then
					local player = Players:GetPlayerByUserId(userId)
					local candidate = player
						and player.Character
						and player.Character:FindFirstChild("HumanoidRootPart")
					if candidate and candidate:IsA("BasePart") then
						local d = (candidate.Position - root.Position).Magnitude
						if d < distance then
							nearest, distance = candidate, d
						end
					end
				end
			end
			if nearest then
				humanoid:MoveTo(nearest.Position)
				if distance < 7 then
					local player = Players:GetPlayerFromCharacter(nearest.Parent)
					if player then
						RunStateService.knockDown(player)
					end
				end
			end
			task.wait(Config.Enemies.Warden.retargetIntervalSeconds)
		end
		if warden.Parent then
			warden:Destroy()
		end
	end)
end

function EnemyService.init()
	RunStateService.onThresholdCrossed = EnemyService.onThresholdCrossed

	-- Build the template before anything tries to clone it.
	local rig = SweeperRig.ensure()
	GlimmerRig.ensure()
	WindowWalkerRig.ensure()
	FicusRig.ensure()
	local sound = rig.PrimaryPart and rig.PrimaryPart:FindFirstChild("Telegraph")
	if sound and sound:IsA("Sound") and sound.SoundId == "" then
		warn(
			"[EnemyService] the Sweeper telegraph has no SoundId - timing and falloff are wired, but it is silent until Chris supplies an audio asset. Enemies are required to telegraph with sound before sight."
		)
	end

	-- This was never called. Sweepers had a spawn function, four tagged spawn
	-- points and a contact path that knocks players down, and none of it ever
	-- ran, so the knockdown rule could not be reached in play at all.
	EnemyService.spawnSweepersAtTaggedPoints()
	EnemyService.spawnStockersAtTaggedPoints()
	EnemyService.spawnFicusAtTaggedPoints()
	-- The cast lives in the mall from minute one. A mannequin and Glimmer only
	-- become dangerous when the player lights up; Storm Charge escalates them,
	-- it does not postpone their existence.
	EnemyService.spawnWindowWalkersAtStoreMarkers()
	task.spawn(function()
		-- A Glimmer needs an active lantern to choose a legal scan target. Keep
		-- trying briefly after departure so turning on the lantern after leaving
		-- the spawn still introduces one during the opening push.
		for _ = 1, 20 do
			task.wait(2)
			if not RunStateService.current() then
				return
			end
			if not Workspace:FindFirstChild("Glimmer") then
				EnemyService.spawnGlimmerForLitTarget()
			end
		end
	end)
	-- Spawn points tagged after startup still populate, matching the loot,
	-- extraction and relay watchers.
	if not watchingSpawnerTags then
		watchingSpawnerTags = true
		CollectionService:GetInstanceAddedSignal("SweeperSpawn")
			:Connect(EnemyService.spawnSweeperAt)
	end
	if not watchingStockerTags then
		watchingStockerTags = true
		CollectionService:GetInstanceAddedSignal("StockerSpawn")
			:Connect(EnemyService.spawnStockerAt)
	end
end

return EnemyService
