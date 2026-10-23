--!strict
-- The glass lift in the Galleria's court.
--
-- Chris: "the elevator does nothing." Two reasons, and the first was not code:
-- the shaft was glazed on all four sides, so there was no way into the car.
-- The map opens one side now and tags the car with the two heights it travels
-- between; this drives it.
--
-- It is deliberately slow. The design intent for the mall is that a fall
-- between floors is lethal and the stairs are the way up, so a lift that beats
-- an escalator would undo that. This is scenery you can ride, not a shortcut.
--
-- Riders are carried by moving their characters with the car each step rather
-- than by standing them on a moving anchored part, which in Roblox slides them
-- off on the way down and leaves them behind on the way up.

local CollectionService = game:GetService("CollectionService")
local Players = game:GetService("Players")
local RunService = game:GetService("RunService")

local TAG = "Lift"
local GATE_TAG = "LiftGate"
local TRAVEL_SECONDS = 5
local REACH = 14

local LiftService = {}

--- Everyone standing on the car when it sets off. Taken once, at departure:
--- joining halfway is not a thing a lift does.
local function ridersOn(car: BasePart): { Model }
	local riders: { Model } = {}
	local top = car.Position.Y + car.Size.Y / 2
	for _, player in ipairs(Players:GetPlayers()) do
		local character = player.Character
		local root = character and character:FindFirstChild("HumanoidRootPart")
		if root and root:IsA("BasePart") then
			local d = root.Position - car.Position
			local feet = root.Position.Y - 3
			if
				math.abs(d.X) < car.Size.X / 2 + 1
				and math.abs(d.Z) < car.Size.Z / 2 + 1
				and feet > top - 2
				and feet < top + 7
			then
				table.insert(riders, character :: Model)
			end
		end
	end
	return riders
end

--- The heights this car serves, lowest first. A `Stops` attribute carries a
--- comma separated list, because Lanternhold's lift calls at four floors and
--- the Galleria's at two; a pair of attributes only ever described the second.
local function stopsOf(car: BasePart): { number }
	local raw = car:GetAttribute("Stops")
	local out: { number } = {}
	if type(raw) == "string" then
		for piece in string.gmatch(raw, "[^,%s]+") do
			local n = tonumber(piece)
			if n then
				table.insert(out, n)
			end
		end
	else
		local low, high = car:GetAttribute("LowY"), car:GetAttribute("HighY")
		if type(low) == "number" and type(high) == "number" then
			out = { low, high }
		end
	end
	table.sort(out)
	return out
end

--- The next floor in the direction of travel, turning round at either end.
local function nextStop(stops: { number }, at: number, rising: boolean): (number, boolean)
	local nearest, index = math.huge, 1
	for i, y in ipairs(stops) do
		if math.abs(y - at) < nearest then
			nearest, index = math.abs(y - at), i
		end
	end
	if rising and index < #stops then
		return stops[index + 1], true
	elseif rising then
		return stops[index - 1] or stops[index], false
	elseif index > 1 then
		return stops[index - 1], false
	end
	return stops[index + 1] or stops[index], true
end

--- The landing doors belonging to this car: the ones standing in its own
--- shaft. Lanternhold's lift has four, the Galleria's has none, and matching
--- on the shaft footprint keeps the two from driving each other's.
local function gatesOf(car: BasePart): { BasePart }
	local out: { BasePart } = {}
	for _, g in ipairs(CollectionService:GetTagged(GATE_TAG)) do
		if g:IsA("BasePart") and g:GetAttribute("Stop") ~= nil then
			local d = g.Position - car.Position
			if math.abs(d.X) < car.Size.X and math.abs(d.Z) < car.Size.Z + 12 then
				table.insert(out, g)
			end
		end
	end
	return out
end

--- Open the door at the floor the car is standing at and shut the rest. An
--- open shaft with a doorway onto it is the same fall Chris already found
--- once by stepping out of this lift, so the default is shut.
local function setGates(gates: { BasePart }, carY: number, moving: boolean)
	for _, g in ipairs(gates) do
		local stop = g:GetAttribute("Stop")
		local here = type(stop) == "number" and not moving and math.abs(carY - stop) < 1.5
		-- CanQuery too, not just CanCollide: an open door that still answers
		-- raycasts is invisible and walk-through but the camera still shoves
		-- off it, which is how half this building's "wonky camera" started.
		g.CanCollide, g.CanQuery = not here, not here
		g.Transparency = here and 1 or 0
	end
end

local function travel(car: BasePart, to: number)
	local from = car.Position.Y
	local riders = ridersOn(car)
	local elapsed = 0
	while elapsed < TRAVEL_SECONDS do
		local dt = RunService.Heartbeat:Wait()
		elapsed = math.min(TRAVEL_SECONDS, elapsed + dt)
		-- Ease in and out, so nobody is thrown off at either end.
		local t = elapsed / TRAVEL_SECONDS
		local eased = t * t * (3 - 2 * t)
		local y = from + (to - from) * eased
		local delta = y - car.Position.Y
		car.CFrame = CFrame.new(car.Position.X, y, car.Position.Z)
		for _, character in ipairs(riders) do
			if character.Parent then
				character:PivotTo(character:GetPivot() + Vector3.new(0, delta, 0))
			end
		end
	end
end

function LiftService.init()
	for _, car in ipairs(CollectionService:GetTagged(TAG)) do
		if not car:IsA("BasePart") then
			continue
		end
		local stops = stopsOf(car)
		if #stops < 2 then
			continue
		end

		local prompt = car:FindFirstChildOfClass("ProximityPrompt")
		if not prompt then
			prompt = Instance.new("ProximityPrompt")
			prompt.ActionText = "Call the lift"
			prompt.ObjectText = "Court lift"
			prompt.HoldDuration = 0.3
			prompt.MaxActivationDistance = REACH
			prompt.RequiresLineOfSight = false
			prompt.Parent = car
		end

		local gates = gatesOf(car)
		setGates(gates, car.Position.Y, false)

		local busy, rising = false, true
		prompt.Triggered:Connect(function()
			if busy then
				return
			end
			busy = true
			local to, nowRising = nextStop(stops, car.Position.Y, rising)
			rising = nowRising
			prompt.Enabled = false
			setGates(gates, car.Position.Y, true)
			travel(car, to)
			setGates(gates, car.Position.Y, false)
			prompt.Enabled = true
			busy = false
		end)
	end
end

return LiftService
