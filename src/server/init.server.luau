--!strict
-- Server entry point. Creates remotes, wires services, starts the run.

local Players = game:GetService("Players")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))

Remotes.init()

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

Players.PlayerAdded:Connect(RunStateService.addPlayer)
Players.PlayerRemoving:Connect(RunStateService.removePlayer)

RunStateService.start()
RunStateService.start_tick()

print("[LastLightkeepers] server up")
