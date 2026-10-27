--!strict
-- Persistence: workshop materials, upgrades, cosmetics, landmark contributions.
-- Server-only. DataStoreService is never touched from a client.

local DataStoreService = game:GetService("DataStoreService")
local Players = game:GetService("Players")

local STORE_NAME = "LightkeeperProfile_v1"
local store: GlobalDataStore? = nil

do
	local ok, result = pcall(function()
		return DataStoreService:GetDataStore(STORE_NAME)
	end)
	if ok then
		store = result
	else
		-- Local, unpublished places can reject DataStore access before a profile
		-- read begins. Keep the game playable, but never write a temporary profile.
		warn(`[PlayerDataService] persistence unavailable: {result}`)
	end
end

local PlayerDataService = {}

export type Profile = {
	materials: { [string]: number },
	upgrades: { [string]: boolean },
	cosmetics: { [string]: boolean },
	contributions: { [string]: number }, -- credited by name, never removable
	-- Which rooms of this keeper's own hall have been opened. Permanent: a room
	-- never closes, so this only ever grows. The crates a room cost are not
	-- subtracted from `contributions` - that total is credit and cannot be spent
	-- away - so what a keeper has left to commit is derived from the two.
	rooms: { [string]: boolean },
	runsCompleted: number,
	-- Set when the DataStore read failed. Never persisted; it exists only to stop
	-- a blank profile overwriting a real one.
	loadFailed: boolean?,
}

local function defaultProfile(): Profile
	return {
		materials = {},
		upgrades = {},
		cosmetics = {},
		contributions = {},
		rooms = {},
		runsCompleted = 0,
	}
end

local cache: { [number]: Profile } = {}

function PlayerDataService.load(player: Player): Profile
	if cache[player.UserId] then
		return cache[player.UserId]
	end

	local profile = defaultProfile()
	if not store then
		profile.loadFailed = true
		cache[player.UserId] = profile
		return profile
	end
	local ok, result = pcall(function()
		return store:GetAsync(tostring(player.UserId))
	end)
	if ok and type(result) == "table" then
		for key, value in pairs(result) do
			profile[key] = value
		end
	elseif not ok then
		-- Never silently hand back a blank profile on a read failure: that is how
		-- players lose a workshop. Flag it and let the caller refuse to save.
		warn(`[PlayerDataService] load failed for {player.UserId}: {result}`)
		profile = defaultProfile()
		profile.loadFailed = true
		cache[player.UserId] = profile
		return profile
	end

	cache[player.UserId] = profile
	return profile
end

function PlayerDataService.save(player: Player)
	local profile = cache[player.UserId]
	if not profile then
		return
	end
	if profile.loadFailed then
		warn(`[PlayerDataService] refusing to save {player.UserId}: profile never loaded cleanly`)
		return
	end
	local ok, err = pcall(function()
		store:SetAsync(tostring(player.UserId), profile)
	end)
	if not ok then
		warn(`[PlayerDataService] save failed for {player.UserId}: {err}`)
	end
end

--- Credits a completed run's payout to the player's workshop.
function PlayerDataService.grantPayout(player: Player, payout: { [string]: number })
	local profile = PlayerDataService.load(player)
	for tier, amount in pairs(payout) do
		profile.materials[tier] = (profile.materials[tier] or 0) + amount
	end
	profile.runsCompleted += 1
end

function PlayerDataService.init()
	Players.PlayerAdded:Connect(function(player)
		PlayerDataService.load(player)
	end)
	Players.PlayerRemoving:Connect(function(player)
		PlayerDataService.save(player)
		cache[player.UserId] = nil
	end)
	game:BindToClose(function()
		for _, player in ipairs(Players:GetPlayers()) do
			PlayerDataService.save(player)
		end
	end)
end

return PlayerDataService
