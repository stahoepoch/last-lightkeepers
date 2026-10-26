--!strict
-- Authoritative run state: Storm Charge, lanterns, knockdowns, banking, wipes.
--
-- Named RunStateService rather than the handoff's "RunService" on purpose:
-- `RunService` collides with Roblox's own service and makes every require in
-- this file ambiguous. Same responsibility, non-colliding name.
--
-- Version one runs one expedition per server, so the state is a module singleton.

local Players = game:GetService("Players")
local RobloxRun = game:GetService("RunService")

local ReplicatedStorage = game:GetService("ReplicatedStorage")
local Shared = ReplicatedStorage:WaitForChild("Shared")
local Config = require(Shared:WaitForChild("Config"))
local Remotes = require(Shared:WaitForChild("Remotes"))
local Types = require(Shared:WaitForChild("Types"))

local SNAPSHOT_INTERVAL = 0.25

-- Below this, a character is falling out of the world rather than standing in
-- it. The mall's lowest floor sits near y = -3; Roblox destroys parts at
-- workspace.FallenPartsDestroyHeight, far below this.
local VOID_HEIGHT = -100

-- Floor to floor in the Galleria is 32 studs, so this downs you for a balcony
-- drop while leaving a jump, a kerb or a bench harmless.
local LETHAL_FALL_STUDS = 26

local RunStateService = {}

local run: Types.RunState? = nil
local snapshotAccumulator = 0

local function emptyHaul(): Types.Haul
	return {
		carried = {},
		banked = {},
	}
end

local function newLightkeeper(userId: number): Types.LightkeeperState
	return {
		userId = userId,
		lanternOn = false,
		lanternCharge = 1,
		knockedDown = false,
		anchorRevivesLeft = Config.StormCharge.anchorRevive.chargesPerRun,
		taggedUntil = nil,
		carryingCrate = false,
		haul = emptyHaul(),
		contactHits = 0,
		lastContactAt = nil,
		movementMode = "walk",
	}
end

local function rootOf(userId: number): BasePart?
	local player = Players:GetPlayerByUserId(userId)
	local character = player and player.Character
	local root = character and character:FindFirstChild("HumanoidRootPart")
	return if root and root:IsA("BasePart") then root else nil
end

local function humanoidOf(userId: number): Humanoid?
	local player = Players:GetPlayerByUserId(userId)
	local character = player and player.Character
	return if character then character:FindFirstChildOfClass("Humanoid") else nil
end

local function clearRescueBeacon(userId: number)
	local root = rootOf(userId)
	local beacon = root and root:FindFirstChild("RescueBeacon")
	if beacon then
		beacon:Destroy()
	end
	local light = root and root:FindFirstChild("RescueBeaconLight")
	if light then
		light:Destroy()
	end
end

-- A readable landmark in a huge, dark mall: no asset dependency, no shadows,
-- and attached to the keeper so a crawling player carries their distress flare.
local function createRescueBeacon(player: Player)
	local root = rootOf(player.UserId)
	if not root then
		return
	end
	clearRescueBeacon(player.UserId)
	local light = Instance.new("PointLight")
	light.Name = "RescueBeaconLight"
	light.Color, light.Brightness, light.Range, light.Shadows =
		Color3.fromRGB(110, 196, 255), 3, 36, false
	light.Parent = root
	local tag = Instance.new("BillboardGui")
	tag.Name = "RescueBeacon"
	tag.Size, tag.StudsOffset, tag.AlwaysOnTop, tag.MaxDistance =
		UDim2.fromOffset(180, 36), Vector3.new(0, 5, 0), true, 170
	tag.Parent = root
	local label = Instance.new("TextLabel")
	label.Size, label.BackgroundTransparency, label.Font, label.TextSize =
		UDim2.fromScale(1, 1), 1, Enum.Font.GothamBold, 16
	label.TextColor3, label.TextStrokeTransparency, label.Text =
		Color3.fromRGB(160, 220, 255), 0.35, "KEEPER DOWN — " .. player.DisplayName
	label.Parent = tag
end

--- A downed Lightkeeper cannot walk. Teammates have to come to them, which is
--- the whole point of the knockdown rule.
local function applyDownedMovement(userId: number, downed: boolean)
	local humanoid = humanoidOf(userId)
	if not humanoid then
		return
	end
	if downed then
		-- Downed is not dead. A keeper can crawl to the rescue zone or call an
		-- extraction, but cannot jump, loot, fight, or outrun the crew.
		humanoid.WalkSpeed = math.max(3, Config.Lantern.knockdown.downedWalkSpeed)
		humanoid.JumpPower = 0
		humanoid.PlatformStand = false
	else
		local state = run and run.lightkeepers[userId]
		local movement = Config.Lantern.movement
		local baseSpeed = if state and state.movementMode == "sprint"
			then movement.sprintSpeed
			elseif state and state.movementMode == "crouch" then movement.crouchSpeed
			else movement.walkSpeed
		local burden = 0
		if state then
			for tier, amount in pairs(state.haul.carried) do
				local def = Config.Loot.tiers[tier]
				burden += amount * (def and def.burden or 0)
			end
		end
		local burdenRule = Config.Loot.burden
		-- The one workshop upgrade that is actually wired: a bought relic
		-- harness scales this player's penalty per point. 1 when nothing is
		-- owned, which is where everyone starts. Looked up lazily so the
		-- shop stays optional - a place without WorkshopService behaves
		-- exactly as it did before.
		local relief = 1
		local shop = script.Parent:FindFirstChild("WorkshopService")
		local buyer = shop and Players:GetPlayerByUserId(userId)
		if buyer then
			local okRelief, value = pcall(function()
				return (require(shop) :: any).relicBurdenMultiplier(buyer)
			end)
			if okRelief and type(value) == "number" then
				relief = value
			end
		end
		local penalty = math.min(
			burden * burdenRule.speedPenaltyPerPoint * relief,
			burdenRule.maxSpeedPenalty * relief
		)
		humanoid.WalkSpeed = math.max(4, baseSpeed * (1 - penalty))
		humanoid.JumpPower = 50
		humanoid.PlatformStand = false
	end
end

function RunStateService.refreshMovement(player: Player)
	local state = run and run.lightkeepers[player.UserId]
	if state and not state.knockedDown then
		applyDownedMovement(player.UserId, false)
	end
end

--- Server owns the actual WalkSpeed. The client only asks for a named mode,
--- so holding Shift cannot smuggle a custom speed into a raid.
function RunStateService.setMovement(player: Player, requested: string)
	local current = run
	if not current or current.finished then
		return
	end
	local state = current.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	if requested ~= "walk" and requested ~= "sprint" and requested ~= "crouch" then
		return
	end
	state.movementMode = requested :: Types.MovementMode
	applyDownedMovement(player.UserId, false)
end

function RunStateService.current(): Types.RunState?
	return run
end

function RunStateService.start()
	run = {
		runId = tostring(os.time()) .. "-" .. tostring(math.random(1000, 9999)),
		startedAt = os.clock(),
		stormCharge = 0,
		crossedThresholds = {},
		lightkeepers = {},
		finished = false,
		finalCallStarted = false,
	}
	for _, player in ipairs(Players:GetPlayers()) do
		RunStateService.addPlayer(player)
	end
end

--- A character that has left the world rather than been hurt in it.
--
-- Falling out of the map destroys the character's parts, which reads to a
-- Humanoid exactly like dying. It is not the same event and must not be
-- treated as one: see the comment in guardCharacter.
local function fellOutOfTheWorld(character: Model): boolean
	local root = character:FindFirstChild("HumanoidRootPart")
	if not root or not root:IsA("BasePart") then
		-- The parts are already gone, which only happens on the way out.
		return true
	end
	-- Comfortably below the lowest floor of the mall, and well above the
	-- height at which Roblox starts destroying things.
	return (root :: BasePart).Position.Y < VOID_HEIGHT
end

--- Enemies knock Lightkeepers down; they do not kill them. This intercepts the
--- lethal case rather than trusting every damage source to be polite about it.
function RunStateService.guardCharacter(player: Player, character: Model)
	local humanoid = character:WaitForChild("Humanoid", 5)
	if not humanoid or not humanoid:IsA("Humanoid") then
		return
	end
	humanoid.BreakJointsOnDeath = false

	humanoid.HealthChanged:Connect(function(health: number)
		if health > 0 then
			return
		end

		-- Falling off the map is not a Sweeper, and must not be charged as one.
		-- Found by Chris in the first human playtest: he fell through a gap,
		-- respawned, and could not move. Intercepting the fall knocked him
		-- down, CharacterAdded re-locked the fresh character because the run
		-- still had him downed, and the only way out was the Anchor Revive -
		-- one per run, paid for with a full lantern. A hole in the floor should
		-- not cost a player their self-revive. Let the respawn happen.
		if fellOutOfTheWorld(character) then
			return
		end

		-- Pull them back off the floor and down them instead.
		humanoid.Health = 1
		RunStateService.knockDown(player)
	end)

	-- A drop between floors has to cost you. Chris, on the mall: "the height
	-- from top to bottom should be devastating, you shouldn't be able to jump
	-- from floor to floor without dying, i want to force you to use the
	-- stairs." Roblox applies no fall damage of its own, so a balcony was a
	-- shortcut rather than a danger.
	--
	-- This measures the actual drop rather than the impact speed, because the
	-- question being asked is "did you come down a storey", and a storey is a
	-- distance. Landing hard enough goes through the knockdown above, so a fall
	-- downs you exactly as a Sweeper would - it does not kill outright.
	-- Sampled on Heartbeat rather than read inside the StateChanged events,
	-- because the character is simulated on its owner's client. The position
	-- the server sees inside the Landed event still lags the real touchdown,
	-- so a measured drop came out far shorter than the drop actually taken: on
	-- a clean rig a thirty stud fall measured eleven and was survived, which
	-- handed back the balcony shortcut this is here to close. Reading position
	-- and FloorMaterial together, every frame, keeps the two consistent.
	local peak: number? = nil
	local airborne = false
	local sampler: RBXScriptConnection? = nil
	sampler = RobloxRun.Heartbeat:Connect(function()
		if not character.Parent then
			if sampler then
				sampler:Disconnect()
			end
			return
		end
		local root = character:FindFirstChild("HumanoidRootPart")
		if not root or not root:IsA("BasePart") or humanoid.Health <= 0 then
			return
		end
		local y = (root :: BasePart).Position.Y
		if humanoid.FloorMaterial == Enum.Material.Air then
			-- Keep the highest point of the fall: a jump at the lip of a
			-- balcony leaves the floor twice and the second exit is lower.
			peak = math.max(peak or y, y)
			airborne = true
		elseif airborne then
			airborne = false
			local from = peak
			peak = nil
			if from and from - y >= LETHAL_FALL_STUDS then
				humanoid.Health = 0
			end
		end
	end)
end

function RunStateService.addPlayer(player: Player)
	if not run or run.finished then
		return
	end
	run.lightkeepers[player.UserId] = newLightkeeper(player.UserId)

	if player.Character then
		RunStateService.guardCharacter(player, player.Character)
	end
	player.CharacterAdded:Connect(function(character)
		RunStateService.guardCharacter(player, character)
		local state = run and run.lightkeepers[player.UserId]
		if state and state.knockedDown then
			applyDownedMovement(player.UserId, true)
		end
	end)
end

function RunStateService.removePlayer(player: Player)
	if not run then
		return
	end
	-- Disconnect follows the wipe rule: without extraction, the entire run haul
	-- is lost. Resolve per-player so one dropout cannot grief the crew.
	local state = run.lightkeepers[player.UserId]
	if state then
		RunStateService.resolveHaul(state, false)
		run.lightkeepers[player.UserId] = nil
	end
end

--- Removes a successful extractor from the active roster without applying the
--- disconnect/wipe loss rule. When the final keeper makes it out, the raid is
--- over; otherwise an already-safe player could still keep the crew from
--- resolving and be counted in a later wipe.
function RunStateService.finishExtraction(player: Player)
	if not run or run.finished or not run.lightkeepers[player.UserId] then
		return
	end
	run.lightkeepers[player.UserId] = nil
	if next(run.lightkeepers) == nil then
		run.finished = true
		RunStateService.onRunEnded("extracted")
	end
end

function RunStateService.activeLanterns(): number
	if not run then
		return 0
	end
	local count = 0
	for _, state in pairs(run.lightkeepers) do
		if state.lanternOn and state.lanternCharge > 0 then
			count += 1
		end
	end
	return count
end

function RunStateService.setLantern(player: Player, on: boolean)
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	if on and state.lanternCharge <= 0 then
		return
	end
	state.lanternOn = on
end

--- Why a request to light the lantern was refused, or nil if it would be
--- granted. The client has no way to work this out on its own: an echo of
--- `false` means both "you asked to turn it off" and "no", which is how a dead
--- lantern came to look like a broken key.
function RunStateService.lanternRefusal(player: Player): string?
	if not run or run.finished then
		return "The expedition is over"
	end
	local state = run.lightkeepers[player.UserId]
	if not state then
		return "The expedition is over"
	end
	if state.knockedDown then
		return "Not while you are down"
	end
	if state.lanternCharge <= 0 then
		return "Lantern spent - no charge left this run"
	end
	return nil
end

function RunStateService.isLanternOn(player: Player): boolean
	if not run then
		return false
	end
	local state = run.lightkeepers[player.UserId]
	return state ~= nil and state.lanternOn and state.lanternCharge > 0
end

--- Storm Charge only ever rises. Darkness slows the climb; nothing reverses it.
function RunStateService.addStormCharge(amount: number)
	if not run or run.finished or amount <= 0 then
		return
	end
	run.stormCharge += amount

	local crossed = Config.thresholdAt(run.stormCharge)
	if crossed and not run.crossedThresholds[crossed.id] then
		run.crossedThresholds[crossed.id] = true
		RunStateService.onThresholdCrossed(crossed)
	end
end

-- Assigned by init.server so EnemyService can react without a circular require.
RunStateService.onThresholdCrossed = function(_threshold: any) end

--- How many Lightkeepers are on this run, down or not.
function RunStateService.crewSize(): number
	if not run then
		return 0
	end
	local count = 0
	for _ in pairs(run.lightkeepers) do
		count += 1
	end
	return count
end

--- Whether this Lightkeeper could stand up on their own right now. Both the
--- wipe check and anchorRevive ask this, so the price can never drift apart
--- between "we wiped you" and "you may revive".
function RunStateService.canAnchorRevive(state: Types.LightkeeperState): boolean
	if not state.knockedDown then
		return false
	end
	local anchor = Config.StormCharge.anchorRevive
	if state.anchorRevivesLeft <= 0 then
		return false
	end
	-- The handoff grants the Anchor Revive to *solo players*: it exists so a
	-- lone Lightkeeper is not hard-wiped on a first knockdown. Granting it to
	-- everyone breaks the other half of the same rule - "if the whole crew goes
	-- down, it wakes in Lanternhold with half of unbanked loot" - because a
	-- four-crew all-down would sit there waiting for someone to press R instead
	-- of wiping. Checked live rather than at spawn, so it tracks a crew that
	-- shrinks to one mid-run.
	if anchor.soloOnly and RunStateService.crewSize() > 1 then
		return false
	end
	if anchor.consumesFullLanternCharge and state.lanternCharge < 1 then
		return false
	end
	return true
end

function RunStateService.knockDown(player: Player)
	if not run then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	state.knockedDown = true
	state.contactHits = 0
	if Config.Lantern.knockdown.lanternForcedOffWhenDowned then
		state.lanternOn = false
	end
	-- Off by Chris's call, 2026-10-20: a downed Lightkeeper keeps the crate.
	--
	-- Note what this branch does and does not do, because a playtest caught it
	-- being a lie: it frees the carry slot but leaves RestorationCrates sitting
	-- in the haul, so a "dropped" crate still extracted at full value. Setting
	-- the flag back on needs the tier removed from state.haul.carried here too,
	-- and a decision about where the crate goes - the floor, or gone.
	if Config.Lantern.knockdown.dropsCarriedCrate then
		state.carryingCrate = false
	end
	applyDownedMovement(player.UserId, true)
	createRescueBeacon(player)
	-- A downed keeper is a crew emergency, not a silent state hidden in a
	-- snapshot. Tell teammates who fell and where; clients render the flare and
	-- directional cue, while the server remains the source of the position.
	local root = rootOf(player.UserId)
	for _, teammate in ipairs(Players:GetPlayers()) do
		if teammate ~= player and run.lightkeepers[teammate.UserId] then
			Remotes.get("DownedSignal"):FireClient(teammate, {
				userId = player.UserId,
				name = player.DisplayName,
				position = if root then root.Position else nil,
			})
		end
	end

	-- A whole crew down is still a dangerous state, but it is not an instant
	-- server-side wipe. Downed keepers may crawl to the active exit and call it;
	-- the enemy already ignores them there. Wiping here deleted their run state
	-- while the client still showed them crawling, which made extraction
	-- impossible. The raid clock remains the final failure condition.
	for _, other in pairs(run.lightkeepers) do
		if not other.knockedDown then
			return
		end
	end

	-- ...unless someone down there can still pay for an Anchor Revive. Without
	-- this the wipe fires inside the same call that knocked the last player
	-- down, so the Anchor Revive could never be reached at all - a solo player
	-- was hard-wiped on their first knockdown even holding a full charge, which
	-- is the exact opposite of what anchorRevive below is for. A crew with no
	-- charge and no anchors left still wipes; that part was always right.
	for _, other in pairs(run.lightkeepers) do
		if RunStateService.canAnchorRevive(other) then
			return
		end
	end

	-- Keep the run alive for the crawl-to-extract rule. Do not call wipe here.
end

local function stand(state: Types.LightkeeperState)
	state.knockedDown = false
	state.contactHits = 0
	clearRescueBeacon(state.userId)
	applyDownedMovement(state.userId, false)
end

--- Solo Anchor Revive. Paid for with a full lantern charge plus a permanent
--- Storm Charge spike: the thematic cost of light, and the reason a solo player
--- is not hard-wiped on a first knockdown.
function RunStateService.anchorRevive(player: Player): boolean
	if not run or run.finished then
		return false
	end
	local state = run.lightkeepers[player.UserId]
	if not state or not state.knockedDown then
		return false
	end

	-- "Consumes a full lantern charge" means it must actually be full.
	if not RunStateService.canAnchorRevive(state) then
		return false
	end
	local anchor = Config.StormCharge.anchorRevive

	state.anchorRevivesLeft -= 1
	-- Spend the charge only if charge is a resource at all. Chris's decision
	-- that the lantern never runs out removes the fuel economy, and zeroing a
	-- gauge that can never refill would strand the player lightless for the
	-- rest of the run - the exact misery the decision was meant to remove.
	-- With unlimited light the anchor's price is what remains: one per run, and
	-- a permanent spike in the meter.
	if Config.lanternDrainPerSecond() > 0 then
		state.lanternCharge = 0
	end
	state.lanternOn = false
	RunStateService.addStormCharge(anchor.stormChargeSpike)
	stand(state)
	return true
end

--- A teammate has completed the engine-managed proximity hold. The prompt
--- supplies the visible 4-second progress ring; this method supplies the
--- authority, re-checking both players and their distance at the moment the
--- revive actually lands. Keeping one timer avoids turning a 4-second hold
--- into an accidental eight-second interaction.
function RunStateService.completeRevive(rescuer: Player, target: Player): boolean
	if not run or run.finished or rescuer == target then
		return false
	end
	local rescuerState = run.lightkeepers[rescuer.UserId]
	local targetState = run.lightkeepers[target.UserId]
	if not rescuerState or not targetState then
		return false
	end
	if rescuerState.knockedDown or not targetState.knockedDown then
		return false
	end

	local a, b = rootOf(rescuer.UserId), rootOf(target.UserId)
	if not a or not b then
		return false
	end
	if (a.Position - b.Position).Magnitude > Config.Lantern.revive.maxReviveDistance then
		return false
	end

	stand(targetState)
	return true
end

--- Stepping into a relay. It may recharge a lantern; it never secures a haul.
---
--- Named `bank` until 2026-10-26, which was a trap: a function called bank
--- that banks nothing is how a reader concludes the mechanic exists. Chris
--- locked the rule that day - "Only extraction secures loot... No banking
--- loot. The relay is a route/intel point or lantern-support point, never
--- insurance." If a Lightkeeper does not leave through the active exit,
--- everything found during that run is gone.
function RunStateService.touchRelay(player: Player)
	if not run or run.finished then
		return
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return
	end
	-- Off by default. See the rechargeAtRelay proposal in lantern.json - it is
	-- Chris's call, not mine, because it softens the charge economy. Crucially,
	-- this is utility, not insurance: the relay does not move loot out of the
	-- carried haul or change its wipe outcome.
	if Config.Lantern.charge.rechargeAtRelay then
		state.lanternCharge = 1
	end
end

--- Final payout for one Lightkeeper. Only extraction keeps a haul; wipes and
--- disconnects deliberately resolve both carried and legacy-banked entries to
--- zero. Keeping the two buckets for now avoids a data migration while making
--- the game rule unambiguous.
function RunStateService.resolveHaul(
	state: Types.LightkeeperState,
	extracted: boolean
): { [Types.LootTier]: number }
	local bankedFraction = if extracted then 1 else Config.Loot.banking.bankedKeptOnWipe
	local carriedFraction = if extracted then 1 else Config.Loot.banking.unbankedKeptOnWipe
	local payout: { [Types.LootTier]: number } = {}

	for tier, amount in pairs(state.haul.banked) do
		payout[tier] = (payout[tier] or 0) + math.floor(amount * bankedFraction)
	end
	for tier, amount in pairs(state.haul.carried) do
		payout[tier] = (payout[tier] or 0) + math.floor(amount * carriedFraction)
	end

	state.haul = emptyHaul()
	return payout
end

function RunStateService.wipe()
	if not run or run.finished then
		return
	end
	run.finished = true
	for userId, state in pairs(run.lightkeepers) do
		state.lanternOn = false
		clearRescueBeacon(userId)
		-- They wake in Lanternhold, on their feet.
		state.knockedDown = false
		applyDownedMovement(userId, false)
		local payout = RunStateService.resolveHaul(state, false)
		local player = Players:GetPlayerByUserId(userId)
		if player then
			Remotes.get("RunEnded"):FireClient(player, {
				outcome = "wipe",
				payout = payout,
			})
		end
	end
	RunStateService.onRunEnded("wipe")
end

RunStateService.onRunEnded = function(_outcome: string) end
RunStateService.onTimeExpired = function()
	RunStateService.wipe()
end

--- A Sweeper reached someone. Hits accumulate and decay, so one Sweeper is
--- survivable and a swarm is not, without giving the player a health bar.
function RunStateService.registerContact(player: Player): boolean
	if not run or run.finished then
		return false
	end
	local state = run.lightkeepers[player.UserId]
	if not state or state.knockedDown then
		return false
	end

	local contact = Config.Lantern.sweeperContact
	local now = os.clock()
	if state.lastContactAt and now - state.lastContactAt < contact.contactCooldownSeconds then
		return false
	end

	state.lastContactAt = now
	state.contactHits += 1
	if contact.knockdownOnContact and state.contactHits >= contact.hitsToKnockDown then
		RunStateService.knockDown(player)
		return true
	end
	return false
end

--- Per-viewer snapshot. Crew-wide numbers plus this player's own state: a
--- broadcast cannot answer "is *my* lantern lit" or "how much charge do I have".
local function buildSnapshot(state: Types.LightkeeperState): Types.RunSnapshot
	local current = run :: Types.RunState
	local active = RunStateService.activeLanterns()
	local crossed = Config.thresholdAt(current.stormCharge)

	local down, total = 0, 0
	for _, other in pairs(current.lightkeepers) do
		total += 1
		if other.knockedDown then
			down += 1
		end
	end

	local haul: { [Types.LootTier]: number } = {}
	for tier, amount in pairs(state.haul.carried) do
		haul[tier] = amount
	end

	return {
		runId = current.runId,
		stormCharge = current.stormCharge,
		stormRate = Config.stormRate(active),
		activeLanterns = active,
		highestThreshold = if crossed then crossed.id else nil,
		wardenActive = current.crossedThresholds.WardenConverges == true,
		yourCharge = state.lanternCharge,
		yourLanternOn = state.lanternOn and state.lanternCharge > 0,
		yourKnockedDown = state.knockedDown,
		yourTagged = state.taggedUntil ~= nil,
		yourAnchorRevivesLeft = state.anchorRevivesLeft,
		yourContactHits = state.contactHits,
		hitsToKnockDown = Config.Lantern.sweeperContact.hitsToKnockDown,
		yourCanAnchorRevive = RunStateService.canAnchorRevive(state),
		yourMovementMode = state.movementMode,
		yourBurden = (function()
			local total = 0
			for tier, amount in pairs(state.haul.carried) do
				local def = Config.Loot.tiers[tier]
				total += amount * (def and def.burden or 0)
			end
			return total
		end)(),
		yourHaul = haul,
		crewDown = down,
		crewTotal = total,
		runSecondsRemaining = math.max(
			0,
			Config.Run.targetMaxSeconds - (os.clock() - current.startedAt)
		),
	}
end

function RunStateService.start_tick()
	RobloxRun.Heartbeat:Connect(function(dt: number)
		if not run or run.finished then
			return
		end
		if
			os.clock() - run.startedAt >= Config.Run.targetMaxSeconds and not run.finalCallStarted
		then
			run.finalCallStarted = true
			RunStateService.onTimeExpired()
			return
		end

		RunStateService.addStormCharge(Config.stormRate(RunStateService.activeLanterns()) * dt)

		local now = os.clock()
		local drain = Config.lanternDrainPerSecond() * dt
		local contactDecay = Config.Lantern.sweeperContact.hitDecaySeconds

		for _, state in pairs(run.lightkeepers) do
			-- Expire Glimmer tags.
			if state.taggedUntil and now >= state.taggedUntil then
				state.taggedUntil = nil
			end

			-- Burn lantern charge. Light is spent, not merely switched on.
			if state.lanternOn and state.lanternCharge > 0 then
				state.lanternCharge = math.max(state.lanternCharge - drain, 0)
				if state.lanternCharge <= 0 and Config.Lantern.charge.autoOffAtEmpty then
					state.lanternOn = false
				end
			end

			-- Sweeper contact fades if you break away.
			if
				state.contactHits > 0
				and state.lastContactAt
				and now - state.lastContactAt >= contactDecay
			then
				state.contactHits -= 1
				state.lastContactAt = now
			end
		end

		snapshotAccumulator += dt
		if snapshotAccumulator >= SNAPSHOT_INTERVAL then
			snapshotAccumulator = 0
			local snapshotEvent = Remotes.get("RunSnapshot")
			for userId, state in pairs(run.lightkeepers) do
				local player = Players:GetPlayerByUserId(userId)
				if player then
					snapshotEvent:FireClient(player, buildSnapshot(state))
				end
			end
		end
	end)
end

return RunStateService
