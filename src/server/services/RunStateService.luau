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
	}
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

function RunStateService.addPlayer(player: Player)
	if not run or run.finished then
		return
	end
	run.lightkeepers[player.UserId] = newLightkeeper(player.UserId)
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
	state.lanternOn = false
	state.carryingCrate = false

	-- Whole crew down: lanterns burn out, partial-loss wipe.
	for _, other in pairs(run.lightkeepers) do
		if not other.knockedDown then
			return
		end
	end
	RunStateService.wipe()
end

function RunStateService.revive(target: Player, bySelf: boolean): boolean
	if not run or run.finished then
		return false
	end
	local state = run.lightkeepers[target.UserId]
	if not state or not state.knockedDown then
		return false
	end

	if bySelf then
		-- Solo Anchor Revive: paid for with a full lantern charge plus a
		-- permanent Storm Charge spike. The thematic cost of light.
		local anchor = Config.StormCharge.anchorRevive
		if state.anchorRevivesLeft <= 0 or state.lanternCharge < 1 then
			return false
		end
		state.anchorRevivesLeft -= 1
		state.lanternCharge = 0
		RunStateService.addStormCharge(anchor.stormChargeSpike)
	end

	state.knockedDown = false
	return true
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

local function buildSnapshot(): Types.RunSnapshot
	local active = RunStateService.activeLanterns()
	local crossed = Config.thresholdAt((run :: Types.RunState).stormCharge)
	return {
		stormCharge = (run :: Types.RunState).stormCharge,
		stormRate = Config.stormRate(active),
		activeLanterns = active,
		highestThreshold = if crossed then crossed.id else nil,
		wardenActive = (run :: Types.RunState).crossedThresholds.WardenConverges == true,
	}
end

function RunStateService.start_tick()
	RobloxRun.Heartbeat:Connect(function(dt: number)
		if not run or run.finished then
			return
		end

		RunStateService.addStormCharge(Config.stormRate(RunStateService.activeLanterns()) * dt)

		-- Expire Glimmer tags.
		local now = os.clock()
		for _, state in pairs(run.lightkeepers) do
			if state.taggedUntil and now >= state.taggedUntil then
				state.taggedUntil = nil
			end
		end

		snapshotAccumulator += dt
		if snapshotAccumulator >= SNAPSHOT_INTERVAL then
			snapshotAccumulator = 0
			Remotes.get("RunSnapshot"):FireAllClients(buildSnapshot())
		end
	end)
end

return RunStateService
