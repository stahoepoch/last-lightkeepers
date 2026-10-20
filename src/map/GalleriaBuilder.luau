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

-- The concourse, as two straight runs meeting at the court.
local ARMS = {
	{ a = Vector2.new(-13.0, -4.5), b = Vector2.new(7.5, -4.5) },
	{ a = Vector2.new(7.5, -4.5), b = Vector2.new(14.5, 2.2) },
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
	for _, n in ipairs({ "Floors", "Shops", "Structure", "Anchors", "Roof" }) do
		local f = Instance.new("Folder")
		f.Name, f.Parent = n, M
		F[n] = f
	end

	local suite1, suite2 = 100, 200
	for ai, arm in ipairs(ARMS) do
		local d = arm.b - arm.a
		local len, dir = d.Magnitude, d.Unit
		local nrm = Vector2.new(-dir.Y, dir.X)
		local ang = math.atan2(dir.Y, dir.X)
		local midX, midZ = (arm.a.X + arm.b.X) / 2, (arm.a.Y + arm.b.Y) / 2
		local rot = CFrame.Angles(0, -ang, 0)

		for lvl = 1, 2 do
			local y = (lvl - 1) * F2F
			part(F.Floors, `Hall{ai}_L{lvl}`, Vector3.new(len * S, 2, HALL * 2 * S), CFrame.new(midX * S, y - 1, midZ * S) * rot, C.tile, Enum.Material.Pebble)
		end

		local bays = math.floor(len / 1.55)
		for side = -1, 1, 2 do
			for i = 0, bays - 1 do
				local along = -len / 2 + len * ((i + 0.5) / bays)
				local bw = (len / bays) - 0.12
				for lvl = 1, 2 do
					local y = (lvl - 1) * F2F
					local offs = HALL + DEPTH / 2
					local cx = midX + dir.X * along + nrm.X * offs * side
					local cz = midZ + dir.Y * along + nrm.Y * offs * side
					part(F.Shops, `Shop_L{lvl}_{ai}_{side}_{i}`, Vector3.new(bw * S, STORE_H, DEPTH * S), CFrame.new(cx * S, y + STORE_H / 2, cz * S) * rot, C.shop, Enum.Material.Slate)

					local fx = midX + dir.X * along + nrm.X * (HALL + 0.1) * side
					local fz = midZ + dir.Y * along + nrm.Y * (HALL + 0.1) * side
					local fascia = part(F.Shops, `Fascia_L{lvl}_{ai}_{side}_{i}`, Vector3.new(bw * S, 4.5, 1.4), CFrame.new(fx * S, y + STORE_H - 2.5, fz * S) * rot, C.fascia)
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
					part(F.Shops, `Trim_L{lvl}_{ai}_{side}_{i}`, Vector3.new(bw * S, 1.2, 1.8), CFrame.new(fx * S, y + STORE_H - 5.2, fz * S) * rot, C.teal)
				end
			end
		end

		-- Salmon columns and teal trusses: most of the recognition.
		local cols = math.floor(len / 1.9)
		for i = 0, cols do
			local along = -len / 2 + len * (i / cols)
			for side = -1, 1, 2 do
				local cx = midX + dir.X * along + nrm.X * (HALL - 0.18) * side
				local cz = midZ + dir.Y * along + nrm.Y * (HALL - 0.18) * side
				part(F.Structure, `Column{ai}_{i}_{side}`, Vector3.new(4.5, F2F * 2 - 2, 4.5), CFrame.new(cx * S, F2F - 1, cz * S), C.column, Enum.Material.Concrete)
			end
			part(F.Roof, `Truss{ai}_{i}`, Vector3.new(2.2, 2.2, HALL * 2 * S), CFrame.new((midX + dir.X * along) * S, F2F * 2 + 10, (midZ + dir.Y * along) * S) * rot, C.teal, Enum.Material.Metal)
		end

		for s = -1, 1, 2 do
			part(F.Roof, `Glass{ai}_{s}`, Vector3.new(len * S, 1.2, HALL * 1.25 * S), CFrame.new((midX + nrm.X * HALL * 0.5 * s) * S, F2F * 2 + 16, (midZ + nrm.Y * HALL * 0.5 * s) * S) * rot * CFrame.Angles(math.rad(s * 24), 0, 0), C.glass, Enum.Material.Glass, 0.55)
		end
		part(F.Roof, `Ridge{ai}`, Vector3.new(len * S, 2.6, 2.6), CFrame.new(midX * S, F2F * 2 + 24, midZ * S) * rot, C.teal, Enum.Material.Metal)

		for side = -1, 1, 2 do
			local rx = midX + nrm.X * (HALL - 0.05) * side
			local rz = midZ + nrm.Y * (HALL - 0.05) * side
			part(F.Structure, `Rail{ai}_{side}`, Vector3.new(len * S, 3.5, 1), CFrame.new(rx * S, F2F + 1.75, rz * S) * rot, C.teal, Enum.Material.Metal)
		end
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

	local bend = ARMS[2].a
	local a2 = (ARMS[2].b - ARMS[2].a).Unit
	local a2ang = math.atan2(a2.Y, a2.X)
	local a2nrm = Vector2.new(-a2.Y, a2.X)

	-- Boscov's closes the straight arm.
	anchor("BOSCOVS", -19.5, -4.5, 12, 11, 0, { { -13.6, -4.5, 0 } })

	-- JCPenney sits in the crook and opens onto BOTH arms. The two entrances
	-- are on the printed plan and are the giveaway that it straddles the bend.
	anchor("JCPENNEY", 6.0, -12.5, 14, 8, 0, { { 2.0, -8.4, 0 }, { 9.6, -8.4, 0 } })

	-- Bon-Ton hangs off the outer flank of the angled arm.
	local bt = bend + a2 * 5.5 - a2nrm * 7.6
	local btDoor = bend + a2 * 5.5 - a2nrm * 3.1
	anchor("BON-TON", bt.X, bt.Y, 11, 9, a2ang, { { btDoor.X, btDoor.Y, a2ang } })

	-- Sears caps the far end of the angled arm, square to it.
	local se = bend + a2 * 16.0
	local seDoor = bend + a2 * 10.2
	anchor("SEARS", se.X, se.Y, 13, 11, a2ang, { { seDoor.X, seDoor.Y, a2ang } })

	M.Parent = map
	return M
end

return GalleriaBuilder
