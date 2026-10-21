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
			if math.abs(d.X) < car.Size.X / 2 + 1
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
		local low = car:GetAttribute("LowY")
		local high = car:GetAttribute("HighY")
		if type(low) ~= "number" or type(high) ~= "number" then
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

		local busy = false
		prompt.Triggered:Connect(function()
			if busy then
				return
			end
			busy = true
			-- Whichever end it is not at.
			local to = if math.abs(car.Position.Y - low) < math.abs(car.Position.Y - high) then high else low
			prompt.Enabled = false
			travel(car, to)
			prompt.Enabled = true
			busy = false
		end)
	end
end

return LiftService
