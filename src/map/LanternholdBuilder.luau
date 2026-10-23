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
	-- Added when the furnishing pass reached for them and found nil: a part
	-- with a nil Color stops the whole Build, so the hub simply did not exist
	-- and the server logged "Color3 expected, got nil" once.
	trim = Color3.fromRGB(168, 160, 146), -- painted metalwork and worktops
	glass = Color3.fromRGB(198, 214, 212),
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
--- Every opening cut by wall(), so doorways can be signed afterwards. The
--- builder is a single pass per place, so this is cleared at the top of it.
local DOORWAYS: { { axis: string, at: number, a: number, b: number, y0: number } } = {}

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
		local half = WALL_T / 2
		if axis == "X" then
			box(parent, `{name}_{index}`, at - half, at + half, py0, py1, a, b, C.plaster)
		else
			box(parent, `{name}_{index}`, a, b, py0, py1, at - half, at + half, C.plaster)
		end
		-- SKIRTING BELONGS TO THE WALL, NOT THE ROOM. Chris: "i don't
		-- understand what these wooden pieces are on all the doors" - they
		-- were the skirting board, which trim() drew round the whole
		-- rectangle of every room, so it ran straight through every doorway
		-- in that room. Making it non-colliding stopped it blocking the door
		-- and left a wooden plank lying across the threshold instead.
		--
		-- Built here it exists only where there is actually a wall, so a
		-- doorway has no skirting across it because it has no wall there.
		-- Lintels (py0 > y0) get none: they are not standing on the floor.
		if py0 <= y0 + 0.01 then
			-- 0.3 proud, not 0.6. Chris: "clipping on the flashing" - every
			-- prop set against a wall was placed off the WALL face and then
			-- intersected the skirting standing proud of it.
			local sk = half + 0.3
			local base
			if axis == "X" then
				base = box(
					parent,
					`{name}_{index}_Skirt`,
					at - sk,
					at + sk,
					py0,
					py0 + 2.2,
					a,
					b,
					C.timber,
					Enum.Material.WoodPlanks
				)
			else
				base = box(
					parent,
					`{name}_{index}_Skirt`,
					a,
					b,
					py0,
					py0 + 2.2,
					at - sk,
					at + sk,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			base.CanCollide, base.CanQuery, base.CanTouch = false, false, false
		end
	end
	local run = lo
	for _, gap in ipairs(cuts) do
		local ga, gb = math.max(lo, gap[1]), math.min(hi, gap[2])
		if gb > ga then
			piece(run, ga, y0, y1)
			piece(ga, gb, y0 + DOOR_H, y1) -- the lintel over the opening
			table.insert(DOORWAYS, { axis = axis, at = at, a = ga, b = gb, y0 = y0 })
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
	[2] = { { 0, 22, 78, 190 }, { 89, 103, 208, 225 } },
	[3] = { { 20, 42, 78, 188 }, { 89, 103, 208, 225 } },
	[4] = { { 62, 84, 116, 238 }, { 89, 103, 208, 225 } },
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
		-- The skirting USED to be here, drawn round the room's whole
		-- rectangle and therefore straight through its doorways. It is built
		-- by wall() now, where the walls actually are. What is left is the
		-- painted band and the gilt line at the ceiling, which are far above
		-- any door and can run the full rectangle honestly.
		--
		-- None of it collides: it is paint.
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
		for _, strip in ipairs({ stencilStrip, giltStrip }) do
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
	opts: { Brightness: number, Range: number, Spacing: number? },
	ceilingHoles: { { number } }?
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
	-- Chris: "lights are suspended from nowhere." Two faults. The rod only
	-- got built when there were more than 5 studs of it, so in a 26 stud
	-- room - the whole bottom floor - the fitting hung in mid air with no rod
	-- at all. And where a rod WAS built it stopped at head - 1, a stud short
	-- of the ceiling it was supposed to hang from.
	-- Sixteen studs up, not twenty-two. Every light here has Shadows off, so
	-- a lamp hung high does not just light its own room, it pours through the
	-- slab into the storey above: the 1st floor measured nearly twice the
	-- light of the bottom floor purely from being sandwiched between the
	-- lamps below it and the lamps above. Hung lower, more of each pool lands
	-- on the floor it belongs to. Still well above head height.
	local mount = math.min(head - 4, floorY + 16)
	for i = 0, nx - 1 do
		for j = 0, nz - 1 do
			local lx = x0 + (x1 - x0) * ((i + 0.5) / nx)
			local lz = z0 + (z1 - z0) * ((j + 0.5) / nz)
			-- Chris: "lamp fixtures are floating weird". A lamp hung under a
			-- hole in the ceiling has nothing above it to hang from, and 59
			-- of them were sitting over the stair wells and the lift shaft
			-- with their rods ending in open air.
			local overHole = false
			for _, h in ipairs(ceilingHoles or {}) do
				if lx > h[1] - 3 and lx < h[2] + 3 and lz > h[3] - 3 and lz < h[4] + 3 then
					overHole = true
				end
			end
			if not overHole then
				if head > mount + 0.4 then
					local rod = box(
						parent,
						"LampRod",
						lx - 0.3,
						lx + 0.3,
						mount,
						head,
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

	-- THE BALUSTRADE COLLIDES, and this is why Chris said the stairs were
	-- "kind of hard to get up". The flight is 18 studs wide in a 22 stud
	-- well, pitched at 15 degrees, and NOTHING on it was solid - posts and
	-- handrail were both CanCollide false. Walk anywhere but the centre line
	-- and you went straight off the side into the drop the well cuts, so
	-- getting up meant threading the middle of a ramp with no edges.
	--
	-- The old comment said a handrail you walk into is worse than one you
	-- clip through. That was true of the version that laid one horizontal
	-- bar through the whole flight at its average height; it is not true of
	-- a balustrade that steps with the treads, which is what this is now.
	--
	-- A solid dado from the tread up to 3.6 stops you walking off, the
	-- handrail above it stops you vaulting it, and the gap between the two
	-- keeps it reading as a balustrade rather than a trench.
	local segs = math.max(6, math.floor(steps / 2))
	for side, sideX in ipairs({ { x0 - 1.4, x0 - 0.2 }, { x1 + 0.2, x1 + 1.4 } }) do
		for i = 0, segs do
			local t = i / segs
			local z = zLow + dz * t
			local ry = yLow + dy * t
			box(
				parent,
				`{name}_RailPost_{side}_{i}`,
				sideX[1],
				sideX[2],
				ry,
				ry + 7.4,
				z - 0.6,
				z + 0.6,
				C.iron,
				Enum.Material.Metal
			)
			if i < segs then
				local nt = (i + 1) / segs
				local nz, ny = zLow + dz * nt, yLow + dy * nt
				local za, zb = math.min(z, nz), math.max(z, nz)
				-- the solid part, which is the bit that keeps you on the stair
				box(
					parent,
					`{name}_RailDado_{side}_{i}`,
					sideX[1],
					sideX[2],
					math.min(ry, ny),
					math.max(ry, ny) + 3.6,
					za,
					zb,
					C.iron,
					Enum.Material.Metal
				)
				box(
					parent,
					`{name}_RailHand_{side}_{i}`,
					sideX[1] - 0.3,
					sideX[2] + 0.3,
					math.min(ry, ny) + 6.2,
					math.max(ry, ny) + 7.4,
					za,
					zb,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
		end
	end
end

--- A balustrade round the open edges of a floor well, so you cannot walk
--- into it from the floor above. `skip` names the one edge the stair arrives
--- through, which has to stay open: "n", "s", "e", "w" or nil for all four.
local function wellRail(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number,
	skip: string?
)
	local T = 1.4
	-- The flanking guards STOP SHORT of the edge the stair comes through.
	-- Run full length and they wrap past its corner, and the gap left
	-- between the end of the guard and the room's own wall was 3.6 studs -
	-- narrower than the 5 stud player, so the landing at the top of stair 1
	-- was sealed off from the rest of the floor and everything above the
	-- 1st floor went unreachable. The landing bridges the well at that end
	-- anyway, so there is nothing there to fence.
	local zA = (skip == "s") and z0 + 6 or z0 - T
	local zB = (skip == "n") and z1 - 6 or z1 + T
	local xA = (skip == "w") and x0 + 6 or x0 - T
	local xB = (skip == "e") and x1 - 6 or x1 + T
	local sides = {
		{ "w", x0 - T, x0, zA, zB },
		{ "e", x1, x1 + T, zA, zB },
		{ "s", xA, xB, z0 - T, z0 },
		{ "n", xA, xB, z1, z1 + T },
	}
	for _, sd in ipairs(sides) do
		if sd[1] ~= skip then
			box(
				parent,
				`{name}_Guard_{sd[1]}`,
				sd[2],
				sd[3],
				y,
				y + 3.6,
				sd[4],
				sd[5],
				C.iron,
				Enum.Material.Metal
			)
			local cap = box(
				parent,
				`{name}_Cap_{sd[1]}`,
				sd[2] - 0.3,
				sd[3] + 0.3,
				y + 3.6,
				y + 4.4,
				sd[4],
				sd[5],
				C.timber,
				Enum.Material.WoodPlanks
			)
			cap.CanQuery = false
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
	-- What the room is called, on a brass plaque above its south wall. The
	-- old build had one fingerpost in the entrance hall with five arms that
	-- overlapped each other and clipped the wall, and Chris: "the whole
	-- signpost does nothing to help". A building this size cannot be signed
	-- from one post. It signs itself, room by room, the way a real one does.
	title: string?,
}

-- --------------------------------------------------------------------------
function LanternholdBuilder.Build(): Model
	lampCount = 0
	table.clear(DOORWAYS)
	-- every room built this pass, so the doorways into it can be signed once
	-- all the walls exist
	local built: { { folder: Instance, room: Room, level: number } } = {}
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
		table.insert(built, { folder = folder, room = room, level = level })
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
		if room.title then
			-- The name goes OVER THE DOORS, outside, in signDoors below.
			-- Chris: "all the labels for the stuff are on the inside or the
			-- sides its just not easy to navigate." This was one brass plate
			-- on the room's own south wall facing into the room, so you could
			-- only read a room's name once you were standing in it, and if
			-- you came in from the north you read the back of it.
			local _ = room.title
		end
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
			-- Range 34 on a 36 grid, not 40. Chris: "theres a strange bright
			-- glow everywhere". At range 40 on a 36 grid every point on the
			-- floor is inside four pools at once, and DONTFUCKUPTHELIGHTS.md
			-- calls that exact failure out: "it is three or four reaching the
			-- same patch of floor". The 1st floor measured a mean of 1.29
			-- with 64% of it over the 1.2 ceiling the contract sets, because
			-- every room on that floor is full weight and the ballroom is
			-- 95 by 300. Pulling the range inside the grid spacing leaves the
			-- pools touching instead of piling up.
			lightRoom(folder, room.x0, room.x1, room.z0, room.z1, y, head, {
				Brightness = (room.light :: number) * 0.62,
				Range = 34,
				Spacing = 36,
			}, WELLS[level + 1])
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
	-- Solid. This gap was the head of the ramp; with the ramp gone it was
	-- a doorway onto nothing.
	wall(F.Shell, "WallSouth", "Z", Z0, -W, W, 0, TOP, {})

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
			-- CHRIS'S OWN ROOM LIST, 2026-10-23, and it supersedes the plans:
			-- "like the bottom floor, just a kitchen, a banquet hall, the two
			-- bathrooms, and an elevator. 1st floor the ballroom, the bar,
			-- front entrance for the steps outside, elevator and secret vaults.
			-- 2nd floor, the york rite room, the egyptian room, elevator, and
			-- stairs going up. third floor museum, library, cigar room,
			-- elevator. that should make it simplified and easy, and then we
			-- can make it all look clean with like props, and stuff. so its
			-- not overly crowded."
			--
			-- So the halls are circulation, not rooms with plaques, and every
			-- name on a wall is somewhere you went on purpose.
			{
				name = "Kitchen",
				x0 = -W,
				x1 = WEST,
				z0 = Z0,
				z1 = 110,
				light = 1.0,
				title = "THE KITCHEN",
			},
			{
				name = "BanquetHall",
				x0 = -W,
				x1 = WEST,
				z0 = 110,
				z1 = Z1,
				light = 1.0,
				title = "THE BANQUET HALL",
			},
			{
				name = "ArrivalHall",
				x0 = WEST,
				x1 = CENTRE,
				z0 = Z0,
				z1 = Z1,
				light = 1.05,
				title = "LANTERNHOLD",
			},
			{
				-- Was "THE RAMP", which was a door and a slope and no room.
				name = "Parlour",
				x0 = CENTRE,
				x1 = EAST,
				z0 = Z0,
				z1 = 95,
				light = 0.95,
				title = "THE PARLOUR",
			},
			-- Chris: "why the washrooms are all connected on the side". They
			-- were the corridor: the east strip ran ramp -> washroom ->
			-- washroom -> lift in a line, so the only way up this side of the
			-- building was through both of them, and they opened into each
			-- other. The corridor is its own lane at x 45..70 now and the two
			-- washrooms hang off it, with no door between them.
			{
				name = "EastCorridor",
				x0 = CENTRE,
				x1 = 70,
				z0 = 95,
				z1 = 195,
				light = 0.95,
			},
			{
				-- A 40 by 50 room rounds to a single lamp on the 36 stud grid,
				-- so the small rooms need a heavier weight to land in the
				-- same place as the big ones.
				name = "BathroomA",
				x0 = 70,
				x1 = EAST,
				z0 = 95,
				z1 = 145,
				light = 1.3,
				title = "WASHROOM",
			},
			{
				name = "BathroomB",
				x0 = 70,
				x1 = EAST,
				z0 = 145,
				z1 = 195,
				light = 1.3,
				title = "WASHROOM",
			},
			{
				name = "ElevatorLobby0",
				x0 = CENTRE,
				x1 = EAST,
				z0 = 195,
				z1 = Z1,
				light = 0.9,
				title = "THE LIFT",
			},
		} :: { Room })
	do
		buildRoom(F.Floor0, 1, r)
	end
	wall(F.Floor0, "Bottom_WestWall", "X", WEST, Z0, Z1, 0, HEADS[1], {
		{ 30, 30 + DOOR_W },
		{ 150, 150 + DOOR_W },
		{ 240, 240 + DOOR_W },
	})
	-- Chris: "all the rooms have too many entrances." Three, not four, and
	-- each one goes somewhere different: the ramp, the corridor, the lift.
	wall(F.Floor0, "Bottom_EastWall", "X", CENTRE, Z0, Z1, 0, HEADS[1], {
		{ 20, 20 + DOOR_W },
		{ 140, 140 + DOOR_W },
		{ 250, 250 + DOOR_W },
	})
	wall(F.Floor0, "Kitchen_North", "Z", 110, -W, WEST, 0, HEADS[1], { { -70, -70 + DOOR_W } })
	-- TWO doors in each of these, not one. The east strip was an enfilade:
	-- five rooms in a line joined only at x 70..81, so every trip up this side
	-- of the building was single file through five identical doorways, and the
	-- lift at x 93..107 opened onto the blank part of the wall three studs in
	-- front of it. The second opening is on the lift's axis, which gives the
	-- strip a second lane and puts a door where the doors actually open.
	-- The washroom block: solid on three sides, one door each off the
	-- corridor, and nothing between the two of them.
	for _, z in ipairs({ 95, 145, 195 }) do
		wall(F.Floor0, `Bottom_Wash_{z}`, "Z", z, 70, EAST, 0, HEADS[1], {})
	end
	wall(F.Floor0, "Bottom_Wash_W", "X", 70, 95, 195, 0, HEADS[1], {
		{ 112, 112 + DOOR_W },
		{ 160, 160 + DOOR_W },
	})

	-- The exit vestibule is WALLS, not a block. It was a solid lump of
	-- limestone from y -2 to the ceiling filling the whole opening, so the
	-- north door - the way out to the road, and now the way you leave for a
	-- raid - was plugged. Everything else in the building was reachable and
	-- the one thing you actually have to do was not.
	wall(F.Floor0, "VestibuleWest", "X", -18, Z1, Z1 + 26, 0, HEADS[1], {})
	wall(F.Floor0, "VestibuleEast", "X", 18, Z1, Z1 + 26, 0, HEADS[1], {})
	box(
		F.Floor0,
		"VestibuleRoof",
		-20,
		20,
		HEADS[1],
		HEADS[1] + 3,
		Z1 - 1,
		Z1 + 27,
		C.limestone,
		Enum.Material.Concrete
	)
	terrazzo(F.Floor0, "ExitFloor", -17, 17, Z1, Z1 + 25, 0)
	-- The ramp is gone. Chris: "the ramp? lets just lose it and decorate
	-- that room." It ran eleven steps out into open ground south of the
	-- building to a door nobody needed, now that you leave for a raid from
	-- the arrival hall. The opening it served is closed with it, below.

	-- ------------------------------------------------------------ 1ST FLOOR
	for _, r in
		ipairs({
			{
				name = "Ballroom",
				x0 = -W,
				x1 = WEST,
				z0 = Z0,
				z1 = Z1,
				light = 1.0,
				floor = Color3.fromRGB(104, 62, 40),
				mat = Enum.Material.WoodPlanks,
			},
			{
				name = "StairHall1",
				x0 = WEST,
				x1 = CENTRE,
				z0 = Z0,
				z1 = 195,
				light = 1.0,
			},
			{
				-- "rename the Bar to something more appropriate" - a temple
				-- social room, next door to a cigar room, is a lounge.
				name = "Bar",
				x0 = WEST,
				x1 = CENTRE,
				z0 = 195,
				z1 = Z1,
				light = 0.95,
				title = "THE LOUNGE",
			},
			{
				name = "FrontEntrance",
				x0 = CENTRE,
				x1 = EAST,
				z0 = Z0,
				z1 = 195,
				light = 1.1,
				title = "THE FRONT STEPS",
			},
			{
				name = "ElevatorLobby1",
				x0 = CENTRE,
				x1 = EAST,
				z0 = 195,
				z1 = 240,
				light = 0.9,
				title = "THE LIFT",
			},
			{
				name = "SecretVaults",
				x0 = CENTRE,
				x1 = EAST,
				z0 = 240,
				z1 = Z1,
				light = 0.75,
				title = "THE VAULTS",
			},
		} :: { Room })
	do
		buildRoom(F.Floor1, 2, r)
	end
	wall(F.Floor1, "First_WestWall", "X", WEST, Z0, Z1, LEVELS[2], HEADS[2], {
		{ 70, 70 + DOOR_W * 2 },
		{ 210, 210 + DOOR_W },
	})
	wall(F.Floor1, "First_EastWall", "X", CENTRE, Z0, Z1, LEVELS[2], HEADS[2], {
		{ 60, 60 + DOOR_W },
		{ 130, 130 + DOOR_W * 2 },
		{ 250, 250 + DOOR_W },
	})
	-- Same two doors, same reason, and on this floor the north one is how you
	-- get from the lift into the vaults without walking the length of the
	-- lobby first.
	for _, z in ipairs({ 195, 240 }) do
		-- One door, on the lift's axis, so front steps -> lift -> vaults is a
		-- straight line up the east side instead of two parallel lanes.
		wall(F.Floor1, `First_East_{z}`, "Z", z, CENTRE, EAST, LEVELS[2], HEADS[2], {
			{ 90, 90 + DOOR_W },
		})
	end

	-- THE CENTRE STRIP GETS ITS WALLS. Washroom, stair hall, bar, washrooms -
	-- four names over one unbroken 60 by 300 run of floor, so the bar was 91%
	-- open and the stair hall 83%. You read a plaque saying THE BAR and
	-- nothing around you changed. The doors line up with the stair head.
	for _, z in ipairs({ 195 }) do
		wall(F.Floor1, `First_Centre_{z}`, "Z", z, WEST, CENTRE, LEVELS[2], HEADS[2], {
			{ 8, 8 + DOOR_W },
		})
	end

	-- The vaults you can walk into: a row of heavy doors off a short passage.
	for i = 0, 3 do
		-- i * 13 from CENTRE + 8: at 15 the last door ran to x 111 and the
		-- east shell wall starts at 108.5, so one of the four was buried in
		-- the outside wall.
		local x = CENTRE + 8 + i * 13
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
			{
				name = "YorkRiteRoom",
				x0 = -W,
				x1 = 0,
				z0 = 96,
				z1 = Z1,
				light = 0.85,
				title = "THE YORK RITE ROOM",
			},
			{
				name = "StairHall2",
				x0 = 0,
				x1 = 50,
				z0 = Z0,
				z1 = Z1,
				light = 1.0,
			},
			{
				name = "ElevatorLobby2",
				x0 = 50,
				x1 = EAST,
				z0 = Z0,
				z1 = Z1,
				light = 0.9,
				title = "THE LIFT",
			},
		} :: { Room })
	do
		buildRoom(F.Floor2, 3, r)
	end
	-- The whole depth now: Egyptian Room and York Rite on the west of it,
	-- the stair hall on the east.
	wall(F.Floor2, "Second_Spine", "X", 0, Z0, Z1, LEVELS[3], HEADS[3], {
		{ 40, 40 + DOOR_W },
		{ 180, 180 + DOOR_W },
	})
	-- Second_West, Second_Gather, Second_North and the two west cross walls
	-- are gone with the rooms they divided. The 2nd floor is four rooms.
	-- To x 0, not x 40. It ran 40 studs past the Egyptian Room's corner and
	-- finished up standing four studs inside the gathering area, which is one
	-- of the walls Chris meant by "walls are weird".
	-- SOLID. Chris: "the two rooms shouldn't connect." The Egyptian Room is
	-- the one room in the building you are supposed to arrive at rather than
	-- pass through, and a 22 stud opening straight into the York Rite room
	-- made the pair of them read as one long hall with a change of carpet.
	-- It is reached from the stair hall through the spine door at z 40..51.
	wall(F.Floor2, "Second_Egypt", "Z", 96, -W, 0, LEVELS[3], HEADS[3], {})
	-- From x 0 now, so the gathering area has a north wall across its whole
	-- width. The first door is on the stair head so you come up out of the
	-- stair and through it.

	-- Two doors. With one at z 140 the lift lobby's only way out was north
	-- through the OES room, and a lift that opens onto a single door through
	-- somebody else's room is most of what "the flow is pretty rough" means.
	-- THE WEST STRIP. York Rite, Chapel and Balcony sat in one 65 by 204 run
	-- of open floor - the chapel stood 72% open and the York Rite room 74%,
	-- which is why neither of them felt like a room you had walked into.

	-- and the north end of the floor, across the stair hall and the lift

	-- Stair hall | lift core, the full depth. The lift is at x 92..108 and
	-- stair C starts at x 64..84, so "elevator, and stairs going up" is one
	-- room on this floor.
	wall(F.Floor2, "Second_East", "X", 50, Z0, Z1, LEVELS[3], HEADS[3], {
		{ 40, 40 + DOOR_W },
		{ 180, 180 + DOOR_W },
	})

	-- ------------------------------------------------------------ 3RD FLOOR
	for _, r in
		ipairs({
			{
				name = "Library",
				x0 = -W,
				x1 = -50,
				z0 = Z0,
				z1 = 120,
				light = 0.85,
				title = "THE LIBRARY",
			},
			{
				name = "CigarRoom",
				x0 = -50,
				x1 = 50,
				z0 = Z0,
				z1 = 120,
				light = 0.8,
				title = "THE CIGAR ROOM",
			},
			{
				name = "Museum",
				x0 = -W,
				x1 = 50,
				z0 = 120,
				z1 = Z1,
				light = 0.9,
				title = "THE MUSEUM",
			},
			{
				name = "ElevatorLobby3",
				x0 = 50,
				x1 = EAST,
				z0 = Z0,
				z1 = Z1,
				light = 0.9,
				title = "THE LIFT",
			},
		} :: { Room })
	do
		buildRoom(F.Floor3, 4, r)
	end
	wall(F.Floor3, "Third_Cigar", "Z", 120, -W, 50, LEVELS[4], HEADS[4], {
		{ -60, -60 + DOOR_W },
		{ 20, 20 + DOOR_W },
	})
	wall(F.Floor3, "Third_Library", "X", -50, Z0, 120, LEVELS[4], HEADS[4], { { 40, 40 + DOOR_W } })
	-- Third_Museum is gone: the museum is the whole north of this floor now.

	-- The top floor's east side was open to the upper hall down its whole
	-- length, so the stair and lift core and the hall read as one 135 stud
	-- room with two names. This is the wall between them.
	wall(F.Floor3, "Third_Core", "X", 50, Z0, Z1, LEVELS[4], HEADS[4], {
		{ 40, 40 + DOOR_W },
		{ 180, 180 + DOOR_W },
	})

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
		-- 18 wide like the other two, over a longer run. Chris: "stairs don't
		-- look uniform or correct" - this one was 20 wide at 25 degrees while
		-- the flights below it are 18 at 15 and 20.
		flight(F.Core, "Stair_2_3", 64, 82, 118, 236, LEVELS[3], LEVELS[4])
		-- and a landing at the head of each, so you step off onto floor
		-- rather than onto the lip of the slab.
		for _, L in ipairs({
			{ 2, 20, 184, 192, LEVELS[2] },
			{ 22, 40, 74, 82, LEVELS[3] },
			{ 64, 82, 232, 240, LEVELS[4] },
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

		-- A BALUSTRADE ROUND EVERY STAIR WELL. Chris: "big open space over
		-- the stairs?" - and nothing at all guarded those edges, so the hole
		-- the stair climbs through was an unfenced drop on the floor above
		-- it. The edge the flight arrives through stays open; the other
		-- three are fenced.
		wellRail(F.Core, "Well_1", 0, 22, 78, 190, LEVELS[2], "n")
		wellRail(F.Core, "Well_2", 20, 42, 78, 188, LEVELS[3], "s")
		wellRail(F.Core, "Well_3", 62, 84, 116, 238, LEVELS[4], "n")

		-- ON THE EAST WALL, and a lift rather than a room. Chris: "the
		-- elevator at one point brings u to an inescapable room, well,
		-- actually, i fell through the floor so i did get out lol."
		--
		-- Both halves of that were this one object, and it was a 34 by 38 stud
		-- shaft - bigger than most of the rooms it opened into. It is 13 by 16
		-- now, which is a lift, and it sits against the east shell wall where
		-- every floor's lift lobby already reaches, so the car arrives in the
		-- room named after it on all four stops instead of two.
		--
		-- It also stood 4 studs clear of its own well on every side, which is
		-- the hole he fell down: a trench right across the mouth you stepped
		-- into on the way out. The well is now the car plus half a stud.
		-- x 88..104, not 92..108. Chris: "the lift's entrance is just
		-- sideways and weird" - the east jamb ran x 106.5..110 and the shell
		-- wall occupies 108.5..111.5, so half of that jamb was buried in the
		-- outside wall and the frame read as one-sided. Four studs west puts
		-- both jambs in the room and the opening centred between them.
		local LX0, LX1, LZ0, LZ1 = 88, 104, 208, 226
		wall(F.Core, "LiftShaftW", "X", LX0, LZ0, LZ1, 0, TOP, {})
		wall(F.Core, "LiftShaftE", "X", LX1, LZ0, LZ1, 0, TOP, {})
		wall(F.Core, "LiftShaftN", "Z", LZ1, LX0, LX1, 0, TOP, {})
		-- THE LIFT IS A DOORWAY IN A WALL NOW. Chris, twice: "the lift's
		-- entrance is just sideways and weird... still awkward and sideways."
		-- Both earlier attempts kept it as free-standing brass - first a slab
		-- across the opening, then two jambs and a head - and all of that
		-- stood at z 205.4..207 while the shaft mouth is at z 208, so the
		-- frame hovered a stud and a half in FRONT of the hole it framed,
		-- didn't line up with the shaft walls either side of it, and read as
		-- an arch dumped in the room rather than a lift.
		--
		-- The shaft gets a proper front wall at its mouth with a door cut in
		-- it, built by the same wall() every other door in the building uses,
		-- so it gets the same reveal, the same lintel and the same skirting
		-- and sits in the plane of the shaft instead of floating off it.
		for i, lvl in ipairs(LEVELS) do
			local top = LEVELS[i + 1] or TOP
			wall(F.Core, `LiftFace_{i}`, "Z", LZ0, LX0 - 1.5, LX1 + 1.5, lvl, top, {
				{ LX0 + 1.5, LX1 - 1.5 },
			})
			-- Chris: "why the elevator is hidden". A lit brass sign over the
			-- opening, flush to the face wall it is mounted on.
			local sign = box(
				F.Core,
				`LiftSign_{i}`,
				LX0 + 1.5,
				LX1 - 1.5,
				lvl + DOOR_H + 0.8,
				lvl + DOOR_H + 5.4,
				LZ0 - 2.2,
				LZ0 - 1.5,
				C.brass,
				Enum.Material.Metal
			)
			sign.CanCollide, sign.CanQuery = false, false
			label(sign, "LIFT", Enum.NormalId.Front, 24)
			local sl = Instance.new("PointLight")
			sl.Color, sl.Brightness, sl.Range, sl.Shadows = WARM, 0.7, 22, false
			sl.Parent = sign

			-- A LANDING DOOR, because the other three sides of this opening
			-- are now a wall and the fourth is a 30 stud drop whenever the
			-- car is at another floor. LiftService opens the one the car is
			-- standing at and keeps the rest shut.
			local gate = box(
				F.Core,
				`LiftGate_{i}`,
				LX0 + 1.5,
				LX1 - 1.5,
				lvl,
				lvl + DOOR_H,
				LZ0 - 0.5,
				LZ0 + 0.5,
				C.iron,
				Enum.Material.Metal
			)
			gate:SetAttribute("Stop", lvl)
			CollectionService:AddTag(gate, "LiftGate")
		end
		local car = box(
			F.Core,
			"LiftCar",
			LX0 + 1.5,
			LX1 - 1.5,
			-2,
			0,
			LZ0 + 0.5,
			LZ1 - 1.5,
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
			-- The east run is cut either side of the spine doorway at
			-- z 40..51. Sealing the room off from the York Rite room left
			-- that door as its only way in, and the seating ran straight
			-- across it seven studs inside: a 2.8 stud step up onto a bench
			-- is more than a Humanoid takes without jumping, so the whole
			-- Egyptian Room went unreachable the moment the other door shut.
			for run, spec in ipairs({
				{ ex0 + inset - 8, ex0 + inset, ez0 + inset, ez1 - inset },
				{ ex1 - inset, ex1 - inset + 8, ez0 + inset, 36 },
				{ ex1 - inset, ex1 - inset + 8, 55, ez1 - inset },
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

		-- IN THE WALL BETWEEN THE KITCHEN AND THE BANQUET HALL, which is what
		-- a serving hatch is. It was at x -90..-58, z 40..50 - the middle of
		-- the kitchen floor, buried under the prep table (x -74..-40) and
		-- running into the ranges (x -104..-86). Pathfinding could not reach
		-- it from any side: every approach landed on the table top.
		--
		-- x -100..-75 keeps it clear of the doorway at x -70..-59.
		local hatch = box(
			F.Fixtures,
			"KitchenHatch",
			-100,
			-75,
			y,
			y + 4.2,
			106,
			114,
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
			-- North of the shaft. This bench ran x 58..100 by z 220..234,
			-- which put it straight through the old lift shaft's west wall
			-- and left a third of it inside the lift.
			"WorkshopBench",
			58,
			100,
			y,
			y + 4.4,
			248,
			262,
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
			268,
			296,
			C.inlay,
			Enum.Material.Fabric
		)
		plot.CanCollide = false
		CollectionService:AddTag(plot, "WorkshopPlot")

		-- The one counter in the building you buy across. data/loot.json feeds
		-- WorkshopUpgrades and GadgetCrafting and nothing sold anything.
		local qm = box(
			F.Fixtures,
			-- Chris: "why is it so smack dab against the doorway?" The east
			-- wall's door into this room is at z 250..261 and the counter ran
			-- z 244..252, so you walked in through the side of it. Twenty
			-- studs south puts it clear of both doors with room to stand at.
			"QuartermasterCounter",
			WEST + 8,
			CENTRE - 8,
			LEVELS[2],
			LEVELS[2] + 4.2,
			224,
			232,
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

		-- A POOL TABLE, not a stage. This was 54 by 30 studs - wider than
		-- some of the rooms - standing flat on the floor with no legs, and
		-- its "rail" was a second slab laid across the whole top, so the
		-- cloth was hidden completely and the thing read as a teal platform.
		do
			local y = LEVELS[4]
			box(
				F.Fixtures,
				"PoolCloth",
				-13,
				13,
				y + 2.6,
				y + 3.4,
				48,
				62,
				Color3.fromRGB(32, 74, 48),
				Enum.Material.Fabric
			)
			for i, r in ipairs({
				{ -14.2, -13, 46.8, 63.2 },
				{ 13, 14.2, 46.8, 63.2 },
				{ -14.2, 14.2, 46.8, 48 },
				{ -14.2, 14.2, 62, 63.2 },
			}) do
				box(
					F.Fixtures,
					`PoolRail_{i}`,
					r[1],
					r[2],
					y + 2.6,
					y + 4.2,
					r[3],
					r[4],
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			for _, cx in ipairs({ -11, 11 }) do
				for _, cz in ipairs({ 50, 60 }) do
					box(
						F.Fixtures,
						"PoolLeg",
						cx - 1.4,
						cx + 1.4,
						y,
						y + 2.6,
						cz - 1.4,
						cz + 1.4,
						C.timber,
						Enum.Material.WoodPlanks
					)
				end
			end
			local shade = box(
				F.Fixtures,
				"PoolLamp",
				-9,
				9,
				y + 11,
				y + 12.6,
				51,
				59,
				C.brass,
				Enum.Material.Metal
			)
			shade.CanCollide, shade.CanQuery = false, false
			local pl = Instance.new("PointLight")
			pl.Color, pl.Brightness, pl.Range, pl.Shadows = WARM, 1.1, 34, false
			pl.Parent = shade
		end

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

	-- ---------------------------------------------------------- furnishings
	-- Chris: "Kitchen? Eating area? Ballroom? Furnishings? Bathrooms? Lol!"
	-- A room with a name and no contents is a labelled box. These are what
	-- each room is FOR, and between them they are most of the difference
	-- between a floor plan and a building.
	do
		local B, L1, L2, L3 = LEVELS[1], LEVELS[2], LEVELS[3], LEVELS[4]

		--- A chair, facing +Z unless flipped.
		local function chair(x: number, z: number, y: number, face: number)
			box(
				F.Fixtures,
				"Chair",
				x - 2.4,
				x + 2.4,
				y,
				y + 2.4,
				z - 2.4,
				z + 2.4,
				C.timber,
				Enum.Material.WoodPlanks
			)
			box(
				F.Fixtures,
				"ChairBack",
				x - 2.4,
				x + 2.4,
				y + 2.4,
				y + 6,
				z + face * 1.8,
				z + face * 2.4,
				C.egyptTeal,
				Enum.Material.Fabric
			)
		end

		-- ---- THE KITCHEN. Ranges down one wall, a long prep table, and the
		-- pass through to the banquet hall where the hatch already is.
		for i = 0, 3 do
			local z = 14 + i * 22
			box(
				F.Fixtures,
				`Range_{i}`,
				-W + 6,
				-W + 24,
				B,
				B + 5,
				z,
				z + 16,
				C.iron,
				Enum.Material.Metal
			)
			box(
				F.Fixtures,
				`RangeHood_{i}`,
				-W + 5,
				-W + 26,
				B + 13,
				B + 19,
				z - 1,
				z + 17,
				C.iron,
				Enum.Material.Metal
			)
		end
		box(F.Fixtures, "PrepTable", -74, -40, B, B + 4.4, 20, 92, C.trim, Enum.Material.Metal)
		for i = 0, 2 do
			box(
				F.Fixtures,
				`KitchenShelf_{i}`,
				-36,
				-22,
				B,
				B + 12,
				16 + i * 30,
				32 + i * 30,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end

		-- ---- THE BANQUET HALL is the eating area, and three long tables is
		-- a refectory. Chairs down both sides of each so it reads as a place
		-- people sit rather than a place tables are stored.
		-- Chris: "banquet hall furnishings are all idk what they are lol."
		-- Fifteen chairs each side of an empty aisle, because the three long
		-- tables the comment promised were never built. Here they are.
		for i = 0, 2 do
			local z = 140 + i * 50
			box(
				F.Fixtures,
				`RefectoryTop_{i}`,
				-88,
				-38,
				B + 4,
				B + 4.8,
				z,
				z + 34,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for _, ex in ipairs({ -84, -42 }) do
				box(
					F.Fixtures,
					`RefectoryLeg_{i}`,
					ex - 1.6,
					ex + 1.6,
					B,
					B + 4,
					z + 2,
					z + 32,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			for k = 0, 4 do
				chair(-94, z + 5 + k * 6, B, 1)
				chair(-32, z + 5 + k * 6, B, -1)
			end
		end
		box(
			F.Fixtures,
			"ServingTable",
			-92,
			-34,
			B + 3,
			B + 4,
			268,
			288,
			C.timber,
			Enum.Material.WoodPlanks
		)

		-- ---- WASHROOMS. Stalls and basins, on both of the bottom floor's
		-- and the ones above. Cheap, and their absence is the kind of thing
		-- that makes a building read as a mock-up.
		-- Needs at least 34 studs of width: stalls are set out from the left
		-- wall and basins from the right, and under that they meet in the
		-- middle and leave no lane a player can walk down.
		-- The door into these is in the x0 wall, in the middle of it. Chris:
		-- "bathrooms have toilets at their entrances" - and they did: the
		-- stalls were set out from x0, so the first thing through the door
		-- was a partition across your face and a pan behind it.
		--
		-- Stalls go against the FAR wall now and the basins sit at the two
		-- ends of the near wall, which leaves the middle of the near wall -
		-- where you come in - clear, and gives you the length of the room to
		-- walk down. Needs 34 studs of width for the two runs plus a lane.
		local function washroom(x0: number, x1: number, z0: number, z1: number, y: number)
			assert(x1 - x0 >= 34, `washroom {x0}..{x1} is too narrow to walk into`)
			for i = 0, 2 do
				local z = z0 + 6 + i * 11
				box(F.Fixtures, "StallWall", x1 - 18, x1, y, y + 11, z, z + 0.8, C.trim)
				box(F.Fixtures, "Pan", x1 - 14, x1 - 9, y, y + 3.4, z + 3, z + 8, C.plaster)
			end
			-- basins at the ends of the near wall, clear of the doorway
			for i, z in ipairs({ z0 + 3, z1 - 10 }) do
				box(F.Fixtures, `Basin_{i}`, x0 + 2, x0 + 10, y + 4, y + 5.4, z, z + 7, C.plaster)
				box(
					F.Fixtures,
					`Mirror_{i}`,
					x0 + 1.2,
					x0 + 1.8,
					y + 7,
					y + 15,
					z,
					z + 7,
					C.glass,
					Enum.Material.Glass,
					0.25
				)
			end
		end
		-- The two bathrooms are both on the bottom floor, which is where
		-- Chris's list puts them and the only floor that has any.
		washroom(70, EAST, 95, 145, B)
		washroom(70, EAST, 145, 195, B)

		-- ---- THE BALLROOM. Chandeliers on the beam line, a dais at the north
		-- end for the band, and chairs round the walls the way a ballroom is
		-- laid out when it is not being danced in.
		for i = 0, 4 do
			local z = 70 + i * 48
			local body = part(
				F.Fixtures,
				`Chandelier_{i}`,
				Vector3.new(16, 3, 16),
				CFrame.new(-62, HEADS[2] - 14, z),
				C.brass,
				Enum.Material.Metal
			)
			body.CanCollide, body.CanQuery = false, false
			local chain = part(
				F.Fixtures,
				`ChandelierChain_{i}`,
				Vector3.new(0.8, 11, 0.8),
				CFrame.new(-62, HEADS[2] - 7, z),
				C.iron,
				Enum.Material.Metal
			)
			chain.CanCollide, chain.CanQuery = false, false
			local l = Instance.new("PointLight")
			l.Color, l.Brightness, l.Range, l.Shadows = WARM, 0.5, 34, false
			l.Parent = body
		end
		box(
			F.Fixtures,
			"BallroomDais",
			-96,
			-28,
			L1,
			L1 + 3,
			262,
			292,
			C.timber,
			Enum.Material.WoodPlanks
		)
		for i = 0, 7 do
			chair(-W + 9, 60 + i * 28, L1, 1)
			chair(WEST - 9, 60 + i * 28, L1, -1)
		end

		-- ---- THE CHAPEL and THE YORK RITE ROOM. Both are meeting rooms:
		-- benches facing a lectern at the far end.
		local function meetingRoom(
			x0: number,
			x1: number,
			z0: number,
			z1: number,
			y: number,
			nm: string
		)
			for i = 0, 4 do
				local z = z0 + 12 + i * 10
				box(
					F.Fixtures,
					`{nm}_Bench_{i}`,
					x0 + 12,
					x1 - 12,
					y + 1.4,
					y + 2.2,
					z,
					z + 5,
					C.timber,
					Enum.Material.WoodPlanks
				)
				box(
					F.Fixtures,
					`{nm}_BenchBack_{i}`,
					x0 + 12,
					x1 - 12,
					y + 2.2,
					y + 6.4,
					z + 4.2,
					z + 5,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			local lectern = box(
				F.Fixtures,
				`{nm}_Lectern`,
				(x0 + x1) / 2 - 5,
				(x0 + x1) / 2 + 5,
				y,
				y + 7,
				z1 - 16,
				z1 - 10,
				C.timber,
				Enum.Material.WoodPlanks
			)
			label(lectern, nm, Enum.NormalId.Front, 16)
		end
		-- One meeting room on the lodge floor, not three. The chapel and the
		-- OES room are not on Chris's list for the 2nd floor.
		meetingRoom(-W, 0, 120, 240, L2, "YORK RITE")

		-- ---- THE FRONT ENTRANCE. A porter's desk and a pair of doors, so
		-- the public way in reads as the front of a building.
		local desk = box(
			F.Fixtures,
			"PorterDesk",
			56,
			90,
			L1,
			L1 + 4.4,
			160,
			172,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(desk, "LANTERNHOLD", Enum.NormalId.Front, 20)
		box(
			F.Fixtures,
			"PorterTop",
			54,
			92,
			L1 + 4.4,
			L1 + 5,
			158,
			174,
			C.dark,
			Enum.Material.Slate
		)

		-- ---- THE MUSEUM. Cases, which is what a museum is, and what makes
		-- the relic bench upstairs mean something.
		for i = 0, 3 do
			local z = 130 + i * 24
			box(
				F.Fixtures,
				`Case_{i}`,
				-W + 10,
				-W + 26,
				L3,
				L3 + 3,
				z,
				z + 16,
				C.timber,
				Enum.Material.WoodPlanks
			)
			box(
				F.Fixtures,
				`CaseGlass_{i}`,
				-W + 10,
				-W + 26,
				L3 + 3,
				L3 + 11,
				z,
				z + 16,
				C.glass,
				Enum.Material.Glass,
				0.62
			)
		end
		-- ------------------------------------------------------------ props
		-- Chris: "we can make it all look clean with like props, and stuff.
		-- so its not overly crowded." These go round the EDGES of rooms and
		-- down the sides of halls, never in the middle of a route, and a lot
		-- of bare floor is left on purpose.

		--- A bench against a wall. `back` is the side the backrest sits on.
		local function bench(
			nm: string,
			x: number,
			z: number,
			y: number,
			long: string,
			back: number
		)
			local hx = long == "X" and 9 or 2.4
			local hz = long == "X" and 2.4 or 9
			box(
				F.Fixtures,
				`{nm}_Seat`,
				x - hx,
				x + hx,
				y + 2.6,
				y + 3.4,
				z - hz,
				z + hz,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for _, e in ipairs({ -1, 1 }) do
				local lx = long == "X" and x + e * (hx - 1.2) or x
				local lz = long == "X" and z or z + e * (hz - 1.2)
				box(
					F.Fixtures,
					`{nm}_Leg`,
					lx - 1,
					lx + 1,
					y,
					y + 2.6,
					lz - 1,
					lz + 1,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			local bx0 = long == "X" and x - hx or x + back * 1.6
			local bx1 = long == "X" and x + hx or x + back * 2.4
			local bz0 = long == "X" and z + back * 1.6 or z - hz
			local bz1 = long == "X" and z + back * 2.4 or z + hz
			box(
				F.Fixtures,
				`{nm}_Back`,
				bx0,
				bx1,
				y + 3.4,
				y + 7.4,
				bz0,
				bz1,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end

		--- A planter. The foliage does not collide, so it cannot wall off a lane.
		local function planter(nm: string, x: number, z: number, y: number)
			local pot = box(
				F.Fixtures,
				`{nm}_Pot`,
				x - 3.4,
				x + 3.4,
				y,
				y + 4.4,
				z - 3.4,
				z + 3.4,
				C.inlay,
				Enum.Material.Slate
			)
			local leaf = box(
				F.Fixtures,
				`{nm}_Leaf`,
				x - 4.2,
				x + 4.2,
				y + 4.4,
				y + 13,
				z - 4.2,
				z + 4.2,
				Color3.fromRGB(58, 84, 52),
				Enum.Material.Grass
			)
			pot.CanQuery, leaf.CanCollide, leaf.CanQuery = false, false, false
		end

		--- A runner down a hall: paint, nothing to trip over.
		local function runner(nm: string, x0: number, x1: number, z0: number, z1: number, y: number)
			local r = box(
				F.Fixtures,
				nm,
				x0,
				x1,
				y + 0.22,
				y + 0.3,
				z0,
				z1,
				Color3.fromRGB(86, 38, 36),
				Enum.Material.Fabric
			)
			r.CanCollide, r.CanQuery, r.CanTouch = false, false, false
		end

		-- ---- THE ARRIVAL HALL: 60 by 300 studs of bare floor, and the first
		-- thing anybody sees. The stair runs up the middle at x 2..20,
		-- z 80..188 with the departure pad beside it, so all of this hugs
		-- the walls and leaves the centre clear.
		runner("HallRunner_S", -6, 36, 46, 78, B)
		runner("HallRunner_N", -6, 36, 192, 296, B)
		for i, z in ipairs({ 62, 200, 224 }) do
			bench(`HallBenchW_{i}`, WEST + 7, z, B, "Z", -1)
		end
		for i, z in ipairs({ 200, 224 }) do
			bench(`HallBenchE_{i}`, CENTRE - 7, z, B, "Z", 1)
		end
		for i, pt in ipairs({
			{ WEST + 8, 185 },
			{ CENTRE - 8, 185 },
			{ WEST + 8, 288 },
			{ CENTRE - 8, 288 },
		}) do
			planter(`HallPlant_{i}`, pt[1], pt[2], B)
		end

		-- ---- THE LIBRARY, which had a name on the door and no books in it.
		for i = 0, 4 do
			local z = 12 + i * 21
			box(
				F.Fixtures,
				`Shelf_{i}`,
				-W + 4,
				-W + 13,
				L3,
				L3 + 17,
				z,
				z + 17,
				C.timber,
				Enum.Material.WoodPlanks
			)
			local bk = box(
				F.Fixtures,
				`Books_{i}`,
				-W + 13,
				-W + 14.2,
				L3 + 2,
				L3 + 15,
				z + 1,
				z + 16,
				C.inlay,
				Enum.Material.Fabric
			)
			bk.CanCollide, bk.CanQuery = false, false
		end
		box(
			F.Fixtures,
			"ReadingTable",
			-92,
			-66,
			L3,
			L3 + 4.4,
			56,
			84,
			C.timber,
			Enum.Material.WoodPlanks
		)
		for _, z in ipairs({ 62, 78 }) do
			chair(-96, z, L3, 1)
			chair(-62, z, L3, -1)
		end

		-- ---- THE LOUNGE. The quartermaster counter is already here; this
		-- is the back shelf and the stools that make it read as a bar. All
		-- of it moved twenty studs south with the counter, which was sitting
		-- across the east door.
		box(
			F.Fixtures,
			"BarBackShelf",
			WEST + 8,
			CENTRE - 8,
			L1,
			L1 + 15,
			236,
			240,
			C.timber,
			Enum.Material.WoodPlanks
		)
		for i = 0, 11 do
			local x = WEST + 11 + i * 4.4
			local b = box(
				F.Fixtures,
				`Bottle_{i}`,
				x - 1,
				x + 1,
				L1 + 9,
				L1 + 12.4,
				234.6,
				236.6,
				C.glass,
				Enum.Material.Glass,
				0.4
			)
			b.CanCollide, b.CanQuery = false, false
		end
		for i = 0, 5 do
			local x = WEST + 12 + i * 7
			box(
				F.Fixtures,
				`Stool_{i}`,
				x - 2.2,
				x + 2.2,
				L1,
				L1 + 4.4,
				217,
				221.4,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end

		-- ---- THE FRONT STEPS. Somewhere to wait by the porter desk.
		for i, z in ipairs({ 48, 92 }) do
			bench(`EntranceBench_{i}`, EAST - 8, z, L1, "Z", 1)
		end

		-- ---- THE CIGAR ROOM. Armchairs round the walls, clear of the table.
		for i, pt in ipairs({ { -40, 18 }, { -40, 100 }, { 36, 18 }, { 36, 100 } }) do
			box(
				F.Fixtures,
				`Armchair_{i}`,
				pt[1] - 5,
				pt[1] + 5,
				L3,
				L3 + 3.4,
				pt[2] - 5,
				pt[2] + 5,
				Color3.fromRGB(122, 68, 56),
				Enum.Material.Fabric
			)
			box(
				F.Fixtures,
				`ArmchairBack_{i}`,
				pt[1] - 5,
				pt[1] + 5,
				L3 + 3.4,
				L3 + 8.6,
				pt[2] + 3.4,
				pt[2] + 5,
				Color3.fromRGB(122, 68, 56),
				Enum.Material.Fabric
			)
		end

		-- ------------------------------------------- the rooms with nothing
		-- Seven rooms measured zero props and three more were under one per
		-- thousand square studs. Everything below hugs a wall: the lift
		-- lobbies carry the lift shaft and a staircase through the middle of
		-- them, so the centre of those rooms has to stay empty.

		--- A side table against a wall, with a lamp on it.
		local function console(nm: string, x: number, z: number, y: number, long: string)
			local hx = long == "X" and 11 or 2.6
			local hz = long == "X" and 2.6 or 11
			box(
				F.Fixtures,
				`{nm}_Top`,
				x - hx,
				x + hx,
				y + 4,
				y + 4.8,
				z - hz,
				z + hz,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for _, e in ipairs({ -1, 1 }) do
				local lx = long == "X" and x + e * (hx - 1.4) or x
				local lz = long == "X" and z or z + e * (hz - 1.4)
				box(
					F.Fixtures,
					`{nm}_Leg`,
					lx - 1,
					lx + 1,
					y,
					y + 4,
					lz - 1,
					lz + 1,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
		end

		--- A glass display case, for the museum.
		local function showcase(
			nm: string,
			x0: number,
			x1: number,
			z0: number,
			z1: number,
			y: number
		)
			box(
				F.Fixtures,
				`{nm}_Plinth`,
				x0,
				x1,
				y,
				y + 3.4,
				z0,
				z1,
				C.timber,
				Enum.Material.WoodPlanks
			)
			local g = box(
				F.Fixtures,
				`{nm}_Glass`,
				x0,
				x1,
				y + 3.4,
				y + 11,
				z0,
				z1,
				C.glass,
				Enum.Material.Glass,
				0.62
			)
			g.CanQuery = false
		end

		-- ---- THE PARLOUR, which is the room the ramp used to run into.
		-- Chris: "lets just lose it and decorate that room."
		do
			local y = B
			runner("ParlourRug", 58, 98, 28, 72, y)
			-- a fireplace on the outside wall, with chairs drawn up to it
			box(
				F.Fixtures,
				"ParlourChimney",
				103,
				EAST,
				y,
				y + 22,
				36,
				64,
				C.limestone,
				Enum.Material.Concrete
			)
			box(F.Fixtures, "ParlourHearth", 99, 103, y, y + 1, 40, 60, C.dark, Enum.Material.Slate)
			local fire = box(
				F.Fixtures,
				"ParlourFire",
				100,
				103,
				y + 1,
				y + 5,
				44,
				56,
				Color3.fromRGB(196, 104, 42),
				Enum.Material.Neon
			)
			fire.CanCollide, fire.CanQuery = false, false
			local fl = Instance.new("PointLight")
			fl.Color, fl.Brightness, fl.Range, fl.Shadows =
				Color3.fromRGB(255, 168, 96), 0.9, 30, false
			fl.Parent = fire
			box(
				F.Fixtures,
				"ParlourMantel",
				100,
				105,
				y + 10,
				y + 11.4,
				34,
				66,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for i, pt in ipairs({ { 88, 36 }, { 88, 64 }, { 70, 42 }, { 70, 58 } }) do
				box(
					F.Fixtures,
					`ParlourChair_{i}`,
					pt[1] - 5,
					pt[1] + 5,
					y,
					y + 3.4,
					pt[2] - 5,
					pt[2] + 5,
					Color3.fromRGB(122, 68, 56),
					Enum.Material.Fabric
				)
				box(
					F.Fixtures,
					`ParlourChairBack_{i}`,
					pt[1] - 5,
					pt[1] - 3.4,
					y + 3.4,
					y + 8.6,
					pt[2] - 5,
					pt[2] + 5,
					Color3.fromRGB(122, 68, 56),
					Enum.Material.Fabric
				)
			end
			-- a card table in the corner away from the fire
			box(
				F.Fixtures,
				"CardTable",
				56,
				72,
				y + 4,
				y + 4.8,
				74,
				90,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for _, c in ipairs({ { 52, 82 }, { 76, 82 } }) do
				box(
					F.Fixtures,
					"CardChair",
					c[1] - 2.4,
					c[1] + 2.4,
					y,
					y + 4.4,
					c[2] - 2.4,
					c[2] + 2.4,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			console("ParlourSideboard", 76, 6, y, "X")
			planter("ParlourPlant_1", 50, 88, y)
		end

		-- ---- THE LIFT LOBBIES. All four are circulation with a shaft and,
		-- upstairs, a staircase running through them, so this is wall work
		-- only: a bench to wait on and something green at the corners.
		do
			-- bottom floor: the lobby also holds the workshop, so it gets
			-- less than the others
			bench("LiftBench_0", EAST - 8, 250, B, "Z", 1)
			planter("LiftPlant_0", CENTRE + 8, 200, B)
		end
		do
			-- 1st floor: a small lobby between the front steps and the vaults
			bench("LiftBench_1", CENTRE + 8, 212, L1, "Z", -1)
			planter("LiftPlant_1", CENTRE + 8, 234, L1)
		end
		for _, spec in ipairs({
			{ y = L2, name = "2" },
			{ y = L3, name = "3" },
		}) do
			-- the 2nd and 3rd floor cores are 60 by 300 with the stair well
			-- down the middle; everything goes on the east wall or the far
			-- ends, clear of the shaft at x 88..104, z 208..226
			local y = spec.y
			bench(`CoreBench_{spec.name}_a`, EAST - 8, 40, y, "Z", 1)
			bench(`CoreBench_{spec.name}_b`, EAST - 8, 70, y, "Z", 1)
			bench(`CoreBench_{spec.name}_c`, EAST - 8, 266, y, "Z", 1)
			console(`CoreTable_{spec.name}`, 56, 285, y, "Z")
			planter(`CorePlant_{spec.name}_a`, 56, 20, y)
			planter(`CorePlant_{spec.name}_b`, EAST - 7, 290, y)
		end

		-- ---- THE STAIR HALLS. Both are mostly stair well, so the furniture
		-- lives in the end of each that is solid floor.
		do
			-- 1st floor: the clear ground is z 0..78, south of the well
			runner("StairHall1Runner", -6, 36, 10, 70, L1)
			bench("StairHall1Bench_a", WEST + 7, 24, L1, "Z", -1)
			bench("StairHall1Bench_b", WEST + 7, 54, L1, "Z", -1)
			console("StairHall1Table", 15, 8, L1, "X")
			planter("StairHall1Plant", CENTRE - 8, 62, L1)
		end
		do
			-- 2nd floor: clear ground is z 0..96 and z 190..300, either side
			-- of the well at x 20..42, z 78..188
			runner("StairHall2Runner", 4, 46, 196, 292, L2)
			bench("StairHall2Bench_a", 6, 30, L2, "Z", -1)
			bench("StairHall2Bench_b", 6, 240, L2, "Z", -1)
			bench("StairHall2Bench_c", 44, 240, L2, "Z", 1)
			console("StairHall2Table", 25, 290, L2, "X")
			planter("StairHall2Plant_a", 6, 62, L2)
			planter("StairHall2Plant_b", 44, 200, L2)
		end

		-- ---- THE EAST CORRIDOR is 25 studs wide, so one narrow table and
		-- a plant at each end is the most it can take and stay walkable.
		console("CorridorTable", 48, 150, B, "Z")
		planter("CorridorPlant_a", 48, 104, B)
		planter("CorridorPlant_b", 48, 188, B)

		-- ---- THE MUSEUM had nine props in 28,800 square studs. Cases down
		-- both long walls and benches to sit and look at them from.
		for i = 0, 5 do
			local z = 132 + i * 26
			showcase(`Case_N_{i}`, -W + 4, -W + 18, z, z + 18, L3)
			showcase(`Case_S_{i}`, 32, 46, z, z + 18, L3)
		end
		for i, z in ipairs({ 170, 230 }) do
			bench(`MuseumBench_{i}`, -30, z, L3, "Z", 1)
		end
		planter("MuseumPlant_a", -W + 10, 290, L3)
		planter("MuseumPlant_b", 40, 290, L3)

		-- ---- THE YORK RITE ROOM is 90 by 204 with one set of benches in
		-- it. An altar in the middle, officers' chairs at the head, and
		-- candle standards at the corners of the altar.
		do
			local y = L2
			box(
				F.Fixtures,
				"YorkAltar",
				-62,
				-42,
				y,
				y + 5.4,
				262,
				280,
				C.timber,
				Enum.Material.WoodPlanks
			)
			box(
				F.Fixtures,
				"YorkAltarTop",
				-64,
				-40,
				y + 5.4,
				y + 6.2,
				260,
				282,
				C.gilt,
				Enum.Material.Metal
			)
			for i, pt in ipairs({ { -66, 258 }, { -38, 258 }, { -66, 284 }, { -38, 284 } }) do
				box(
					F.Fixtures,
					`YorkStand_{i}`,
					pt[1] - 1.2,
					pt[1] + 1.2,
					y,
					y + 11,
					pt[2] - 1.2,
					pt[2] + 1.2,
					C.brass,
					Enum.Material.Metal
				)
				local g = box(
					F.Fixtures,
					`YorkFlame_{i}`,
					pt[1] - 2,
					pt[1] + 2,
					y + 11,
					y + 14,
					pt[2] - 2,
					pt[2] + 2,
					Color3.fromRGB(255, 244, 214),
					Enum.Material.Neon
				)
				g.CanCollide, g.CanQuery = false, false
				local gl = Instance.new("PointLight")
				gl.Color, gl.Brightness, gl.Range, gl.Shadows = WARM, 0.5, 24, false
				gl.Parent = g
			end
			for i, x in ipairs({ -74, -52, -30 }) do
				box(
					F.Fixtures,
					`YorkOfficer_{i}`,
					x - 4,
					x + 4,
					y,
					y + 4.4,
					290,
					298,
					C.timber,
					Enum.Material.WoodPlanks
				)
				box(
					F.Fixtures,
					`YorkOfficerBack_{i}`,
					x - 4,
					x + 4,
					y + 4.4,
					y + 13,
					296,
					298,
					C.egyptTeal,
					Enum.Material.Fabric
				)
			end
			for i, z in ipairs({ 120, 160, 200 }) do
				bench(`YorkSideBench_{i}`, -W + 7, z, y, "Z", -1)
			end
		end

		-- ---- THE FRONT STEPS. A rug, plants and a notice board, so the
		-- public way in looks like a lobby rather than a corridor.
		runner("EntranceRug", 56, 100, 120, 186, L1)
		planter("EntrancePlant_a", CENTRE + 8, 14, L1)
		planter("EntrancePlant_b", EAST - 8, 14, L1)
		box(
			F.Fixtures,
			"NoticeBoard",
			CENTRE + 1.4,
			CENTRE + 2.2,
			L1 + 6,
			L1 + 16,
			90,
			118,
			C.timber,
			Enum.Material.WoodPlanks
		)
		for i, z in ipairs({ 30, 60 }) do
			bench(`EntranceBenchB_{i}`, CENTRE + 8, z, L1, "Z", -1)
		end
	end

	-- --------------------------------------------------------------- outside
	do
		local y = LEVELS[1]
		-- YOU ARRIVE AT ONE END AND LEAVE FROM THE OTHER, which is what the
		-- plan already said and the old build ignored. Chris: "Leaving for
		-- raids should happen where?"
		--
		-- The bottom floor plan has the ramp entrance bottom-right and an
		-- EXIT at the top centre. So that is the flow: home up the ramp into
		-- the arrival hall at the south, and out by walking the length of the
		-- building - past the Lamp Wall, the kitchen and the banquet hall -
		-- and through the north door onto the road.
		--
		-- The old build returned you inside at the south and departed you
		-- from a pad outside the south-east corner: the same end, reading as
		-- nothing. Walking the hall to leave makes going out a decision you
		-- take rather than a button you happen to find.
		box(
			F.Shell,
			"SouthApron",
			-W - 30,
			W + 30,
			-3,
			-1,
			Z0 - 92,
			Z0 - 10,
			C.limestone,
			Enum.Material.Concrete
		)
		box(
			F.Shell,
			"RoadApron",
			-70,
			70,
			-3,
			-1,
			Z1 + 44,
			Z1 + 120,
			C.limestone,
			Enum.Material.Concrete
		)
		for i = 0, 5 do
			box(
				F.Shell,
				`RoadStep_{i}`,
				-26,
				26,
				-0.5 - i * 0.4,
				0.4 - i * 0.4,
				Z1 + 26 + i * 2.6,
				Z1 + 32 + i * 2.6,
				C.limestone,
				Enum.Material.Concrete
			)
		end
		-- ON THE THRESHOLD, inside the exit vestibule, rather than out on the
		-- road. Three attempts at the outdoor version all left the pad
		-- unreachable: the walk out crosses a floor edge, three steps and an
		-- apron, and the navmesh would not link the last of them however the
		-- heights were trimmed. Every room in the building pathed and the one
		-- thing you have to do did not.
		--
		-- The threshold is the better place regardless. You stand in the
		-- doorway with the road lit in front of you and take the prompt
		-- there; the steps, the apron and the lamps are what you are looking
		-- at while you do it, which is what they were for.
		-- IN THE ARRIVAL HALL, at the far end of it. Chris: "why can't the
		-- raids be in this giant bottom hallway? why walk to the outside on
		-- the other side of the building?"
		--
		-- No reason at all. You come home onto the return point at the south
		-- end of this hall and you leave from the north end of the same hall,
		-- so the one room you cannot miss is the room that does both. The
		-- door, the steps and the road lamps stay where they are as the thing
		-- you are walking towards.
		local departure =
			box(F.Markers, "Departure", 22, 44, 0, 0.4, 90, 130, C.brass, Enum.Material.Metal)
		departure.CanCollide = false
		label(departure, "LEAVE FOR A RAID", Enum.NormalId.Top, 26)
		-- Eighty-odd studs from where you land, in the east lane of the hall
		-- beside the staircase, not 240 studs away at the other end of it.
		-- The middle of this hall is the stair (x 2..20, z 80..188), so the
		-- pad goes in the clear lane next to it: you come home at the south
		-- end, and the way out and the way upstairs are side by side in front
		-- of you. Everything else in the building is somewhere you choose to
		-- go, not somewhere the exit makes you walk past.
		local board = box(
			F.Fixtures,
			"DepartureBoard",
			22,
			44,
			HEADS[1] - 17,
			HEADS[1] - 10,
			87,
			88.2,
			C.brass,
			Enum.Material.Metal
		)
		board.CanCollide, board.CanQuery = false, false
		label(board, "LEAVE FOR A RAID", Enum.NormalId.Front, 22)
		local bl = Instance.new("PointLight")
		bl.Color, bl.Brightness, bl.Range, bl.Shadows = WARM, 1.4, 44, false
		bl.Parent = board
		CollectionService:AddTag(departure, "Departure")
		for i, x in ipairs({ -34, 34 }) do
			box(
				F.Markers,
				`RoadLampPost_{i}`,
				x - 1.2,
				x + 1.2,
				-1,
				13,
				Z1 + 61,
				Z1 + 65,
				C.iron,
				Enum.Material.Metal
			)
			local head = box(
				F.Markers,
				`RoadLamp_{i}`,
				x - 2.8,
				x + 2.8,
				13,
				16,
				Z1 + 60,
				Z1 + 66,
				C.brass,
				Enum.Material.Metal
			)
			local l = Instance.new("PointLight")
			l.Color, l.Brightness, l.Range, l.Shadows = WARM, 1.3, 40, false
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

	-- ------------------------------------------------- signs over the doors
	-- One brass plate over every opening into a named room, on the OUTSIDE
	-- face, so you read where you are going before you walk in rather than
	-- after. A room with four doors gets four signs.
	for _, entry in ipairs(built) do
		local room, y = entry.room, LEVELS[entry.level]
		if room.title then
			local n = 0
			for _, d in ipairs(DOORWAYS) do
				if math.abs(d.y0 - y) < 0.5 then
					-- which of the room's four edges does this opening sit in,
					-- and which way is "out" of the room from there
					-- The room has to own most of the opening. A doorway that
					-- straddles two rooms' corner used to get a plate from
					-- each, in the same place, and the two texts rendered on
					-- top of each other: "THE BANQUET HALL" over
					-- "LANTERNHOLD" came out as one unreadable smear.
					local share = 0
					if d.axis == "X" then
						share = math.min(d.b, room.z1) - math.max(d.a, room.z0)
					else
						share = math.min(d.b, room.x1) - math.max(d.a, room.x0)
					end
					local owns = share >= (d.b - d.a) * 0.6
					local out: number? = nil
					if owns and d.axis == "X" and d.b > room.z0 + 0.5 and d.a < room.z1 - 0.5 then
						if math.abs(d.at - room.x0) < 0.6 then
							out = -1
						elseif math.abs(d.at - room.x1) < 0.6 then
							out = 1
						end
					elseif
						owns
						and d.axis == "Z"
						and d.b > room.x0 + 0.5
						and d.a < room.x1 - 0.5
					then
						if math.abs(d.at - room.z0) < 0.6 then
							out = -1
						elseif math.abs(d.at - room.z1) < 0.6 then
							out = 1
						end
					end
					if out then
						n += 1
						local o = out :: number
						local near, far = WALL_T / 2 * o, (WALL_T / 2 + 0.7) * o
						local lo, hi = math.min(near, far), math.max(near, far)
						local plate
						if d.axis == "X" then
							plate = box(
								entry.folder,
								`{room.name}_Sign_{n}`,
								d.at + lo,
								d.at + hi,
								y + DOOR_H + 0.8,
								y + DOOR_H + 5.2,
								d.a,
								d.b,
								C.brass,
								Enum.Material.Metal
							)
						else
							plate = box(
								entry.folder,
								`{room.name}_Sign_{n}`,
								d.a,
								d.b,
								y + DOOR_H + 0.8,
								y + DOOR_H + 5.2,
								d.at + lo,
								d.at + hi,
								C.brass,
								Enum.Material.Metal
							)
						end
						plate.CanCollide, plate.CanQuery, plate.CanTouch = false, false, false
						local face
						if d.axis == "X" then
							face = o < 0 and Enum.NormalId.Left or Enum.NormalId.Right
						else
							face = o < 0 and Enum.NormalId.Front or Enum.NormalId.Back
						end
						label(plate, room.title :: string, face, 15)
					end
				end
			end
		end
	end

	M.Parent = Workspace
	return M
end

return LanternholdBuilder
