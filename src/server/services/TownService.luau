--!strict
-- The town's restoration: one per keeper, not one for the world.
--
-- Chris, 2026-10-26: "Make raids solo or with your friends, not one giant
-- persistent world... Your personal Lanternhold: upgrades, relic displays,
-- personal restoration progress. Friends can visit: bring a party into the
-- host's Lanternhold, help raid, see what they've rebuilt." And why: "the
-- Well, Bell Tower, and Long Bridge are rebuilt by you and the people you
-- play with, which is much more emotionally satisfying than contributing a
-- crate to an invisible global number."
--
-- This file used to be the opposite of that. Two days ago it was one atomic
-- global key - UpdateAsync deltas, a flush timer, a cross-server re-read,
-- retries - because the landmarks were then meant to be the whole town coming
-- back at once. All of that is gone. A keeper's restoration is their own
-- persisted crate count, which `PlayerDataService` has recorded all along:
--
--   profile.contributions.RestorationCrates
--
-- So there is no store of our own here any more, no batching and no delta. It
-- also quietly closes the one thing that was still untestable: the global
-- version could not be trusted until a real published place had exercised
-- UpdateAsync against a live key, and nothing in Studio could ever do it.
-- Reading a figure the profile already carries has no such hole.
--
-- WHOSE TOWN THE HALL SHOWS
--
-- The three landmark plates are parts in the world, so they can only show one
-- town, and "bring a party into the host's Lanternhold" says which: the host's.
-- The host is the first keeper to arrive, and they stay the host for the whole
-- session even if they leave - reassigning would make the Bell Tower vanish
-- mid-visit, which is the same thing Chris objected to about per-server
-- totals, only faster.
--
-- WHO A HAND-IN CREDITS  <-- the one decision here worth overruling
--
-- Every keeper standing in the hall, each on their own town. A crew of three
-- hands in six crates and all three towns move six.
--
-- The alternative is to credit only the host, since it is their hall. I did
-- not, for two reasons. It leaves a visitor with nothing to show for a night
-- of raiding in a friend's hold, which makes visiting a favour rather than
-- something you want to do; and it only works while the host is still in the
-- server, so a hand-in after they log off would have nowhere to go. Crediting
-- the crew needs no host at all.
--
-- It does mean one crate can advance three towns. That costs nothing
-- mechanically - the landmarks are spectacle, Chris's call: "a stat bonus
-- would turn a monument into a grind target" - but it is a generosity
-- decision, not a technical one, so it is yours to reverse. One line in
-- `contribute` does it.

local Players = game:GetService("Players")

local PlayerDataService = require(script.Parent:WaitForChild("PlayerDataService"))

local CRATES = "RestorationCrates"

-- Held for the session, not re-derived: `hostPlayer` goes nil when they leave
-- but `hostName` and `hostTotal` stay, so the hall keeps showing the town the
-- party came to see.
local hostPlayer: Player? = nil
local hostName: string? = nil
local hostTotal = 0
local hostState = "unknown"

-- This session's crates per keeper, used ONLY where there is no profile to
-- read. An unpublished place has no DataStore, so every profile comes back
-- `loadFailed` and the persisted figure is permanently unavailable - which
-- would leave all three plates reading "restoration unknown" in Studio
-- forever, and the thresholds unseeable by the person who asked for them.
-- Showing the session's own tally instead, clearly labelled, is what makes
-- this testable at all. It is never mistaken for the real thing: `scope`
-- returns "offline" and the plates say so.
local session: { [number]: number } = {}

local TownService = {}

--- Returns the keeper's crates and where the figure came from: "home" from a
--- loaded profile, "offline" from this session's tally.
local function cratesOf(player: Player): (number, string)
	local profile = PlayerDataService.load(player)
	if profile.loadFailed then
		-- Never read 0 off a profile that did not load and call it theirs.
		-- "Your town is gone" is the worst thing this module could say.
		return session[player.UserId] or 0, "offline"
	end
	local contributions = profile.contributions or {}
	return contributions[CRATES] or 0, "home"
end

--- Crates this keeper has brought home. Their own town's progress.
function TownService.totalFor(player: Player): (number, string)
	return cratesOf(player)
end

local function readHost()
	if hostPlayer and hostPlayer.Parent then
		hostTotal, hostState = cratesOf(hostPlayer)
	end
end

--- The keeper whose Lanternhold this is: the first to arrive. Later arrivals
--- are visitors in their hall.
function TownService.claim(player: Player)
	if hostName then
		return
	end
	hostPlayer = player
	hostName = player.DisplayName
	readHost()
end

--- Whose town the plates are showing, for a label. nil in an empty server.
function TownService.hostName(): string?
	return hostName
end

--- True when `player` is the keeper whose hall this is.
function TownService.isHost(player: Player): boolean
	return hostPlayer ~= nil and hostPlayer == player
end

--- What the hall can honestly say about the restoration:
---
---   "empty"    nobody has arrived yet; there is no town to show
---   "home"     the host's persisted figure, read cleanly. The real thing.
---   "offline"  no profile to read - an unpublished place - so the figure is
---              this session's own tally and must be labelled as such
function TownService.scope(): string
	if not hostName then
		return "empty"
	end
	return hostState
end

--- The host's crates: what the landmark plates read.
function TownService.total(): number
	return hostTotal
end

--- Hand `count` crates to the town. Credits every keeper presently in the
--- hall, each on their own permanent roll, and saves them.
---
--- This is the only writer of `contributions` now, so the roll stays one
--- number in one place rather than being added to here and in the hand-in.
function TownService.contribute(count: number)
	if count <= 0 then
		return
	end
	for _, player in ipairs(Players:GetPlayers()) do
		local profile = PlayerDataService.load(player)
		if profile.loadFailed then
			-- Nowhere to persist it. Count it for the session so the hall is
			-- still honest about what happened in front of the player, and
			-- `scope` keeps saying "offline" so nobody reads it as saved.
			session[player.UserId] = (session[player.UserId] or 0) + count
		else
			profile.contributions = profile.contributions or {}
			profile.contributions[CRATES] = (profile.contributions[CRATES] or 0) + count
			PlayerDataService.save(player)
		end
	end
	readHost()
end

function TownService.init()
	for _, player in ipairs(Players:GetPlayers()) do
		TownService.claim(player)
	end
	Players.PlayerAdded:Connect(function(player)
		TownService.claim(player)
	end)
	Players.PlayerRemoving:Connect(function(player)
		if hostPlayer == player then
			-- The name and the figure stay. Only the live handle goes.
			hostPlayer = nil
		end
	end)
end

return TownService
