--!strict
-- The Rojo Lighting service cannot be safely swapped during a sync, and that
-- made the visual baseline depend on whoever last changed Studio's Lighting
-- object. This one runtime owner makes every playtest begin from the approved
-- readable profile instead of inheriting a stale or accidentally black/white
-- editor setting.

local Lighting = game:GetService("Lighting")

local LightingService = {}

-- Ambient and EnvironmentDiffuseScale are ZERO, and that is not a taste call.
--
-- Both are UNCONDITIONAL fill: they light every surface whether or not a lamp
-- is on, and they ignore Brightness and ExposureCompensation entirely. At
-- 46,52,64 and 0.72, against this palette, both maps render white - and
-- because the fill ignores the lights, turning every light in the game off
-- changes nothing, which is what made this so hard to pin down.
--
-- Proven by bisection in a live session: with all six scalars at zero and
-- every light in Workspace disabled, the screen goes black. Add back only the
-- sun at 0.45, the sky at 28,34,48 and all 45 lamps, leaving these two at
-- zero, and the mall is correct - dark, with a warm pool around the keeper.
-- Chris called that build good. The same values in Edit render correctly
-- black at ClockTime 2; at 46,52,64 they render white.
--
-- tests/lighting.test.mjs guards this table, and .githooks/pre-commit refuses
-- a commit that breaks it. If these need to change, change the test and
-- DONTFUCKUPTHELIGHTS.md in the same commit.
local PROFILE = {
	Brightness = 0.45,
	ExposureCompensation = -0.05,
	Ambient = Color3.fromRGB(0, 0, 0),
	OutdoorAmbient = Color3.fromRGB(28, 34, 48),
	EnvironmentDiffuseScale = 0,
	EnvironmentSpecularScale = 0,
	ShadowSoftness = 0.35,
	ClockTime = 2,
	GlobalShadows = true,
	Technology = Enum.Technology.Future,
}

function LightingService.init()
	for property, value in pairs(PROFILE) do
		(Lighting :: any)[property] = value
	end
	-- Bloom or a colour-correction effect left enabled in the place turns the
	-- same calibrated lamps into a white-out. Disable them for play rather than
	-- deleting a designer's editor object; the source stays recoverable.
	for _, effect in ipairs(Lighting:GetChildren()) do
		if effect:IsA("PostEffect") then
			effect.Enabled = false
		end
	end
	print("[LightingService] calibrated raid profile applied")
end

return LightingService
