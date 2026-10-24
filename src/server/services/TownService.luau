--!strict
-- The town's restoration total: one number, shared by every server.
--
-- Chris, 2026-10-24: "The landmarks are the whole town coming back - not one
-- server's temporary room. Use one atomic global crate total for the
-- experience, then every Lanternhold reads it and shows the same restoration
-- stage. Per-server would make the Bell Tower randomly finished for one group
-- and gone for the next, which kills the premise."
--
-- So this is deliberately NOT a per-player store. `PlayerDataService` already
-- records each keeper's own crates in `profile.contributions` and that keeps
-- working untouched - it is the roll, the "credited by name, never removable"
-- part. This is the other half: how far the town itself has come, which is one
-- figure for the whole experience.
--
-- WHAT THIS DOES NOT DO, ON PURPOSE
--
-- It does not know what a landmark is and it has no thresholds in it. Chris:
-- "Do not build landmark thresholds yet. Crate totals need a deliberate shared
-- persistent store first; otherwise the town resets every session." This is
-- that store. The 10 / 25 / 50 logic comes after it, against a number that is
-- finally real.
--
-- WHY A DELTA AND NOT A WRITE
--
-- Two servers handing in crates at the same moment must not clobber each
-- other, so every write is an `UpdateAsync` that ADDS to whatever is already
-- there. Read-then-write would lose one of the two. That is the "atomic" part
-- and it is the whole reason this module exists rather than a counter in
-- HubService.
--
-- WHY IT BATCHES
--
-- DataStore budgets are per key, and the town total is a single key that every
-- server writes to. Handing in crates is a player action in a co-op game, so a
-- busy evening could easily out-run the write budget. Contributions are added
-- to a pending figure immediately - so the hall reads correctly at once - and
-- flushed to the store on a timer and at shutdown.

local DataStoreService = game:GetService("DataStoreService")

local STORE_NAME = "TownRestoration_v1"
local KEY = "CratesDelivered"
local FLUSH_SECONDS = 15
local REFRESH_SECONDS = 60
local WRITE_ATTEMPTS = 3

local store: GlobalDataStore? = nil
do
	local ok, result = pcall(function()
		return DataStoreService:GetDataStore(STORE_NAME)
	end)
	if ok then
		store = result
	else
		-- Unpublished places reject DataStore outright. Say so once, the way
		-- PlayerDataService does, and keep the hall playable.
		warn(`[TownService] town total unavailable: {result}`)
	end
end

-- `banked` is what the store last told us. `pending` is what this server has
-- taken in and not yet flushed. The town's total is the sum, so a crate counts
-- the moment it is handed over rather than at the next flush.
local banked = 0
local pending = 0
local everRead = false

local TownService = {}

--- False when the total cannot be trusted: no store, or we have never managed
--- a clean read. Callers should show nothing rather than show zero - "the town
--- has done nothing" is a worse lie than "we do not know yet".
function TownService.available(): boolean
	return store ~= nil and everRead
end

--- Crates delivered to the town, everywhere, ever.
function TownService.total(): number
	return banked + pending
end

local function refresh(): boolean
	if not store then
		return false
	end
	local ok, result = pcall(function()
		return (store :: GlobalDataStore):GetAsync(KEY)
	end)
	if not ok then
		warn(`[TownService] read failed: {result}`)
		return false
	end
	banked = if type(result) == "number" then result else 0
	everRead = true
	return true
end

--- Push `pending` into the store as a delta. Returns the crates that could not
--- be written, which stay pending for the next attempt.
local function flush(): number
	if not store or pending <= 0 then
		return pending
	end
	local delta = pending
	for attempt = 1, WRITE_ATTEMPTS do
		local ok, result = pcall(function()
			return (store :: GlobalDataStore):UpdateAsync(KEY, function(current)
				return (if type(current) == "number" then current else 0) + delta
			end)
		end)
		if ok then
			banked = if type(result) == "number" then result else banked + delta
			everRead = true
			pending -= delta
			return pending
		end
		warn(`[TownService] write attempt {attempt} failed: {result}`)
		if attempt < WRITE_ATTEMPTS then
			task.wait(attempt * 2)
		end
	end
	-- Keep them pending. They are not lost from the player's own roll either
	-- way: HubService has already credited the keeper's profile by name.
	return pending
end

--- Hand `count` crates to the town. Counts immediately; reaches the store on
--- the next flush.
function TownService.contribute(count: number)
	if count <= 0 then
		return
	end
	pending += count
end

function TownService.init()
	refresh()

	task.spawn(function()
		local sinceRefresh = 0
		while true do
			task.wait(FLUSH_SECONDS)
			flush()
			sinceRefresh += FLUSH_SECONDS
			if sinceRefresh >= REFRESH_SECONDS then
				sinceRefresh = 0
				-- Re-read so this server sees other servers' crates. Without
				-- this the hall would only ever show its own contribution.
				refresh()
			end
		end
	end)

	game:BindToClose(function()
		flush()
	end)
end

return TownService
