--!strict
-- Remote accessors. The server creates them; both sides resolve them by name.
--
-- Every remote here is a *request*. The client never sends a reward, a loot
-- amount, or a Storm Charge value - it asks, and the server decides. Treat any
-- payload arriving from a client as hostile input.

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local RunService = game:GetService("RunService")

local FOLDER_NAME = "Remotes"

local EVENTS = {
	"LanternRequest", -- (on: boolean)
	"MovementRequest", -- named walk, sprint, or crouch mode
	"PingRequest", -- (worldPosition: Vector3)
	"PingSignal", -- server -> client, teammate world marker
	-- NOT LISTENED TO. Pickup and extraction run through InteractionService's
	-- ProximityPrompts, whose hold IS the mechanic - a relic is 2.5 seconds of
	-- standing exposed. A remote straight to the handler skipped that, so the
	-- listeners were removed; see LootService.init. Firing these does nothing.
	-- If a client path ever wants them back, the hold has to be enforced on
	-- the server rather than assumed.
	"LootRequest", -- (lootInstance: Instance)
	"ExtractionRequest", -- (extractionId: string)
	"ReviveRequest", -- solo Anchor Revive; no client-selected target
	"RunSnapshot", -- server -> client, HUD state
	"RunEnded", -- server -> client, outcome summary
	"ExpeditionRequest", -- client -> server, hub board action
	"ExpeditionState", -- server -> client, destination board state
	"DownedSignal", -- server -> client, teammate flare / distance alert
	"ExtractionState", -- server -> client, active holdout countdown
}

local Remotes = {}

local function folder(): Instance
	if RunService:IsServer() then
		local existing = ReplicatedStorage:FindFirstChild(FOLDER_NAME)
		if existing then
			return existing
		end
		local created = Instance.new("Folder")
		created.Name = FOLDER_NAME
		created.Parent = ReplicatedStorage
		return created
	end
	return ReplicatedStorage:WaitForChild(FOLDER_NAME)
end

--- Server-only. Creates every remote once, at startup.
function Remotes.init()
	assert(RunService:IsServer(), "Remotes.init is server-only")
	local parent = folder()
	for _, name in ipairs(EVENTS) do
		if not parent:FindFirstChild(name) then
			local event = Instance.new("RemoteEvent")
			event.Name = name
			event.Parent = parent
		end
	end
end

function Remotes.get(name: string): RemoteEvent
	local parent = folder()
	if RunService:IsServer() then
		local event = parent:FindFirstChild(name)
		return (
			assert(event, `remote "{name}" was not created; call Remotes.init first`) :: any
		) :: RemoteEvent
	end
	return parent:WaitForChild(name) :: RemoteEvent
end

return Remotes
