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
-- Builds Workspace.Map.ClaudeGalleria and touches nothing else, so it can sit
-- beside the other mall for comparison.

local Workspace = game:GetService("Workspace")

local GalleriaBuilder = {}

local S = 26 -- studs per model unit
local F2F = 22 -- floor to floor
local STORE_H = 18
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
local COURT_R = 2.55 -- how far back from the bend each arm starts
local COURT_OUT = 2.85 -- outer radius of the court floor
local LIFT = 0.4 -- the court stands this much proud of the concourse

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
local ARMS = {
	{ a = BEND, b = Vector2.new(-16, 0) }, -- west, to Boscov's
	{ a = BEND, b = Vector2.new(11, 11) }, -- south-east, to Sears
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
	local map = Workspace:FindFirstChild("Map") or Workspace
	local old = map:FindFirstChild("ClaudeGalleria")
	if old then
		old:Destroy()
	end

	local M = Instance.new("Model")
	M.Name = "ClaudeGalleria"
	local F: { [string]: Folder } = {}
	for _, n in ipairs({ "Floors", "Shops", "Structure", "Anchors", "Roof", "Court", "Escalators" }) do
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

	-- Escalators are one sloped deck with treads drawn on it, not a stack of
	-- steps. Discrete step boxes do not produce a navmesh a Sweeper can path
	-- over - that was learned on the other mall's stairs, where
	-- PathfindingService returned NoPath at every attempt.
	local ESC_W = 16
	local function escalator(name: string, x1: number, z1: number, x2: number, z2: number, yaw: number)
		local run = Vector2.new(x2 - x1, z2 - z1).Magnitude * S
		local hyp = math.sqrt(run * run + F2F * F2F)
		local pitch = math.atan2(F2F, run)
		local cf = CFrame.new((x1 + x2) / 2 * S, F2F / 2 - 1, (z1 + z2) / 2 * S)
			* CFrame.Angles(0, -yaw, 0)
			* CFrame.Angles(0, 0, pitch)
		part(F.Escalators, `Deck_{name}`, Vector3.new(hyp, 2, ESC_W), cf, C.shell, Enum.Material.Metal)
		for k = 1, 12 do
			local t = ((k - 0.5) / 12 - 0.5) * hyp
			local tread = part(F.Escalators, `Tread_{name}_{k}`, Vector3.new(1.1, 0.5, ESC_W - 1), cf * CFrame.new(t, 1.2, 0), C.teal, Enum.Material.Metal)
			tread.CanCollide = false
		end
		for s = -1, 1, 2 do
			part(F.Escalators, `Balustrade_{name}_{s}`, Vector3.new(hyp, 7, 1.4), cf * CFrame.new(0, 4.5, s * (ESC_W / 2 + 0.7)), C.teal, Enum.Material.Metal)
		end
	end

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
		for k, frac in ipairs({ 0.32, 0.7 }) do
			local side = if k == 1 then 1 else -1
			local centre = -len / 2 + len * frac
			local run = 1.7
			local off = (WELL - ESC_W / (2 * S)) * side
			local x1, z1 = at(centre - run / 2, off)
			local x2, z2 = at(centre + run / 2, off)
			escalator(`{ai}_{k}`, x1, z1, x2, z2, ang)
			table.insert(landings, { arm = ai, side = side, from = centre - run / 2 - 0.15, to = centre + run / 2 + 0.15 })
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
					local rx, rz = at((t0 + t1) / 2, WELL * side)
					part(F.Structure, `Rail{ai}_{side}_{i}`, Vector3.new((t1 - t0) * S + 0.4, 3.5, 1), CFrame.new(rx * S, F2F + 1.75, rz * S) * rot, C.teal, Enum.Material.Metal)
				end
			end
		end

		local bays = math.floor(len / 1.55)
		for side = -1, 1, 2 do
			-- One outer shell per side. The bays have gaps between them, so
			-- without this the mall is a row of free-standing boxes with daylight
			-- between every pair rather than a building with an outside wall.
			local sx, sz = at(0, (HALL + DEPTH) * side)
			part(F.Shops, `Shell{ai}_{side}`, Vector3.new(len * S, F2F * 2, 2), CFrame.new(sx * S, F2F, sz * S) * rot, C.shell, Enum.Material.Concrete)

			for i = 0, bays - 1 do
				local along = -len / 2 + len * ((i + 0.5) / bays)
				local bw = (len / bays) - 0.12
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
					label(fascia, `{list[((i + ai * 3) % #list) + 1]}   {n}`, if side == 1 then Enum.NormalId.Back else Enum.NormalId.Front)
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
				part(F.Structure, `Column{ai}_{i}_{side}`, Vector3.new(7, F2F * 2 - 2, 7), CFrame.new(cx * S, F2F - 1, cz * S), C.column, Enum.Material.Concrete)
			end
			local tx, tz = at(along, 0)
			part(F.Roof, `Truss{ai}_{i}`, Vector3.new(2.2, 2.2, HALL * 2 * S), CFrame.new(tx * S, F2F * 2 + 10, tz * S) * rot, C.teal, Enum.Material.Metal)
		end

		-- Planters down the middle of the concourse, under the well.
		local plants = math.max(1, math.floor(len / 3))
		for i = 0, plants - 1 do
			local px, pz = at(-len / 2 + len * ((i + 0.5) / plants), 0)
			part(F.Structure, `Planter{ai}_{i}`, Vector3.new(26, 7, 16), CFrame.new(px * S, 2.5, pz * S) * rot, C.column, Enum.Material.Concrete)
		end

		for s = -1, 1, 2 do
			local gx, gz = at(0, HALL * 0.5 * s)
			part(F.Roof, `Glass{ai}_{s}`, Vector3.new(len * S, 1.2, HALL * 1.25 * S), CFrame.new(gx * S, F2F * 2 + 16, gz * S) * rot * CFrame.Angles(math.rad(s * 24), 0, 0), C.glass, Enum.Material.Glass, 0.55)
		end
		part(F.Roof, `Ridge{ai}`, Vector3.new(len * S, 2.6, 2.6), CFrame.new(midX * S, F2F * 2 + 24, midZ * S) * rot, C.teal, Enum.Material.Metal)
	end

	-- The court.
	disc(F.Court, "CourtRim", COURT_OUT + 0.06, -1 + LIFT / 2, 2 + LIFT, C.teal, Enum.Material.Metal)
	disc(F.Court, "CourtFloor", COURT_OUT, -1 + LIFT, 2, C.tile, Enum.Material.Pebble)

	-- The balcony carries round the court as two arcs. It stops where each arm's
	-- well opens into the court's, because that opening is the point of a court:
	-- one continuous void from the skylight down to the fountain.
	local RING = 20
	local rMid = (WELL + COURT_OUT) / 2
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
			local w = 2 * math.pi * rMid / RING * 1.35
			local yaw = CFrame.Angles(0, -(th + math.pi / 2), 0)
			part(F.Court, `CourtBalcony_{i}`, Vector3.new(w * S, 2, (COURT_OUT - WELL) * S), CFrame.new(p.X * S, F2F - 1 + LIFT, p.Y * S) * yaw, C.tile, Enum.Material.Pebble)
			local r = Vector2.new(math.cos(th), math.sin(th)) * WELL
			part(F.Court, `CourtRail_{i}`, Vector3.new(2 * math.pi * WELL / RING * S * 1.4, 3.5, 1), CFrame.new(r.X * S, F2F + 1.75 + LIFT, r.Y * S) * yaw, C.teal, Enum.Material.Metal)
		end
	end

	disc(F.Court, "FountainBasin", 0.5, 3, 8, C.teal, Enum.Material.Marble)
	disc(F.Court, "FountainWater", 0.44, 6.4, 0.8, C.glass, Enum.Material.Glass, 0.35)
	part(F.Court, "FountainStem", Vector3.new(6, 15, 6), CFrame.new(0, 10, 0), C.column, Enum.Material.Concrete)

	disc(F.Roof, "CourtSkylight", COURT_OUT - 0.15, F2F * 2 + 20, 1.2, C.glass, Enum.Material.Glass, 0.55)
	for i = 0, 7 do
		part(F.Roof, `CourtTruss_{i}`, Vector3.new(COURT_OUT * 2 * S, 2.2, 2.2), CFrame.new(0, F2F * 2 + 17, 0) * CFrame.Angles(0, -math.pi * i / 8, 0), C.teal, Enum.Material.Metal)
	end

	-- Anchors are buildings hung off the mall, not signs on the end of a
	-- corridor. Chris, reading the printed plan and the aerial: "those anchor
	-- stores need to be anchors... bon ton is like inside the mall, and sears
	-- is sideways." Two storeys of brick with a parapet, squared to whichever
	-- arm they meet, and glazed mall entrances cut into the face that touches
	-- the concourse.
	local function anchor(name: string, cx: number, cz: number, w: number, d: number, yaw: number, doors: { { number } })
		local rot = CFrame.Angles(0, -yaw, 0)
		local h = F2F * 2
		part(F.Anchors, `Anchor_{name}`, Vector3.new(w * S, h, d * S), CFrame.new(cx * S, h / 2, cz * S) * rot, C.brick, Enum.Material.Brick)
		part(F.Anchors, `Parapet_{name}`, Vector3.new(w * S + 3, 5, d * S + 3), CFrame.new(cx * S, h + 2, cz * S) * rot, C.shell, Enum.Material.Concrete)
		local band = part(F.Anchors, `Sign_{name}`, Vector3.new(w * S * 0.55, 20, d * S * 0.55), CFrame.new(cx * S, h - 16, cz * S) * rot, C.teal)
		for _, f in ipairs({ Enum.NormalId.Front, Enum.NormalId.Back, Enum.NormalId.Left, Enum.NormalId.Right }) do
			label(band, name, f)
		end
		for i, e in ipairs(doors) do
			part(F.Anchors, `Entry_{name}_{i}`, Vector3.new(36, 26, 3), CFrame.new(e[1] * S, 13, e[2] * S) * CFrame.Angles(0, -e[3], 0), C.glass, Enum.Material.Glass, 0.35)
		end
	end

	local se = (ARMS[2].b - ARMS[2].a).Unit
	local seAng = math.atan2(se.Y, se.X)
	local seNE = Vector2.new(se.Y, -se.X) -- north-east flank of the Sears arm

	-- Every anchor is placed from the mall's own geometry rather than from a
	-- hand-typed centre, because hand-typed centres drift the moment HALL or
	-- DEPTH changes. EDGE is the outer face of the shop band; an anchor that
	-- flanks an arm puts its near face there, and an anchor that closes an arm
	-- puts its near face on the arm's end. Chris: "it looks like boscov's is
	-- detached lol" - it was, by two units, and shrinking DEPTH from 3.4 to 3.2
	-- had quietly opened a smaller gap at JCPenney too while burying Bon-Ton
	-- inside the shops.
	local EDGE = HALL + DEPTH

	-- Boscov's closes the west arm, so its east face is the arm's end.
	local bosc = Vector2.new(ARMS[1].b.X - 13 / 2, 0)
	anchor("BOSCOVS", bosc.X, bosc.Y, 13, 14, 0, { { ARMS[1].b.X, 0, -math.pi / 2 } })

	-- Sears closes the south-east arm the same way, squared to it.
	local sears = ARMS[2].b + se * (14 / 2)
	local searsDoor = ARMS[2].b + se * 0.3
	anchor("SEARS", sears.X, sears.Y, 14, 13, seAng, { { searsDoor.X, searsDoor.Y, seAng - math.pi / 2 } })

	-- JCPenney and Bon-Ton both sit on the NORTH side, on opposite arms, with
	-- the bend between them. Three earlier versions had one or both on the
	-- south, which is what Chris kept seeing as wrong: on the aerial there is
	-- open ground between them right at the court, and putting them on the same
	-- flank closed that gap up.
	local jcpZ = -(EDGE + 11 / 2)
	anchor("JCPENNEY", -6, jcpZ, 14, 11, 0, { { -9, -EDGE, 0 }, { -3, -EDGE, 0 } })

	local bt = BEND + se * 5.0 + seNE * (EDGE + 10 / 2)
	local btDoor = BEND + se * 5.0 + seNE * EDGE
	anchor("BON-TON", bt.X, bt.Y, 12, 10, seAng, { { btDoor.X, btDoor.Y, seAng + math.pi / 2 } })

	M.Parent = map
	return M
end

return GalleriaBuilder
