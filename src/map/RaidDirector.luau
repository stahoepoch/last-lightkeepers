--!strict
-- One map-side wrinkle per generated raid. This intentionally owns presentation
-- and accessible geometry only; enemy pressure, loot values, and run state
-- remain in the gameplay lane.

local CollectionService = game:GetService("CollectionService")

local RaidDirector = {}

local function candidates(mall: Model, prefix: string): { BasePart }
	local found = {}
	for _, instance in ipairs(mall:GetDescendants()) do
		if instance:IsA("BasePart") and string.sub(instance.Name, 1, #prefix) == prefix then
			table.insert(found, instance)
		end
	end
	return found
end

local function mark(mall: Model, eventId: string, at: BasePart)
	local marker = Instance.new("Part")
	marker.Name = `RaidEvent_{eventId}`
	marker.Size = Vector3.new(2, 2, 2)
	marker.CFrame = at.CFrame * CFrame.new(0, 3, 0)
	marker.Anchored, marker.CanCollide, marker.CanTouch, marker.Transparency = true, false, false, 1
	marker:SetAttribute("EventId", eventId)
	marker.Parent = mall:FindFirstChild("Gameplay") or mall
	CollectionService:AddTag(marker, "RaidEvent")
	mall:SetAttribute("RaidEvent", eventId)
end

local function blackoutWing(mall: Model, random: Random): boolean
	local arm = tostring(random:NextInteger(1, 2))
	local changed: BasePart? = nil
	for _, fixture in ipairs(candidates(mall, `Downlight{arm}_`)) do
		local light = fixture:FindFirstChildOfClass("PointLight")
		if light then
			light.Enabled = false
			changed = fixture
		end
	end
	if changed then
		mark(mall, `BlackoutWing{arm}`, changed)
		return true
	end
	return false
end

local function openShutter(mall: Model, random: Random): boolean
	local shutters = candidates(mall, "Shutter_")
	if #shutters == 0 then
		return false
	end
	local shutter = shutters[random:NextInteger(1, #shutters)]
	shutter.CanCollide = false
	shutter.Transparency = 0.82
	shutter:SetAttribute("OpenedByRaidEvent", true)
	mark(mall, "OpenedShutter", shutter)
	return true
end

local function emergencyBeacon(mall: Model, random: Random): boolean
	local directories = candidates(mall, "Directory")
	if #directories == 0 then
		return false
	end
	local directory = directories[random:NextInteger(1, #directories)]
	local beacon = Instance.new("PointLight")
	beacon.Name = "EmergencyBeacon"
	beacon.Color = Color3.fromRGB(210, 112, 60)
	beacon.Brightness, beacon.Range, beacon.Shadows = 0.45, 24, false
	beacon.Parent = directory
	mark(mall, "EmergencyBeacon", directory)
	return true
end

function RaidDirector.Apply(mall: Model)
	local random = Random.new()
	local events = { blackoutWing, openShutter, emergencyBeacon }
	local first = random:NextInteger(1, #events)
	for offset = 0, #events - 1 do
		if events[((first + offset - 1) % #events) + 1](mall, random) then
			return
		end
	end
	warn("[RaidDirector] no eligible map-side event")
end

return RaidDirector
