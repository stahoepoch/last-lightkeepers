--!strict
-- Lanternhold — the Johnstown Masonic Temple, Valley Pike.
--
-- Rewritten from Chris's own four floor plans and his photographs of the
-- inside. The previous version was built from a programme he described from
-- memory; the levels were roughly right and nearly every room in them was
-- invented, which is what he kept hitting: "lanternhold itself doesn't feel
-- right, i know its the masonic temple, but it doesn't feel right, look
-- right, or lol, smell right."
--
-- The plans and photographs are in docs/reference/lanternhold/ and the
-- reading of them is docs/LANTERNHOLD-PLAN.md. READ THOSE BEFORE EDITING
-- THIS. Anything here that contradicts the plans is a bug in this file.
--
-- J. C. Fulton and Son of Uniontown, 1927-1933, cornerstone 1931. Art Deco:
-- pale limestone, flat roof, and a tall central entrance bay of fluted
-- pilasters that read like organ pipes. Chris: "we are a very special temple,
-- you'll never see another one like it besides philly or england, we have a
-- huge place and it's ornate."
--
-- ORNATE IS THE BRIEF. The old build was plain coloured boxes and that is
-- the whole of "doesn't look right". What makes this building is terrazzo
-- with dark inlay banding underfoot, cream plaster over a dark wood base, a
-- painted green-and-gold stencil band where wall meets ceiling, beamed
-- ceilings, brass door furniture, and Art Deco iron balustrades with a
-- circle-and-chevron motif. Those six things are in nearly every photograph,
-- they are cheap, and they go everywhere.
--
-- STRUCTURE. Rooms are declared as rectangles in a table per floor and built
-- by one function rather than written out by hand. The old file grew
-- organically and every change had to hunt for its own coordinates; this way
-- a room that moves takes its floor, skirting, stencil, ceiling and lighting
-- with it.
--
--   require(workspace.Map.LanternholdBuilder).Build()
-- Builds Workspace.Lanternhold, deliberately NOT under Workspace.Map, which
-- Rojo owns and reconciles on every sync.

local CollectionService = game:GetService("CollectionService")
local Workspace = game:GetService("Workspace")

local LanternholdBuilder = {}

-- ------------------------------------------------------------------ the grid
-- The plans are portrait: the building is deeper than it is wide. X runs west
-- (the left of the plan) to east; Z runs from the bottom of the plan, where
-- the ramp entrance is, to the top, where the exit is.
local W = 110 -- half-width, so 220 across
local Z0, Z1 = 0, 300

-- The three vertical bands every plan shares: a west block of big rooms, a
-- centre strip carrying the stairs, and an east block of small rooms with the
-- elevator in it.
local WEST, CENTRE, EAST = -15, 45, W

-- Floor levels and the ceiling over each. NOT a constant storey: the ballroom
-- and the Egyptian Room are grand rooms and are taller than the service
-- floors. That is both true of the building and the thing that stops four
-- identical slabs reading as a car park. A player is five studs, so these
-- are 3.2, 5.2, 5.6 and 4 player heights - against the old build's 6.4.
-- Chris: "the ceiling is far too low, i mean our temple is massive." It was.
-- I over-corrected off the Galleria, whose 32 stud storey was itself too tall
-- for a mall - but a temple is not a mall, and the Egyptian Room in the
-- photographs is a very tall room full of very tall columns.
--
-- Storey heights are 26, 38, 42 and 30 studs now. Against a five stud player
-- that is 5.2, 7.6, 8.4 and 6, and the grand rooms are the tall ones.
local LEVELS = { 0, 30, 72, 118 }
local HEADS = { 26, 68, 114, 148 }
local TOP = 154

local WALL_T = 3
local DOOR_W = 11
local DOOR_H = 13 -- a door a person walks through, with reach over it

local C = {
	-- Read off the photographs rather than invented.
	limestone = Color3.fromRGB(214, 208, 192), -- the outside, pale and flat
	plaster = Color3.fromRGB(226, 219, 203), -- cream walls above the base
	terrazzo = Color3.fromRGB(176, 168, 150), -- speckled cream, polished
	inlay = Color3.fromRGB(74, 68, 60), -- the dark bands set into it
	timber = Color3.fromRGB(78, 46, 28), -- dark stained doors and rails
	brass = Color3.fromRGB(190, 152, 86),
	iron = Color3.fromRGB(38, 35, 33), -- the deco balustrades
	stencil = Color3.fromRGB(96, 116, 86), -- the painted border band, green
	gilt = Color3.fromRGB(198, 164, 92), -- and its gold
	egyptGold = Color3.fromRGB(198, 158, 74),
	egyptBlue = Color3.fromRGB(58, 62, 118), -- the carpet
	egyptTeal = Color3.fromRGB(46, 108, 112), -- the seating
	dark = Color3.fromRGB(32, 30, 32),
}

local WARM = Color3.fromRGB(255, 216, 168)

-- ------------------------------------------------------------------ helpers
local function part(
	parent: Instance,
	name: string,
	size: Vector3,
	cf: CFrame,
	colour: Color3,
	mat: Enum.Material?,
	trans: number?
): Part
	local p = Instance.new("Part")
	p.Name, p.Size, p.CFrame, p.Color = name, size, cf, colour
	p.Material = mat or Enum.Material.SmoothPlastic
	p.Anchored = true
	p.Transparency = trans or 0
	p.TopSurface, p.BottomSurface = Enum.SurfaceType.Smooth, Enum.SurfaceType.Smooth
	p.Parent = parent
	return p
end

--- A box from two opposite corners. Ordering is forced, because an earlier
--- version computed x1 - x0 raw and a negative width clamped to nothing: a
--- door leaf came out a thousandth of a stud thick and nobody could see why.
local function box(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	y0: number,
	y1: number,
	z0: number,
	z1: number,
	colour: Color3,
	mat: Enum.Material?,
	trans: number?
): Part
	local lo = Vector3.new(math.min(x0, x1), math.min(y0, y1), math.min(z0, z1))
	local hi = Vector3.new(math.max(x0, x1), math.max(y0, y1), math.max(z0, z1))
	local size = hi - lo
	return part(
		parent,
		name,
		Vector3.new(math.max(size.X, 0.05), math.max(size.Y, 0.05), math.max(size.Z, 0.05)),
		CFrame.new((lo + hi) / 2),
		colour,
		mat,
		trans
	)
end

local function label(host: BasePart, text: string, face: Enum.NormalId, size: number?)
	local gui = Instance.new("SurfaceGui")
	gui.Face, gui.Adornee, gui.AlwaysOnTop = face, host, false
	gui.CanvasSize = Vector2.new(400, 100)
	gui.Parent = host
	local t = Instance.new("TextLabel")
	t.Size, t.BackgroundTransparency = UDim2.fromScale(1, 1), 1
	t.Font, t.Text = Enum.Font.GothamBold, text
	t.TextSize = size or 24
	t.TextColor3 = Color3.fromRGB(246, 240, 226)
	t.Parent = gui
end

--- A wall along one axis with doorways cut in it, each with a lintel over.
--- `gaps` are spans in the along-axis coordinate.
local function wall(
	parent: Instance,
	name: string,
	axis: string,
	at: number,
	from: number,
	to: number,
	y0: number,
	y1: number,
	gaps: { { number } }
)
	local lo, hi = math.min(from, to), math.max(from, to)
	local cuts = table.clone(gaps)
	table.sort(cuts, function(a, b)
		return a[1] < b[1]
	end)
	local index = 0
	local function piece(a: number, b: number, py0: number, py1: number)
		if b - a < 0.05 or py1 - py0 < 0.05 then
			return
		end
		index += 1
		if axis == "X" then
			box(
				parent,
				`{name}_{index}`,
				at - WALL_T / 2,
				at + WALL_T / 2,
				py0,
				py1,
				a,
				b,
				C.plaster
			)
		else
			box(
				parent,
				`{name}_{index}`,
				a,
				b,
				py0,
				py1,
				at - WALL_T / 2,
				at + WALL_T / 2,
				C.plaster
			)
		end
	end
	local run = lo
	for _, gap in ipairs(cuts) do
		local ga, gb = math.max(lo, gap[1]), math.min(hi, gap[2])
		if gb > ga then
			piece(run, ga, y0, y1)
			piece(ga, gb, y0 + DOOR_H, y1) -- the lintel over the opening
			run = gb
		end
	end
	piece(run, hi, y0, y1)
end

-- Where each floor is OPEN, so the stairs and the lift have something to
-- come up through.
--
-- This is the fault that made the whole building unwalkable on first build:
-- every slab was laid across the full footprint, so each flight climbed
-- neatly into the underside of the floor above and stopped. Pathfinding
-- reported NOPATH to every room on every upper level and only the bottom
-- floor could be reached at all - Chris: "the stairs need staggered more, i
-- can't get up them". The treads were never the problem; there was no hole.
--
-- Indexed by the level the hole is cut IN, so WELLS[2] is what is missing
-- from the 1st floor's slab.
-- Each well is sized to the flight that arrives in it, and ends exactly
-- where that flight's top tread ends. The first version made them all the
-- same generous rectangle, which cut the hole six studs PAST the top of the
-- stair - so every flight climbed to the right height and then stopped at
-- the edge of a six stud gap with the floor on the far side of it. Still
-- NOPATH to every upper room, for a completely different reason than the
-- first time.
--
-- Stair A arrives at z 186 on the 1st, B at z 80 on the 2nd, C at z 226 on
-- the 3rd, and each well stops on that line.
local WELLS: { [number]: { { number } } } = {
	[2] = { { 0, 22, 78, 188 }, { 64, 102, 196, 238 } },
	[3] = { { 20, 42, 78, 188 }, { 64, 102, 196, 238 } },
	[4] = { { 58, 102, 126, 226 }, { 64, 102, 196, 238 } },
}

--- Everything in `rect` that is not inside any of `holes`, as a list of
--- rectangles. Each hole splits every rectangle it overlaps into at most four
--- strips, which is enough: a floor here never has more than two wells and
--- they are nowhere near each other.
local function subtract(rect: { number }, holes: { { number } }): { { number } }
	local pieces = { rect }
	for _, hole in ipairs(holes) do
		local next_: { { number } } = {}
		for _, r in ipairs(pieces) do
			local ox0, ox1 = math.max(r[1], hole[1]), math.min(r[2], hole[2])
			local oz0, oz1 = math.max(r[3], hole[3]), math.min(r[4], hole[4])
			if ox1 <= ox0 or oz1 <= oz0 then
				table.insert(next_, r) -- no overlap, keep whole
			else
				if r[3] < oz0 then
					table.insert(next_, { r[1], r[2], r[3], oz0 })
				end
				if oz1 < r[4] then
					table.insert(next_, { r[1], r[2], oz1, r[4] })
				end
				if r[1] < ox0 then
					table.insert(next_, { r[1], ox0, oz0, oz1 })
				end
				if ox1 < r[2] then
					table.insert(next_, { ox1, r[2], oz0, oz1 })
				end
			end
		end
		pieces = next_
	end
	return pieces
end

--- Terrazzo with the dark bands set into it. This is the floor in every
--- photograph of the building and it is the cheapest single thing that makes
--- a room read as this place rather than as a grey box.
local function terrazzoSolid(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number
)
	-- Concrete, not Pebble. Chris: "the texture is ugly" - Pebble is a coarse
	-- lumpy aggregate and the floors in the photographs are POLISHED
	-- terrazzo, which is close to flat.
	box(parent, name, x0, x1, y - 2, y, z0, z1, C.terrazzo, Enum.Material.Concrete)
	local inset = 7
	for _, spec in ipairs({
		{ x0 + inset, x0 + inset + 2.5, z0 + inset, z1 - inset },
		{ x1 - inset - 2.5, x1 - inset, z0 + inset, z1 - inset },
		{ x0 + inset, x1 - inset, z0 + inset, z0 + inset + 2.5 },
		{ x0 + inset, x1 - inset, z1 - inset - 2.5, z1 - inset },
	}) do
		local a, b, c, d = spec[1], spec[2], spec[3], spec[4]
		if b - a > 0.2 and d - c > 0.2 then
			-- Sitting ON the slab, not sunk through its surface. Chris: "the
			-- floor makes me sick, it keeps flickering" - that is z-fighting
			-- and it was mine. The band ran y - 0.08 to y + 0.05 while the
			-- slab top is exactly y, so the two were coplanar across the whole
			-- band and the renderer could not pick one. Standing proud of the
			-- slab gives each visible face its own depth.
			local band =
				box(parent, `{name}_Band`, a, b, y, y + 0.12, c, d, C.inlay, Enum.Material.Slate)
			band.CanCollide, band.CanQuery = false, false
		end
	end
end

--- The same floor, with the stair and lift wells left open in it.
local function terrazzo(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number,
	holes: { { number } }?
)
	if holes and #holes > 0 then
		for i, piece in ipairs(subtract({ x0, x1, z0, z1 }, holes)) do
			terrazzoSolid(parent, `{name}_{i}`, piece[1], piece[2], piece[3], piece[4], y)
		end
		return
	end
	terrazzoSolid(parent, name, x0, x1, z0, z1, y)
end

--- The dark wood base at the floor and the painted stencil band where wall
--- meets ceiling. Two strips, and between them a room stops looking like a
--- box with paint on it.
local function trim(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number,
	head: number
)
	for _, side in ipairs({
		{ x0, x0 + 1.2, z0, z1 },
		{ x1 - 1.2, x1, z0, z1 },
		{ x0, x1, z0, z0 + 1.2 },
		{ x0, x1, z1 - 1.2, z1 },
	}) do
		local a, b, c, d = side[1], side[2], side[3], side[4]
		-- NONE of this collides. Chris: "why are there wood blocks stopping me
		-- from moving everywhere?" - this skirting. It runs the full rectangle
		-- of every room, which means it also runs straight across every
		-- doorway in that room, and at three studs a Humanoid will not step
		-- over it. Forty rooms of decorative trim became forty rooms you
		-- could not walk out of.
		--
		-- Skirting, a painted band and a gilt line are paint. The fix is not
		-- to cut them around the doors, it is for none of them to be solid.
		local baseStrip =
			box(parent, `{name}_Base`, a, b, y, y + 2.2, c, d, C.timber, Enum.Material.WoodPlanks)
		local stencilStrip =
			box(parent, `{name}_Stencil`, a, b, head - 3.4, head - 1.6, c, d, C.stencil)
		local giltStrip = box(
			parent,
			`{name}_Gilt`,
			a,
			b,
			head - 1.6,
			head - 1,
			c,
			d,
			C.gilt,
			Enum.Material.Metal
		)
		for _, strip in ipairs({ baseStrip, stencilStrip, giltStrip }) do
			strip.CanCollide, strip.CanQuery, strip.CanTouch = false, false, false
		end
	end
end

--- A beamed plaster ceiling.
local function ceilingOver(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number,
	bay: number,
	holes: { { number } }?
)
	-- The ceiling takes the same wells as the floor above it. A stairwell
	-- with a hole in the slab and a ceiling slab under it is still capped.
	for i, r in ipairs(subtract({ x0, x1, z0, z1 }, holes or {})) do
		box(parent, `{name}_{i}`, r[1], r[2], y, y + 2, r[3], r[4], C.plaster)
		local span = r[4] - r[3]
		local n = math.max(1, math.floor(span / bay))
		for j = 0, n do
			local z = r[3] + span * (j / n)
			box(parent, `{name}_Beam_{i}_{j}`, r[1], r[2], y - 2.2, y, z - 1.6, z + 1.6, C.plaster)
		end
	end
end

local lampCount = 0
--- A fitting. Non-colliding: a ceiling lamp is not something to walk into.
local function lamp(
	parent: Instance,
	x: number,
	y: number,
	z: number,
	brightness: number,
	range: number
)
	lampCount += 1
	local glass = part(
		parent,
		`Lamp_{lampCount}`,
		Vector3.new(2.6, 3, 2.6),
		CFrame.new(x, y, z),
		C.brass,
		Enum.Material.Metal
	)
	glass.CanCollide, glass.CanQuery = false, false
	local light = Instance.new("PointLight")
	light.Color, light.Brightness, light.Range, light.Shadows = WARM, brightness, range, false
	light.Parent = glass
end

--- Lamps on a grid across a room, so a room cannot come out part-lit. The
--- spacing has to stay inside the lamp's reach or the diagonals go dark; 40
--- against a range of 40 leaves the worst point 28 away, which is inside it.
local function lightRoom(
	parent: Instance,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	floorY: number,
	head: number,
	opts: { Brightness: number, Range: number, Spacing: number? }
)
	local pitch = opts.Spacing or 44
	local nx = math.max(1, math.round((x1 - x0) / pitch))
	local nz = math.max(1, math.round((z1 - z0) / pitch))
	-- Hung on a drop rather than fixed flat to the ceiling. Range is a
	-- sphere, so in a 42 stud room a fitting tight under the ceiling spends
	-- nearly all its reach getting down to the floor and lights nothing -
	-- which is exactly how the Galleria's ground concourse ended up lit by a
	-- stripe down the middle. Twenty two studs is above head height and still
	-- over the floor.
	local mount = math.min(head - 4, floorY + 22)
	for i = 0, nx - 1 do
		for j = 0, nz - 1 do
			local lx = x0 + (x1 - x0) * ((i + 0.5) / nx)
			local lz = z0 + (z1 - z0) * ((j + 0.5) / nz)
			if head - 4 > mount + 1 then
				local rod = box(
					parent,
					"LampRod",
					lx - 0.3,
					lx + 0.3,
					mount,
					head - 1,
					lz - 0.3,
					lz + 0.3,
					C.iron,
					Enum.Material.Metal
				)
				rod.CanCollide, rod.CanQuery = false, false
			end
			lamp(parent, lx, mount, lz, opts.Brightness, opts.Range)
		end
	end
end

--- The Art Deco balustrade from the stair-hall photograph: dark iron uprights
--- with a circle between each pair, under a dark wood handrail. It is the
--- signature of the stair hall and the old build had nothing like it.
local function decoRail(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number
)
	local alongX = (x1 - x0) >= (z1 - z0)
	local length = if alongX then x1 - x0 else z1 - z0
	local n = math.max(2, math.floor(length / 7))
	local midX, midZ = (x0 + x1) / 2, (z0 + z1) / 2
	for i = 0, n do
		local t = i / n
		local px = if alongX then x0 + (x1 - x0) * t else midX
		local pz = if alongX then midZ else z0 + (z1 - z0) * t
		box(
			parent,
			`{name}_Post_{i}`,
			px - 0.4,
			px + 0.4,
			y,
			y + 6.2,
			pz - 0.4,
			pz + 0.4,
			C.iron,
			Enum.Material.Metal
		)
		if i < n then
			local nt = (i + 1) / n
			local qx = if alongX then x0 + (x1 - x0) * nt else px
			local qz = if alongX then pz else z0 + (z1 - z0) * nt
			local mx, mz = (px + qx) / 2, (pz + qz) / 2
			box(
				parent,
				`{name}_Ring_{i}`,
				mx - 1.5,
				mx + 1.5,
				y + 2.4,
				y + 3.6,
				mz - 1.5,
				mz + 1.5,
				C.iron,
				Enum.Material.Metal
			)
		end
	end
	if alongX then
		box(
			parent,
			`{name}_Hand`,
			x0,
			x1,
			y + 6.2,
			y + 7.4,
			midZ - 0.9,
			midZ + 0.9,
			C.timber,
			Enum.Material.WoodPlanks
		)
		box(
			parent,
			`{name}_Kick`,
			x0,
			x1,
			y + 0.2,
			y + 1,
			midZ - 0.5,
			midZ + 0.5,
			C.iron,
			Enum.Material.Metal
		)
	else
		box(
			parent,
			`{name}_Hand`,
			midX - 0.9,
			midX + 0.9,
			y + 6.2,
			y + 7.4,
			z0,
			z1,
			C.timber,
			Enum.Material.WoodPlanks
		)
		box(
			parent,
			`{name}_Kick`,
			midX - 0.5,
			midX + 0.5,
			y + 0.2,
			y + 1,
			z0,
			z1,
			C.iron,
			Enum.Material.Metal
		)
	end
end

--- A flight, built as treads you walk up rather than a ramp, with the yellow
--- nosing strips the building actually has on its stairs today.
local function flight(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	zLow: number,
	zHigh: number,
	yLow: number,
	yHigh: number
)
	-- A SOLID RAMP with the steps as non-colliding paint on top.
	--
	-- Built as real stepped geometry first, and it did not work: a walking
	-- agent got NoPath from the foot of the stair to a third of the way up,
	-- while the same agent with AgentCanJump succeeded. Roblox's navmesh will
	-- not link a run of discrete 1.4 stud platforms into a walkable surface,
	-- so every upper floor in the building was unreachable on foot.
	--
	-- GalleriaBuilder already learned this and says so at its escalators:
	-- treads "have to be either real geometry you stand on or paint; a raised
	-- ghost step is the one thing they cannot be." The ramp is the geometry;
	-- the treads are the paint.
	local dz = zHigh - zLow
	local dy = yHigh - yLow
	local hyp = math.sqrt(dz * dz + dy * dy)
	local pitch = math.atan2(dy, dz)
	local cf = CFrame.new((x0 + x1) / 2, (yLow + yHigh) / 2, (zLow + zHigh) / 2)
		* CFrame.Angles(-pitch, 0, 0)
	part(
		parent,
		`{name}_Ramp`,
		Vector3.new(x1 - x0, 2, hyp),
		cf,
		C.terrazzo,
		Enum.Material.Concrete
	)

	-- The steps, flush with the ramp surface so there is nothing to catch on.
	local steps = math.max(10, math.floor(math.abs(dy) / 1.4))
	for i = 0, steps - 1 do
		local t = (i + 0.5) / steps
		local tread = part(
			parent,
			`{name}_Tread_{i}`,
			Vector3.new(x1 - x0, 0.4, hyp / steps - 0.3),
			cf * CFrame.new(0, 1.1, (t - 0.5) * hyp),
			C.inlay,
			Enum.Material.Slate
		)
		tread.CanCollide, tread.CanQuery, tread.CanTouch = false, false, false
		local nose = part(
			parent,
			`{name}_Nose_{i}`,
			Vector3.new(x1 - x0, 0.45, 0.8),
			cf * CFrame.new(0, 1.2, (t - 0.5) * hyp - hyp / steps / 2),
			C.gilt,
			Enum.Material.Metal
		)
		nose.CanCollide, nose.CanQuery, nose.CanTouch = false, false, false
	end

	-- The rail follows the flight, and nothing on a stair collides: a
	-- handrail you walk into is worse than one you clip through.
	for side, sideX in ipairs({ { x0 - 1.4, x0 - 0.2 }, { x1 + 0.2, x1 + 1.4 } }) do
		local rails = math.max(3, math.floor(steps / 2))
		for i = 0, rails do
			local t = i / rails
			local z = zLow + dz * t
			local ry = yLow + dy * t
			local post = box(
				parent,
				`{name}_RailPost_{side}_{i}`,
				sideX[1],
				sideX[2],
				ry,
				ry + 7,
				z - 0.5,
				z + 0.5,
				C.iron,
				Enum.Material.Metal
			)
			post.CanCollide, post.CanQuery = false, false
			if i < rails then
				local nt = (i + 1) / rails
				local nz, ny = zLow + dz * nt, yLow + dy * nt
				local hand = box(
					parent,
					`{name}_RailHand_{side}_{i}`,
					sideX[1],
					sideX[2],
					math.min(ry, ny) + 6.2,
					math.max(ry, ny) + 7.4,
					math.min(z, nz),
					math.max(z, nz),
					C.timber,
					Enum.Material.WoodPlanks
				)
				hand.CanCollide, hand.CanQuery = false, false
			end
		end
	end
end

type Room = {
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	light: number?,
	floor: Color3?,
	mat: Enum.Material?,
}

-- --------------------------------------------------------------------------
function LanternholdBuilder.Build(): Model
	lampCount = 0
	local existing = Workspace:FindFirstChild("Lanternhold")
	if existing then
		existing:Destroy()
	end

	local M = Instance.new("Model")
	M.Name = "Lanternhold"

	local F: { [string]: Folder } = {}
	for _, n in ipairs({
		"Shell",
		"Floor0",
		"Floor1",
		"Floor2",
		"Floor3",
		"Core",
		"Fixtures",
		"Markers",
	}) do
		local f = Instance.new("Folder")
		f.Name, f.Parent = n, M
		F[n] = f
	end

	local function buildRoom(folder: Instance, level: number, room: Room)
		local y, head = LEVELS[level], HEADS[level]
		local wells = WELLS[level]
		terrazzo(folder, `{room.name}_Floor`, room.x0, room.x1, room.z0, room.z1, y, wells)
		if room.floor then
			-- Also standing proud rather than flush, for the same reason, and
			-- above the inlay bands so a carpeted room does not fight them.
			for i, r in
				ipairs(
					subtract({ room.x0 + 1, room.x1 - 1, room.z0 + 1, room.z1 - 1 }, wells or {})
				)
			do
				local finish = box(
					folder,
					`{room.name}_Finish_{i}`,
					r[1],
					r[2],
					y,
					y + 0.2,
					r[3],
					r[4],
					room.floor,
					room.mat or Enum.Material.Fabric
				)
				finish.CanCollide = false
			end
		end
		trim(folder, room.name, room.x0, room.x1, room.z0, room.z1, y, head)
		-- The ceiling of THIS room is the underside of the floor above, so it
		-- takes the next level's wells, not its own.
		ceilingOver(
			folder,
			`{room.name}_Ceiling`,
			room.x0,
			room.x1,
			room.z0,
			room.z1,
			head,
			34,
			WELLS[level + 1]
		)
		if (room.light or 0) > 0 then
			-- The per-room numbers are relative weights, not absolute
			-- brightness. The first build used them raw at range 40 on a 44
			-- stud grid and came out 26% of the floor blown, peaking at 3.22 -
			-- fittings stacking through the walls between rooms, because every
			-- light in both maps has Shadows off.
			--
			-- Smaller pools closer together is the fix rather than fewer:
			-- range 30 on a 34 grid leaves the worst point between four lamps
			-- 24 away, inside reach, while barely reaching the next room.
			lightRoom(folder, room.x0, room.x1, room.z0, room.z1, y, head, {
				Brightness = (room.light :: number) * 0.62,
				Range = 40,
				Spacing = 36,
			})
		end
	end

	-- ------------------------------------------------------------- the shell
	for i, lvl in ipairs(LEVELS) do
		terrazzo(F.Shell, `Slab_{i}`, -W, W, Z0, Z1, lvl, WELLS[i])
	end
	box(F.Shell, "Roof", -W, W, TOP, TOP + 3, Z0, Z1, C.limestone, Enum.Material.Concrete)
	box(
		F.Shell,
		"Parapet",
		-W - 2,
		W + 2,
		TOP + 3,
		TOP + 9,
		Z0 - 2,
		Z1 + 2,
		C.limestone,
		Enum.Material.Concrete
	)
	wall(F.Shell, "WallWest", "X", -W, Z0, Z1, 0, TOP, {})
	wall(F.Shell, "WallEast", "X", W, Z0, Z1, 0, TOP, {})
	wall(F.Shell, "WallNorth", "Z", Z1, -W, W, 0, TOP, { { -18, 18 } })
	-- Two ways in, on two levels: the ramp into the bottom floor at the east
	-- end of the south front, and the outside steps into the 1st floor at the
	-- middle of it. The old build had one.
	wall(F.Shell, "WallSouth", "Z", Z0, -W, W, 0, TOP, { { 58, 100 } })

	-- The entrance bay: a projecting block of fluted pilasters, the single
	-- most recognisable thing about the outside.
	do
		local bx0, bx1 = -34, 34
		box(
			F.Shell,
			"EntranceBay",
			bx0,
			bx1,
			0,
			TOP - 6,
			Z0 - 14,
			Z0,
			C.limestone,
			Enum.Material.Concrete
		)
		for i = 0, 10 do
			local x = bx0 + (bx1 - bx0) * (i / 10)
			box(
				F.Shell,
				`Flute_{i}`,
				x - 1.6,
				x + 1.6,
				16,
				TOP - 10,
				Z0 - 15.8,
				Z0 - 13.4,
				C.limestone,
				Enum.Material.Concrete
			)
		end
		for i = 0, 8 do
			box(
				F.Shell,
				`FrontStep_{i}`,
				-22,
				22,
				i * 2.25,
				i * 2.25 + 2.25,
				Z0 - 34 + i * 2.3,
				Z0 - 14,
				C.limestone,
				Enum.Material.Concrete
			)
		end
	end

	-- Tall narrow window slots on both flanks. Nothing behind them: it is
	-- night, and the point is the silhouette seen from the departure pad.
	for _, sx in ipairs({ -W, W }) do
		for i = 0, 7 do
			local z = Z1 * ((i + 0.5) / 8)
			box(
				F.Shell,
				`Window_{sx}_{i}`,
				sx - 1.6,
				sx + 1.6,
				24,
				TOP - 16,
				z - 4,
				z + 4,
				C.dark,
				Enum.Material.Glass,
				0.35
			)
		end
	end

	-- --------------------------------------------------------- BOTTOM FLOOR
	-- Arrival. In at the south-east off the ramp, up the east edge past the
	-- rooms and bathrooms, stair core in the middle, banquet hall to the west
	-- with the kitchen behind it.
	for _, r in
		ipairs({
			{ name = "Kitchen", x0 = -W, x1 = WEST, z0 = Z0, z1 = 110, light = 1.0 },
			{ name = "BanquetHall", x0 = -W, x1 = WEST, z0 = 110, z1 = Z1, light = 1.0 },
			{ name = "ArrivalHall", x0 = WEST, x1 = CENTRE, z0 = Z0, z1 = 60, light = 1.1 },
			{ name = "StairHall0", x0 = WEST, x1 = CENTRE, z0 = 60, z1 = 205, light = 1.0 },
			{ name = "NorthLobby", x0 = WEST, x1 = CENTRE, z0 = 205, z1 = Z1, light = 0.95 },
			{ name = "RampEntrance", x0 = CENTRE, x1 = EAST, z0 = Z0, z1 = 55, light = 1.0 },
			{ name = "EastRoom0", x0 = CENTRE, x1 = EAST, z0 = 55, z1 = 95, light = 0.85 },
			{ name = "BathroomA", x0 = CENTRE, x1 = EAST, z0 = 95, z1 = 130, light = 0.8 },
			{ name = "BathroomB", x0 = CENTRE, x1 = EAST, z0 = 130, z1 = 165, light = 0.8 },
			{ name = "ElevatorLobby0", x0 = CENTRE, x1 = EAST, z0 = 165, z1 = 205, light = 0.9 },
			{ name = "NorthEastRoom", x0 = CENTRE, x1 = EAST, z0 = 205, z1 = Z1, light = 0.85 },
		} :: { Room })
	do
		buildRoom(F.Floor0, 1, r)
	end
	wall(F.Floor0, "Bottom_WestWall", "X", WEST, Z0, Z1, 0, HEADS[1], {
		{ 30, 30 + DOOR_W },
		{ 150, 150 + DOOR_W },
		{ 240, 240 + DOOR_W },
	})
	wall(F.Floor0, "Bottom_EastWall", "X", CENTRE, Z0, Z1, 0, HEADS[1], {
		{ 20, 20 + DOOR_W },
		{ 110, 110 + DOOR_W },
		{ 180, 180 + DOOR_W },
		{ 250, 250 + DOOR_W },
	})
	wall(F.Floor0, "Kitchen_North", "Z", 110, -W, WEST, 0, HEADS[1], { { -70, -70 + DOOR_W } })
	for _, z in ipairs({ 95, 130, 165, 205 }) do
		wall(
			F.Floor0,
			`Bottom_East_{z}`,
			"Z",
			z,
			CENTRE,
			EAST,
			0,
			HEADS[1],
			{ { 70, 70 + DOOR_W } }
		)
	end

	box(
		F.Floor0,
		"ExitVestibule",
		-18,
		18,
		-2,
		HEADS[1],
		Z1,
		Z1 + 26,
		C.limestone,
		Enum.Material.Concrete
	)
	terrazzo(F.Floor0, "ExitFloor", -17, 17, Z1, Z1 + 25, 0)
	for i = 0, 10 do
		box(
			F.Floor0,
			`Ramp_{i}`,
			70,
			104,
			-1 - i * 0.62,
			0.4 - i * 0.62,
			Z0 - 4 - i * 4,
			Z0 + 2 - i * 4,
			C.limestone,
			Enum.Material.Concrete
		)
	end

	-- ------------------------------------------------------------ 1ST FLOOR
	for _, r in
		ipairs({
			{
				name = "Ballroom",
				x0 = -W,
				x1 = WEST,
				z0 = 30,
				z1 = Z1,
				light = 1.0,
				floor = Color3.fromRGB(104, 62, 40),
				mat = Enum.Material.WoodPlanks,
			},
			{ name = "WomensRestroom", x0 = WEST, x1 = CENTRE, z0 = Z0, z1 = 50, light = 0.8 },
			{ name = "StairHall1", x0 = WEST, x1 = CENTRE, z0 = 50, z1 = 195, light = 1.0 },
			{ name = "Bar", x0 = WEST, x1 = CENTRE, z0 = 195, z1 = 258, light = 0.95 },
			{ name = "Bathrooms1", x0 = WEST, x1 = CENTRE, z0 = 258, z1 = Z1, light = 0.8 },
			{ name = "Storage1", x0 = CENTRE, x1 = EAST, z0 = Z0, z1 = 58, light = 0.7 },
			{ name = "TinyRoom1", x0 = CENTRE, x1 = EAST, z0 = 58, z1 = 118, light = 0.8 },
			{ name = "FrontEntrance", x0 = CENTRE, x1 = EAST, z0 = 118, z1 = 195, light = 1.1 },
			{ name = "ElevatorLobby1", x0 = CENTRE, x1 = EAST, z0 = 195, z1 = 240, light = 0.9 },
			{ name = "SecretVaults", x0 = CENTRE, x1 = EAST, z0 = 240, z1 = Z1, light = 0.75 },
		} :: { Room })
	do
		buildRoom(F.Floor1, 2, r)
	end
	wall(F.Floor1, "First_WestWall", "X", WEST, Z0, Z1, LEVELS[2], HEADS[2], {
		{ 70, 70 + DOOR_W * 2 },
		{ 210, 210 + DOOR_W },
	})
	wall(F.Floor1, "First_EastWall", "X", CENTRE, Z0, Z1, LEVELS[2], HEADS[2], {
		{ 130, 130 + DOOR_W * 2 },
		{ 250, 250 + DOOR_W },
	})
	for _, z in ipairs({ 58, 118, 195, 240 }) do
		wall(
			F.Floor1,
			`First_East_{z}`,
			"Z",
			z,
			CENTRE,
			EAST,
			LEVELS[2],
			HEADS[2],
			{ { 70, 70 + DOOR_W } }
		)
	end

	-- The vaults you can walk into: a row of heavy doors off a short passage.
	for i = 0, 3 do
		local x = CENTRE + 10 + i * 15
		box(
			F.Fixtures,
			`VaultDoor_{i}`,
			x,
			x + 11,
			LEVELS[2],
			LEVELS[2] + 13,
			262,
			264,
			C.iron,
			Enum.Material.Metal
		)
		box(
			F.Fixtures,
			`VaultWheel_{i}`,
			x + 4,
			x + 7,
			LEVELS[2] + 5,
			LEVELS[2] + 8,
			260.6,
			262.2,
			C.brass,
			Enum.Material.Metal
		)
	end
	local vaultSign = box(
		F.Fixtures,
		"VaultSign",
		CENTRE + 8,
		CENTRE + 58,
		LEVELS[2] + 15,
		LEVELS[2] + 19,
		263,
		264,
		C.timber,
		Enum.Material.WoodPlanks
	)
	label(vaultSign, "THE VAULTS", Enum.NormalId.Back, 26)

	box(
		F.Fixtures,
		"BarCounter",
		WEST + 8,
		CENTRE - 8,
		LEVELS[2],
		LEVELS[2] + 4.2,
		206,
		214,
		C.timber,
		Enum.Material.WoodPlanks
	)
	box(
		F.Fixtures,
		"BarTop",
		WEST + 6,
		CENTRE - 6,
		LEVELS[2] + 4.2,
		LEVELS[2] + 4.8,
		204,
		216,
		C.dark,
		Enum.Material.Slate
	)
	box(
		F.Fixtures,
		"BackBar",
		WEST + 8,
		CENTRE - 8,
		LEVELS[2],
		LEVELS[2] + 14,
		236,
		239,
		C.timber,
		Enum.Material.WoodPlanks
	)

	-- ------------------------------------------------------------ 2ND FLOOR
	-- The lodge floor. The Egyptian Room is the Blue Lodge Room, bottom left,
	-- and it is the heart of the building. Its own lighting is set below, so
	-- it is declared unlit here.
	for _, r in
		ipairs({
			{
				name = "EgyptianRoom",
				x0 = -W,
				x1 = 0,
				z0 = Z0,
				z1 = 96,
				light = 0,
				floor = C.egyptBlue,
			},
			{ name = "PrepareRoom", x0 = 0, x1 = 40, z0 = 30, z1 = 70, light = 0.8 },
			{ name = "GatheringArea", x0 = 40, x1 = EAST, z0 = Z0, z1 = 100, light = 1.0 },
			{ name = "CrossHall", x0 = -W, x1 = -45, z0 = 96, z1 = 122, light = 0.9 },
			{ name = "YorkRiteRoom", x0 = -W, x1 = -45, z0 = 122, z1 = 192, light = 0.85 },
			{ name = "ChapelRoom", x0 = -W, x1 = -45, z0 = 192, z1 = 252, light = 0.85 },
			{ name = "Balcony2", x0 = -W, x1 = -45, z0 = 252, z1 = Z1, light = 0.8 },
			{ name = "Hallway2", x0 = -45, x1 = 0, z0 = 96, z1 = 258, light = 0.95 },
			{ name = "PreparingRoom", x0 = -45, x1 = 0, z0 = 258, z1 = Z1, light = 0.8 },
			{ name = "Bathroom2", x0 = 0, x1 = 26, z0 = 258, z1 = Z1, light = 0.8 },
			{ name = "OESRoom", x0 = 26, x1 = EAST, z0 = 250, z1 = Z1, light = 0.9 },
			{ name = "ElevatorLobby2", x0 = 50, x1 = EAST, z0 = 205, z1 = 250, light = 0.9 },
			{ name = "StairHall2", x0 = 50, x1 = EAST, z0 = 100, z1 = 205, light = 1.0 },
			{ name = "EastRoom2", x0 = 0, x1 = 50, z0 = 100, z1 = 205, light = 0.85 },
		} :: { Room })
	do
		buildRoom(F.Floor2, 3, r)
	end
	wall(F.Floor2, "Second_Spine", "X", 0, 96, Z1, LEVELS[3], HEADS[3], {
		{ 130, 130 + DOOR_W },
		{ 230, 230 + DOOR_W },
	})
	wall(F.Floor2, "Second_West", "X", -45, 96, Z1, LEVELS[3], HEADS[3], {
		{ 150, 150 + DOOR_W },
		{ 215, 215 + DOOR_W },
		{ 265, 265 + DOOR_W },
	})
	wall(
		F.Floor2,
		"Second_Egypt",
		"Z",
		96,
		-W,
		40,
		LEVELS[3],
		HEADS[3],
		{ { -70, -70 + DOOR_W * 2 } }
	)
	wall(
		F.Floor2,
		"Second_Gather",
		"Z",
		100,
		40,
		EAST,
		LEVELS[3],
		HEADS[3],
		{ { 60, 60 + DOOR_W } }
	)
	wall(F.Floor2, "Second_East", "X", 50, 100, 250, LEVELS[3], HEADS[3], { { 140, 140 + DOOR_W } })

	-- ------------------------------------------------------------ 3RD FLOOR
	for _, r in
		ipairs({
			{ name = "Library", x0 = -W, x1 = -50, z0 = Z0, z1 = 120, light = 0.85 },
			{ name = "CigarRoom", x0 = -50, x1 = 70, z0 = Z0, z1 = 120, light = 0.8 },
			{ name = "EastRoom3", x0 = 70, x1 = EAST, z0 = Z0, z1 = 120, light = 0.8 },
			{ name = "Museum", x0 = -W, x1 = -25, z0 = 120, z1 = 232, light = 0.9 },
			{ name = "TinyRoom3", x0 = -W, x1 = -25, z0 = 232, z1 = Z1, light = 0.8 },
			{ name = "UpperHall3", x0 = -25, x1 = 50, z0 = 120, z1 = Z1, light = 0.9 },
			{ name = "StairHall3", x0 = 50, x1 = EAST, z0 = 120, z1 = 240, light = 0.95 },
			{ name = "ElevatorLobby3", x0 = 50, x1 = EAST, z0 = 240, z1 = Z1, light = 0.9 },
		} :: { Room })
	do
		buildRoom(F.Floor3, 4, r)
	end
	wall(F.Floor3, "Third_Cigar", "Z", 120, -W, EAST, LEVELS[4], HEADS[4], {
		{ -60, -60 + DOOR_W },
		{ 20, 20 + DOOR_W },
	})
	wall(F.Floor3, "Third_Library", "X", -50, Z0, 120, LEVELS[4], HEADS[4], { { 40, 40 + DOOR_W } })
	wall(
		F.Floor3,
		"Third_Museum",
		"X",
		-25,
		120,
		Z1,
		LEVELS[4],
		HEADS[4],
		{ { 160, 160 + DOOR_W } }
	)

	-- -------------------------------------------------------------- the core
	-- NOT one stack. The plans put the stair in the centre on the lower
	-- floors and move it to the east bay for the upper ones, with the
	-- elevator always beside it. The old build stacked one identical core on
	-- every level, and that single fact is most of why the hub read as a
	-- warren rather than a building.
	do
		-- Side by side in their own lanes, not stacked in one shaft.
		--
		-- Both flights used the same x band, so the bottom tread of the upper
		-- flight landed exactly on the top tread of the lower one: you climbed
		-- the first stair correctly and arrived underneath the second, with
		-- nowhere to step off. The profile up the shaft read 0 -> 9 studs and
		-- then jumped to 59 as the raycast found the flight above, and
		-- pathfinding said NOPATH to every room on every upper floor.
		--
		-- Two lanes with a well each is both the fix and what the stair hall
		-- in the photograph actually is - two runs beside each other sharing a
		-- balustrade, not one flight folded over itself.
		-- Each ramp runs to the EXACT edge of its well. Stopping two studs
		-- short left a two stud gap between the top of the ramp and the slab,
		-- and the navmesh will not bridge it: every point on the ramp was
		-- reachable right up to z = 186, and the floor at z = 196 was not.
		-- The whole building hung on those two studs.
		flight(F.Core, "Stair_0_1", 2, 20, 80, 188, LEVELS[1], LEVELS[2])
		flight(F.Core, "Stair_1_2", 22, 40, 186, 78, LEVELS[2], LEVELS[3])
		flight(F.Core, "Stair_2_3", 62, 98, 128, 226, LEVELS[3], LEVELS[4])
		-- and a landing at the head of each, so you step off onto floor
		-- rather than onto the lip of the slab.
		for _, L in ipairs({
			{ 2, 20, 184, 192, LEVELS[2] },
			{ 22, 40, 74, 82, LEVELS[3] },
			{ 62, 98, 222, 230, LEVELS[4] },
		}) do
			box(
				F.Core,
				`StairLanding_{L[5]}`,
				L[1],
				L[2],
				L[5] - 2,
				L[5],
				L[3],
				L[4],
				C.terrazzo,
				Enum.Material.Concrete
			)
		end

		-- In the elevator lobby, which is where the plan puts it: between the
		-- front entrance and the vaults. It was inside the vault room, and
		-- its shaft was cutting that room's floor out from under itself.
		local LX0, LX1, LZ0, LZ1 = 66, 100, 198, 236
		wall(F.Core, "LiftShaftW", "X", LX0, LZ0, LZ1, 0, TOP, {})
		wall(F.Core, "LiftShaftE", "X", LX1, LZ0, LZ1, 0, TOP, {})
		wall(F.Core, "LiftShaftN", "Z", LZ1, LX0, LX1, 0, TOP, {})
		for i, lvl in ipairs(LEVELS) do
			box(
				F.Core,
				`LiftSurround_{i}`,
				LX0 - 2,
				LX1 + 2,
				lvl,
				lvl + 14,
				LZ0 - 2.6,
				LZ0 - 1,
				C.brass,
				Enum.Material.Metal
			)
		end
		local car = box(
			F.Core,
			"LiftCar",
			LX0 + 2,
			LX1 - 2,
			-2,
			0,
			LZ0 + 2,
			LZ1 - 2,
			C.timber,
			Enum.Material.WoodPlanks
		)
		car:SetAttribute("Stops", "0,30,72,118")
		CollectionService:AddTag(car, "Lift")
	end

	-- ----------------------------------------------------- THE EGYPTIAN ROOM
	-- The showpiece, and the reason the building is worth modelling at all.
	-- Gold lotus columns painted with figures, a coffered ceiling on broad
	-- yellow beams, blue-purple carpet, tiered teal seating round the walls,
	-- and in the middle a gold altar with four flame-topped lamp standards.
	do
		local ex0, ex1, ez0, ez1 = -W, 0, Z0, 96
		local y, head = LEVELS[3], HEADS[3]

		for i = 0, 4 do
			local z = ez0 + (ez1 - ez0) * ((i + 0.5) / 5)
			for side, x in ipairs({ ex0 + 13, ex1 - 13 }) do
				-- Scaled off the room, not fixed. In a 42 stud room a 22 stud
				-- column is a bollard, and the photographs are all column.
				local shaft = (head - y) * 0.72
				box(
					F.Fixtures,
					`EgyptCol_{i}_{side}`,
					x - 5,
					x + 5,
					y,
					y + shaft,
					z - 5,
					z + 5,
					C.egyptGold,
					Enum.Material.Concrete
				)
				box(
					F.Fixtures,
					`EgyptCap_{i}_{side}`,
					x - 7,
					x + 7,
					y + shaft,
					y + shaft + 5,
					z - 7,
					z + 7,
					C.egyptGold,
					Enum.Material.Concrete
				)
				box(
					F.Fixtures,
					`EgyptBand_{i}_{side}`,
					x - 5.4,
					x + 5.4,
					y + shaft - 1.4,
					y + shaft,
					z - 5.4,
					z + 5.4,
					C.gilt,
					Enum.Material.Metal
				)
				box(
					F.Fixtures,
					`EgyptPanel_{i}_{side}`,
					x - 5.3,
					x + 5.3,
					y + 7,
					y + shaft - 5,
					z - 2.2,
					z + 2.2,
					C.egyptTeal
				)
			end
		end

		-- Inset from both end walls. The first beam sat half inside the south
		-- shell wall, which was the only clipping in the whole building.
		for i = 0, 5 do
			local z = ez0 + 8 + (ez1 - ez0 - 16) * (i / 5)
			box(
				F.Fixtures,
				`EgyptBeam_{i}`,
				ex0 + 2,
				ex1 - 2,
				head - 5,
				head - 1.6,
				z - 3,
				z + 3,
				C.egyptGold,
				Enum.Material.Concrete
			)
		end

		for tier = 0, 1 do
			local inset = 15 + tier * 9
			local h = y + 1.6 + tier * 1.8
			for run, spec in ipairs({
				{ ex0 + inset - 8, ex0 + inset, ez0 + inset, ez1 - inset },
				{ ex1 - inset, ex1 - inset + 8, ez0 + inset, ez1 - inset },
				{ ex0 + inset, ex1 - inset, ez0 + inset - 8, ez0 + inset },
			}) do
				box(
					F.Fixtures,
					`EgyptTier_{tier}_{run}`,
					spec[1],
					spec[2],
					y,
					h,
					spec[3],
					spec[4],
					C.timber,
					Enum.Material.WoodPlanks
				)
				box(
					F.Fixtures,
					`EgyptSeat_{tier}_{run}`,
					spec[1],
					spec[2],
					h,
					h + 1.2,
					spec[3],
					spec[4],
					C.egyptTeal,
					Enum.Material.Fabric
				)
			end
		end

		-- The altar and its four lamp standards. Relics belong here, not on
		-- plinths in the entrance hall - Chris: "relics shouldn't be in the
		-- middle of the floor."
		local cx, cz = (ex0 + ex1) / 2, (ez0 + ez1) / 2
		local altar = box(
			F.Markers,
			"Altar",
			cx - 9,
			cx + 9,
			y,
			y + 7,
			cz - 6,
			cz + 6,
			C.egyptGold,
			Enum.Material.Metal
		)
		box(
			F.Fixtures,
			"AltarTop",
			cx - 10.5,
			cx + 10.5,
			y + 7,
			y + 8,
			cz - 7.5,
			cz + 7.5,
			C.egyptTeal,
			Enum.Material.Fabric
		)
		label(altar, "THE KEEPING ALTAR", Enum.NormalId.Front, 18)
		for i, off in ipairs({ { -15, -11 }, { 15, -11 }, { -15, 11 }, { 15, 11 } }) do
			local lx, lz = cx + off[1], cz + off[2]
			box(
				F.Fixtures,
				`AltarLampPost_{i}`,
				lx - 1.4,
				lx + 1.4,
				y,
				y + 15,
				lz - 1.4,
				lz + 1.4,
				C.egyptGold,
				Enum.Material.Metal
			)
			local globe = part(
				F.Fixtures,
				`AltarLampGlobe_{i}`,
				Vector3.new(4, 6, 4),
				CFrame.new(lx, y + 22, lz),
				Color3.fromRGB(248, 242, 228)
			)
			globe.CanCollide, globe.CanQuery = false, false
			local light = Instance.new("PointLight")
			light.Color, light.Brightness, light.Range, light.Shadows = WARM, 1.1, 42, false
			light.Parent = globe
		end

		for i = 0, 3 do
			local px = ex0 + 26 + i * 18
			local slot = box(
				F.Markers,
				`RelicPlinth_{i}`,
				px,
				px + 11,
				y,
				y + 6,
				ez1 - 14,
				ez1 - 5,
				C.egyptGold,
				Enum.Material.Metal
			)
			slot:SetAttribute("Slot", i)
			CollectionService:AddTag(slot, "RelicPlinth")
			local plate = box(
				F.Fixtures,
				`RelicPlate_{i}`,
				px + 1,
				px + 10,
				y + 6,
				y + 6.6,
				ez1 - 14.4,
				ez1 - 13.8,
				C.brass,
				Enum.Material.Metal
			)
			label(plate, "UNNAMED", Enum.NormalId.Front, 16)
		end

		-- Lit low and warm: a lodge room is candles and gold, not an office.
		lightRoom(
			F.Fixtures,
			ex0,
			ex1,
			ez0,
			ez1,
			y,
			head,
			{ Brightness = 0.7, Range = 42, Spacing = 38 }
		)
	end

	-- ------------------------------------------------------ the hub fittings
	do
		local y = LEVELS[1]

		box(
			F.Fixtures,
			"LampWallFace",
			WEST + 2,
			WEST + 5,
			y,
			y + 18,
			8,
			52,
			C.timber,
			Enum.Material.WoodPlanks
		)
		local plaque = box(
			F.Fixtures,
			"LampWallPlaque",
			WEST + 5,
			WEST + 5.8,
			y + 11,
			y + 15,
			14,
			46,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(plaque, "KEEPERS OUT", Enum.NormalId.Right, 26)
		for i = 0, 7 do
			local g = part(
				F.Fixtures,
				`KeeperLamp_{i}`,
				Vector3.new(2.6, 3.4, 2.6),
				CFrame.new(WEST + 9, y + 9, 12 + i * 5),
				C.brass,
				Enum.Material.Metal
			)
			g.CanCollide, g.CanQuery = false, false
			g:SetAttribute("Slot", i)
			CollectionService:AddTag(g, "LampWallSlot")
			if i % 3 ~= 2 then
				local l = Instance.new("PointLight")
				l.Color, l.Brightness, l.Range, l.Shadows = WARM, 0.8, 22, false
				l.Parent = g
			end
		end

		local hatch = box(
			F.Fixtures,
			"KitchenHatch",
			-90,
			-58,
			y,
			y + 4.2,
			40,
			50,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(hatch, "HAND IT IN", Enum.NormalId.Top, 22)
		CollectionService:AddTag(hatch, "KitchenContribution")

		local board = box(
			F.Fixtures,
			"ClubBoard",
			-W + 4,
			-W + 5.2,
			y + 6,
			y + 16,
			150,
			200,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(board, "THE GOOD CHEER CLUB", Enum.NormalId.Right, 24)
		CollectionService:AddTag(board, "ClubBoard")

		for i = 0, 2 do
			local z = 140 + i * 50
			box(
				F.Fixtures,
				`BanquetTable_{i}`,
				-92,
				-34,
				y + 3,
				y + 3.8,
				z,
				z + 32,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for run, dx in ipairs({ -98, -28 }) do
				box(
					F.Fixtures,
					`BanquetBench_{i}_{run}`,
					dx,
					dx + 6,
					y + 1.8,
					y + 2.4,
					z + 2,
					z + 30,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
		end

		local vice = box(
			F.Fixtures,
			"WorkshopBench",
			58,
			100,
			y,
			y + 4.4,
			220,
			234,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(vice, "THE WORKSHOP", Enum.NormalId.Front, 20)
		CollectionService:AddTag(vice, "WorkshopBench")
		local plot = box(
			F.Markers,
			"WorkshopPlot",
			56,
			102,
			y,
			y + 0.3,
			238,
			268,
			C.inlay,
			Enum.Material.Fabric
		)
		plot.CanCollide = false
		CollectionService:AddTag(plot, "WorkshopPlot")

		-- The one counter in the building you buy across. data/loot.json feeds
		-- WorkshopUpgrades and GadgetCrafting and nothing sold anything.
		local qm = box(
			F.Fixtures,
			"QuartermasterCounter",
			WEST + 8,
			CENTRE - 8,
			LEVELS[2],
			LEVELS[2] + 4.2,
			244,
			252,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(qm, "QUARTERMASTER", Enum.NormalId.Front, 20)
		CollectionService:AddTag(qm, "Quartermaster")

		local rb = box(
			F.Fixtures,
			"RelicBench",
			-100,
			-62,
			LEVELS[4],
			LEVELS[4] + 4,
			150,
			200,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(rb, "THE WORKROOM", Enum.NormalId.Right, 18)
		CollectionService:AddTag(rb, "RelicBench")

		for i, nm in ipairs({ "THE WELL", "THE BELL TOWER", "THE LONG BRIDGE" }) do
			local z = 130 + (i - 1) * 54
			local site = box(
				F.Markers,
				`LandmarkSite_{i}`,
				-W + 8,
				-W + 26,
				y,
				y + 1.6,
				z,
				z + 34,
				C.terrazzo,
				Enum.Material.Concrete
			)
			site:SetAttribute("Landmark", nm)
			label(site, nm, Enum.NormalId.Top, 16)
			CollectionService:AddTag(site, "LandmarkSite")
		end

		box(
			F.Fixtures,
			"PoolTable",
			-30,
			24,
			LEVELS[4],
			LEVELS[4] + 4.4,
			40,
			70,
			C.egyptTeal,
			Enum.Material.Fabric
		)
		box(
			F.Fixtures,
			"PoolRail",
			-32,
			26,
			LEVELS[4] + 4.4,
			LEVELS[4] + 5.2,
			38,
			72,
			C.timber,
			Enum.Material.WoodPlanks
		)

		for i = 0, 3 do
			local z = 16 + i * 26
			box(
				F.Fixtures,
				`LibraryShelf_{i}`,
				-W + 8,
				-W + 20,
				LEVELS[4],
				LEVELS[4] + 14,
				z,
				z + 18,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end
	end

	-- --------------------------------------------------------------- outside
	do
		local y = LEVELS[1]
		box(
			F.Shell,
			"Apron",
			-W - 40,
			W + 40,
			-3,
			-1,
			Z0 - 92,
			Z0 - 10,
			C.limestone,
			Enum.Material.Concrete
		)
		local departure = box(
			F.Markers,
			"Departure",
			62,
			112,
			-1,
			-0.6,
			Z0 - 74,
			Z0 - 44,
			C.brass,
			Enum.Material.Metal
		)
		departure.CanCollide = false
		label(departure, "TO THE ROAD", Enum.NormalId.Top, 24)
		CollectionService:AddTag(departure, "Departure")
		for i, x in ipairs({ 58, 116 }) do
			box(
				F.Markers,
				`RoadLampPost_{i}`,
				x - 1.2,
				x + 1.2,
				-1,
				12,
				Z0 - 61,
				Z0 - 57,
				C.iron,
				Enum.Material.Metal
			)
			local head = box(
				F.Markers,
				`RoadLamp_{i}`,
				x - 2.6,
				x + 2.6,
				12,
				15,
				Z0 - 62,
				Z0 - 56,
				C.brass,
				Enum.Material.Metal
			)
			local l = Instance.new("PointLight")
			l.Color, l.Brightness, l.Range, l.Shadows = WARM, 1.2, 38, false
			l.Parent = head
		end

		local ret =
			box(F.Markers, "ReturnPoint", -16, 16, y, y + 0.4, 18, 44, C.brass, Enum.Material.Metal)
		ret.CanCollide = false
		CollectionService:AddTag(ret, "ReturnPoint")

		-- A real SpawnLocation, so arriving in the hub does not depend on a
		-- script running in the right order.
		local pad = Instance.new("SpawnLocation")
		pad.Name = "HallSpawn"
		pad.Size = Vector3.new(30, 1, 24)
		pad.CFrame = CFrame.new(0, y + 0.5, 31)
		pad.Anchored, pad.Neutral, pad.CanCollide = true, true, false
		pad.Transparency = 1
		pad.Parent = F.Markers
	end

	M.Parent = Workspace
	return M
end

return LanternholdBuilder
