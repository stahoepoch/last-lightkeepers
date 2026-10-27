--!strict
-- The solo Anchor Revive keypress.
--
-- Teammate revives are ProximityPrompts on the downed player's character,
-- created and handled server-side. This exists because you cannot walk up to
-- yourself: a solo Lightkeeper needs a key.
--
-- The server re-checks everything (one charge per run, a full lantern, actually
-- downed), so this only expresses intent.

local ContextActionService = game:GetService("ContextActionService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))
local Players = game:GetService("Players")

local player = Players.LocalPlayer
local reviveRequest = Remotes.get("ReviveRequest")

local downed = false
local anchorAvailable = true

Remotes.get("RunSnapshot").OnClientEvent:Connect(function(snapshot)
	downed = snapshot.yourKnockedDown == true
	-- A full charge is the price. Showing the prompt when it cannot be paid
	-- would read as a bug rather than a cost.
	--
	-- Ask the server rather than re-deriving the price here: this copy knew
	-- nothing about anchorRevivesLeft, so a player out of anchors still got a
	-- live key that silently did nothing. The HUD explains the refusal.
	anchorAvailable = snapshot.yourCanAnchorRevive == true
end)

ContextActionService:BindAction("AnchorRevive", function(_, state)
	if state ~= Enum.UserInputState.Begin then
		return Enum.ContextActionResult.Pass
	end
	if downed and anchorAvailable then
		reviveRequest:FireServer()
	end
	return Enum.ContextActionResult.Sink
end, true, Enum.KeyCode.R, Enum.KeyCode.ButtonX)

ContextActionService:SetTitle("AnchorRevive", "Anchor")
