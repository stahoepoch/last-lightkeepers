--!strict
-- Server-owned Lanternhold departure board. Map builders provide tagged
-- Departure / ReturnPoint parts; this module never guesses a map position.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))

local ExpeditionService = {}
local routes = {
	Galleria = { title = "The Galleria", subtitle = "Flooded retail district", danger = "High", available = true },
}
local selected = "Galleria"
local launching = false
local raidActive = false
local token = 0
local departureEndsAt: number? = nil

local function snapshot(message: string?)
	return {
		selected = selected,
		routes = routes,
		launching = launching,
		raidActive = raidActive,
		departureEndsAt = departureEndsAt,
		message = message or "Choose a raid destination.",
	}
end
local function send(player: Player, message: string?)
	Remotes.get("ExpeditionState"):FireClient(player, snapshot(message))
end
local function broadcast(message: string?)
	for _, player in ipairs(Players:GetPlayers()) do send(player, message) end
end
local function taggedPart(tag: string): BasePart?
	for _, instance in ipairs(CollectionService:GetTagged(tag)) do
		if instance:IsA("BasePart") then return instance end
	end
	return nil
end
function ExpeditionService.returnPlayerToHub(player: Player)
	local point, character = taggedPart("ReturnPoint"), player.Character
	if point and character then character:PivotTo(point.CFrame * CFrame.new(0, 5, 0)) end
end
function ExpeditionService.endRaid()
	raidActive, launching, departureEndsAt = false, false, nil
end
ExpeditionService.onLaunch = function(_route: string) end

local function launch()
	if not launching then return end
	launching = false
	departureEndsAt = nil
	raidActive = true
	broadcast("The road is open.")
	ExpeditionService.onLaunch(selected)
end
local function request(player: Player, action: any, route: any)
	if launching then return end
	if action == "select" and typeof(route) == "string" and routes[route] and routes[route].available then
		selected = route
		broadcast(`Route selected: {routes[selected].title}`)
	elseif action == "launch" then
		launching = true
		departureEndsAt = workspace:GetServerTimeNow() + 5
		token += 1
		local mine = token
		broadcast("Leaving Lanternhold in 5 seconds.")
		task.delay(5, function() if launching and token == mine then launch() end end)
	end
end
function ExpeditionService.open(player: Player) send(player) end
function ExpeditionService.init()
	Remotes.get("ExpeditionRequest").OnServerEvent:Connect(request)
	local departure = taggedPart("Departure")
	if departure then
		local prompt = departure:FindFirstChild("ExpeditionPrompt") :: ProximityPrompt?
		if not prompt then
			prompt = Instance.new("ProximityPrompt")
			prompt.Name, prompt.ActionText, prompt.ObjectText = "ExpeditionPrompt", "Choose raid", "The road board"
			prompt.HoldDuration, prompt.MaxActivationDistance, prompt.RequiresLineOfSight = 0.3, 14, false
			prompt.Parent = departure
		end
		prompt.Triggered:Connect(ExpeditionService.open)
	end
end
return ExpeditionService
