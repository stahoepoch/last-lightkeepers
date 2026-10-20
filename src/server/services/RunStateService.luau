--!strict
-- Authoritative run state: Storm Charge, lanterns, knockdowns, banking, wipes.
--
-- Named RunStateService rather than the handoff's "RunService" on purpose:
-- `RunService` collides with Roblox's own service and makes every require in
-- this file ambiguous. Same responsibility, non-colliding name.
--
-- Version one runs one expedition per server, so the state is a module singleton.

local Players = game:GetService("Players")
local RobloxRun = game:GetService("RunService")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Remotes = require(Shared:WaitForChild("Remotes"))
local Types = require(Shared:WaitForChild("Types"))

local SNAPSHOT_INTERVAL = 0.25

local RunStateService = {}

local run: Types.RunState? = nil
local snapshotAccumulator = 0

local function emptyHaul(): Types.Haul
	return {
		carried = {},
		banked = {},
	}
end

local function newLightkeeper(userId: number): Types.LightkeeperState
	return {
		userId = userId,
		lanternOn = false,
		lanternCharge = 1,
		knockedDown = false,
		anchorRevivesLeft = Config.StormCharge.anchorRevive.chargesPerRun,
		taggedUntil = nil,
		carryingCrate = false,
		haul = emptyHaul(),
		contactHits = 0,
		lastContactAt = nil,
		reviveProgress = 0,
		reviveBy = nil,
	}
end

local function rootOf(userId: number): BasePart?
	local player = Players:GetPlayerByUserId(userId)
	local character = player and player.Character
	local root = character and character:FindFirstChild("HumanoidRootPart")
	return if root and root:IsA("BasePart") then root else nil
end

local function humanoidOf(userId: number): Humanoid?
	local player = Players:GetPlayerByUserId(userId)
	local character = player and player.Character
	return if character then character:FindFirstChildOfClass("Humanoid") else nil
end

--- A downed Lightkeeper cannot walk. Teammates have to come to them, which is
--- the whole point of the knockdown rule.
local function applyDownedMovement(userId: number, downed: boolean)
	local humanoid = humanoidOf(userId)
	if not humanoid then
		return
	end
	if downed then
		humanoid.WalkSpeed = Config.Lantern.knockdown.downedWalkSpeed
		humanoid.JumpPower = 0
		humanoid.PlatformStand = true
	else
		humanoid.WalkSpeed = 16
		humanoid.JumpPower = 50
		humanoid.PlatformStand = false
	end
end

function RunStateService.current(): Types.RunState?
	return run
end

function RunStateService.start()
	run = {
		runId = tostring(os.time()) .. "-" .. tostring(math.random(1000, 9999)),
		startedAt = os.clock(),
		stormCharge = 0,
		crossedThresholds = {},
		lightkeepers = {},
		finished = false,
	}
	for _, player in ipairs(Players:GetPlayers()) do
		RunStateService.addPlayer(player)
	end
end

--- Enemies knock Lightkeepers down; they do not kill them. This intercepts the
--- lethal case rather than trusting every damage source to be polite about it.
function RunStateService.guardCharacter(player: Player, character: Model)
	local humanoid = character:WaitForChild("Humanoid", 5)
	if not humanoid or not humanoid:IsA("Humanoid") then
		return
	end
	humanoid.BreakJointsOnDeath = false

	humanoid.HealthChanged:Connect(function(health: number)
		if health > 0 then
			return
		end
		-- Pull them back off the floor and down them instead.
		humanoid.Health = 1
		RunStateService.knockDown(player)
	end)
end

function RunStateService.addPlayer(player: Player)
	if not run or run.finished then
		return
	end
	run.lightkeepers[player.UserId] = newLightkeeper(player.UserId)

	if player.Character then
		RunStateService.guardCharacter(player, player.Character)
	end
	player.CharacterAdded:Connect(function(character)
		RunStateService.guardCharacter(player, character)
		local state = run and run.lightkeepers[player.UserId]
		if state and state.knockedDown then
			applyDownedMovement(player.UserId, true)
		end
	end)
end

function RunStateService.removePlayer(player: Player)
	if not run then
		return
	end
	-- Disconnect follows the wipe rule: banked loot is already safe, carried
	-- loot is halved. Resolved per-player so one dropout cannot grief the crew.
	local state = run.lightkeepers[player.UserId]
	if state then
		RunStateService.resolveHaul(state, false)
		run.lightkeepers[player.UserId] = nil
	end
end

function RunStateService.activeLanterns(): number
	if not run then
		return 0
	end
	local count = 0
	for _, state in pairs(run.lightkeepers) do
		if state.lanternOn and state.lanternCharge > 0 then
			count += 1
		end
	end
	return count
end

function RunStateService.setLantern(player: Player, on: boolean)
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	if on and state.lanternCharge <= 0 then
		return
	end
	state.lanternOn = on
end

function RunStateService.isLanternOn(player: Player): boolean
	if not run then
		return false
	end
	local state = run.lightkeepers[player.UserId]
	return state ~= nil and state.lanternOn and state.lanternCharge > 0
end

--- Storm Charge only ever rises. Darkness slows the climb; nothing reverses it.
function RunStateService.addStormCharge(amount: number)
	if not run or run.finished or amount <= 0 then
		return
	end
	run.stormCharge += amount

	local crossed = Config.thresholdAt(run.stormCharge)
	if crossed and not run.crossedThresholds[crossed.id] then
		run.crossedThresholds[crossed.id] = true
		RunStateService.onThresholdCrossed(crossed)
	end
end

-- Assigned by init.server so EnemyService can react without a circular require.
RunStateService.onThresholdCrossed = function(_threshold: any) end

function RunStateService.knockDown(player: Player)
	if not run then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	state.knockedDown = true
	state.contactHits = 0
	state.reviveProgress = 0
	state.reviveBy = nil
	if Config.Lantern.knockdown.lanternForcedOffWhenDowned then
		state.lanternOn = false
	end
	if Config.Lantern.knockdown.dropsCarriedCrate then
		state.carryingCrate = false
	end
	applyDownedMovement(player.UserId, true)

	-- Whole crew down: lanterns burn out, partial-loss wipe.
	for _, other in pairs(run.lightkeepers) do
		if not other.knockedDown then
			return
		end
	end
	RunStateService.wipe()
end

local function stand(state: Types.LightkeeperState)
	state.knockedDown = false
	state.reviveProgress = 0
	state.reviveBy = nil
	state.contactHits = 0
	applyDownedMovement(state.userId, false)
end

--- Solo Anchor Revive. Paid for with a full lantern charge plus a permanent
--- Storm Charge spike: the thematic cost of light, and the reason a solo player
--- is not hard-wiped on a first knockdown.
function RunStateService.anchorRevive(player: Player): boolean
	if not run or run.finished then
		return false
	end
	local state = run.lightkeepers[player.UserId]
	if not state or not state.knockedDown then
		return false
	end

	local anchor = Config.StormCharge.anchorRevive
	if state.anchorRevivesLeft <= 0 then
		return false
	end
	-- "Consumes a full lantern charge" means it must actually be full.
	if anchor.consumesFullLanternCharge and state.lanternCharge < 1 then
		return false
	end

	state.anchorRevivesLeft -= 1
	state.lanternCharge = 0
	state.lanternOn = false
	RunStateService.addStormCharge(anchor.stormChargeSpike)
	stand(state)
	return true
end

--- A teammate starts a revive. It is a hold, advanced in the tick while the
--- rescuer stays close; walking away cancels it. Reviving has to cost the
--- rescuer's attention, or carrying and distracting stop mattering.
function RunStateService.beginRevive(rescuer: Player, target: Player): boolean
	if not run or run.finished or rescuer == target then
		return false
	end
	local rescuerState = run.lightkeepers[rescuer.UserId]
	local targetState = run.lightkeepers[target.UserId]
	if not rescuerState or not targetState then
		return false
	end
	if rescuerState.knockedDown or not targetState.knockedDown then
		return false
	end

	local a, b = rootOf(rescuer.UserId), rootOf(target.UserId)
	if not a or not b then
		return false
	end
	if (a.Position - b.Position).Magnitude > Config.Lantern.revive.maxReviveDistance then
		return false
	end

	targetState.reviveBy = rescuer.UserId
	return true
end

--- A rescuer let go of the prompt. Clears only their own claim, so one player
--- releasing cannot cancel another's revive on the same target.
function RunStateService.cancelRevive(target: Player)
	if not run then
		return
	end
	local state = run.lightkeepers[target.UserId]
	if state and state.knockedDown then
		state.reviveProgress = 0
		state.reviveBy = nil
	end
end

--- Advances every in-progress revive. Called from the tick.
local function stepRevives(dt: number)
	if not run then
		return
	end
	local hold = Config.Lantern.revive.teammateHoldSeconds
	local maxDistance = Config.Lantern.revive.maxReviveDistance

	for _, state in pairs(run.lightkeepers) do
		if not state.knockedDown or not state.reviveBy then
			continue
		end

		local rescuerState = run.lightkeepers[state.reviveBy]
		local a, b = rootOf(state.reviveBy), rootOf(state.userId)
		local inRange = a ~= nil
			and b ~= nil
			and (a.Position - b.Position).Magnitude <= maxDistance

		if not rescuerState or rescuerState.knockedDown or not inRange then
			-- Cancelled: the rescuer went down, left, or walked away.
			state.reviveProgress = 0
			state.reviveBy = nil
			continue
		end

		state.reviveProgress += dt
		if state.reviveProgress >= hold then
			stand(state)
		end
	end
end

--- Moves carried loot into the bank. Banked loot survives a wipe outright.
function RunStateService.bank(player: Player)
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	for tier, amount in pairs(state.haul.carried) do
		state.haul.banked[tier] = (state.haul.banked[tier] or 0) + amount
		state.haul.carried[tier] = nil
	end

	-- Off by default. See the rechargeAtRelay proposal in lantern.json - it is
	-- Chris's call, not mine, because it softens the charge economy.
	if Config.Lantern.charge.rechargeAtRelay then
		state.lanternCharge = 1
	end
end

--- Final payout for one Lightkeeper. `extracted` keeps everything carried.
function RunStateService.resolveHaul(state: Types.LightkeeperState, extracted: boolean): { [Types.LootTier]: number }
	local keptFraction = if extracted then 1 else Config.Loot.banking.unbankedKeptOnWipe
	local payout: { [Types.LootTier]: number } = {}

	for tier, amount in pairs(state.haul.banked) do
		payout[tier] = (payout[tier] or 0) + amount
	end
	for tier, amount in pairs(state.haul.carried) do
		payout[tier] = (payout[tier] or 0) + math.floor(amount * keptFraction)
	end

	state.haul = emptyHaul()
	return payout
end

function RunStateService.wipe()
	if not run or run.finished then
		return
	end
	run.finished = true
	for userId, state in pairs(run.lightkeepers) do
		state.lanternOn = false
		-- They wake in Lanternhold, on their feet.
		state.knockedDown = false
		state.reviveBy = nil
		state.reviveProgress = 0
		applyDownedMovement(userId, false)
		local payout = RunStateService.resolveHaul(state, false)
		local player = Players:GetPlayerByUserId(userId)
		if player then
			Remotes.get("RunEnded"):FireClient(player, {
				outcome = "wipe",
				payout = payout,
			})
		end
	end
	RunStateService.onRunEnded("wipe")
end

RunStateService.onRunEnded = function(_outcome: string) end

--- A Sweeper reached someone. Hits accumulate and decay, so one Sweeper is
--- survivable and a swarm is not, without giving the player a health bar.
function RunStateService.registerContact(player: Player): boolean
	if not run or run.finished then
		return false
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return false
	end

	local contact = Config.Lantern.sweeperContact
	local now = os.clock()
	if state.lastContactAt and now - state.lastContactAt < contact.contactCooldownSeconds then
		return false
	end

	state.lastContactAt = now
	state.contactHits += 1
	if contact.knockdownOnContact and state.contactHits >= contact.hitsToKnockDown then
		RunStateService.knockDown(player)
		return true
	end
	return false
end

--- Per-viewer snapshot. Crew-wide numbers plus this player's own state: a
--- broadcast cannot answer "is *my* lantern lit" or "how much charge do I have".
local function buildSnapshot(state: Types.LightkeeperState): Types.RunSnapshot
	local current = run :: Types.RunState
	local active = RunStateService.activeLanterns()
	local crossed = Config.thresholdAt(current.stormCharge)

	local down, total = 0, 0
	for _, other in pairs(current.lightkeepers) do
		total += 1
		if other.knockedDown then
			down += 1
		end
	end

	return {
		stormCharge = current.stormCharge,
		stormRate = Config.stormRate(active),
		activeLanterns = active,
		highestThreshold = if crossed then crossed.id else nil,
		wardenActive = current.crossedThresholds.WardenConverges == true,
		yourCharge = state.lanternCharge,
		yourLanternOn = state.lanternOn and state.lanternCharge > 0,
		yourKnockedDown = state.knockedDown,
		yourTagged = state.taggedUntil ~= nil,
		crewDown = down,
		crewTotal = total,
	}
end

function RunStateService.start_tick()
	RobloxRun.Heartbeat:Connect(function(dt: number)
		if not run or run.finished then
			return
		end

		RunStateService.addStormCharge(Config.stormRate(RunStateService.activeLanterns()) * dt)

		local now = os.clock()
		local drain = Config.lanternDrainPerSecond() * dt
		local contactDecay = Config.Lantern.sweeperContact.hitDecaySeconds

		for _, state in pairs(run.lightkeepers) do
			-- Expire Glimmer tags.
			if state.taggedUntil and now >= state.taggedUntil then
				state.taggedUntil = nil
			end

			-- Burn lantern charge. Light is spent, not merely switched on.
			if state.lanternOn and state.lanternCharge > 0 then
				state.lanternCharge = math.max(state.lanternCharge - drain, 0)
				if state.lanternCharge <= 0 and Config.Lantern.charge.autoOffAtEmpty then
					state.lanternOn = false
				end
			end

			-- Sweeper contact fades if you break away.
			if state.contactHits > 0 and state.lastContactAt and now - state.lastContactAt >= contactDecay then
				state.contactHits -= 1
				state.lastContactAt = now
			end
		end

		stepRevives(dt)

		snapshotAccumulator += dt
		if snapshotAccumulator >= SNAPSHOT_INTERVAL then
			snapshotAccumulator = 0
			local snapshotEvent = Remotes.get("RunSnapshot")
			for userId, state in pairs(run.lightkeepers) do
				local player = Players:GetPlayerByUserId(userId)
				if player then
					snapshotEvent:FireClient(player, buildSnapshot(state))
				end
			end
		end
	end)
end

return RunStateService
