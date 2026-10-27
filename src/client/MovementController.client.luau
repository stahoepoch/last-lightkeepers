--!strict
-- Movement requests are intentionally tiny and server-checked. Shift / left
-- stick sprint and C / B crouch; releasing either returns to a normal walk.

local ContextActionService = game:GetService("ContextActionService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Remotes = require(ReplicatedStorage:WaitForChild("Shared"):WaitForChild("Remotes"))
local request = Remotes.get("MovementRequest")

local heldSprint = false
local heldCrouch = false

local function send()
	request:FireServer(if heldCrouch then "crouch" elseif heldSprint then "sprint" else "walk")
end

ContextActionService:BindAction("Sprint", function(_, state)
	heldSprint = state == Enum.UserInputState.Begin
	send()
	return Enum.ContextActionResult.Sink
end, false, Enum.KeyCode.LeftShift, Enum.KeyCode.ButtonL3)

ContextActionService:BindAction("Crouch", function(_, state)
	heldCrouch = state == Enum.UserInputState.Begin
	send()
	return Enum.ContextActionResult.Sink
end, false, Enum.KeyCode.C, Enum.KeyCode.ButtonB)
