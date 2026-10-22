--!strict
-- A lint for a built map.
--
-- Every map fault that has cost this project an evening was invisible in the
-- source and obvious in the built world: relic caches destroyed by a column
-- grid, two escalators buried under a food court deck, a landing rail standing
-- across the only way into the food court, markers deleted for sitting inside
-- a planter. None of those were findable by reading the builder. All of them
-- were findable in about a second by measuring the thing it produced.
--
-- So this is the measuring, written down and repeatable. It reports; it never
-- edits. Run it on either map from the command bar:
--
--   print(require(workspace.Map.MapAudit).Run(workspace.ClaudeGalleria))
--   print(require(workspace.Map.MapAudit).Run(workspace.Lanternhold))
--
-- It is deliberately not required by the game. Nothing at runtime depends on
-- it and it costs a Play session nothing.
--
-- TWO THINGS TO KNOW BEFORE BELIEVING IT.
--
-- 1. Run reachability in a PLAY session, never in Edit. Roblox bakes the
--    navmesh lazily, so a model built seconds ago reports NoPath for half the
--    map in Edit and paths perfectly in Play. This cost an hour: every route
--    across the Galleria came back NoPath in Edit and every one of them was
--    fine. darkFloor and clipping are geometry only and are honest in Edit.
--
-- 2. wallGaps has known edge effects and they are not faults. A wall standing
--    on the boundary of its own floor slab overhangs it at the ends, and a
--    header band over a doorway has no floor under it by definition. Both
--    report. Check what is actually under a finding before acting on it.
--
-- Every check here has been wrong at least once and each wrong version is
-- written up at the function that fixed it. That history is the point: an
-- alarming number from a young instrument is the instrument until proven
-- otherwise.

local PathfindingService = game:GetService("PathfindingService")
local Workspace = game:GetService("Workspace")

local MapAudit = {}

export type Finding = {
	kind: string,
	detail: string,
	at: Vector3?,
}

-- How dark a patch of floor has to be before it counts as unlit. A Roblox
-- PointLight falls off with distance; this sums an inverse-square-ish
-- contribution from every light that reaches the point, which is not the
-- renderer's model but is monotonic in the same direction, and that is all a
-- lint needs. 0.08 is roughly "one house lamp at the far edge of its range".
local DARK_THRESHOLD = 0.08

-- Sampling step across the floor, in studs. Twenty keeps a 260 by 230 hub
-- under two hundred probes a floor, which runs in about a second.
local FLOOR_STEP = 20

-- How much two parts have to interpenetrate before it is clipping rather than
-- two surfaces meeting. Walls meet floors; a bench inside a column does not.
local CLIP_FRACTION = 0.25

local function illuminanceAt(point: Vector3, lights: { PointLight }): number
	local total = 0
	for _, light in ipairs(lights) do
		if light.Enabled then
			local source = light.Parent
			if source and source:IsA("BasePart") then
				local distance = (source.Position - point).Magnitude
				if distance < light.Range then
					-- Linear falloff to the range edge, scaled by brightness.
					-- Crude on purpose: the question is "is anything lighting
					-- this", not "what does the renderer output".
					total += light.Brightness * (1 - distance / light.Range)
				end
			end
		end
	end
	return total
end

local function overlapVolume(a: BasePart, b: BasePart): number
	-- Axis-aligned bounds overlap. Rotated parts make this an overestimate of
	-- the true intersection, so a rotated pair needs a bigger overlap before it
	-- is reported - which is the safe direction for a lint that must not cry
	-- wolf.
	local aMin, aMax = a.Position - a.Size / 2, a.Position + a.Size / 2
	local bMin, bMax = b.Position - b.Size / 2, b.Position + b.Size / 2
	local dx = math.min(aMax.X, bMax.X) - math.max(aMin.X, bMin.X)
	local dy = math.min(aMax.Y, bMax.Y) - math.max(aMin.Y, bMin.Y)
	local dz = math.min(aMax.Z, bMax.Z) - math.max(aMin.Z, bMin.Z)
	if dx <= 0 or dy <= 0 or dz <= 0 then
		return 0
	end
	return dx * dy * dz
end

--- Floor that nothing lights. The commonest and least visible fault: a room
--- whose lamps were hand-placed on coordinates that cover part of it.
function MapAudit.darkFloor(model: Model): { Finding }
	local lights: { PointLight } = {}
	for _, instance in ipairs(model:GetDescendants()) do
		if instance:IsA("PointLight") then
			table.insert(lights, instance)
		end
	end

	local box, size = model:GetBoundingBox()
	local origin = box.Position
	local params = RaycastParams.new()
	params.FilterType = Enum.RaycastFilterType.Include
	params.FilterDescendantsInstances = { model }

	local findings: { Finding } = {}
	local sampled, dark = 0, 0
	local x = -size.X / 2
	while x <= size.X / 2 do
		local z = -size.Z / 2
		while z <= size.Z / 2 do
			-- Probe down the whole height and take every floor on the way, so
			-- an upper storey is audited as well as the ground.
			local from = origin + Vector3.new(x, size.Y / 2, z)
			local travelled = 0
			for _ = 1, 8 do
				local hit = Workspace:Raycast(
					from - Vector3.new(0, travelled, 0),
					Vector3.new(0, -(size.Y - travelled), 0),
					params
				)
				if not hit then
					break
				end
				travelled = (from.Y - hit.Position.Y) + 2
				-- Only count a surface a player could actually stand on, INSIDE
				-- the building. The first version of this counted every upward
				-- face it hit and reported the Lanternhold 71% unlit on 99
				-- lights - because it was counting the roof, the wall caps and
				-- the beam tops, none of which anybody stands on and none of
				-- which has a lamp over it. A number that alarming from an
				-- instrument this young is the instrument, not the building.
				--
				-- So: a player's height of clear space above, and a ceiling
				-- somewhere above that. The headroom test throws out the
				-- surfaces too tight to enter; the ceiling test throws out the
				-- roof and the ground outside, which have nothing above them.
				local standable = hit.Normal.Y > 0.7 and hit.Instance.CanCollide
				if standable then
					local head = Workspace:Raycast(
						hit.Position + Vector3.new(0, 0.5, 0),
						Vector3.new(0, 44, 0),
						params
					)
					local clear = if head then (head.Position.Y - hit.Position.Y) else math.huge
					standable = clear >= 7 and clear < math.huge
				end
				if standable then
					sampled += 1
					local standing = hit.Position + Vector3.new(0, 3, 0)
					local lux = illuminanceAt(standing, lights)
					if lux < DARK_THRESHOLD then
						dark += 1
						table.insert(findings, {
							kind = "dark",
							detail = `unlit floor on {hit.Instance.Name} (lux {string.format(
								"%.3f",
								lux
							)})`,
							at = standing,
						})
					end
				end
			end
			z += FLOOR_STEP
		end
		x += FLOOR_STEP
	end

	table.insert(findings, 1, {
		kind = "summary",
		detail = `{dark} of {sampled} floor samples unlit ({string.format(
			"%.0f",
			100 * dark / math.max(sampled, 1)
		)}%), {#lights} lights`,
	})
	return findings
end

--- Furniture buried in the building.
---
--- Deliberately NOT "any two parts that interpenetrate". That was the first
--- version and it reported 175 pairs in the hub, nearly all of them correct
--- construction: a stair foot resting on its slab, a pilaster set into the
--- wall it decorates, a parapet on the roof, a plaque fixed to a wall. A
--- building is made of parts that overlap on purpose and no volume test can
--- tell those from a mistake.
---
--- What is always a mistake is a FIXTURE inside STRUCTURE - a bench in a
--- column, a rack through a wall, a table inside a stair. That is the thing
--- you walk into, and it is the only pairing worth reporting.
function MapAudit.clipping(model: Model): { Finding }
	local STRUCTURAL =
		{ Structure = true, Shell = true, Anchors = true, Floors = true, Core = true }
	local FURNITURE = { Fixtures = true, Gameplay = true, Markers = true }

	local parts: { BasePart } = {}
	for _, instance in ipairs(model:GetDescendants()) do
		if instance:IsA("BasePart") and instance.CanCollide and instance.CanQuery then
			local folder = instance:FindFirstAncestorWhichIsA("Folder")
			if folder and FURNITURE[folder.Name] then
				table.insert(parts, instance)
			end
		end
	end

	local params = OverlapParams.new()
	params.FilterType = Enum.RaycastFilterType.Include
	params.FilterDescendantsInstances = { model }
	params.MaxParts = 40

	local findings: { Finding } = {}
	local seen: { [string]: boolean } = {}
	for _, part in ipairs(parts) do
		local mine = part.Size.X * part.Size.Y * part.Size.Z
		if mine > 0 then
			for _, other in ipairs(Workspace:GetPartsInPart(part, params)) do
				local folder = other:FindFirstAncestorWhichIsA("Folder")
				if other ~= part and other.CanCollide and folder and STRUCTURAL[folder.Name] then
					local theirs = other.Size.X * other.Size.Y * other.Size.Z
					local fraction = overlapVolume(part, other) / math.min(mine, theirs)
					if fraction >= CLIP_FRACTION then
						local key = part.Name .. "|" .. other.Name
						if not seen[key] then
							seen[key] = true
							table.insert(findings, {
								kind = "clip",
								detail = `{part.Name} [{folder and part.Parent and part.Parent.Name}] is {string.format(
									"%.0f",
									fraction * 100
								)}% inside {other.Name} [{folder.Name}]`,
								at = part.Position,
							})
						end
					end
				end
			end
		end
	end
	return findings
end

--- Walls with daylight under or over them.
---
--- NOT "a wall with no doorway in it" - that was the first version of this
--- check and it reported every wall in both maps. Walls here are built as
--- segments with the doors as the gaps BETWEEN segments, so a segment with no
--- hole in it is the normal case and means nothing.
---
--- What does mean something is a wall that fails to meet the floor under it.
--- That is a seam you can see through or fall into, it is invisible in the
--- source because the wall and the floor are written in different places, and
--- it is what "walls that dont work" looks like from inside the room.
function MapAudit.wallGaps(model: Model, wallPrefixes: { string }): { Finding }
	local findings: { Finding } = {}
	local params = RaycastParams.new()
	params.FilterType = Enum.RaycastFilterType.Include
	params.FilterDescendantsInstances = { model }

	-- Under this, two surfaces are meeting with a rounding error.
	local SEAM = 1.5

	for _, instance in ipairs(model:GetDescendants()) do
		if instance:IsA("BasePart") and instance.CanCollide then
			local isWall = false
			for _, prefix in ipairs(wallPrefixes) do
				if string.sub(instance.Name, 1, #prefix) == prefix then
					isWall = true
					break
				end
			end
			-- A lintel is the top of a doorway. It is meant to stand clear of
			-- the floor - that is what makes it a door - so it is not a wall
			-- for this check. Skipping these takes the hub from four findings
			-- to none, and all four were doorway heads.
			if isWall and string.find(instance.Name, "_Head_") then
				isWall = false
			end
			-- A wall is long and thin. Requiring only "longer than 8" let a
			-- ten stud square column through, and the department store column
			-- grid alone produced 51 findings, every one of them a pillar
			-- standing exactly where it should. A column is not a wall and the
			-- probe means nothing on one.
			local isRun = instance.Size.X > 30
				and instance.Size.X > instance.Size.Z * 3
				and instance.Size.Y > 4
			if isWall and isRun then
				local half = instance.Size.X / 2
				local worst, worstAt, missing = 0, nil, false
				-- The probe starts ABOVE the wall's foot and excludes the wall
				-- itself. The first version started 0.4 studs BELOW the foot,
				-- which is already inside the slab the wall stands on, and a
				-- ray cast from inside a part does not hit it - so every wall
				-- in the building reported a 2 stud gap, 2 being the slab
				-- thickness it was passing through. Forty findings, all of
				-- them the probe.
				local selfOnly = RaycastParams.new()
				selfOnly.FilterType = Enum.RaycastFilterType.Exclude
				selfOnly.FilterDescendantsInstances = { instance }
				local step = math.max(6, instance.Size.X / 6)
				local t = -half + 3
				while t <= half - 3 do
					local foot = (instance.CFrame * CFrame.new(t, -instance.Size.Y / 2, 0)).Position
					local below = Workspace:Raycast(
						foot + Vector3.new(0, 2, 0),
						Vector3.new(0, -18, 0),
						selfOnly
					)
					if not below then
						missing = true
					else
						local gap = foot.Y - below.Position.Y
						if gap > worst then
							worst, worstAt = gap, foot
						end
					end
					t += step
				end
				if missing then
					table.insert(findings, {
						kind = "floating",
						detail = `{instance.Name} [{instance.Parent and instance.Parent.Name}] has no floor beneath part of it`,
						at = instance.Position,
					})
				elseif worst > SEAM then
					table.insert(findings, {
						kind = "undercut",
						detail = `{instance.Name} [{instance.Parent and instance.Parent.Name}] stands {string.format(
							"%.1f",
							worst
						)} studs clear of the floor under it`,
						at = worstAt,
					})
				end
			end
		end
	end
	return findings
end

--- Can you actually walk from here to there. Takes a list of named points.
function MapAudit.reachable(from: Vector3, targets: { { name: string, at: Vector3 } }): { Finding }
	local findings: { Finding } = {}
	for _, target in ipairs(targets) do
		local path = PathfindingService:CreatePath({
			AgentRadius = 2.5,
			AgentHeight = 5,
			AgentCanJump = false,
		})
		local ok = pcall(function()
			path:ComputeAsync(from, target.at)
		end)
		if not ok or path.Status ~= Enum.PathStatus.Success then
			table.insert(findings, {
				kind = "nopath",
				detail = `no route to {target.name}`,
				at = target.at,
			})
		else
			local waypoints = path:GetWaypoints()
			local walked = 0
			for i = 2, #waypoints do
				walked += (waypoints[i].Position - waypoints[i - 1].Position).Magnitude
			end
			local straight = (from - target.at).Magnitude
			-- A detour past three is a route that goes somewhere else first,
			-- which is how the sealed escalators showed up.
			if straight > 40 and walked > straight * 3 then
				table.insert(findings, {
					kind = "detour",
					detail = `{target.name}: {math.floor(walked)} studs walked for {math.floor(
						straight
					)} straight (x{string.format("%.1f", walked / straight)})`,
					at = target.at,
				})
			end
		end
	end
	return findings
end

--- Everything, as a printable report.
function MapAudit.Run(model: Model, wallPrefixes: { string }?): string
	local lines = { `=== {model.Name} ===` }
	local function section(title: string, findings: { Finding }, cap: number)
		table.insert(lines, `\n-- {title}`)
		if #findings == 0 then
			table.insert(lines, "   clean")
			return
		end
		for index, finding in ipairs(findings) do
			if index > cap then
				table.insert(lines, `   ... and {#findings - cap} more`)
				break
			end
			local where = if finding.at
				then string.format(" @(%.0f,%.0f,%.0f)", finding.at.X, finding.at.Y, finding.at.Z)
				else ""
			table.insert(lines, `   [{finding.kind}] {finding.detail}{where}`)
		end
	end

	section("lighting", MapAudit.darkFloor(model), 14)
	section("clipping", MapAudit.clipping(model), 20)
	section(
		"walls",
		MapAudit.wallGaps(
			model,
			wallPrefixes
				or { "Wall", "Core", "Landing", "Kitchen", "Museum", "Ballroom", "Lodge", "Shell" }
		),
		14
	)
	return table.concat(lines, "\n")
end

return MapAudit
