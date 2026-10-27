--!strict
-- Server entry point. Creates remotes, wires services, starts the run.

local Players = game:GetService("Players")
local Workspace = game:GetService("Workspace")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Remotes = require(Shared:WaitForChild("Remotes"))

Remotes.init()

-- Rojo owns Workspace.Map, so each sync deliberately removes the generated
-- mall and its CollectionService tags. Build the authoritative runtime slice
-- before any service scans or watches those tags. This keeps a clean Play
-- session playable without asking a designer to run a command-bar step.
local Map = Workspace:WaitForChild("Map")

-- The Galleria is the mall now. FloodedMallBuilder used to be built here and
-- on every launch, and because it owned the only SpawnLocation, every raid
-- landed in it no matter what - Chris: "starting the raid still sends me to
-- codex's old mall". GalleriaBuilder emits its own spawn, loot, Sweeper
-- posts, relay and extractions, so the old generated mall is no longer built
-- or required.
local GalleriaBuilder = require(Map:WaitForChild("GalleriaBuilder"))

-- Lanternhold, if it is in the project. Built once and well clear of the mall:
-- the hub is where you leave from and come back to, not part of the expedition.
-- Deliberately not parented under Workspace.Map, because FloodedMallBuilder
-- clears every child of Map on the first line of its build. When it is absent,
-- every hub fixture below finds nothing and does nothing.
local LANTERNHOLD_AT = Vector3.new(-6000, 0, 0)
local lanternholdModule = Map:FindFirstChild("LanternholdBuilder")
if lanternholdModule and lanternholdModule:IsA("ModuleScript") then
	local hall = require(lanternholdModule).Build()
	hall:PivotTo(hall:GetPivot() + LANTERNHOLD_AT)
end

local Services = script:WaitForChild("services")
local LightingService = require(Services:WaitForChild("LightingService"))
local RunStateService = require(Services:WaitForChild("RunStateService"))
local LootService = require(Services:WaitForChild("LootService"))
local EnemyService = require(Services:WaitForChild("EnemyService"))
local PlayerDataService = require(Services:WaitForChild("PlayerDataService"))
local InteractionService = require(Services:WaitForChild("InteractionService"))
-- The Lanternhold wiring hangs off four seams and nothing else:
-- HubService.init() below,
-- HubService.lightFor on PlayerAdded and after each restart,
-- HubService.takeDown inside LootService.onExtracted, and HubService.putOut at
-- the top of RunStateService.onRunEnded. Move or replace any of those and the
-- Lamp Wall stops updating without erroring - a player's lamp just never
-- lights. See NOTES-FOR-CODEX.md.
local HubService = require(Services:WaitForChild("HubService"))
-- WorkshopService is committed and self-contained; this require and the
-- init() below are the only seams it has, and they ride along with this file
-- for the same reason HubService's do.
local WorkshopService = require(Services:WaitForChild("WorkshopService"))
local TownService = require(Services:WaitForChild("TownService"))
local LiftService = require(Services:WaitForChild("LiftService"))
local ExpeditionService = require(Services:WaitForChild("ExpeditionService"))
local PingService = require(Services:WaitForChild("PingService"))
local LedgerService = require(Services:WaitForChild("LedgerService"))
local RestorationService = require(Services:WaitForChild("RestorationService"))

LightingService.init()
PlayerDataService.init()
LootService.init()
-- After the hall is built: they bind to tagged fixtures that must already exist.
-- Before HubService: the crate hand-in credits the town total.
TownService.init()
HubService.init()
WorkshopService.init()
-- The Galleria's court lift. The mall is rebuilt on every expedition restart,
-- so this is called again from the restart below or it stops answering.
LiftService.init()
-- The hall's rooms and the ladder's rungs. Both read the host's books, so both
-- go after TownService.init(); both are no-ops until the map lane tags their
-- parts, like every other hall fixture.
--
-- LedgerService.init() used to be called from inside HubService.init() because
-- this file required an untracked ExpeditionService and could not be committed.
-- That is no longer true (61e84d5), so it is started from here with everything
-- else, where it can be found.
LedgerService.init()
RestorationService.init()
-- Opening the generator room is half of rung one, so the rung's panel has to
-- hear about it. LedgerService cannot require RestorationService - that module
-- already requires it, to ask whether its room is open - so the two are joined
-- here instead.
LedgerService.onRoomOpened = function(_player, _roomId)
	RestorationService.refresh()
end
ExpeditionService.init()
PingService.init()

-- Client requests. Each one is validated server-side; nothing here trusts a payload.
local lanternRequest = Remotes.get("LanternRequest")
lanternRequest.OnServerEvent:Connect(function(player, on)
	if type(on) ~= "boolean" then
		return
	end
	-- Work out the refusal before acting, so the reason describes the request
	-- the player actually made.
	local refusal = if on then RunStateService.lanternRefusal(player) else nil
	RunStateService.setLantern(player, on)
	-- Echo the authoritative result, and why if it was a no. The client must not
	-- keep showing a light the server denied, and the player deserves to know
	-- the difference between a dead lantern and a dead key.
	lanternRequest:FireClient(player, RunStateService.isLanternOn(player), refusal)
end)

Remotes.get("MovementRequest").OnServerEvent:Connect(function(player, mode)
	if typeof(mode) == "string" then
		RunStateService.setMovement(player, mode)
	end
end)

-- This remote is deliberately solo-only. Teammate revive owns its real
-- ProximityPrompt hold, so accepting a player-selected target here would make
-- that four-second interaction bypassable.
Remotes.get("ReviveRequest").OnServerEvent:Connect(function(player)
	RunStateService.anchorRevive(player)
end)

-- Payouts land in the profile, not in a client-sent number.
LootService.onExtracted = function(player, payout)
	PlayerDataService.grantPayout(player, payout)
	PlayerDataService.save(player)
	-- Extracting takes your lamp down off the wall rather than putting it out.
	-- Coming back is not the same as not coming back, and the Lamp Wall is
	-- where everyone else reads the difference.
	HubService.takeDown(player)
	-- A successful player returns immediately; their extraction is not held
	-- hostage by teammates choosing to risk a longer push.
	ExpeditionService.returnPlayerToHub(player)
end

-- The clock's last word is not an abrupt wipe: it announces a short Final
-- Call at the currently active exit, then resolves the crew still inside.
RunStateService.onTimeExpired = function()
	for _, player in ipairs(Players:GetPlayers()) do
		Remotes.get("ExtractionState"):FireClient(player, {
			active = true,
			final = true,
			via = "FINAL CALL",
			exit = LootService.finalExit(),
			caller = "THE STORM",
			endsAt = Workspace:GetServerTimeNow() + 15,
		})
	end
	task.delay(15, LootService.finalEvacuate)
end

-- A finished run used to leave the world dead: you could still walk around, but
-- the lantern refused to light, loot refused to be picked up and nothing said
-- why, because every service checks `run.finished` and silently returns. Chris
-- hit exactly that - "pressing F still doesn't turn my lantern on, it flickers",
-- then "same thing with E, nothing works".
--
-- So the expedition resets instead. This is deliberately not a Lanternhold flow:
-- the hub does not exist yet and inventing one is a design decision, not a bug
-- fix. The crew simply goes back out, with the mall rebuilt and the robots
-- cleared, which is the smallest thing that stops the world being a dead end.
local RESTART_DELAY_SECONDS = 6

RunStateService.onRunEnded = function(_outcome: string)
	ExpeditionService.endRaid()
	-- Whatever is still burning belongs to somebody who did not come back.
	HubService.putOut()
	task.delay(RESTART_DELAY_SECONDS, function()
		-- Order matters: clear the old robots before the rebuild re-tags the
		-- spawn points, or the watcher adds a second set on top of the first.
		EnemyService.clearSweepers()
		for _, player in ipairs(Players:GetPlayers()) do
			ExpeditionService.returnPlayerToHub(player)
		end
	end)
end

ExpeditionService.onLaunch = function(_route: string)
	-- Take the spawn off the model we just built, not out of Workspace.Map.
	-- The Galleria deliberately parents itself beside Map rather than inside
	-- it, because Rojo owns Map and replaces its children with the source
	-- modules on every sync - a mall built into Map disappears mid-session.
	local mall = GalleriaBuilder.Build()
	-- Establish the authoritative roster before any system binds to the freshly
	-- built map. Interactions and enemy contacts must never see a playable mall
	-- while `RunStateService.current()` is nil.
	RunStateService.start()
	LootService.beginRun()
	EnemyService.init()
	InteractionService.init()
	local raidSpawn: SpawnLocation? = nil
	for _, instance in ipairs(mall:GetDescendants()) do
		if instance:IsA("SpawnLocation") then
			raidSpawn = instance
			break
		end
	end
	LiftService.init()
	for _, player in ipairs(Players:GetPlayers()) do
		HubService.lightFor(player)
		if raidSpawn and player.Character then
			player.Character:PivotTo(raidSpawn.CFrame * CFrame.new(0, 5, 0))
		end
	end
end

-- Studio's local test player often already exists by the time this script is
-- reached.  Listening only to PlayerAdded therefore missed its first
-- CharacterAdded event: Roblox chose the SpawnLocation inside the old,
-- designer-preview Galleria and the road board appeared over that stale,
-- unlit model.  Bind both present and future players before attempting the
-- initial hub return, so the only pre-raid spawn is Lanternhold.
local function bindPlayerToHub(player: Player)
	local function returnAfterSpawn(character: Model)
		-- CharacterAdded can precede Roblox applying its SpawnLocation pivot. A
		-- short defer lets that built-in placement finish, then makes Lanternhold
		-- authoritative rather than racing it.
		task.delay(0.8, function()
			-- Studio can replace the initial character while applying its chosen
			-- SpawnLocation.  Do not require object identity here: the current
			-- character, whichever one won that race, belongs in the hub.
			if player.Parent and player.Character then
				ExpeditionService.returnPlayerToHub(player)
			end
		end)
	end
	player.CharacterAdded:Connect(returnAfterSpawn)
	if player.Character then
		returnAfterSpawn(player.Character)
	end
end

Players.PlayerAdded:Connect(bindPlayerToHub)
Players.PlayerRemoving:Connect(RunStateService.removePlayer)

RunStateService.start_tick()
for _, player in ipairs(Players:GetPlayers()) do
	bindPlayerToHub(player)
end

print("[LastLightkeepers] server up")
