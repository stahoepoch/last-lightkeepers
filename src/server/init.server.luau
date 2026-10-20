--!strict
-- Server entry point. Creates remotes, wires services, starts the run.

local Players = game:GetService("Players")
local Workspace = game:GetService("Workspace")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))

Remotes.init()

-- Rojo owns Workspace.Map, so each sync deliberately removes the generated
-- mall and its CollectionService tags. Build the authoritative runtime slice
-- before any service scans or watches those tags. This keeps a clean Play
-- session playable without asking a designer to run a command-bar step.
local Map = Workspace:WaitForChild("Map")
local FloodedMallBuilder = require(Map:WaitForChild("FloodedMallBuilder"))
FloodedMallBuilder.Build()

local Services = script:WaitForChild("services")
local RunStateService = require(Services:WaitForChild("RunStateService"))
local LootService = require(Services:WaitForChild("LootService"))
local EnemyService = require(Services:WaitForChild("EnemyService"))
local PlayerDataService = require(Services:WaitForChild("PlayerDataService"))
local InteractionService = require(Services:WaitForChild("InteractionService"))

PlayerDataService.init()
LootService.init()
EnemyService.init()
-- After LootService: prompts call into it.
InteractionService.init()

-- Client requests. Each one is validated server-side; nothing here trusts a payload.
local lanternRequest = Remotes.get("LanternRequest")
lanternRequest.OnServerEvent:Connect(function(player, on)
	if type(on) ~= "boolean" then
		return
	end
	RunStateService.setLantern(player, on)
	-- Echo the authoritative result: setLantern can refuse (no charge, knocked
	-- down), and the client must not keep showing a light the server denied.
	lanternRequest:FireClient(player, RunStateService.isLanternOn(player))
end)

Remotes.get("ReviveRequest").OnServerEvent:Connect(function(player, targetUserId)
	if type(targetUserId) ~= "number" then
		return
	end
	if targetUserId == player.UserId then
		-- Solo Anchor Revive: paid for with a full lantern charge.
		RunStateService.anchorRevive(player)
	else
		-- Teammate revive is a hold the server advances while they stay close.
		local target = Players:GetPlayerByUserId(targetUserId)
		if target then
			RunStateService.beginRevive(player, target)
		end
	end
end)

-- Payouts land in the profile, not in a client-sent number.
LootService.onExtracted = function(player, payout)
	PlayerDataService.grantPayout(player, payout)
	PlayerDataService.save(player)
end

-- A finished run used to leave the world dead: you could still walk around, but
-- the lantern refused to light, loot refused to be picked up and nothing said
-- why, because every service checks `run.finished` and silently returns. Chris
-- hit exactly that - "pressing F still doesn't turn my lantern on, it flickers",
-- then "same thing with E, nothing works".
--
-- So the expedition resets instead. This is deliberately not a Lanternhold flow:
-- the hub does not exist yet and inventing one is a design decision, not a bug
-- fix. The crew simply goes back out, with the mall rebuilt and the robots
-- cleared, which is the smallest thing that stops the world being a dead end.
local RESTART_DELAY_SECONDS = 6

RunStateService.onRunEnded = function(_outcome: string)
	task.delay(RESTART_DELAY_SECONDS, function()
		-- Order matters: clear the old robots before the rebuild re-tags the
		-- spawn points, or the watcher adds a second set on top of the first.
		EnemyService.clearSweepers()
		FloodedMallBuilder.Build()
		RunStateService.start()
	end)
end

Players.PlayerAdded:Connect(RunStateService.addPlayer)
Players.PlayerRemoving:Connect(RunStateService.removePlayer)

RunStateService.start()
RunStateService.start_tick()

print("[LastLightkeepers] server up")
