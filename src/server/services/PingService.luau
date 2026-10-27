--!strict
-- Team pings: a quick directional callout, never a client-authoritative
-- teleport marker. The server makes sure it is near the caller and rate-limits
-- it before showing it to the crew.

local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))
local RunStateService = require(script.Parent:WaitForChild("RunStateService"))

local PingService = {}
local lastPing: { [number]: number } = {}
local MAX_RANGE = 80
local COOLDOWN = 1.2

function PingService.init()
	Remotes.get("PingRequest").OnServerEvent:Connect(function(player, position)
		if typeof(position) ~= "Vector3" then
			return
		end
		local run = RunStateService.current()
		local state = run and run.lightkeepers[player.UserId]
		local character = player.Character
		local root = character and character:FindFirstChild("HumanoidRootPart")
		if
			not run
			or run.finished
			or not state
			or state.knockedDown
			or not root
			or not root:IsA("BasePart")
		then
			return
		end
		if (root.Position - position).Magnitude > MAX_RANGE then
			return
		end
		local now = os.clock()
		if now - (lastPing[player.UserId] or -math.huge) < COOLDOWN then
			return
		end
		lastPing[player.UserId] = now
		for _, teammate in ipairs(Players:GetPlayers()) do
			Remotes.get("PingSignal")
				:FireClient(teammate, { position = position, caller = player.DisplayName })
		end
	end)
end

return PingService
