--!strict
-- The Johnstown Galleria, greybox.
--
-- Built from the mall's own published floor plan and interior photographs, not
-- from an idea of what a 1992 mall looks like. Two things were established by
-- looking rather than assuming, and both killed an earlier version of this file:
--
--   1. The plan is a DOGLEG, not a cross. One long straight concourse and a
--      second arm kicking off it at an angle. Boscov's closes the straight end;
--      Sears and Bon-Ton sit at the far end of the angled arm; JCPenney is in
--      the crook of the bend. An earlier attempt was a cruciform with four
--      corner anchors - what "four anchors, corner-placed" reasonably suggests,
--      and completely wrong.
--
--   2. The look is teal steel and salmon concrete. The signature is a high
--      gabled roof on exposed teal trusses, salmon columns down the concourse,
--      teal balcony rails and cream tile underfoot. Those four things carry
--      most of the recognition. The shop boxes do not.
--
-- A previous version placed all 97 real tenant units read out of the mall's own
-- glTF model. It was accurate on paper and unreadable in practice: the real
-- outlines are angled and L-shaped, so axis-aligned boxes overlapped in 55
-- places across 66 of the 97 units. Oriented minimum-area rectangles cut that
-- to 40 pairs and could not fix it, because a rectangle cannot be an L.
-- Rasterising them into a union made overlap impossible and produced scattered
-- blobs rather than rooms. A tenant schedule is not architecture.
--
-- Run from the command bar:
--   require(workspace.Map.GalleriaBuilder).Build()
-- Builds Workspace.ClaudeGalleria and touches nothing else, so it can sit
-- beside the other mall for comparison.

local CollectionService = game:GetService("CollectionService")
local Workspace = game:GetService("Workspace")

local GalleriaBuilder = {}

local S = 26 -- studs per model unit, roughly a foot

-- Floor to floor is deliberately taller than a real mall's twenty feet. Chris:
-- "the height from top to bottom should be devastating, you shouldn't be able
-- to jump from floor to floor without dying, i want to force you to use the
-- stairs." A thirty-two stud drop reads as three storeys from the balcony and
-- is well past the lethal threshold the run state service applies.
local F2F = 32
local STORE_H = 26
local HALL = 1.7 -- concourse half-width
local DEPTH = 3.2 -- shop depth

-- The section. Level one is the full width of the concourse; level two is a
-- balcony down each side with an open well between them, so the skylight drops
-- daylight all the way to the tile and you can see the other level from either
-- one. An earlier version decked level two across the whole width, which sealed
-- level one into a lightless corridor and left the balcony rails standing on a
-- floor that had no edge to rail.
local BAL = 1.1 -- balcony depth at level two
local WELL = HALL - BAL -- half-width of the open well

-- The arms stop short of the bend and a round court fills the middle. Round,
-- because the arms meet at 135 degrees and a square plaza there leaves a wedge
-- of nothing on the outside of the corner. Stopping the arms short also keeps
-- the two concourse floors from overlapping at the bend, which would have put
-- two tiled surfaces in the same plane and set them fighting.
local COURT_R = 3.6 -- how far back from the bend each arm starts
local COURT_OUT = 3.9 -- outer radius of the court floor
local LIFT = 0.5 -- the court stands this much proud of the concourse
-- The court's void is far wider than the arms' well. Chris: "that whole center
-- area where the fountain is should all be opened up." At 2.4 the hole is a
-- hundred and twenty five studs across and the fountain sits in daylight.
local COURT_WELL = 2.4

local C = {
	tile = Color3.fromRGB(232, 228, 217),
	teal = Color3.fromRGB(62, 142, 138),
	column = Color3.fromRGB(228, 180, 172),
	shop = Color3.fromRGB(42, 42, 46),
	fascia = Color3.fromRGB(238, 230, 220),
	glass = Color3.fromRGB(180, 210, 218),
	shell = Color3.fromRGB(206, 198, 186),
	brick = Color3.fromRGB(176, 150, 136),
}

-- The concourse: two straight runs meeting at the court, with proportions
-- taken off the Google Earth view rather than guessed. The bend sits near the
-- middle and the arms are close to the same length. An earlier version had one
-- arm twice the other, which put the court in the wrong place and threw every
-- anchor off with it - Chris spotted it by orienting the aerial to match.
-- North is -Z and east is +X, so a plan view of this is directly comparable
-- with the Google Maps image. Boscov's closes the west arm, Sears the
-- south-east arm, and the court is the bend between them.
local BEND = Vector2.new(0, 0)
-- Lengthened from sixteen units an arm. Chris: "the scale of the mall is too
-- small basically, to go from one end to the other should take at least 1 min."
-- At sixteen the whole dogleg was 820 studs, which is under a minute at a
-- walk. It is now about 1340 end to end, and the real Galleria is roughly
-- twelve hundred feet long, so the longer arms are also closer to the plan -
-- the stubby version was wrong twice over.
local ARMS = {
	{ a = BEND, b = Vector2.new(-25.5, 0) }, -- west, to Boscov's
	{ a = BEND, b = Vector2.new(18, 18) }, -- south-east, to Sears
}

local L1_NAMES = {
	"GNC", "WALDENBOOKS", "FOOT LOCKER", "KAY JEWELERS", "SPENCER", "PAYLESS",
	"CLAIRES", "AT&T", "AUNTIE ANNES", "SAM GOODY", "GLOW ROLLER RINK",
	"ISLAND CUISINE", "MISTER JAYS", "RADIO SHACK", "B. DALTON", "THINGS REMEMBERED",
}
local L2_NAMES = {
	"ITALIAN OVEN", "JAKES PUB", "T-MOBILE", "MAURICES", "OPA! GREEK",
	"NAPOLI PIZZA", "GARDNERS CANDIES", "DARK BREWS", "PITT JOHNSTOWN",
	"ARCADE OF THRONES", "ROSALINDAS", "SENTINELS KEEP", "GALLERIA INK",
	"A.O.N. WRESTLING", "SWEATRX", "MORNING STAR",
}

local function part(parent: Instance, name: string, size: Vector3, cf: CFrame, colour: Color3, mat: Enum.Material?, trans: number?): Part
	local p = Instance.new("Part")
	p.Name, p.Size, p.CFrame, p.Color = name, size, cf, colour
	p.Material = mat or Enum.Material.SmoothPlastic
	p.Anchored = true
	p.Transparency = trans or 0
	p.TopSurface, p.BottomSurface = Enum.SurfaceType.Smooth, Enum.SurfaceType.Smooth
	p.Parent = parent
	return p
end

local function label(host: BasePart, text: string, face: Enum.NormalId)
	local g = Instance.new("SurfaceGui")
	g.Face, g.CanvasSize, g.Parent = face, Vector2.new(520, 110), host
	local l = Instance.new("TextLabel")
	l.Size, l.BackgroundTransparency = UDim2.fromScale(1, 1), 1
	l.Font, l.TextScaled = Enum.Font.GothamBold, true
	l.TextColor3, l.Text = Color3.fromRGB(245, 245, 240), text
	l.Parent = g
end

function GalleriaBuilder.Build(): Model
	-- Deliberately NOT inside Workspace.Map. FloodedMallBuilder clears every
	-- child of Map except itself on the first line of its build, so anything
	-- parented there is destroyed the moment the server starts - this mall
	-- included, along with Codex's own FloodedMall and GalleriaDetails models.
	-- Until that is sorted out between the two builders, this one sits beside
	-- Map rather than in it, so it survives a Play session.
	local map = Workspace
	local old = map:FindFirstChild("ClaudeGalleria")
	if old then
		old:Destroy()
	end

	local M = Instance.new("Model")
	M.Name = "ClaudeGalleria"
	local F: { [string]: Folder } = {}
	for _, n in ipairs({ "Floors", "Shops", "Structure", "Anchors", "Roof", "Court", "Escalators", "Fixtures" }) do
		local f = Instance.new("Folder")
		f.Name, f.Parent = n, M
		F[n] = f
	end

	-- A flat disc, for the court and the fountain. A Part cylinder is extruded
	-- along its own X, so it has to be stood on end to lie down.
	local function disc(parent: Instance, name: string, radius: number, y: number, h: number, colour: Color3, mat: Enum.Material?, trans: number?): Part
		local p = part(parent, name, Vector3.new(h, radius * 2 * S, radius * 2 * S), CFrame.new(0, y, 0) * CFrame.Angles(0, 0, math.pi / 2), colour, mat, trans)
		p.Shape = Enum.PartType.Cylinder
		return p
	end

	-- The columns are round and salmon. With the teal trusses that is most of
	-- what makes a photograph of this place recognisable; square posts read as
	-- any mall anywhere. Takes world studs, not model units.
	local function pillar(folder: Instance, name: string, wx: number, wz: number, y: number, h: number, dia: number, colour: Color3): Part
		local p = part(folder, name, Vector3.new(h, dia, dia), CFrame.new(wx, y, wz) * CFrame.Angles(0, 0, math.pi / 2), colour, Enum.Material.Concrete)
		p.Shape = Enum.PartType.Cylinder
		return p
	end

	-- Escalators are one sloped deck with treads drawn on it, not a stack of
	-- steps. Discrete step boxes do not produce a navmesh a Sweeper can path
	-- over - that was learned on the other mall's stairs, where
	-- PathfindingService returned NoPath at every attempt.
	local ESC_W = 14
	-- (x1, z1) is the foot and (x2, z2) is the head. The yaw is taken from that
	-- vector rather than passed in, because the deck's pitch always lifts its
	-- own +X end: given the arm's yaw instead, a run asked to climb back down
	-- the arm was built climbing the other way, and its landing ended up at the
	-- foot, hanging a storey above the floor with the deck rising away from it.
	-- That is why nothing on level two could be reached after the rescale.
	local function escalator(folder: Instance, name: string, x1: number, z1: number, x2: number, z2: number, base: number)
		local run = Vector2.new(x2 - x1, z2 - z1).Magnitude * S
		local hyp = math.sqrt(run * run + F2F * F2F)
		local pitch = math.atan2(F2F, run)
		local yaw = math.atan2(z2 - z1, x2 - x1)
		local cf = CFrame.new((x1 + x2) / 2 * S, base + F2F / 2 - 1, (z1 + z2) / 2 * S)
			* CFrame.Angles(0, -yaw, 0)
			* CFrame.Angles(0, 0, pitch)
		part(folder, `Deck_{name}`, Vector3.new(hyp, 2, ESC_W), cf, C.shell, Enum.Material.Metal)
		for k = 1, 16 do
			local t = ((k - 0.5) / 16 - 0.5) * hyp
			local tread = part(folder, `Tread_{name}_{k}`, Vector3.new(1.2, 0.5, ESC_W - 1), cf * CFrame.new(t, 1.2, 0), C.teal, Enum.Material.Metal)
			tread.CanCollide = false
		end
		for sgn = -1, 1, 2 do
			part(folder, `Balustrade_{name}_{sgn}`, Vector3.new(hyp, 8, 1.4), cf * CFrame.new(0, 5, sgn * (ESC_W / 2 + 0.7)), C.teal, Enum.Material.Metal)
		end
	end

	-- Where a flanking department store cuts through the shop band. Boscov's
	-- and Sears close an arm, so their doors open straight onto the end of the
	-- concourse. JCPenney and Bon-Ton do not - they sit alongside an arm, and
	-- their mall-facing wall lands on the OUTSIDE of the inline shops. Without
	-- opening the band here their entrances face the back of a shoe shop, which
	-- is what pathfinding found: every route into JCPenney failed while
	-- Boscov's was fine.
	local PORTAL_W = 2.6
	local seUnit = (ARMS[2].b - ARMS[2].a).Unit
	local portals = {
		{ arm = 1, side = 1, at = Vector2.new(-13, 0) },
		{ arm = 1, side = 1, at = Vector2.new(-7, 0) },
		{ arm = 2, side = -1, at = seUnit * 8 },
	}

	local suite1, suite2 = 100, 200
	local landings: { { arm: number, side: number, from: number, to: number } } = {}

	for ai, arm in ipairs(ARMS) do
		local dir = (arm.b - arm.a).Unit
		local a = arm.a + dir * COURT_R
		local b = arm.b
		local len = (b - a).Magnitude
		local nrm = Vector2.new(-dir.Y, dir.X)
		local ang = math.atan2(dir.Y, dir.X)
		local midX, midZ = (a.X + b.X) / 2, (a.Y + b.Y) / 2
		local rot = CFrame.Angles(0, -ang, 0)

		-- Everything on an arm is placed as (distance along, distance across).
		-- It is the only way both arms can share one body of code when one of
		-- them runs at 45 degrees.
		local function at(along: number, off: number): (number, number)
			return midX + dir.X * along + nrm.X * off, midZ + dir.Y * along + nrm.Y * off
		end

		part(F.Floors, `Hall{ai}_L1`, Vector3.new(len * S, 2, HALL * 2 * S), CFrame.new(midX * S, -1, midZ * S) * rot, C.tile, Enum.Material.Pebble)
		for side = -1, 1, 2 do
			local bx, bz = at(0, (HALL + WELL) / 2 * side)
			part(F.Floors, `Balcony{ai}_{side}`, Vector3.new(len * S, 2, BAL * S), CFrame.new(bx * S, F2F - 1, bz * S) * rot, C.tile, Enum.Material.Pebble)
			-- The edge band seen from the floor below: a cream soffit under the
			-- balcony slab, which is most of what reads as this mall from L1.
			local ex, ez = at(0, WELL * side)
			part(F.Structure, `Soffit{ai}_{side}`, Vector3.new(len * S, 5, 1.6), CFrame.new(ex * S, F2F - 4.5, ez * S) * rot, C.fascia)
		end

		-- Two escalators per arm, on opposite sides. Each sits in the well with
		-- its upper edge flush to the balcony edge, so the top of the run lands
		-- on the balcony with nothing to bridge.
		for k, frac in ipairs({ 0.3, 0.68 }) do
			local centre = -len / 2 + len * frac
			local run = 2.0
			-- A pair side by side in the middle of the well, which is how every
			-- photograph of this place shows them, with a landing at the head
			-- wide enough to reach both balconies. That landing doubles as the
			-- bridge across the well. The earlier single deck hugged one edge
			-- and there was no way to cross.
			local sense = if k == 1 then 1 else -1
			for e = -1, 1, 2 do
				local off = e * (ESC_W / 2 + 0.8) / S
				local x1, z1 = at(centre - sense * run / 2, off)
				local x2, z2 = at(centre + sense * run / 2, off)
				escalator(F.Escalators, `{ai}_{k}_{e}`, x1, z1, x2, z2, 0)
			end
			local head = centre + sense * (run / 2 + 0.45)
			local hx, hz = at(head, 0)
			part(F.Floors, `Landing{ai}_{k}`, Vector3.new(0.95 * S, 2, WELL * 2 * S), CFrame.new(hx * S, F2F - 1, hz * S) * rot, C.tile, Enum.Material.Pebble)
			-- The far end of the landing is a two storey drop into the well.
			local ex, ez = at(head + sense * 0.5, 0)
			part(F.Structure, `LandingRail{ai}_{k}`, Vector3.new(1, 3.4, WELL * 2 * S), CFrame.new(ex * S, F2F + 1.7, ez * S) * rot, C.fascia)
			part(F.Structure, `LandingCap{ai}_{k}`, Vector3.new(1.8, 1.2, WELL * 2 * S), CFrame.new(ex * S, F2F + 4, ez * S) * rot, C.teal, Enum.Material.Metal)
			for side = -1, 1, 2 do
				local lo = math.min(centre - run / 2, head) - 0.55
				local hi = math.max(centre + run / 2, head) + 0.55
				table.insert(landings, { arm = ai, side = side, from = lo, to = hi })
			end
		end

		-- The food court: upper level, decked right across the well, counters
		-- down one flank and the tables out over the drop. Chris: "we aint got
		-- no food court". The Galleria's own sits exactly like this, up a level
		-- and open to the court below.
		if ai == 2 then
			local fcAt = -len / 2 + len * 0.49
			local fcLen = 4.0
			local fx, fz = at(fcAt, 0)
			part(F.Floors, "FoodCourtDeck", Vector3.new(fcLen * S, 2, WELL * 2 * S), CFrame.new(fx * S, F2F - 1, fz * S) * rot, C.tile, Enum.Material.Pebble)
			for side = -1, 1, 2 do
				table.insert(landings, { arm = ai, side = side, from = fcAt - fcLen / 2 - 0.1, to = fcAt + fcLen / 2 + 0.1 })
			end
			for e = -1, 1, 2 do
				local ex, ez = at(fcAt + e * (fcLen / 2 + 0.03), 0)
				part(F.Structure, `FCRail_{e}`, Vector3.new(1, 3.4, WELL * 2 * S), CFrame.new(ex * S, F2F + 1.7, ez * S) * rot, C.fascia)
				part(F.Structure, `FCCap_{e}`, Vector3.new(1.8, 1.2, WELL * 2 * S), CFrame.new(ex * S, F2F + 4, ez * S) * rot, C.teal, Enum.Material.Metal)
			end
			-- One row of tables down the middle, not two rows across. The deck
			-- is only the width of the well - about thirty studs - and tables
			-- seventeen studs wide set either side of the centre overhung both
			-- edges and sealed the deck off from the balcony entirely.
			local seats = 6
			for i = 0, seats - 1 do
				local t = fcAt - fcLen / 2 + fcLen * ((i + 0.5) / seats)
				local tx, tz = at(t, 0)
				part(F.Fixtures, `FCPedestal_{i}`, Vector3.new(4, 4, 4), CFrame.new(tx * S, F2F + 2, tz * S) * rot, C.teal, Enum.Material.Metal)
				part(F.Fixtures, `FCTable_{i}`, Vector3.new(12, 1.2, 12), CFrame.new(tx * S, F2F + 4.6, tz * S) * rot, C.fascia, Enum.Material.WoodPlanks)
				for c = -1, 1, 2 do
					local sx, sz = at(t + c * 0.26, 0)
					part(F.Fixtures, `FCChair_{i}_{c}`, Vector3.new(7, 3, 9), CFrame.new(sx * S, F2F + 1.5, sz * S) * rot, C.column, Enum.Material.WoodPlanks)
				end
			end
		end

		-- The balcony rail runs the length of the well except where an escalator
		-- comes up through it; its own balustrades guard those stretches.
		for side = -1, 1, 2 do
			local segs = math.max(6, math.floor(len / 0.5))
			for i = 0, segs - 1 do
				local t0 = -len / 2 + len * (i / segs)
				local t1 = -len / 2 + len * ((i + 1) / segs)
				local blocked = false
				for _, l in ipairs(landings) do
					if l.arm == ai and l.side == side and t1 > l.from and t0 < l.to then
						blocked = true
					end
				end
				if not blocked then
					-- A pale panel under a teal cap, not a slab of teal. The cap
					-- is the line that reads along the whole balcony in the
					-- photographs.
					local rx, rz = at((t0 + t1) / 2, WELL * side)
					local wlen = (t1 - t0) * S + 0.4
					part(F.Structure, `RailPanel{ai}_{side}_{i}`, Vector3.new(wlen, 3.4, 0.9), CFrame.new(rx * S, F2F + 1.7, rz * S) * rot, C.fascia)
					part(F.Structure, `RailCap{ai}_{side}_{i}`, Vector3.new(wlen, 1.2, 1.8), CFrame.new(rx * S, F2F + 4, rz * S) * rot, C.teal, Enum.Material.Metal)
				end
			end
		end

		local bays = math.floor(len / 1.55)
		local function portalAt(t: number, sideOf: number): boolean
			for _, p in ipairs(portals) do
				if p.arm == ai and p.side == sideOf then
					local a0 = (p.at - Vector2.new(midX, midZ)):Dot(dir)
					if math.abs(t - a0) < PORTAL_W / 2 then
						return true
					end
				end
			end
			return false
		end

		for side = -1, 1, 2 do
			-- Snap each portal to the bays it actually swallows. Sizing the
			-- corridor to the nominal portal width instead left a strip of
			-- missing floor at each jamb, and the three-stud slot between two
			-- adjacent bays became a hole straight through to level one.
			local corridors = {}
			for _, p in ipairs(portals) do
				if p.arm == ai and p.side == side then
					local a0 = (p.at - Vector2.new(midX, midZ)):Dot(dir)
					local lo, hi = math.huge, -math.huge
					for i = 0, bays - 1 do
						local bAt = -len / 2 + len * ((i + 0.5) / bays)
						local bw = (len / bays) - 0.12
						if math.abs(bAt - a0) < PORTAL_W / 2 then
							lo = math.min(lo, bAt - bw / 2 - 0.06)
							hi = math.max(hi, bAt + bw / 2 + 0.06)
						end
					end
					if lo < hi then
						table.insert(corridors, { lo = lo, hi = hi })
					end
				end
			end
			table.sort(corridors, function(p, q)
				return p.lo < q.lo
			end)

			-- One outer shell per side, in runs, with a hole at each corridor.
			local spans, cursor = {}, -len / 2
			for _, c in ipairs(corridors) do
				if c.lo > cursor then
					table.insert(spans, { cursor, c.lo })
				end
				cursor = math.max(cursor, c.hi)
			end
			if cursor < len / 2 then
				table.insert(spans, { cursor, len / 2 })
			end
			for si, sp in ipairs(spans) do
				local smid, slen = (sp[1] + sp[2]) / 2, sp[2] - sp[1]
				local sx, sz = at(smid, (HALL + DEPTH) * side)
				part(F.Shops, `Shell{ai}_{side}_{si}`, Vector3.new(slen * S, F2F * 2, 2), CFrame.new(sx * S, F2F, sz * S) * rot, C.shell, Enum.Material.Concrete)
			end

			-- The corridor itself: one unbroken floor and ceiling per level,
			-- with a jamb each side.
			for ci, c in ipairs(corridors) do
				local cmid, clen = (c.lo + c.hi) / 2, c.hi - c.lo
				for lvl = 1, 2 do
					local y = (lvl - 1) * F2F
					local px, pz = at(cmid, (HALL + DEPTH / 2) * side)
					part(F.Shops, `PortalFloor_{ai}_{side}_{ci}_{lvl}`, Vector3.new(clen * S, 2, DEPTH * S), CFrame.new(px * S, y - 1, pz * S) * rot, C.tile, Enum.Material.Pebble)
					part(F.Shops, `PortalCeil_{ai}_{side}_{ci}_{lvl}`, Vector3.new(clen * S, 1, DEPTH * S), CFrame.new(px * S, y + STORE_H, pz * S) * rot, C.fascia)
					for e = -1, 1, 2 do
						local jx, jz = at(cmid + e * clen / 2, (HALL + DEPTH / 2) * side)
						part(F.Shops, `PortalJamb_{ai}_{side}_{ci}_{lvl}_{e}`, Vector3.new(2, STORE_H, DEPTH * S), CFrame.new(jx * S, y + STORE_H / 2, jz * S) * rot, C.fascia)
					end
				end
			end

			for i = 0, bays - 1 do
				local along = -len / 2 + len * ((i + 0.5) / bays)
				local bw = (len / bays) - 0.12
				if portalAt(along, side) then
					continue
				end
				for lvl = 1, 2 do
					local y = (lvl - 1) * F2F
					local tag = `L{lvl}_{ai}_{side}_{i}`
					local cx, cz = at(along, (HALL + DEPTH / 2) * side)

					-- A bay is a room, not a block. Most are shuttered, which is
					-- what a dead mall looks like and also means the ones you can
					-- walk into are the ones worth walking into.
					part(F.Shops, `BayFloor_{tag}`, Vector3.new(bw * S, 1, DEPTH * S), CFrame.new(cx * S, y - 0.5, cz * S) * rot, C.tile, Enum.Material.Pebble)
					part(F.Shops, `BayCeil_{tag}`, Vector3.new(bw * S, 1, DEPTH * S), CFrame.new(cx * S, y + STORE_H, cz * S) * rot, C.shop)
					for e = -1, 1, 2 do
						local dx, dz = at(along + e * bw / 2, (HALL + DEPTH / 2) * side)
						part(F.Shops, `Divider_{tag}_{e}`, Vector3.new(1.5, STORE_H, DEPTH * S), CFrame.new(dx * S, y + STORE_H / 2, dz * S) * rot, C.shop, Enum.Material.Slate)
					end

						local fx, fz = at(along, (HALL + 0.08) * side)
					if (i + ai + lvl) % 3 == 0 then
						local doorW = 14
						local paneW = (bw * S - doorW) / 2
						for e = -1, 1, 2 do
							local px, pz = at(along + e * (doorW / 2 + paneW / 2) / S, (HALL + 0.08) * side)
							part(F.Shops, `Glass_{tag}_{e}`, Vector3.new(paneW, STORE_H - 5, 1), CFrame.new(px * S, y + (STORE_H - 5) / 2, pz * S) * rot, C.glass, Enum.Material.Glass, 0.45)
						end
					else
						part(F.Shops, `Shutter_{tag}`, Vector3.new(bw * S, STORE_H - 5, 1.2), CFrame.new(fx * S, y + (STORE_H - 5) / 2, fz * S) * rot, C.shop, Enum.Material.DiamondPlate)
					end

					local fascia = part(F.Shops, `Fascia_{tag}`, Vector3.new(bw * S, 4.5, 1.4), CFrame.new(fx * S, y + STORE_H - 2.5, fz * S) * rot, C.fascia)
					local list = if lvl == 1 then L1_NAMES else L2_NAMES
					local n
					if lvl == 1 then
						n = suite1
						suite1 += 2
					else
						n = suite2
						suite2 += 2
					end
					-- Front is a part's -Z face and the fascia's local +Z points away
					-- from the concourse, so the side that faces a shopper is -Z on
					-- the positive flank and +Z on the negative one. This was the
					-- wrong way round, which put every sign inside its own shop.
					label(fascia, `{list[((i + ai * 3) % #list) + 1]}   {n}`, if side == 1 then Enum.NormalId.Front else Enum.NormalId.Back)
					part(F.Shops, `Trim_{tag}`, Vector3.new(bw * S, 1.2, 1.8), CFrame.new(fx * S, y + STORE_H - 5.2, fz * S) * rot, C.teal)
				end
			end
		end

		-- Salmon columns and teal trusses: most of the recognition.
		local cols = math.floor(len / 1.9)
		for i = 0, cols do
			local along = -len / 2 + len * (i / cols)
			for side = -1, 1, 2 do
				local cx, cz = at(along, (HALL - 0.18) * side)
				pillar(F.Structure, `Column{ai}_{i}_{side}`, cx * S, cz * S, F2F - 1, F2F * 2 - 2, 10, C.column)
				-- The teal uplighter bowl near the head of each column.
				part(F.Structure, `Uplight{ai}_{i}_{side}`, Vector3.new(15, 3, 15), CFrame.new(cx * S, F2F * 2 - 11, cz * S), C.teal, Enum.Material.Metal)
			end
			local tx, tz = at(along, 0)
			part(F.Roof, `Truss{ai}_{i}`, Vector3.new(2.2, 2.2, HALL * 2 * S), CFrame.new(tx * S, F2F * 2 + 10, tz * S) * rot, C.teal, Enum.Material.Metal)
		end

		-- Planters, benches and kiosks down the middle of the concourse. The
		-- photographs are full of them, and an empty corridor reads as a
		-- corridor rather than as a mall.
		local props = math.max(2, math.floor(len / 2.2))
		for i = 0, props - 1 do
			local t = -len / 2 + len * ((i + 0.5) / props)
			-- Keep the middle of the concourse clear at the ends, where the
			-- department store doors are, and under the escalators, whose feet
			-- land on this floor. A kiosk had parked itself squarely in
			-- Boscov's entrance.
			local clear = math.abs(t) < len / 2 - 1.8
			for _, l in ipairs(landings) do
				if l.arm == ai and t > l.from - 0.6 and t < l.to + 0.6 then
					clear = false
				end
			end
			local px, pz = at(t, 0)
			local kind = i % 3
			if clear then
			if kind == 0 then
				part(F.Fixtures, `Planter{ai}_{i}`, Vector3.new(30, 7, 18), CFrame.new(px * S, 2.5, pz * S) * rot, C.column, Enum.Material.Concrete)
				part(F.Fixtures, `Shrub{ai}_{i}`, Vector3.new(25, 10, 14), CFrame.new(px * S, 10, pz * S) * rot, Color3.fromRGB(74, 112, 72), Enum.Material.Grass)
			elseif kind == 1 then
				for b = -1, 1, 2 do
					local bx, bz = at(t, b * 0.3)
					part(F.Fixtures, `Bench{ai}_{i}_{b}`, Vector3.new(36, 3, 11), CFrame.new(bx * S, 4.5, bz * S) * rot, C.fascia, Enum.Material.WoodPlanks)
				end
			else
				part(F.Fixtures, `Kiosk{ai}_{i}`, Vector3.new(46, 15, 27), CFrame.new(px * S, 7.5, pz * S) * rot, C.fascia)
				part(F.Fixtures, `KioskRoof{ai}_{i}`, Vector3.new(54, 2, 35), CFrame.new(px * S, 16, pz * S) * rot, C.teal, Enum.Material.Metal)
			end
			end
		end

		for s = -1, 1, 2 do
			local gx, gz = at(0, HALL * 0.5 * s)
			part(F.Roof, `Glass{ai}_{s}`, Vector3.new(len * S, 1.2, HALL * 1.25 * S), CFrame.new(gx * S, F2F * 2 + 16, gz * S) * rot * CFrame.Angles(math.rad(s * 24), 0, 0), C.glass, Enum.Material.Glass, 0.55)
		end
		-- A flat deck over the shop bands. The gable only spans the concourse,
		-- so without this every bay is open to the sky and the mall reads from
		-- above as two rows of topless boxes beside a glass tube.
		for side = -1, 1, 2 do
			local dx, dz = at(0, (HALL + DEPTH / 2) * side)
			part(F.Roof, `Deck{ai}_{side}`, Vector3.new(len * S, 2, DEPTH * S), CFrame.new(dx * S, F2F * 2 + 1, dz * S) * rot, C.shell, Enum.Material.Concrete)
		end
		part(F.Roof, `Ridge{ai}`, Vector3.new(len * S, 2.6, 2.6), CFrame.new(midX * S, F2F * 2 + 24, midZ * S) * rot, C.teal, Enum.Material.Metal)
	end

	-- The court.
	-- The rim is a skirt below the floor, not a disc level with it. Sized to
	-- the same top it would z-fight the floor across the whole court, which
	-- is the striping that showed up the moment the court was first built.
	disc(F.Court, "CourtRim", COURT_OUT + 0.06, -1, 2.4, C.teal, Enum.Material.Metal)
	disc(F.Court, "CourtFloor", COURT_OUT, -1 + LIFT, 2, C.tile, Enum.Material.Pebble)

	-- The balcony carries round the court as two arcs. It stops where each arm's
	-- well opens into the court's, because that opening is the point of a court:
	-- one continuous void from the skylight down to the fountain.
	local RING = 30
	local rMid = (COURT_WELL + COURT_OUT) / 2
	for i = 0, RING - 1 do
		local th = 2 * math.pi * (i + 0.5) / RING
		local p = Vector2.new(math.cos(th), math.sin(th)) * rMid
		local blocked = false
		for _, arm in ipairs(ARMS) do
			local dir = (arm.b - arm.a).Unit
			local nrm = Vector2.new(-dir.Y, dir.X)
			if p:Dot(dir) > 0 and math.abs(p:Dot(nrm)) < WELL + 0.3 then
				blocked = true
			end
		end
		if not blocked then
			-- Sized on the OUTER chord, not the mid radius: a box sized for the
			-- middle leaves a wedge of nothing at the outer edge of every joint,
			-- and the court comes out looking like a saw blade.
			local w = 2 * math.pi * COURT_OUT / RING * 1.08
			-- Neighbouring segments overlap, because a box sized to the outer
			-- chord has to. Overlapping boxes that share a top plane stripe
			-- against each other, so every other one is lifted by a hair -
			-- invisible underfoot, and enough that nothing fights.
			local jitter = (i % 2) * 0.05
			local yaw = CFrame.Angles(0, -(th + math.pi / 2), 0)
			part(F.Court, `CourtBalcony_{i}`, Vector3.new(w * S, 2, (COURT_OUT - COURT_WELL) * S), CFrame.new(p.X * S, F2F - 1 + LIFT + jitter, p.Y * S) * yaw, C.tile, Enum.Material.Pebble)
			-- The court is wider than the concourse, so the arms' outer walls do
			-- not reach round it. Without this the court is open to the sky on
			-- both flanks and you can see straight out of the building.
			local wp = Vector2.new(math.cos(th), math.sin(th)) * (COURT_OUT + 0.1)
			part(F.Court, `CourtWall_{i}`, Vector3.new(w * S, F2F * 2, 2), CFrame.new(wp.X * S, F2F, wp.Y * S) * yaw, C.shell, Enum.Material.Concrete)
			local r = Vector2.new(math.cos(th), math.sin(th)) * COURT_WELL
			part(F.Court, `CourtRail_{i}`, Vector3.new(2 * math.pi * COURT_WELL / RING * S * 1.25, 3.5, 1), CFrame.new(r.X * S, F2F + 1.75 + LIFT + jitter, r.Y * S) * yaw, C.teal, Enum.Material.Metal)
		end
	end

	disc(F.Court, "FountainBasin", 0.5, 3, 8, C.teal, Enum.Material.Marble)
	disc(F.Court, "FountainWater", 0.44, 6.9, 0.8, C.glass, Enum.Material.Glass, 0.35)
	part(F.Court, "FountainStem", Vector3.new(6, 15, 6), CFrame.new(0, 10, 0), C.column, Enum.Material.Concrete)

	-- The same deck carried round the court, open in the middle so the court
	-- keeps its skylight. The opening has to be at least as wide as the court
	-- void, or the atrium is roofed over just outside the rail and the fountain
	-- sits under a ceiling.
	for i = 0, 19 do
		local th = 2 * math.pi * (i + 0.5) / 20
		local rr = (COURT_WELL + 0.1 + COURT_OUT + 0.2) / 2
		local p = Vector2.new(math.cos(th), math.sin(th)) * rr
		local w = 2 * math.pi * rr / 20 * 1.4
		part(F.Roof, `CourtDeck_{i}`, Vector3.new(w * S, 2, (COURT_OUT + 0.2 - COURT_WELL - 0.1) * S), CFrame.new(p.X * S, F2F * 2 + 1 + (i % 2) * 0.05, p.Y * S) * CFrame.Angles(0, -(th + math.pi / 2), 0), C.shell, Enum.Material.Concrete)
	end

	-- The clock. It is in half the photographs of this place, standing at the
	-- edge of the court where both arms can see it.
	do
		local a = math.pi * 0.78
		local kx, kz = math.cos(a) * (COURT_WELL + 0.5) * S, math.sin(a) * (COURT_WELL + 0.5) * S
		pillar(F.Court, "ClockPost", kx, kz, F2F * 0.95, F2F * 1.9, 11, C.column)
		local face = part(F.Court, "ClockFace", Vector3.new(6, 26, 26), CFrame.new(kx, F2F * 1.85, kz) * CFrame.Angles(0, 0, math.pi / 2), C.fascia)
		face.Shape = Enum.PartType.Cylinder
		part(F.Court, "ClockRim", Vector3.new(4, 30, 30), CFrame.new(kx, F2F * 1.85, kz) * CFrame.Angles(0, 0, math.pi / 2), C.teal, Enum.Material.Metal).Shape = Enum.PartType.Cylinder
	end

	-- A dark band of tile round the fountain, the way the floor pattern rings
	-- it in the photographs.
	disc(F.Court, "FountainBand", 0.95, -1 + LIFT + 0.05, 2, Color3.fromRGB(58, 60, 64), Enum.Material.Pebble)

	disc(F.Roof, "CourtSkylight", COURT_OUT - 0.15, F2F * 2 + 20, 1.2, C.glass, Enum.Material.Glass, 0.55)
	-- Stepped, not stacked: eight full-diameter bars at one height all
	-- cross at the hub and fight there. A couple of studs of rise each turns
	-- the crossing into a lattice instead.
	for i = 0, 7 do
		part(F.Roof, `CourtTruss_{i}`, Vector3.new(COURT_OUT * 2 * S, 2.2, 2.2), CFrame.new(0, F2F * 2 + 17 + i * 0.25, 0) * CFrame.Angles(0, -math.pi * i / 8, 0), C.teal, Enum.Material.Metal)
	end

	-- A glass lift in the court. Chris asked for one by the fountain; it is
	-- scenery for now, in that the car does not travel - making it carry a
	-- player properly is gameplay code, not geometry, and a lift that looks
	-- rideable and is not would be worse than no lift at all.
	do
		local lx, lz = -1.3, -1.3
		local shaft = 1.0
		for i = 0, 3 do
			local a = math.pi / 2 * i
			local ox, oz = math.cos(a) * shaft / 2, math.sin(a) * shaft / 2
			local wide = if i % 2 == 0 then 0.08 else shaft
			local deep = if i % 2 == 0 then shaft else 0.08
			part(F.Court, `LiftGlass_{i}`, Vector3.new(wide * S, F2F + 14, deep * S), CFrame.new((lx + ox) * S, (F2F + 14) / 2, (lz + oz) * S), C.glass, Enum.Material.Glass, 0.6)
		end
		for i = 0, 3 do
			local a = math.pi / 2 * i + math.pi / 4
			part(F.Court, `LiftPost_{i}`, Vector3.new(3, F2F + 16, 3), CFrame.new((lx + math.cos(a) * shaft * 0.72) * S, (F2F + 16) / 2, (lz + math.sin(a) * shaft * 0.72) * S), C.teal, Enum.Material.Metal)
		end
		part(F.Court, "LiftCar", Vector3.new(shaft * S - 5, 2, shaft * S - 5), CFrame.new(lx * S, 2, lz * S), C.shell, Enum.Material.Metal)
		part(F.Court, "LiftHead", Vector3.new(shaft * S + 5, 4, shaft * S + 5), CFrame.new(lx * S, F2F + 16, lz * S), C.teal, Enum.Material.Metal)
	end

	-- Department stores. They were solid brick blocks, which is what Chris hit:
	-- "i can't go into any of the dept stores, but when i can, they should also
	-- be highly populated, those are the primary areas, they should be huge,
	-- like a real dept. store. with escalators in there too."
	--
	-- So each one is now a building: two floors, a perimeter wall with a real
	-- opening onto the mall, its own escalator pair through a void in the upper
	-- slab, a column grid and rows of fixtures. At fourteen by fifteen units
	-- that is about 130,000 square feet a floor, which is a true Boscov's.
	local WALL_T = 3 / S -- wall thickness, in model units
	local DOORW = 2.2 -- mall entrance width, in model units
	local DOOR_H = 26

	local function department(name: string, cx: number, cz: number, w: number, d: number, yaw: number, front: string, doors: { number }, doorsUp: { number })
		local H = F2F * 2
		local rot = CFrame.Angles(0, -yaw, 0)
		local cos, sin = math.cos(yaw), math.sin(yaw)
		-- Local plan coordinates, so each store can be written as though it were
		-- square to the page and then turned to meet whichever arm it serves.
		local function at(lx: number, lz: number): (number, number)
			return cx + lx * cos - lz * sin, cz + lx * sin + lz * cos
		end
		local function box(folder: Instance, nm: string, lx: number, lz: number, sx: number, sz: number, y: number, h: number, colour: Color3, mat: Enum.Material?, trans: number?): Part
			local wx, wz = at(lx, lz)
			return part(folder, nm, Vector3.new(sx * S, h, sz * S), CFrame.new(wx * S, y, wz * S) * rot, colour, mat, trans)
		end

		-- The void in the upper slab that the escalators rise through.
		local vw, vd = 3.2, 3.0

		-- Keep an aisle clear in line with every door, front, upper and rear.
		-- A tall clothes rack had planted itself directly inside Boscov's back
		-- door, which is the one the extraction picker may choose, and no route
		-- to it existed.
		local aisleAcross: { number } = { 0 } -- the rear door is always centred
		local frontIsX = front:sub(2) == "X"
		for _, list in ipairs({ doors, doorsUp }) do
			for _, g in ipairs(list) do
				table.insert(aisleAcross, g)
			end
		end
		local function onAisle(lx: number, lz: number): boolean
			local v = if frontIsX then lz else lx
			for _, a in ipairs(aisleAcross) do
				if math.abs(v - a) < 0.85 then
					return true
				end
			end
			return false
		end

		box(F.Anchors, `Floor1_{name}`, 0, 0, w, d, -1, 2, C.tile, Enum.Material.Pebble)
		-- Level two is four slabs around the void rather than one slab, because
		-- an escalator needs somewhere to come up.
		box(F.Anchors, `Floor2_{name}_W`, -(w + vw) / 4, 0, (w - vw) / 2, d, F2F - 1, 2, C.tile, Enum.Material.Pebble)
		box(F.Anchors, `Floor2_{name}_E`, (w + vw) / 4, 0, (w - vw) / 2, d, F2F - 1, 2, C.tile, Enum.Material.Pebble)
		box(F.Anchors, `Floor2_{name}_N`, 0, -(d + vd) / 4, vw, (d - vd) / 2, F2F - 1, 2, C.tile, Enum.Material.Pebble)
		box(F.Anchors, `Floor2_{name}_S`, 0, (d + vd) / 4, vw, (d - vd) / 2, F2F - 1, 2, C.tile, Enum.Material.Pebble)
		box(F.Anchors, `Roof_{name}`, 0, 0, w, d, H + 1, 2, C.shell, Enum.Material.Concrete)

		-- Rails round the void, so the drop is guarded but visible.
		-- The rail round the void has to break where each escalator arrives.
		-- Run continuously it stands squarely across the head of the run, and
		-- no route to a store's upper floor exists at all - which is what
		-- pathfinding found for all four of them.
		local ESC_GAP = ESC_W / S / 2 + 0.12
		for sgn = -1, 1, 2 do
			local land = sgn * 0.62 -- where this side's escalator comes up
			for _, seg in ipairs({ { -vd / 2, land - ESC_GAP }, { land + ESC_GAP, vd / 2 } }) do
				local lo, hi = seg[1], seg[2]
				if hi - lo > 0.05 then
					box(F.Anchors, `VoidRail_{name}_X{sgn}_{math.floor(lo * 100)}`, sgn * vw / 2, (lo + hi) / 2, 0.06, hi - lo, F2F + 1.75, 3.5, C.teal, Enum.Material.Metal)
				end
			end
			box(F.Anchors, `VoidRail_{name}_Z{sgn}`, 0, sgn * vd / 2, vw, 0.06, F2F + 1.75, 3.5, C.teal, Enum.Material.Metal)
		end

		-- Perimeter. Only the face that meets the mall is cut for doors.
		-- The wall is built in four horizontal bands rather than one slab with
		-- lintels punched through it, because the two floors do not want their
		-- doors in the same place. A store that closes an arm meets the
		-- concourse at level one on the centreline, but at level two the middle
		-- is the open well - the balconies run either side of it. Boscov's
		-- upper entrance was a doorway onto thin air, two storeys above the
		-- tile, which is why nothing could reach its first floor.
		local bands = {
			{ 0, DOOR_H, doors },
			{ DOOR_H, F2F, {} },
			{ F2F, F2F + DOOR_H, doorsUp },
			{ F2F + DOOR_H, H, {} },
		}
		-- The face opposite the mall gets a ground-floor way out. Chris, on the
		-- run shape: every match activates one extraction, "selected from
		-- far-side exits, never near the active spawn", and each store should
		-- offer "at least one viable path through it toward the randomized
		-- far-side exit". These are the candidates; which one goes live on a
		-- given run is the run logic's call, not the map's.
		local backFace = (if front:sub(1, 1) == "+" then "-" else "+") .. front:sub(2)
		for _, face in ipairs({ { "X", 1 }, { "X", -1 }, { "Z", 1 }, { "Z", -1 } }) do
			local axis, sgn = face[1] :: string, face[2] :: number
			local along = if axis == "X" then d else w
			local here = (if sgn > 0 then "+" else "-") .. axis
			local isFront = front == here
			local isBack = backFace == here
			for bi, band in ipairs(bands) do
				local y0, y1 = band[1] :: number, band[2] :: number
				local gaps = if isFront then band[3] :: { number } elseif isBack and bi == 1 then { 0 } else {}
				local cuts = table.clone(gaps)
				table.sort(cuts)
				local runs = {}
				local cursor = -along / 2
				for _, g in ipairs(cuts) do
					if g - DOORW / 2 > cursor then
						table.insert(runs, { cursor, g - DOORW / 2 })
					end
					cursor = math.max(cursor, g + DOORW / 2)
				end
				if cursor < along / 2 then
					table.insert(runs, { cursor, along / 2 })
				end
				for i, run in ipairs(runs) do
					local mid, rlen = (run[1] + run[2]) / 2, run[2] - run[1]
					if rlen <= 0.001 then
						continue
					end
					if axis == "X" then
						box(F.Anchors, `Wall_{name}_{axis}{sgn}_{bi}_{i}`, sgn * w / 2, mid, WALL_T, rlen, (y0 + y1) / 2, y1 - y0, C.brick, Enum.Material.Brick)
					else
						box(F.Anchors, `Wall_{name}_{axis}{sgn}_{bi}_{i}`, mid, sgn * d / 2, rlen, WALL_T, (y0 + y1) / 2, y1 - y0, C.brick, Enum.Material.Brick)
					end
				end
			end
		end

		-- A landing outside the back door, flush with the shop floor, carrying
		-- the marker the extraction picker reads.
		do
			local isX = backFace:sub(2) == "X"
			local sgn = if backFace:sub(1, 1) == "+" then 1 else -1
			local ox = if isX then sgn * (w / 2 + 1.1) else 0
			local oz = if isX then 0 else sgn * (d / 2 + 1.1)
			box(F.Anchors, `ExitApron_{name}`, ox, oz, if isX then 2.4 else 3.0, if isX then 3.0 else 2.4, -1, 2, C.shell, Enum.Material.Concrete)
			-- The marker does not carry anyone: a raised slab this thin is not
			-- picked up as ground, and every route that ended standing on it
			-- came back NoPath while the apron an inch away was reachable. It
			-- is paint on the floor, and the apron underneath is the floor.
			local pad = box(F.Anchors, `Exit_{name}`, ox, oz, 1.5, 1.5, 0.3, 0.6, C.teal, Enum.Material.Metal)
			pad.CanCollide = false
			pad:SetAttribute("Store", name)
			CollectionService:AddTag(pad, "ExtractCandidate")
		end

		box(F.Anchors, `Parapet_{name}`, 0, 0, w + 0.12, d + 0.12, H + 5, 6, C.shell, Enum.Material.Concrete)

		-- Sign band on the face that meets the mall.
		do
			local isX = front:sub(2) == "X"
			local sgn = if front:sub(1, 1) == "+" then 1 else -1
			local bandLen = (if isX then d else w) * 0.5
			local band
			if isX then
				band = box(F.Anchors, `Sign_{name}`, sgn * (w / 2 + 0.06), 0, 0.08, bandLen, H - 18, 22, C.teal)
			else
				band = box(F.Anchors, `Sign_{name}`, 0, sgn * (d / 2 + 0.06), bandLen, 0.08, H - 18, 22, C.teal)
			end
			for _, f in ipairs({ Enum.NormalId.Front, Enum.NormalId.Back, Enum.NormalId.Left, Enum.NormalId.Right }) do
				label(band, name, f)
			end
		end

		-- One escalator up and one down, through the void, along the store's
		-- long axis so each run lands on the slab rather than in the hole.
		for k, sgn in ipairs({ -1, 1 }) do
			local z = sgn * 0.62
			local ax, az = at(-sgn * vw / 2, z)
			local bx, bz = at(sgn * vw / 2, z)
			escalator(F.Escalators, `{name}_{k}`, ax, az, bx, bz, 0)
		end

		-- Column grid, skipping the void.
		local gx, gz = math.max(2, math.floor(w / 3.2)), math.max(2, math.floor(d / 3.2))
		for i = 0, gx do
			for j = 0, gz do
				local lx = -w / 2 + w * (i / gx)
				local lz = -d / 2 + d * (j / gz)
				-- The circulation zone is the void PLUS an apron either side of
				-- it, because that apron is where the escalators land. Keeping
				-- things out of the void alone was not enough: a clothes rack
				-- stood at the head of both of Sears' runs and nothing could
				-- reach its upper floor.
				-- Columns keep off the door aisles too. One stood dead centre in
				-- Boscov's rear doorway, splitting a fifty-seven stud opening
				-- into two slots that nothing would path through.
				local inWay = (math.abs(lx) < vw / 2 + 1.4 and math.abs(lz) < vd / 2 + 0.3)
					or onAisle(lx, lz)
				if not inWay then
					local px, pz = at(lx, lz)
					pillar(F.Structure, `DeptColumn_{name}_{i}_{j}`, px * S, pz * S, H / 2, H, 10, C.column)
				end
			end
		end

		-- Fixtures: rows of racks and counters on both floors. A department
		-- store that is an empty shed reads as a car park, and these are the
		-- rooms the run is actually about.
		local rows = math.max(3, math.floor(d / 1.1))
		for lvl = 0, 1 do
			local y = lvl * F2F
			for r = 0, rows - 1 do
				local lz = -d / 2 + d * ((r + 0.5) / rows)
				if math.abs(lz) > vd / 2 + 0.25 or lvl == 0 then
					for c = 0, 3 do
						local lx = -w / 2 + w * ((c + 0.5) / 4)
						local inWay = (math.abs(lx) < vw / 2 + 1.4 and math.abs(lz) < vd / 2 + 0.3)
							or onAisle(lx, lz)
						if not inWay then
							local tall = (r + c) % 3 == 0
							box(F.Fixtures, `Rack_{name}_{lvl}_{r}_{c}`, lx, lz, w / 5.4, 0.28, y + (if tall then 7 else 3.5), if tall then 14 else 7, if tall then C.shop else C.fascia, Enum.Material.Metal)
						end
					end
				end
			end
		end
	end

	local se = (ARMS[2].b - ARMS[2].a).Unit
	local seAng = math.atan2(se.Y, se.X)
	local seNE = Vector2.new(se.Y, -se.X) -- north-east flank of the Sears arm

	-- Every store is placed from the mall's own geometry rather than from a
	-- hand-typed centre, because hand-typed centres drift the moment HALL or
	-- DEPTH changes. EDGE is the outer face of the shop band; a store that
	-- flanks an arm puts its near face there, and one that closes an arm puts
	-- its near face on the arm's end. Chris: "it looks like boscov's is
	-- detached lol" - it was, by two units, and shrinking DEPTH from 3.4 to 3.2
	-- had quietly opened a smaller gap at JCPenney too while burying Bon-Ton
	-- inside the shops.
	local EDGE = HALL + DEPTH

	-- Boscov's closes the west arm, so its east face is the arm's end.
	department("BOSCOVS", ARMS[1].b.X - 14 / 2, 0, 14, 15, 0, "+X", { 0 }, { -(HALL + WELL) / 2, (HALL + WELL) / 2 })

	-- Sears closes the south-east arm the same way, squared to it.
	local sears = ARMS[2].b + se * (15 / 2)
	department("SEARS", sears.X, sears.Y, 15, 14, seAng, "-X", { 0 }, { -(HALL + WELL) / 2, (HALL + WELL) / 2 })

	-- JCPenney and Bon-Ton both sit on the NORTH side, on opposite arms, with
	-- the bend between them. Three earlier versions had one or both on the
	-- south, which is what Chris kept seeing as wrong: on the aerial there is
	-- open ground between them right at the court, and putting them on the same
	-- flank closed that gap up.
	department("JCPENNEY", -10, -(EDGE + 12 / 2), 15, 12, 0, "+Z", { -3, 3 }, { -3, 3 })

	local bt = BEND + se * 8 + seNE * (EDGE + 11 / 2)
	department("BON-TON", bt.X, bt.Y, 13, 11, seAng, "+Z", { 0 }, { 0 })

	M.Parent = map
	return M
end

return GalleriaBuilder
