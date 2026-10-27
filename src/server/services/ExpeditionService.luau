--!strict
-- Server-owned Lanternhold departure board. Map builders provide tagged
-- Departure / ReturnPoint parts; this module never guesses a map position.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))

local RestorationService = require(script.Parent:WaitForChild("RestorationService"))

local ExpeditionService = {}

-- The board's own luck, not the run's. One generator rolls one false start.
local rng = Random.new()
local routes = {
	Galleria = {
		title = "The Galleria",
		subtitle = "Flooded retail district",
		danger = "High",
		available = true,
	},
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
		-- What the board runs on, and what that costs. Sent on every snapshot
		-- so the trade is on the board before anyone presses launch, which is
		-- the whole reason rung one's reward reads as a reward.
		power = RestorationService.departureLines(),
		message = message or "Choose a raid destination.",
	}
end
local function send(player: Player, message: string?)
	Remotes.get("ExpeditionState"):FireClient(player, snapshot(message))
end
local function broadcast(message: string?)
	for _, player in ipairs(Players:GetPlayers()) do
		send(player, message)
	end
end
local function taggedPart(tag: string): BasePart?
	for _, instance in ipairs(CollectionService:GetTagged(tag)) do
		if instance:IsA("BasePart") then
			return instance
		end
	end
	return nil
end
function ExpeditionService.returnPlayerToHub(player: Player)
	local point, character = taggedPart("ReturnPoint"), player.Character
	if point and character then
		character:PivotTo(point.CFrame * CFrame.new(0, 5, 0))
	end
end
function ExpeditionService.endRaid()
	raidActive, launching, departureEndsAt = false, false, nil
end
ExpeditionService.onLaunch = function(_route: string) end

local function launch()
	if not launching then
		return
	end
	launching = false
	departureEndsAt = nil
	raidActive = true
	broadcast("The road is open.")
	ExpeditionService.onLaunch(selected)
end
local function request(player: Player, action: any, route: any)
	if launching then
		return
	end
	if
		action == "select"
		and typeof(route) == "string"
		and routes[route]
		and routes[route].available
	then
		selected = route
		broadcast(`Route selected: {routes[selected].title}`)
	elseif action == "launch" then
		-- An unpowered board winds up slowly and sometimes dies instead. This
		-- is rung one's reward stated as a cost: restoring the generator buys
		-- five seconds and certainty. RestorationService owns both numbers, so
		-- nothing here knows which rung powers the board.
		local seconds, falseStart = RestorationService.departureTiming()
		launching = true
		departureEndsAt = workspace:GetServerTimeNow() + seconds
		token += 1
		local mine = token
		broadcast(`Leaving Lanternhold in {seconds} seconds.`)
		task.delay(seconds, function()
			if not launching or token ~= mine then
				return
			end
			-- Rolled at the end rather than at the press, so a failure costs
			-- the wait. A board that refused instantly would cost nothing and
			-- teach nobody to want the generator fixed.
			if falseStart > 0 and rng:NextNumber() < falseStart then
				launching = false
				departureEndsAt = nil
				broadcast("The board goes dark. Nothing leaves. Try it again.")
				return
			end
			launch()
		end)
	end
end
function ExpeditionService.open(player: Player)
	send(player)
end
function ExpeditionService.init()
	Remotes.get("ExpeditionRequest").OnServerEvent:Connect(request)
	local departure = taggedPart("Departure")
	if departure then
		local prompt = departure:FindFirstChild("ExpeditionPrompt") :: ProximityPrompt?
		if not prompt then
			prompt = Instance.new("ProximityPrompt")
			prompt.Name, prompt.ActionText, prompt.ObjectText =
				"ExpeditionPrompt", "Choose raid", "The road board"
			prompt.HoldDuration, prompt.MaxActivationDistance, prompt.RequiresLineOfSight =
				0.3, 14, false
			prompt.Parent = departure
		end
		prompt.Triggered:Connect(ExpeditionService.open)
	end
end
return ExpeditionService
