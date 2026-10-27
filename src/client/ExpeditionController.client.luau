--!strict
local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local RunService = game:GetService("RunService")
local Remotes = require(ReplicatedStorage:WaitForChild("Shared"):WaitForChild("Remotes"))
local gui = Instance.new("ScreenGui")
gui.Name, gui.ResetOnSpawn, gui.Enabled, gui.Parent =
	"ExpeditionBoard", false, false, Players.LocalPlayer:WaitForChild("PlayerGui")
local panel = Instance.new("Frame")
panel.AnchorPoint, panel.Position, panel.Size =
	Vector2.new(0.5, 0.5), UDim2.fromScale(0.5, 0.5), UDim2.fromOffset(520, 300)
panel.BackgroundColor3, panel.BorderColor3, panel.BorderSizePixel, panel.Parent =
	Color3.fromRGB(31, 29, 29), Color3.fromRGB(191, 155, 91), 2, gui
local function text(name: string, position: UDim2, size: UDim2, font: Enum.Font, textSize: number)
	local label = Instance.new("TextLabel")
	label.Name, label.Position, label.Size, label.BackgroundTransparency = name, position, size, 1
	label.Font, label.TextSize, label.TextColor3, label.TextXAlignment, label.TextWrapped, label.Parent =
		font, textSize, Color3.fromRGB(243, 225, 188), Enum.TextXAlignment.Left, true, panel
	return label
end
local title =
	text("Title", UDim2.fromOffset(28, 20), UDim2.fromOffset(464, 36), Enum.Font.GothamBold, 27)
title.Text = "THE ROAD BOARD"
local route = Instance.new("TextButton")
route.Position, route.Size, route.BackgroundColor3, route.BorderColor3, route.Parent =
	UDim2.fromOffset(28, 76),
	UDim2.fromOffset(464, 110),
	Color3.fromRGB(56, 49, 42),
	Color3.fromRGB(191, 155, 91),
	panel
route.Font, route.TextSize, route.TextColor3, route.TextXAlignment, route.TextYAlignment =
	Enum.Font.GothamBold,
	21,
	Color3.fromRGB(255, 222, 161),
	Enum.TextXAlignment.Left,
	Enum.TextYAlignment.Top
local launch = Instance.new("TextButton")
launch.Position, launch.Size, launch.BackgroundColor3, launch.TextColor3, launch.Font, launch.TextSize, launch.Parent =
	UDim2.fromOffset(28, 215),
	UDim2.fromOffset(280, 56),
	Color3.fromRGB(191, 155, 91),
	Color3.fromRGB(28, 25, 22),
	Enum.Font.GothamBold,
	18,
	panel
local status = text(
	"Status",
	UDim2.fromOffset(325, 215),
	UDim2.fromOffset(165, 56),
	Enum.Font.GothamMedium,
	14
)
local selected = "Galleria"
local departureEndsAt: number? = nil
route.Activated:Connect(function()
	Remotes.get("ExpeditionRequest"):FireServer("select", selected)
end)
launch.Activated:Connect(function()
	Remotes.get("ExpeditionRequest"):FireServer("launch")
end)
Remotes.get("ExpeditionState").OnClientEvent:Connect(function(state)
	selected = state.selected or selected
	local routeData = state.routes and state.routes[selected]
	if routeData then
		local lines = {
			` {routeData.title}`,
			` {routeData.subtitle}  •  {routeData.danger} danger`,
		}
		-- What the board runs on, straight from the server. It is on the board
		-- before anyone presses launch on purpose: an unpowered departure that
		-- fails is only fair if the board said in advance that it might.
		local power = state.power
		if type(power) == "table" then
			table.insert(lines, "")
			table.insert(lines, ` {tostring(power.line)}`)
			if type(power.exits) == "table" and #power.exits > 0 then
				table.insert(lines, "")
				table.insert(lines, " WAYS OUT")
				for _, exit in ipairs(power.exits) do
					table.insert(lines, `   {tostring(exit)}`)
				end
			end
		end
		route.Text = table.concat(lines, "\n")
	end
	status.Text = tostring(state.message or "")
	departureEndsAt = if typeof(state.departureEndsAt) == "number"
		then state.departureEndsAt
		else nil
	if state.raidActive then
		gui.Enabled = false
		return
	end
	launch.Text = if state.launching then "DEPARTING" else "START RAID"
	launch.Active, launch.AutoButtonColor, gui.Enabled =
		not state.launching, not state.launching, true
end)

-- The board receives a single authoritative server timestamp, then counts down
-- locally. This is not a decorative five-second pause: players can see exactly
-- when the road opens and the text cannot get stuck on "forming".
RunService.RenderStepped:Connect(function()
	if gui.Enabled and departureEndsAt then
		local remaining = math.max(0, departureEndsAt - workspace:GetServerTimeNow())
		status.Text = string.format("Leaving Lanternhold in %d…", math.ceil(remaining))
		if remaining <= 0 then
			departureEndsAt = nil
		end
	end
end)
