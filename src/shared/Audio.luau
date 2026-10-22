--!strict
-- Every sound the game wants, in one table, with the one thing none of them
-- has yet: an asset id.
--
-- Chris, walking the mall: "where is the music and more sound effects?" The
-- honest answer is that there is no music system at all - the only Sound in
-- the project is the enemy telegraph cue in the four rigs - and that the
-- thing blocking sound is not code. It is an upload, and only Chris can do
-- that. assets/audio holds five masters we made and own, and not one of them
-- has ever been uploaded, so none of them has an id. The single id the game
-- does reference, 9118959688, is a PLACEHOLDER and someone else's ("Servo
-- Motor 7", Pro Sound
-- Effects). It is flagged in assets/MANIFEST.md as must-not-ship and is
-- currently failing to load anyway.
--
-- So this is the system built ready. Every cue is declared with an empty id.
-- `Audio.make` returns nil for a cue with no id and the callers skip it, so
-- the whole thing is silent and harmless today and comes alive the moment the
-- ids are filled in - one edit, in one table, in this file.
--
-- THE RULE, from docs/ASSETS.md: an id that appears here must also appear in
-- assets/MANIFEST.md with the master it came from. Do not paste an id you
-- found by searching the Roblox library. Verifying that an asset exists is
-- not owning it - it lives on someone else's licence and cannot port to a
-- Steam build.

local Audio = {}

export type Cue = {
	id: string, -- "rbxassetid://..." once uploaded; "" until then
	master: string, -- the file in assets/ this was made from, "" if unrecorded
	kind: string, -- "loop" or "oneShot"
	volume: number,
	note: string,
}

-- Looping beds and layers. The director crossfades between these; it never
-- starts or stops them abruptly.
-- Declared as a local first: Luau has no type annotation on a field
-- assignment, so `Audio.cues: {...} = {...}` is a parse error.
local cues: { [string]: Cue } = {
	HubAmbience = {
		id = "",
		master = "",
		kind = "loop",
		volume = 0.30,
		note = "Lanternhold room tone. Warm, close, a building with people in it.",
	},
	HubMusic = {
		id = "",
		master = "",
		kind = "loop",
		volume = 0.22,
		note = "Quiet music, hub only. Stops the moment the road opens.",
	},
	RaidAmbience = {
		id = "",
		master = "",
		kind = "loop",
		volume = 0.34,
		note = "The Galleria: water moving, structure settling, far-off drips.",
	},
	RaidTension1 = {
		id = "",
		master = "",
		kind = "loop",
		volume = 0.0,
		note = "Fades in at PatrolsAlert. A layer over RaidAmbience, not a replacement.",
	},
	RaidTension2 = {
		id = "",
		master = "",
		kind = "loop",
		volume = 0.0,
		note = "Fades in at PatrolsHunt, on top of Tension1.",
	},
	WardenTheme = {
		id = "",
		master = "",
		kind = "loop",
		volume = 0.0,
		note = "Only while the Warden is up. It is the meter with a body; this is the meter with a sound.",
	},
	ExtractionHold = {
		id = "",
		master = "",
		kind = "loop",
		volume = 0.0,
		note = "Over everything while a holdout is counting down.",
	},

	-- One-shots.
	ExtractionCalled = {
		id = "",
		master = "",
		kind = "oneShot",
		volume = 0.55,
		note = "Fires once when an exit is called, for the whole crew.",
	},
	LanternOn = {
		id = "",
		master = "",
		kind = "oneShot",
		volume = 0.4,
		note = "The decision the game is named after. Should feel like a cost.",
	},
	LanternOff = { id = "", master = "", kind = "oneShot", volume = 0.35, note = "" },
	Downed = {
		id = "",
		master = "",
		kind = "oneShot",
		volume = 0.5,
		note = "You, not a teammate. Teammates have DownedSignal already.",
	},
	RunSurvived = { id = "", master = "", kind = "oneShot", volume = 0.5, note = "" },
	RunWiped = { id = "", master = "", kind = "oneShot", volume = 0.5, note = "" },

	-- Already recorded and owned, and still not uploaded. The four enemy rigs
	-- hardcode a borrowed id instead of reading these; migrating them is a
	-- one-line change per rig and belongs to whoever owns those files.
	SweeperTelegraph = {
		id = "",
		master = "assets/audio/sweeper-telegraph-v1.wav",
		kind = "oneShot",
		volume = 0.5,
		note = "OWNED MASTER, NOT UPLOADED.",
	},
	GlimmerScan = {
		id = "",
		master = "assets/audio/glimmer-scan-v1.wav",
		kind = "oneShot",
		volume = 0.5,
		note = "OWNED MASTER, NOT UPLOADED.",
	},
	StockerTelegraph = {
		id = "",
		master = "assets/audio/stocker-telegraph-v1.wav",
		kind = "oneShot",
		volume = 0.5,
		note = "OWNED MASTER, NOT UPLOADED.",
	},
	WindowWalkerWake = {
		id = "",
		master = "assets/audio/window-walker-wake-v1.wav",
		kind = "oneShot",
		volume = 0.5,
		note = "OWNED MASTER, NOT UPLOADED.",
	},
	WardenApproach = {
		id = "",
		master = "assets/audio/warden-approach-v1.wav",
		kind = "oneShot",
		volume = 0.6,
		note = "OWNED MASTER, NOT UPLOADED.",
	},
}

Audio.cues = cues

--- A Sound for a cue, or nil if it has no id yet. Callers must handle nil:
--- that is the whole mechanism by which this system stays silent and
--- harmless until the uploads happen.
function Audio.make(name: string, parent: Instance): Sound?
	local cue = cues[name]
	if not cue or cue.id == "" then
		return nil
	end
	local sound = Instance.new("Sound")
	sound.Name = name
	sound.SoundId = cue.id
	sound.Volume = cue.volume
	sound.Looped = cue.kind == "loop"
	sound.Parent = parent
	return sound
end

--- What is still owed, split by what has to happen to it.
function Audio.owed(): { needsRecording: { string }, needsUpload: { string } }
	local recording, upload = {}, {}
	for name, cue in pairs(cues) do
		if cue.id == "" then
			if cue.master == "" then
				table.insert(recording, name)
			else
				table.insert(upload, name)
			end
		end
	end
	table.sort(recording)
	table.sort(upload)
	return { needsRecording = recording, needsUpload = upload }
end

--- Printed once at startup so the gap is visible in the output rather than
--- being something you notice by the game being quiet.
function Audio.report()
	local owed = Audio.owed()
	if #owed.needsUpload == 0 and #owed.needsRecording == 0 then
		return
	end
	if #owed.needsUpload > 0 then
		warn(
			`[Audio] {#owed.needsUpload} cue(s) have a master in assets/audio and no id - these only need uploading: {table.concat(
				owed.needsUpload,
				", "
			)}`
		)
	end
	if #owed.needsRecording > 0 then
		warn(
			`[Audio] {#owed.needsRecording} cue(s) have no master yet: {table.concat(
				owed.needsRecording,
				", "
			)}`
		)
	end
end

return Audio
