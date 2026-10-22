--!strict
-- The ambience and music director.
--
-- Chris: "where is the music and more sound effects?" There was no music
-- system at all - the only Sound in the project is the telegraph cue in the
-- four enemy rigs. This is that system, built against src/shared/Audio.luau,
-- which holds every cue and the one thing none of them has yet: an id.
--
-- IT IS SILENT TODAY AND THAT IS CORRECT. `Audio.make` returns nil for a cue
-- with no id, every layer here is optional, and the whole director no-ops on
-- a cue it has no sound for. Fill the ids in Audio.luau and this comes alive
-- without a line changing here.
--
-- Layers, not tracks. The raid bed runs the whole time and the tension
-- layers fade in over it as Storm Charge crosses its thresholds, so the sound
-- rises with the meter rather than cutting between pieces. The meter is the
-- game's clock and this is meant to be the sound of it.

local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local SoundService = game:GetService("SoundService")
local TweenService = game:GetService("TweenService")

local Shared = ReplicatedStorage:WaitForChild("Shared")
local Audio = require(Shared:WaitForChild("Audio")) :: any
local Remotes = require(Shared:WaitForChild("Remotes")) :: any

local player = Players.LocalPlayer

Audio.report()

-- Everything lives under one folder in SoundService so it is obvious in the
-- explorer what this controller owns, and so nothing is parented to a
-- character that respawns out from under it.
local root = Instance.new("Folder")
root.Name = "Ambience"
root.Parent = SoundService

local layers: { [string]: Sound } = {}
for _, name in ipairs({
	"HubAmbience",
	"HubMusic",
	"RaidAmbience",
	"RaidTension1",
	"RaidTension2",
	"WardenTheme",
	"ExtractionHold",
}) do
	local sound = Audio.make(name, root)
	if sound then
		sound.Volume = 0
		sound:Play()
		layers[name] = sound
	end
end

local FADE = TweenInfo.new(1.6, Enum.EasingStyle.Sine, Enum.EasingDirection.Out)
local target: { [string]: number } = {}

--- Fade a layer toward a volume. Silent no-op if that cue has no id.
local function set(name: string, volume: number)
	local sound = layers[name]
	if not sound then
		return
	end
	if target[name] == volume then
		return
	end
	target[name] = volume
	TweenService:Create(sound, FADE, { Volume = volume }):Play()
end

local function shot(name: string)
	local sound = Audio.make(name, root)
	if not sound then
		return
	end
	sound.Ended:Once(function()
		sound:Destroy()
	end)
	sound:Play()
end

-- The loudest a cue gets, read off its declared volume so the levels live in
-- one place and this file only decides when, never how loud.
local function full(name: string): number
	local cue = Audio.cues[name]
	return if cue then math.max(cue.volume, 0.3) else 0.3
end

-- --------------------------------------------------------------- the state
local inRaid = false
local holdout = false

local function apply(snapshot: any?)
	if not inRaid then
		set("HubAmbience", full("HubAmbience"))
		set("HubMusic", full("HubMusic"))
		for _, name in ipairs({
			"RaidAmbience",
			"RaidTension1",
			"RaidTension2",
			"WardenTheme",
			"ExtractionHold",
		}) do
			set(name, 0)
		end
		return
	end

	set("HubAmbience", 0)
	set("HubMusic", 0)
	set("RaidAmbience", full("RaidAmbience"))
	set("ExtractionHold", if holdout then full("ExtractionHold") else 0)

	local threshold = snapshot and snapshot.highestThreshold or nil
	local warden = snapshot and snapshot.wardenActive == true
	-- Thresholds arrive as the id strings from data/storm-charge.json. An
	-- unknown one leaves the layers where they are rather than guessing.
	set("RaidTension1", if threshold ~= nil then full("RaidTension1") else 0)
	set(
		"RaidTension2",
		if threshold == "PatrolsHunt" or threshold == "WardenConverges"
			then full("RaidTension2")
			else 0
	)
	set("WardenTheme", if warden or threshold == "WardenConverges" then full("WardenTheme") else 0)
end

-- --------------------------------------------------------------- the wiring
local lastLantern: boolean? = nil
local lastDowned = false

Remotes.get("RunSnapshot").OnClientEvent:Connect(function(snapshot)
	inRaid = true
	apply(snapshot)

	if snapshot.yourLanternOn ~= lastLantern then
		if lastLantern ~= nil then
			shot(if snapshot.yourLanternOn then "LanternOn" else "LanternOff")
		end
		lastLantern = snapshot.yourLanternOn
	end
	if snapshot.yourKnockedDown and not lastDowned then
		shot("Downed")
	end
	lastDowned = snapshot.yourKnockedDown == true
end)

Remotes.get("ExtractionState").OnClientEvent:Connect(function(state)
	local nowActive = state and state.active == true
	if nowActive and not holdout then
		shot("ExtractionCalled")
	end
	holdout = nowActive
	apply(nil)
end)

Remotes.get("RunEnded").OnClientEvent:Connect(function(outcome)
	inRaid, holdout = false, false
	lastLantern, lastDowned = nil, false
	local survived = typeof(outcome) == "table" and outcome.outcome == "extracted"
	shot(if survived then "RunSurvived" else "RunWiped")
	apply(nil)
end)

-- The hub board only talks while you are in the hub, so it is the cleanest
-- signal that the raid is over and you are home.
Remotes.get("ExpeditionState").OnClientEvent:Connect(function()
	inRaid = false
	apply(nil)
end)

player.CharacterAdded:Connect(function()
	lastDowned = false
end)

apply(nil)
