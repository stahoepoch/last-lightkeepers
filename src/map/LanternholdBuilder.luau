--!strict
-- Lanternhold — the hub, built as the four-storey hall it is taken from.
--
-- Chris went and stood in the real building and sent back its programme:
-- basement, then floors one, two and three, with a lift inside. The basement
-- is the kitchen and food rooms, one floor is a giant ballroom, the second has
-- the lodge rooms - an Egyptian room and a Gothic chapel - and the third is a
-- museum and storage. "We can make basement the lobby beginning."
--
-- That is better than anything invented, so the hub is the building now rather
-- than a hall with rooms off it:
--
--   BASEMENT  the Landing you arrive in, the Community Kitchen, the Lamp Wall
--             and your workshop corner. Lit, warm, lived in.
--   FLOOR 1   the ballroom. Eight thousand square feet, built, roofed, unlit.
--   FLOOR 2   the lodge rooms. Egyptian and Norman Gothic, both dark and shut.
--   FLOOR 3   the museum - the Keeping Room, where relics are named - and the
--             storage rooms, which are not lit either.
--
-- The dark was always the point and now it has four floors to be dark on. From
-- the basement you see up the stair well past three unlit landings, which is
-- the argument for going back out there, made without a word.
--
-- The building is real, so the detail is checkable: J. C. Fulton and Son of
-- Uniontown designed it for the Johnstown masons in the mid-1920s, in Moxham.
-- The ballroom is about 8,000 square feet, the Egyptian room is original, and
-- the castle-styled room has wood beams and slate floors. Chris puts the lodge
-- rooms on the second floor; the Society of Architectural Historians puts the
-- Egyptian room on the third. He is in the building and I am not.
--
-- Geometry is in studs. A player is about five studs tall, so a table top is
-- two and a bit and a door is eight. The previous version had tables at nine
-- studs and doors at twenty-two, which is what Chris meant by "some of the
-- stuff in lanternhold is huge and not to scale with players."
--
--   require(workspace.Map.LanternholdBuilder).Build()
-- Builds Workspace.Lanternhold. Deliberately NOT under Workspace.Map, because
-- FloodedMallBuilder clears every child of Map on the first line of its build.

local CollectionService = game:GetService("CollectionService")
local Workspace = game:GetService("Workspace")

local LanternholdBuilder = {}

-- The plan. South is -Z: the road arrives from the farms.
local W = 130 -- half-width
local Z0, Z1 = 0, 230 -- south and north walls
-- The core holds the stairs, the lift and a walkway past both. It used to be
-- too narrow for all three, so the lift ended up east of it - which put the
-- shaft squarely in front of the ballroom and lodge doorways, blocking the
-- view into the rooms the whole hub is built around.
local CORE_X0, CORE_X1 = -126, -50
local CORE_Z0, CORE_Z1 = 16, 92

-- The back hall, north of the landing. BH_SPINE is the wall across it, which
-- leaves a corridor between the landing door and the two rooms behind; and
-- BH_DIVIDE is the wall between those rooms. Named rather than typed into
-- eight places, because the lighting grids are derived from the same numbers
-- and a room whose lamps stop at a wall that has moved is the fault this
-- building has had twice already.
local BH_SPINE = 118
local BH_DIVIDE = 34

-- Floor levels, as the height you stand on, and the ceiling over each.
local LEVELS = { 0, 26, 64, 92 }
local HEADS = { 24, 62, 90, 118 }
local TOP = 126

local WALL_T = 4
local DOOR_H = 8 -- a door a person walks through
local DOOR_W = 14

local C = {
	stone = Color3.fromRGB(206, 196, 176), -- cast stone, not brick. See the photograph.
	trim = Color3.fromRGB(176, 166, 148),
	timber = Color3.fromRGB(120, 84, 56),
	board = Color3.fromRGB(168, 150, 128),
	dark = Color3.fromRGB(34, 32, 36),
	brass = Color3.fromRGB(198, 160, 92),
	glass = Color3.fromRGB(196, 214, 212),
	cloth = Color3.fromRGB(122, 74, 62),
	grass = Color3.fromRGB(94, 104, 74),
	egypt = Color3.fromRGB(150, 116, 58),
	gothic = Color3.fromRGB(86, 90, 104),
	-- Read off Chris's photographs from inside the building.
	terrazzo = Color3.fromRGB(168, 160, 144), -- speckled cream, polished; see DONTFUCKUPTHELIGHTS.md
	inlay = Color3.fromRGB(92, 84, 74), -- the dark bands set into it
	hardwood = Color3.fromRGB(104, 62, 40), -- the ballroom floor, red-brown
	plaster = Color3.fromRGB(202, 196, 184), -- the coffered ceilings; lowered to stop blowing out beside a lamp
	iron = Color3.fromRGB(46, 42, 40), -- the balustrades
	stencil = Color3.fromRGB(92, 112, 84), -- the painted border band
	gilt = Color3.fromRGB(176, 148, 86),
}
local WARM = Color3.fromRGB(255, 214, 160)

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
	return part(
		parent,
		name,
		Vector3.new(x1 - x0, y1 - y0, z1 - z0),
		CFrame.new((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
		colour,
		mat,
		trans
	)
end

--- A wall with doorways cut in it, each with a lintel over it.
local function wall(
	parent: Instance,
	name: string,
	axis: string,
	at: number,
	from: number,
	to: number,
	y0: number,
	y1: number,
	colour: Color3,
	gaps: { { number } },
	headH: number?
)
	-- Doorways are door height by default. The openings onto the rooms that are
	-- shut take a taller head, because the whole point of those rooms is that
	-- you stand at the barrier and see what you are not allowed into - and an
	-- eight stud hole shows you a skirting board.
	local head = math.min(y0 + (headH or DOOR_H), y1)
	local cuts = table.clone(gaps)
	table.sort(cuts, function(a, b)
		return a[1] < b[1]
	end)
	local runs, cursor = {}, from
	for _, g in ipairs(cuts) do
		if g[1] > cursor then
			table.insert(runs, { cursor, g[1] })
		end
		cursor = math.max(cursor, g[2])
	end
	if cursor < to then
		table.insert(runs, { cursor, to })
	end
	local h = WALL_T / 2
	for i, r in ipairs(runs) do
		if r[2] - r[1] > 0.01 then
			if axis == "X" then
				box(
					parent,
					`{name}_{i}`,
					at - h,
					at + h,
					y0,
					y1,
					r[1],
					r[2],
					colour,
					Enum.Material.Concrete
				)
			else
				box(
					parent,
					`{name}_{i}`,
					r[1],
					r[2],
					y0,
					y1,
					at - h,
					at + h,
					colour,
					Enum.Material.Concrete
				)
			end
		end
	end
	for i, g in ipairs(cuts) do
		if y1 > head then
			if axis == "X" then
				box(
					parent,
					`{name}_Head_{i}`,
					at - h,
					at + h,
					head,
					y1,
					g[1],
					g[2],
					colour,
					Enum.Material.Concrete
				)
			else
				box(
					parent,
					`{name}_Head_{i}`,
					g[1],
					g[2],
					head,
					y1,
					at - h,
					at + h,
					colour,
					Enum.Material.Concrete
				)
			end
		end
	end
end

local function label(host: BasePart, text: string, face: Enum.NormalId, size: number?)
	local g = Instance.new("SurfaceGui")
	g.Face, g.CanvasSize, g.Parent = face, Vector2.new(600, 160), host
	local l = Instance.new("TextLabel")
	l.Size, l.BackgroundTransparency = UDim2.fromScale(1, 1), 1
	l.Font, l.TextScaled = Enum.Font.GothamBold, true
	l.TextSize = size or 34
	l.TextColor3, l.Text = Color3.fromRGB(240, 226, 200), text
	l.Parent = g
end

local function lamp(
	parent: Instance,
	name: string,
	x: number,
	y: number,
	z: number,
	lit: boolean
): Part
	local glass =
		part(parent, name, Vector3.new(3, 4.5, 3), CFrame.new(x, y, z), C.dark, Enum.Material.Metal)
	local bracket = part(
		parent,
		`{name}_Bracket`,
		Vector3.new(1, 1, 3.5),
		CFrame.new(x, y + 2.6, z),
		C.brass,
		Enum.Material.Metal
	)
	-- A fitting hung from the ceiling is not something to walk into. These
	-- were collidable, which hung a hundred small obstacles inside the beams
	-- and soffits they are fixed to.
	glass.CanCollide, glass.CanQuery = false, false
	bracket.CanCollide, bracket.CanQuery = false, false
	if lit then
		local light = Instance.new("PointLight")
		-- 1.0 at range 34, not 1.5 at 42. Twelve of these on the hub floor at
		-- the old setting overlapped into a flood rather than twelve pools,
		-- and against plaster at 238,232,222 the hall went white even with the
		-- Lighting block correct. The mall reads well at 0.85 and range 46;
		-- this is the same idea - a lamp lights its own patch of floor and the
		-- dark between patches is the point.
		light.Color, light.Brightness, light.Range = WARM, 1.0, 34
		-- Shadows off everywhere. A lantern that casts them strobes as you walk
		-- past the posts, which is the flicker Chris hit in the mall.
		light.Shadows = false
		light.Parent = glass
	end
	return glass
end

--- Lamps on a grid across a whole room, so a room cannot come out part-lit.
---
--- Every lamp in this building used to be a hand-typed coordinate, and the
--- result was what Chris walked into: "the lighting is awful in the
--- lanternhold". Not dim lamps - missing ones. The ballroom had none at all
--- over 180 by 230 studs, both lodge rooms had none, the museum had three for
--- a whole floor, the kitchen had two and both at the same x, and the stair
--- core - the only way between floors - had none on any level. The basement
--- landing was the one room anybody had covered, and it was covered because
--- somebody sat and typed twelve pairs of numbers.
---
--- Same lesson as the Galleria's relic caches: a list of coordinates is a
--- thing that goes stale silently. The grid comes off the room's own
--- rectangle, so a room that changes shape keeps its light.
---
--- Spacing has to stay inside the lamp's reach or the grid leaves holes at the
--- diagonals. At 46 studs the worst point between four lamps is 32.5 from all
--- of them, inside a 34 stud range; past about 48 the middle goes black.
local function lightRoom(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	ceiling: number,
	-- Deliberately a table with capitalised keys rather than three positional
	-- numbers. tests/lighting.test.mjs guards every lamp in src/map by matching
	-- `Brightness = <number>`, and a positional argument is invisible to it -
	-- so the first version of this helper quietly took nine lamp brightnesses
	-- out from under the guard that exists because this exact thing has cost
	-- the project several evenings. Written this way the values are both
	-- readable at the call site and still checked.
	opts: { Brightness: number, Range: number, Spacing: number? }
)
	local pitch = opts.Spacing or 46
	local nx = math.max(1, math.round((x1 - x0) / pitch))
	local nz = math.max(1, math.round((z1 - z0) / pitch))
	for i = 0, nx - 1 do
		for j = 0, nz - 1 do
			local x = x0 + (x1 - x0) * ((i + 0.5) / nx)
			local z = z0 + (z1 - z0) * ((j + 0.5) / nz)
			local glass = lamp(parent, `{name}_{i}_{j}`, x, ceiling - 5, z, true)
			local light = glass:FindFirstChildOfClass("PointLight")
			if light then
				light.Brightness, light.Range = opts.Brightness, opts.Range
			end
		end
	end
end

--- A fingerpost: one post, an arm per destination, each arm swung to actually
--- point at the thing it names. The hub's rooms are spread over a 260 by 230
--- building and Chris walked it cold: "the whole place is strange". The
--- research on this is consistent - put hints in the world rather than in a
--- wall of text, and keep the extras to the sides of the route rather than
--- across it. An arm that points the wrong way is worse than no arm, so the
--- yaw is derived from the target rather than typed.
local function fingerpost(
	parent: Instance,
	name: string,
	x: number,
	z: number,
	base: number,
	arms: { { text: string, tx: number, tz: number } }
)
	part(
		parent,
		`{name}_Post`,
		Vector3.new(2.2, 15, 2.2),
		CFrame.new(x, base + 7.5, z),
		C.iron,
		Enum.Material.Metal
	)
	part(
		parent,
		`{name}_Finial`,
		Vector3.new(3.4, 1, 3.4),
		CFrame.new(x, base + 15.2, z),
		C.gilt,
		Enum.Material.Metal
	)
	-- Stack the arms so the ones pointing nearly the same way sit furthest
	-- apart vertically. Two of these rooms are both away to the west, only 12
	-- degrees apart, and at even spacing their boards overlapped into an
	-- unreadable pile - Chris: "signs don't match up". Sorting by bearing and
	-- then dealing the heights out from the two ends inward puts angular
	-- neighbours at opposite ends of the post.
	table.sort(arms, function(a, b)
		return math.atan2(a.tz - z, a.tx - x) < math.atan2(b.tz - z, b.tx - x)
	end)
	local slot, lo, hi = {}, 1, #arms
	for i = 1, #arms do
		if i % 2 == 1 then
			slot[i] = lo
			lo += 1
		else
			slot[i] = hi
			hi -= 1
		end
	end
	for i, arm in ipairs(arms) do
		local yaw = math.atan2(arm.tz - z, arm.tx - x)
		local y = base + 14.5 - (slot[i] - 1) * 4.1
		local board = part(
			parent,
			`{name}_Arm{i}`,
			Vector3.new(17, 2.4, 0.5),
			CFrame.new(x, y, z) * CFrame.Angles(0, -yaw, 0) * CFrame.new(9.5, 0, 0),
			C.timber,
			Enum.Material.WoodPlanks
		)
		board.CanCollide = false
		label(board, arm.text, Enum.NormalId.Front, 22)
		label(board, arm.text, Enum.NormalId.Back, 22)
	end
end

--- A room's own floor finish, laid ON the structural slab rather than beside
--- it. Every room used to build its own two-stud slab at exactly the height
--- the shell had already put one, which left two collidable surfaces sharing a
--- plane and made the rooms above unreachable from their own landings.
local function finish(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number,
	colour: Color3,
	mat: Enum.Material?
)
	local p = box(parent, name, x0, x1, y, y + 0.2, z0, z1, colour, mat or Enum.Material.WoodPlanks)
	p.CanCollide = false
	return p
end

--- A square column with a moulded capital and a dark plinth. Every column in
--- the building is like this - square, cream, with a stepped capital - and a
--- plain post reads as scaffolding beside it.
local function column(
	parent: Instance,
	name: string,
	x: number,
	z: number,
	y0: number,
	y1: number,
	w: number
)
	local h = w / 2
	box(
		parent,
		`{name}_Plinth`,
		x - h - 0.4,
		x + h + 0.4,
		y0,
		y0 + 2.5,
		z - h - 0.4,
		z + h + 0.4,
		C.timber,
		Enum.Material.WoodPlanks
	)
	box(
		parent,
		`{name}_Shaft`,
		x - h,
		x + h,
		y0 + 2.5,
		y1 - 2.5,
		z - h,
		z + h,
		C.plaster,
		Enum.Material.Concrete
	)
	box(
		parent,
		`{name}_Capital`,
		x - h - 1,
		x + h + 1,
		y1 - 2.5,
		y1,
		z - h - 1,
		z + h + 1,
		C.plaster,
		Enum.Material.Concrete
	)
end

--- A coffered ceiling: a grid of beams with the decorative band on their faces.
--- The ballroom's ceiling is the single most recognisable thing in the photos
--- of this building, and a flat slab throws all of it away.
local function coffers(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number,
	bay: number,
	band: Color3,
	soffit: boolean?
)
	local nx = math.max(1, math.floor((x1 - x0) / bay))
	local nz = math.max(1, math.floor((z1 - z0) / bay))
	-- Only where there is no floor overhead already. Every room but the top one
	-- has the slab above serving as its ceiling, and laying a soffit there puts
	-- three collidable surfaces in the same two-stud band - which is what made
	-- the museum unreachable from its own landing.
	if soffit then
		box(parent, `{name}_Soffit`, x0, x1, y, y + 2, z0, z1, C.plaster, Enum.Material.Concrete)
	end
	for i = 0, nx do
		local x = x0 + (x1 - x0) * (i / nx)
		box(
			parent,
			`{name}_BeamX_{i}`,
			x - 2,
			x + 2,
			y - 3,
			y,
			z0,
			z1,
			C.plaster,
			Enum.Material.Concrete
		)
		box(
			parent,
			`{name}_BandX_{i}`,
			x - 2.3,
			x + 2.3,
			y - 2.2,
			y - 1.2,
			z0,
			z1,
			band,
			Enum.Material.Concrete
		)
	end
	for j = 0, nz do
		local z = z0 + (z1 - z0) * (j / nz)
		box(
			parent,
			`{name}_BeamZ_{j}`,
			x0,
			x1,
			y - 3,
			y,
			z - 2,
			z + 2,
			C.plaster,
			Enum.Material.Concrete
		)
		box(
			parent,
			`{name}_BandZ_{j}`,
			x0,
			x1,
			y - 2.2,
			y - 1.2,
			z - 2.3,
			z + 2.3,
			band,
			Enum.Material.Concrete
		)
	end
end

--- The dark skirting band that runs round every room at about a foot high.
local function baseband(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number
)
	-- Skirting never collides. Built solid it is a three-stud wall round every
	-- room, and it ran straight across the front doors, the kitchen opening and
	-- the stair well - the whole building became unreachable from the road.
	for _, spec in ipairs({
		{ x0, x0 + 1, z0, z1 },
		{ x1 - 1, x1, z0, z1 },
		{ x0, x1, z0, z0 + 1 },
		{ x0, x1, z1 - 1, z1 },
	}) do
		local b = box(
			parent,
			`{name}_{spec[1]}_{spec[3]}`,
			spec[1],
			spec[2],
			y,
			y + 3,
			spec[3],
			spec[4],
			C.timber,
			Enum.Material.WoodPlanks
		)
		b.CanCollide = false
	end
end

--- Terrazzo with dark bands set into it, which is the floor of every hall and
--- stair in the building.
local function terrazzo(
	parent: Instance,
	name: string,
	x0: number,
	x1: number,
	z0: number,
	z1: number,
	y: number
)
	box(parent, `{name}`, x0, x1, y - 2, y, z0, z1, C.terrazzo, Enum.Material.Pebble)
	local n = math.max(1, math.floor((z1 - z0) / 46))
	for i = 1, n - 1 do
		local z = z0 + (z1 - z0) * (i / n)
		box(
			parent,
			`{name}_Band_{i}`,
			x0,
			x1,
			y,
			y + 0.3,
			z - 5,
			z + 5,
			C.inlay,
			Enum.Material.Pebble
		).CanCollide =
			false
	end
	box(
		parent,
		`{name}_BandEdgeA`,
		x0 + 10,
		x0 + 16,
		y,
		y + 0.3,
		z0,
		z1,
		C.inlay,
		Enum.Material.Pebble
	).CanCollide =
		false
	box(
		parent,
		`{name}_BandEdgeB`,
		x1 - 16,
		x1 - 10,
		y,
		y + 0.3,
		z0,
		z1,
		C.inlay,
		Enum.Material.Pebble
	).CanCollide =
		false
end

--- A flight as one ramp with treads drawn on it. Discrete steps do not produce
--- a navmesh anything can path over - that fault has kept the other map's
--- stairs unusable since the first playtest.
--- A flight as one ramp with treads drawn on it, and a flat pad at each end.
--- Discrete steps do not produce a navmesh anything can path over, which is
--- the fault that has kept the other map's stairs unusable since the first
--- playtest. The pads matter just as much: a ramp ends in a tilted wedge, and
--- the navmesh will not generate on that last sliver.
---
--- The pads butt against the ramp rather than overlapping it. Overlapped, the
--- pad's flat top stood a stud and a third proud of the sloping deck beneath
--- it, and pathfinding walked three quarters of the way up every flight and
--- stopped dead at that lip.
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
	local dirZ = if zHigh > zLow then 1 else -1
	local PAD = 14
	-- The ramp stops short of the nominal top; the head pad covers the rest.
	local zTop = zHigh - dirZ * PAD
	local run = math.abs(zTop - zLow)
	local rise = yHigh - yLow
	local hyp = math.sqrt(run * run + rise * rise)
	local pitch = math.atan2(rise, run)
	local cf = CFrame.new((x0 + x1) / 2, (yLow + yHigh) / 2 - 1, (zLow + zTop) / 2)
		* CFrame.Angles(0, if dirZ > 0 then 0 else math.pi, 0)
		* CFrame.Angles(-pitch, 0, 0)
	part(parent, `{name}_Deck`, Vector3.new(x1 - x0, 2, hyp), cf, C.trim, Enum.Material.Concrete)
	for k = 1, 14 do
		local t = ((k - 0.5) / 14 - 0.5) * hyp
		-- Flush with the ramp, not standing on it. At 1.2 they were 0.4 studs
		-- proud and non-colliding, so you walked the smooth deck underneath
		-- while your legs passed through every visible step - the same ghost
		-- step the Galleria escalators had, and the likeliest reading of
		-- Chris's "the stairs going up are all weird". Either they are real
		-- geometry you stand on or they are paint; they cannot be a raised
		-- thing you pass through.
		local tread = part(
			parent,
			`{name}_Tread_{k}`,
			Vector3.new(x1 - x0 - 1, 0.4, 1),
			cf * CFrame.new(0, 0.82, t),
			C.stone,
			Enum.Material.Concrete
		)
		tread.CanCollide, tread.CanTouch, tread.CanQuery = false, false, false
	end
	-- One handrail, on the outer side only. Railed on both, the flight is a
	-- walled corridor and the rail on the landing side stands between the top
	-- of the stair and the floor it arrives at.
	-- The balustrade is wrought iron with circles and zigzags in it, not a
	-- plain bar: it is the most distinctive thing in the stair hall.
	local railX = -((x1 - x0) / 2 - 0.4)
	part(
		parent,
		`{name}_RailTop`,
		Vector3.new(1.4, 0.9, hyp),
		cf * CFrame.new(railX, 4, 0),
		C.timber,
		Enum.Material.WoodPlanks
	)
	part(
		parent,
		`{name}_RailBottom`,
		Vector3.new(0.7, 0.6, hyp),
		cf * CFrame.new(railX, 0.9, 0),
		C.iron,
		Enum.Material.Metal
	)
	for k = 1, 10 do
		local t = ((k - 0.5) / 10 - 0.5) * hyp
		part(
			parent,
			`{name}_Baluster_{k}`,
			Vector3.new(0.5, 3.2, 0.5),
			cf * CFrame.new(railX, 2.3, t),
			C.iron,
			Enum.Material.Metal
		)
		local ring = part(
			parent,
			`{name}_Ring_{k}`,
			Vector3.new(0.5, 2.2, 2.2),
			cf * CFrame.new(railX, 2.3, t + hyp / 20),
			C.iron,
			Enum.Material.Metal
		)
		ring.Shape = Enum.PartType.Cylinder
		ring.CanCollide = false
	end

	local function pad(nm: string, y: number, za: number, zb: number)
		box(
			parent,
			nm,
			x0,
			x1,
			y - 2,
			y,
			math.min(za, zb),
			math.max(za, zb),
			C.trim,
			Enum.Material.Concrete
		)
	end
	pad(`{name}_Foot`, yLow, zLow - dirZ * PAD, zLow)
	pad(`{name}_Head`, yHigh, zTop, zTop + dirZ * (PAD + 6))
end

function LanternholdBuilder.Build(): Model
	local old = Workspace:FindFirstChild("Lanternhold")
	if old then
		old:Destroy()
	end

	local M = Instance.new("Model")
	M.Name = "Lanternhold"
	local F: { [string]: Folder } = {}
	for _, n in ipairs({
		"Ground",
		"Shell",
		"Core",
		"Basement",
		"Ballroom",
		"Lodge",
		"Museum",
		"Fixtures",
		"Markers",
	}) do
		local f = Instance.new("Folder")
		f.Name, f.Parent = n, M
		F[n] = f
	end

	-- ------------------------------------------------------------- outside
	box(F.Ground, "Ground", -420, 420, -8, -2, -260, 400, C.grass, Enum.Material.Grass)
	box(F.Ground, "Road", -30, 30, -2, 0, -260, -30, C.trim, Enum.Material.Concrete)
	box(F.Ground, "Porch", -60, 60, -2, 0, -30, Z0, C.trim, Enum.Material.Concrete)

	-- ------------------------------------------------------------- the shell
	-- Pale cast stone, tall recessed window slots between flat pilasters, a
	-- carved band under a plain parapet, and a blank stair tower at one end.
	-- That is the building in Chris's photograph, and nothing like the brick
	-- box this used to be.
	-- Every slab above the basement is built around the stair well rather than
	-- across it. Laid solid, the flights ran straight up into the underside of
	-- the floor above and no route between storeys existed at all - the hall
	-- had four floors and no way between them.
	-- Terrazzo in the basement and the stair halls, polished hardwood above.
	-- Both are in the photographs and they are nothing like the plain boards
	-- this had: the terrazzo is speckled cream with dark bands set into it, and
	-- the ballroom floor is a deep red-brown.
	for i, lvl in ipairs(LEVELS) do
		local wood = i >= 2
		if i == 1 then
			terrazzo(F.Shell, `Slab_{i}`, -W, W, Z0, Z1, lvl)
		else
			local function slab(nm, x0, x1, z0, z1)
				if wood then
					box(
						F.Shell,
						nm,
						x0,
						x1,
						lvl - 2,
						lvl,
						z0,
						z1,
						C.hardwood,
						Enum.Material.WoodPlanks
					)
				else
					terrazzo(F.Shell, nm, x0, x1, z0, z1, lvl)
				end
			end
			slab(`Slab_{i}_E`, CORE_X1, W, Z0, Z1)
			slab(`Slab_{i}_W`, -W, CORE_X0, Z0, Z1)
			slab(`Slab_{i}_S`, CORE_X0, CORE_X1, Z0, CORE_Z0)
			slab(`Slab_{i}_N`, CORE_X0, CORE_X1, CORE_Z1, Z1)
			-- The north wing and the south strip are bare slab on every upper
			-- level: real floor you can walk onto, outside every room anybody
			-- programmed, with nothing over it. The audit put 37% of the hub's
			-- standable floor in the dark and nearly all of it was here.
			--
			-- Lighting it does not make it a room, and it should become one -
			-- that is the next piece of the overhaul and Chris should say what
			-- these are. But a lit empty floor reads as a building not yet
			-- fitted out, and an unlit one reads as the lights being broken,
			-- which is what he has been telling us.
			lightRoom(
				F.Shell,
				`NorthWing_{i}`,
				CORE_X0,
				CORE_X1,
				CORE_Z1,
				Z1,
				HEADS[i],
				{ Brightness = 0.7, Range = 38 }
			)
			lightRoom(
				F.Shell,
				`SouthStrip_{i}`,
				CORE_X0,
				CORE_X1,
				Z0,
				CORE_Z0,
				HEADS[i],
				{ Brightness = 0.7, Range = 38 }
			)
		end
	end
	box(F.Shell, "Roof", -W, W, TOP, TOP + 2, Z0, Z1, C.dark, Enum.Material.Slate)
	wall(F.Shell, "WallWest", "X", -W, Z0, Z1, 0, TOP, C.stone, {})
	wall(F.Shell, "WallEast", "X", W, Z0, Z1, 0, TOP, C.stone, {})
	wall(F.Shell, "WallNorth", "Z", Z1, -W, W, 0, TOP, C.stone, {})
	wall(F.Shell, "WallSouth", "Z", Z0, -W, W, 0, TOP, C.stone, { { -DOOR_W, DOOR_W } })

	for i = 0, 8 do
		local z = Z0 + 14 + i * 24
		for _, x in ipairs({ -W - 2, W + 2 }) do
			box(
				F.Shell,
				`Pilaster_{i}_{x}`,
				x - 2,
				x + 2,
				0,
				TOP,
				z - 5,
				z + 5,
				C.stone,
				Enum.Material.Concrete
			)
		end
		if i < 8 then
			for _, x in ipairs({ -W - 1, W + 1 }) do
				-- One tall recessed slot per bay: in the photograph the glazing
				-- runs most of the height rather than floor by floor.
				box(
					F.Shell,
					`Glazing_{i}_{x}`,
					x - 1,
					x + 1,
					30,
					TOP - 16,
					z + 7,
					z + 22,
					C.glass,
					Enum.Material.Glass,
					0.45
				)
			end
		end
	end
	box(
		F.Shell,
		"Frieze",
		-W - 4,
		W + 4,
		TOP - 14,
		TOP - 8,
		Z0 - 4,
		Z1 + 4,
		C.trim,
		Enum.Material.Concrete
	)
	box(
		F.Shell,
		"Parapet",
		-W - 4,
		W + 4,
		TOP,
		TOP + 8,
		Z0 - 4,
		Z1 + 4,
		C.stone,
		Enum.Material.Concrete
	)
	box(
		F.Shell,
		"Tower",
		-W - 10,
		-W + 16,
		0,
		TOP + 22,
		Z0 + 6,
		Z0 + 44,
		C.stone,
		Enum.Material.Concrete
	)

	for e = -1, 1, 2 do
		-- Stood OPEN, against the facade, instead of closed across the opening.
		-- Closed, they put a solid slab between where you land and the only
		-- thing you are meant to do: a line of sight check from the arrival
		-- point to the departure pad hit FrontDoor_1 at 23 studs. You arrived
		-- facing the way out and could not see it - Chris: "going to a raid
		-- isn't really explanatory". A hall with its doors flung open also
		-- reads as somewhere expecting you back.
		-- box() takes bounds as x1 - x0 with no min/max of its own, so a leaf
		-- hinged on the west side handed it a negative width and Roblox
		-- clamped that to 0.001: one door was a paper sliver. Order the pair.
		local hinge = e * (DOOR_W / 2)
		local da, db = hinge, hinge + e * 0.8
		local pa, pb = hinge + e * 0.8, hinge + e * 1
		box(
			F.Shell,
			`FrontDoor_{e}`,
			math.min(da, db),
			math.max(da, db),
			0,
			DOOR_H,
			Z0 - 7.6,
			Z0 - 0.6,
			C.timber,
			Enum.Material.WoodPlanks
		)
		box(
			F.Shell,
			`FrontPlate_{e}`,
			math.min(pa, pb),
			math.max(pa, pb),
			4,
			5.6,
			Z0 - 5.4,
			Z0 - 3.4,
			C.brass,
			Enum.Material.Metal
		)
	end

	local name =
		box(F.Shell, "NameStone", -40, 40, 15, 23, Z0 - 3.2, Z0 - 2, C.trim, Enum.Material.Concrete)
	label(name, "LANTERNHOLD", Enum.NormalId.Front)

	-- ------------------------------------------------------------- the core
	-- Stairs and the lift in one well, open the whole height. From the basement
	-- you can see three unlit landings above you.
	for i = 2, #LEVELS do
		local up = i % 2 == 0
		-- Do NOT widen this to the full depth of the core. Running it
		-- CORE_Z0 + 2 to CORE_Z1 - 2 does flatten the flights - 40 degrees on
		-- the ballroom-to-lodge run drops to 33, and all three land between 24
		-- and 33 - but it also pushes each head pad far enough that the upper
		-- flights stop connecting: CoreLanding_3, CoreLanding_4 and the whole
		-- museum went unreachable on pathfinding. The 8 stud inset is what
		-- keeps the pads inside the core and the landings linked. Tried and
		-- reverted; the steep flight is the price.
		flight(
			F.Core,
			`Stair_{i}`,
			CORE_X0 + 6,
			CORE_X0 + 32,
			if up then CORE_Z0 + 8 else CORE_Z1 - 8,
			if up then CORE_Z1 - 8 else CORE_Z0 + 8,
			LEVELS[i - 1],
			LEVELS[i]
		)
	end
	-- No full-height wall down the east side of the core. Each floor already
	-- has its own wall in that plane with its own doorway in it - the
	-- ballroom's barred opening, the lodge doors, the museum's - and a solid
	-- shaft wall standing in front of all of them blocked every one. The
	-- museum became unreachable and the dark rooms you are meant to see into
	-- could not even be looked at.
	for i, lvl in ipairs(LEVELS) do
		terrazzo(F.Core, `CoreLanding_{i}`, CORE_X0 + 32, CORE_X1, CORE_Z0, CORE_Z1, lvl)
		baseband(F.Core, `CoreBase_{i}`, CORE_X0, CORE_X1, CORE_Z0, CORE_Z1, lvl)
		-- The stair core had no light on any level. It is the only way between
		-- floors, the flights are steep enough that one was reverted for being
		-- 40 degrees, and you were climbing them blind. Lit over the landing
		-- AND over the flight, because a lit landing above a black flight is
		-- still a black flight.
		lightRoom(
			F.Core,
			`CoreLight_{i}`,
			CORE_X0 + 2,
			CORE_X1,
			CORE_Z0,
			CORE_Z1,
			HEADS[i],
			{ Brightness = 1.1, Range = 36, Spacing = 40 }
		)
	end

	-- Set so there is a walkway on both sides: the stair ends at CORE_X0 + 32,
	-- then fourteen studs of landing, then the shaft, then twelve more to the
	-- core wall. Pushed hard against the stair it left a two stud slot you
	-- could not step off the flight into.
	local LIFT_X0, LIFT_X1 = CORE_X0 + 46, CORE_X0 + 64
	local LIFT_Z0, LIFT_Z1 = CORE_Z0 + 48, CORE_Z0 + 72
	wall(F.Core, "LiftWallW", "X", LIFT_X0, LIFT_Z0, LIFT_Z1, 0, TOP, C.trim, {})
	wall(F.Core, "LiftWallE", "X", LIFT_X1, LIFT_Z0, LIFT_Z1, 0, TOP, C.trim, {})
	wall(F.Core, "LiftWallN", "Z", LIFT_Z1, LIFT_X0, LIFT_X1, 0, TOP, C.trim, {})
	local car = box(
		F.Core,
		"LiftCar",
		LIFT_X0 + 2,
		LIFT_X1 - 2,
		-2,
		0,
		LIFT_Z0 + 2,
		LIFT_Z1 - 2,
		C.trim,
		Enum.Material.Metal
	)
	car:SetAttribute("Stops", "0,26,64,92")
	CollectionService:AddTag(car, "Lift")

	-- ------------------------------------------------------------- basement
	local B = LEVELS[1]
	wall(F.Basement, "LandingNorth", "Z", 86, CORE_X1, W, B, HEADS[1], C.stone, { { -30, -2 } })
	-- The basement's own wall onto the stair hall, with its door.
	wall(
		F.Basement,
		"CoreDoor",
		"X",
		CORE_X1,
		CORE_Z0,
		CORE_Z1,
		B,
		HEADS[1],
		C.stone,
		{ { 24, 44 } }
	)
	-- Stops at the core. Carried across it, this ceiling capped the stair well
	-- and the first flight rose straight into its underside, which is why no
	-- floor above the basement could be reached.
	-- The basement is a column grid under a beamed ceiling, which is what the
	-- photograph shows and what the room needs to stop reading as a hangar.
	coffers(F.Basement, "LandingCeiling", CORE_X1, W, Z0, 86, HEADS[1], 46, C.plaster)
	for i = 0, 3 do
		for j = 0, 1 do
			column(F.Basement, `LandingColumn_{i}_{j}`, -30 + i * 44, 28 + j * 40, B, HEADS[1], 9)
		end
	end
	baseband(F.Basement, "LandingBase", CORE_X1, W, Z0, 86, B)
	-- Four lamps lit the whole hub floor and Chris walked it: "the lanternhold
	-- is still pretty dark on its own". A hub wants to read as a refuge,
	-- precisely because the mall does not - if both are dark the lantern you
	-- carry out means nothing. Density rather than brightness: the same 1.5
	-- lamp on a grid, so the room is evenly lit instead of having four hot
	-- spots and a lot of black between them.
	lightRoom(
		F.Basement,
		"LandingLamp",
		CORE_X1,
		W,
		Z0,
		86,
		HEADS[1],
		{ Brightness = 1.2, Range = 38, Spacing = 42 }
	)

	-- The Lamp Wall: one lantern per Lightkeeper out, tagged and numbered.
	box(
		F.Fixtures,
		"LampWallFace",
		W - WALL_T - 3,
		W - WALL_T,
		B,
		B + 20,
		14,
		70,
		C.timber,
		Enum.Material.WoodPlanks
	)
	-- Topped out at B + 22, under the basement ceiling at 24. At B + 27 the
	-- plaque stood through the ballroom floor slab above it, which runs 24 to
	-- 26 - a third of it was inside the next storey. The only real clipping
	-- the audit found in the whole building.
	local plaque = box(
		F.Fixtures,
		"LampWallPlaque",
		W - WALL_T - 4,
		W - WALL_T - 3.2,
		B + 16,
		B + 22,
		24,
		60,
		C.timber,
		Enum.Material.WoodPlanks
	)
	label(plaque, "KEEPERS OUT", Enum.NormalId.Left, 30)
	for i = 0, 7 do
		local g = lamp(F.Fixtures, `KeeperLamp_{i}`, W - WALL_T - 6, B + 11, 18 + i * 7, i % 3 ~= 2)
		g:SetAttribute("Slot", i)
		CollectionService:AddTag(g, "LampWallSlot")
	end

	-- The Community Kitchen and, behind it, the food room. This is what the
	-- floor was for, which is why the hall has a kitchen before an armoury.
	wall(F.Basement, "KitchenNorth", "Z", 150, -W, CORE_X1, B, HEADS[1], C.stone, { { -96, -68 } })
	-- The kitchen keeps the painted border band: a green and gold stencil run
	-- along the beam faces, which is the prettiest thing in the building and
	-- costs two colours.
	coffers(F.Basement, "KitchenCeiling", -W, CORE_X1, CORE_Z1, 150, HEADS[1], 40, C.stencil)
	for i = 0, 1 do
		column(F.Basement, `KitchenColumn_{i}`, -104 + i * 46, 118, B, HEADS[1], 9)
	end
	baseband(F.Basement, "KitchenBase", -W, CORE_X1, CORE_Z1, 150, B)
	-- Two lamps, both on the same x, for an eighty by sixty room. The west
	-- half of the kitchen was unlit floor.
	lightRoom(
		F.Basement,
		"KitchenLamp",
		-W,
		CORE_X1,
		CORE_Z1,
		150,
		HEADS[1],
		{ Brightness = 1.2, Range = 38, Spacing = 42 }
	)
	-- The basement slab runs the whole 260 by 230 and only two parts of it
	-- were lit: the landing, which stops at z = 86, and the kitchen, which
	-- stops at z = 150. Everything north of those was dark floor you could
	-- walk onto.
	lightRoom(
		F.Basement,
		"BackCorridorLamp",
		CORE_X1,
		W,
		86,
		BH_SPINE,
		HEADS[1],
		{ Brightness = 0.95, Range = 38 }
	)
	-- Lit per room rather than as one grid over bare slab: the back hall is
	-- rooms now, and a grid that straddles a wall leaves the far side dim.
	lightRoom(F.Basement, "LongRoomLamp", CORE_X1, BH_DIVIDE, BH_SPINE, Z1, HEADS[1], {
		Brightness = 1.1,
		Range = 38,
	})
	lightRoom(F.Basement, "StoreLamp", BH_DIVIDE, W, BH_SPINE, Z1, HEADS[1], {
		Brightness = 0.9,
		Range = 38,
	})
	lightRoom(
		F.Basement,
		"StoreRoomLamp",
		-W,
		CORE_X1,
		150,
		Z1,
		HEADS[1],
		{ Brightness = 0.9, Range = 38 }
	)
	-- And the strip south of the core, which is the floor you arrive on.
	lightRoom(
		F.Basement,
		"ArrivalLamp",
		-W,
		CORE_X1,
		Z0,
		CORE_Z0,
		HEADS[1],
		{ Brightness = 1.0, Range = 38 }
	)
	for i = 0, 1 do
		local x0 = -120 + i * 42
		box(
			F.Fixtures,
			`Table_{i}`,
			x0,
			x0 + 34,
			B + 2.2,
			B + 3,
			98,
			130,
			C.timber,
			Enum.Material.WoodPlanks
		)
		for _, z in ipairs({ 101, 127 }) do
			box(
				F.Fixtures,
				`TableLeg_{i}_{z}`,
				x0 + 3,
				x0 + 5,
				B,
				B + 2.2,
				z - 1,
				z + 1,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end
		for _, dx in ipairs({ -4, 38 }) do
			box(
				F.Fixtures,
				`Bench_{i}_{dx}`,
				x0 + dx - 2,
				x0 + dx + 2,
				B + 1.4,
				B + 2,
				100,
				128,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end
	end
	box(F.Fixtures, "Servery", -124, -60, B, B + 3, 138, 146, C.timber, Enum.Material.WoodPlanks)
	box(
		F.Fixtures,
		"ServeryTop",
		-126,
		-58,
		B + 3,
		B + 3.6,
		137,
		147,
		C.trim,
		Enum.Material.Concrete
	)
	local hatch = box(
		F.Fixtures,
		"KitchenContribution",
		-104,
		-80,
		B + 3.6,
		B + 4,
		139,
		145,
		C.brass,
		Enum.Material.Metal
	)
	hatch.CanCollide = false
	label(hatch, "HAND IT IN", Enum.NormalId.Top, 24)
	CollectionService:AddTag(hatch, "KitchenContribution")
	local roll = box(
		F.Fixtures,
		"ClubBoard",
		-W + WALL_T + 3.2,
		-W + WALL_T + 4,
		B + 4,
		B + 16,
		96,
		132,
		C.timber,
		Enum.Material.WoodPlanks
	)
	label(roll, "THE GOOD CHEER CLUB", Enum.NormalId.Right, 30)
	CollectionService:AddTag(roll, "ClubBoard")

	-- Your workshop corner, where visitors walk past it.
	-- East of the core. It was sitting in the stairwell, across the foot of the
	-- first flight.
	-- East of the core and clear of the lift shaft, which the last position ran
	-- straight through.
	box(
		F.Fixtures,
		"WorkBench",
		12,
		34,
		B + 2.4,
		B + 3.2,
		20,
		30,
		C.timber,
		Enum.Material.WoodPlanks
	)
	for _, x in ipairs({ 15, 31 }) do
		box(
			F.Fixtures,
			`WBLeg_{x}`,
			x - 1,
			x + 1,
			B,
			B + 2.4,
			24,
			26,
			C.timber,
			Enum.Material.WoodPlanks
		)
	end
	local vice =
		box(F.Fixtures, "Vice", 30, 34, B + 3.2, B + 4.6, 23, 27, C.brass, Enum.Material.Metal)
	CollectionService:AddTag(vice, "WorkshopBench")
	local shop =
		box(F.Markers, "WorkshopPlot", 8, 38, B, B + 0.4, 14, 38, C.cloth, Enum.Material.Fabric)
	shop.CanCollide = false
	CollectionService:AddTag(shop, "WorkshopPlot")

	local departure =
		box(F.Markers, "Departure", -16, 16, B, B + 0.4, -26, -6, C.brass, Enum.Material.Metal)
	departure.CanCollide = false
	label(departure, "TO THE ROAD", Enum.NormalId.Top, 26)
	CollectionService:AddTag(departure, "Departure")

	-- The one thing a hub has to do is get a new player into a run without
	-- being told, and a brass plate flush with the floor does not do it. Two
	-- lamps mark the pad, the pad itself is lit, and a beam runs to it from
	-- where you land. Roblox's own onboarding docs describe testing exactly
	-- this - a guided arrow - and a staff post on funnel events found players
	-- stalling on a step until a small icon was put over the thing they were
	-- meant to use. This is that icon.
	do
		-- Brightness 1.4 and range 42, the same as every other lamp in the
		-- building. These were 2.2 and 2.6 at range 52, which is nearly twice
		-- the house lamp, and the Lighting block was tuned for a hall lit by
		-- house lamps. Marking the way out does not require out-shouting the
		-- room: the posts and the pad read as the departure because they are
		-- the only lit thing outside the doors, not because they are hot.
		for _, x in ipairs({ -20, 20 }) do
			box(
				F.Markers,
				`RoadLampPost_{x}`,
				x - 1.2,
				x + 1.2,
				B,
				B + 9,
				-18,
				-14,
				C.iron,
				Enum.Material.Metal
			)
			local head = box(
				F.Markers,
				`RoadLamp_{x}`,
				x - 2.6,
				x + 2.6,
				B + 9,
				B + 12,
				-18.6,
				-13.4,
				C.gilt,
				Enum.Material.Metal
			)
			local lamp = Instance.new("PointLight")
			lamp.Color, lamp.Brightness, lamp.Range, lamp.Shadows =
				Color3.fromRGB(255, 196, 120), 1.4, 42, false
			lamp.Parent = head
		end
	end
	local ret =
		box(F.Markers, "ReturnPoint", -16, 16, B, B + 0.4, 12, 32, C.brass, Enum.Material.Metal)
	ret.CanCollide = false
	CollectionService:AddTag(ret, "ReturnPoint")

	-- The hub owns the spawn. Until now the only SpawnLocation in the game was
	-- inside the Galleria, and the Galleria is not built until a raid is
	-- launched - so at server start Roblox had nowhere to put a player and
	-- dropped them into the void. Chris: "i have no clue where i am spawning",
	-- landing at y = -75 standing on his own torso.
	--
	-- A real SpawnLocation here also means arriving in the hub does not depend
	-- on a script running in the right order.
	local pad = Instance.new("SpawnLocation")
	pad.Name = "HallSpawn"
	pad.Size = Vector3.new(24, 1, 16)
	pad.CFrame = ret.CFrame
	pad.Anchored, pad.Neutral, pad.CanCollide = true, true, false
	pad.Transparency = 1
	pad.Parent = F.Markers

	-- Just inside the doors and to one side of the walk, so it is in frame
	-- when you land and again when you come back in, and across nobody's path.
	-- Each arm points at the room it names: the Lamp Wall on the east wall,
	-- the Good Cheer Club in the kitchen, the stair core for the Keeping Room
	-- three floors up, and the workshop bench.
	fingerpost(F.Markers, "HallPost", 20, 6, B, {
		{ text = "THE LAMP WALL", tx = 120, tz = 42 },
		{ text = "THE WORKSHOP", tx = 23, tz = 26 },
		{ text = "THE GOOD CHEER CLUB", tx = -92, tz = 142 },
		{ text = "THE KEEPING ROOM", tx = 60, tz = 49 },
	})

	-- No floor beam. It was a glowing strip and Chris did not want one.
	-- The lit lamp posts outside the open doors are the signpost now.

	finish(F.Basement, "StoreFloorDark", -W, CORE_X1, 150, Z1, B, C.dark)

	-- ------------------------------------------------- the back hall
	-- Chris, asked which wing to make rooms of first: "Do the basement back
	-- hall first - it's the floor people actually use."
	--
	-- Everything north of the landing was bare slab: 180 by 144 studs of real
	-- floor you could walk onto, outside every room anybody had programmed.
	-- It was lit in the last commit, and lighting an empty room only makes it
	-- a lit empty room.
	--
	-- A corridor across the back of the landing, entered through the doorway
	-- already cut in LandingNorth, with a room either side. Both are things
	-- the hub needed and had nowhere to put: the Good Cheer Club has been
	-- named on the fingerpost and on a board in the kitchen without ever
	-- having a room, and the supply tiers in data/loot.json feed
	-- WorkshopUpgrades with no counter in the building to hand anything over.
	do
		local CEIL = HEADS[1]
		-- The Long Room's door lines up with the landing door at x -30..-2,
		-- so the walk continues straight instead of crossing the corridor to
		-- find a way on.
		wall(F.Basement, "BackSpine", "Z", BH_SPINE, CORE_X1, W, B, CEIL, C.stone, {
			{ -24, -24 + DOOR_W },
			{ 72, 72 + DOOR_W },
		})
		wall(F.Basement, "BackDivide", "X", BH_DIVIDE, BH_SPINE, Z1, B, CEIL, C.stone, {
			{ 168, 168 + DOOR_W },
		})
		coffers(F.Basement, "BackCorridorCeiling", CORE_X1, W, 86, BH_SPINE, CEIL, 40, C.plaster)
		coffers(
			F.Basement,
			"LongRoomCeiling",
			CORE_X1,
			BH_DIVIDE,
			BH_SPINE,
			Z1,
			CEIL,
			44,
			C.stencil
		)
		coffers(F.Basement, "StoreCeiling", BH_DIVIDE, W, BH_SPINE, Z1, CEIL, 40, C.plaster)
		baseband(F.Basement, "BackCorridorBase", CORE_X1, W, 86, BH_SPINE, B)
		baseband(F.Basement, "LongRoomBase", CORE_X1, BH_DIVIDE, BH_SPINE, Z1, B)
		baseband(F.Basement, "StoreBase", BH_DIVIDE, W, BH_SPINE, Z1, B)
		finish(
			F.Basement,
			"LongRoomFloor",
			CORE_X1,
			BH_DIVIDE,
			BH_SPINE,
			Z1,
			B,
			C.timber,
			Enum.Material.WoodPlanks
		)

		-- Two columns a room, on the same grid the landing uses, so the back
		-- of the building reads as the same building.
		for i = 0, 1 do
			column(F.Basement, `LongRoomColumn_{i}`, -28 + i * 44, 150 + i * 40, B, CEIL, 9)
			column(F.Basement, `StoreColumn_{i}`, 58 + i * 44, 150 + i * 40, B, CEIL, 9)
		end

		-- ---- the Long Room ------------------------------------------------
		-- The Good Cheer Club's actual hall: long tables down the middle, a
		-- stove at the end, and the landmark footings along the west wall
		-- where anyone walking past sees what the town is building.
		for i = 0, 1 do
			local z0 = 134 + i * 46
			box(
				F.Fixtures,
				`ClubTable_{i}`,
				-36,
				20,
				B + 2.2,
				B + 3,
				z0,
				z0 + 30,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for _, dx in ipairs({ -34, 18 }) do
				box(
					F.Fixtures,
					`ClubTableLeg_{i}_{dx}`,
					dx,
					dx + 2,
					B,
					B + 2.2,
					z0 + 2,
					z0 + 28,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			for _, dx in ipairs({ -42, 24 }) do
				box(
					F.Fixtures,
					`ClubBench_{i}_{dx}`,
					dx,
					dx + 6,
					B + 1.4,
					B + 2,
					z0 + 2,
					z0 + 28,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
		end
		local stove =
			box(F.Fixtures, "ClubStove", 14, 28, B, B + 7, 214, 226, C.iron, Enum.Material.Metal)
		box(F.Fixtures, "ClubFlue", 19, 23, B + 7, CEIL, 218, 222, C.iron, Enum.Material.Metal)
		label(stove, "THE GOOD CHEER CLUB", Enum.NormalId.Front, 20)

		-- Landmark footings. data/loot.json sends RestorationCrates to
		-- LanternholdLandmarks and nothing in the building was that, so a
		-- crate carried all the way home had nowhere to go. These are the
		-- somewhere: three footings, named, with room to become something.
		--
		-- The tag is map contract offered to gameplay, exactly as
		-- KitchenContribution and WorkshopPlot already are. Nothing consumes
		-- it yet and nothing here assumes what it will do.
		for i, nm in ipairs({ "THE WELL", "THE BELL TOWER", "THE LONG BRIDGE" }) do
			local z0 = 124 + (i - 1) * 34
			local site = box(
				F.Markers,
				`LandmarkSite_{i}`,
				CORE_X1 + 4,
				CORE_X1 + 22,
				B,
				B + 1.6,
				z0,
				z0 + 24,
				C.terrazzo,
				Enum.Material.Concrete
			)
			site:SetAttribute("Landmark", nm)
			label(site, nm, Enum.NormalId.Top, 16)
			CollectionService:AddTag(site, "LandmarkSite")
		end

		-- ---- the Quartermaster's store -------------------------------------
		local counter = box(
			F.Fixtures,
			"QuartermasterCounter",
			44,
			48,
			B,
			B + 3.4,
			126,
			168,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(counter, "QUARTERMASTER", Enum.NormalId.Left, 20)
		CollectionService:AddTag(counter, "Quartermaster")
		box(
			F.Fixtures,
			"QuartermasterTop",
			42,
			52,
			B + 3.4,
			B + 3.9,
			126,
			168,
			C.brass,
			Enum.Material.Metal
		)

		-- Shelving with a gangway between the runs, so the room can be walked
		-- rather than looked into.
		for i = 0, 2 do
			local x0 = 62 + i * 24
			box(
				F.Fixtures,
				`StoreShelf_{i}`,
				x0,
				x0 + 10,
				B,
				B + 11,
				128,
				196,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end
		for i = 0, 3 do
			local x0 = 58 + i * 18
			-- Kept clear of the north shell wall, whose inner face is at 228.
			-- The first run of these reached 232 and two of the four stood a
			-- third of the way inside it.
			box(
				F.Fixtures,
				`StoreCrate_{i}`,
				x0,
				x0 + 12,
				B,
				B + 6,
				200 + (i % 2) * 12,
				212 + (i % 2) * 12,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end
	end

	-- ------------------------------------------------------------- ballroom
	-- Eight thousand square feet, built and unlit, with its chandeliers fitted
	-- and never switched on. From the landing you see the whole volume and a
	-- rope across the doorway.
	local BR = LEVELS[2]
	-- Not a black void. The ballroom is a magnificent room that happens to be
	-- unlit, and you are meant to see that through the doorway - a coffered
	-- ceiling on a grid of capitalled columns over a deep red-brown floor. If
	-- it reads as an empty black box there is nothing to want.
	wall(
		F.Ballroom,
		"BallroomWest",
		"X",
		CORE_X1,
		Z0,
		Z1,
		BR,
		HEADS[2],
		C.stone,
		{ { 30, 30 + DOOR_W * 2 } },
		24
	)
	coffers(F.Ballroom, "BallroomCeiling", CORE_X1, W, Z0, Z1, HEADS[2], 50, C.plaster)
	for i = 0, 2 do
		for j = 0, 4 do
			column(
				F.Ballroom,
				`BallroomColumn_{i}_{j}`,
				-40 + i * 60,
				24 + j * 46,
				BR,
				HEADS[2],
				11
			)
		end
	end
	baseband(F.Ballroom, "BallroomBase", CORE_X1, W, Z0, Z1, BR)
	-- The chandeliers were modelled and dead. They are the room's own fitting,
	-- so they carry the light rather than a grid of lamps hung beside them -
	-- but five fittings in a line at x = 20 cannot cover a room 180 wide, so a
	-- weaker grid fills the flanks behind them.
	for i = 0, 4 do
		local z = 30 + i * 42
		local body = part(
			F.Ballroom,
			`Chandelier_{i}`,
			Vector3.new(14, 2, 14),
			CFrame.new(20, HEADS[2] - 10, z),
			C.brass,
			Enum.Material.Metal
		)
		part(
			F.Ballroom,
			`ChandelierChain_{i}`,
			Vector3.new(0.8, 10, 0.8),
			CFrame.new(20, HEADS[2] - 4, z),
			C.dark,
			Enum.Material.Metal
		)
		local light = Instance.new("PointLight")
		light.Color, light.Brightness, light.Range, light.Shadows = WARM, 1.0, 46, false
		light.Parent = body
	end
	lightRoom(
		F.Ballroom,
		"BallroomLamp",
		CORE_X1,
		W,
		Z0,
		Z1,
		HEADS[2],
		{ Brightness = 0.75, Range = 40 }
	)
	-- A rail beside the opening, not a barrier across it. At three and a half
	-- studs spanning the whole doorway this was a wall to a Humanoid, and the
	-- room is open now.
	local rail = box(
		F.Ballroom,
		"BallroomRail",
		CORE_X1 - 1,
		CORE_X1 + 1,
		BR,
		BR + 3.4,
		30,
		36,
		C.brass,
		Enum.Material.Metal
	)
	rail.CanCollide = false
	local bn = box(
		F.Ballroom,
		"BallroomNotice",
		CORE_X1 - 1.8,
		CORE_X1 - 1.2,
		BR + 0.8,
		BR + 3,
		36,
		52,
		C.timber,
		Enum.Material.WoodPlanks
	)
	label(bn, "THE BALLROOM", Enum.NormalId.Left, 18)

	-- ------------------------------------------------------------- lodge
	-- The two lodge rooms, shut and dark and visible through their doorways.
	-- The Egyptian room in ochre; the Norman Gothic chapel in grey stone with
	-- timber beams and a slate floor. These are the rooms worth lighting.
	local LD = LEVELS[3]
	finish(F.Lodge, "EgyptFloor", CORE_X1, W, Z0, 120, LD, C.egypt, Enum.Material.Slate)
	finish(F.Lodge, "GothicFloor", CORE_X1, W, 120, Z1, LD, C.gothic, Enum.Material.Slate)
	wall(F.Lodge, "LodgeSpine", "Z", 120, CORE_X1, W, LD, HEADS[3], C.stone, {})
	wall(
		F.Lodge,
		"LodgeWest",
		"X",
		CORE_X1,
		Z0,
		Z1,
		LD,
		HEADS[3],
		C.stone,
		{ { 40, 40 + DOOR_W }, { 150, 150 + DOOR_W } },
		18
	)
	for i = 0, 3 do
		local z = 24 + i * 24
		box(
			F.Lodge,
			`EgyptColumn_{i}`,
			30,
			38,
			LD,
			HEADS[3],
			z,
			z + 8,
			C.egypt,
			Enum.Material.Concrete
		)
	end
	for i = 0, 3 do
		local z = 136 + i * 22
		box(
			F.Lodge,
			`GothicBeam_{i}`,
			CORE_X1,
			W,
			HEADS[3] - 6,
			HEADS[3] - 3,
			z,
			z + 4,
			C.timber,
			Enum.Material.WoodPlanks
		)
	end
	-- Both rooms lit and open. The Egyptian room keeps its ochre and the chapel
	-- its grey, so they still read as two different rooms rather than two lit
	-- boxes, and the chapel sits a shade dimmer than the room below it.
	lightRoom(
		F.Lodge,
		"EgyptLamp",
		CORE_X1,
		W,
		Z0,
		120,
		HEADS[3],
		{ Brightness = 0.85, Range = 34 }
	)
	lightRoom(
		F.Lodge,
		"GothicLamp",
		CORE_X1,
		W,
		120,
		Z1,
		HEADS[3],
		{ Brightness = 0.8, Range = 34 }
	)
	for _, spec in ipairs({
		{ "Egypt", 40, "THE EGYPTIAN ROOM" },
		{ "Gothic", 150, "THE CHAPEL" },
	}) do
		local nm, z, text = spec[1] :: string, spec[2] :: number, spec[3] :: string
		local bar = box(
			F.Lodge,
			`{nm}Bar`,
			CORE_X1 - 1,
			CORE_X1 + 1,
			LD,
			LD + 3.4,
			z,
			z + DOOR_W,
			C.brass,
			Enum.Material.Metal
		)
		bar.CanCollide = false
		local n = box(
			F.Lodge,
			`{nm}Notice`,
			CORE_X1 - 1.8,
			CORE_X1 - 1.2,
			LD + 0.8,
			LD + 3,
			z + 1,
			z + DOOR_W - 1,
			C.timber,
			Enum.Material.WoodPlanks
		)
		label(n, text, Enum.NormalId.Left, 16)
	end

	-- ------------------------------------------------------------- museum
	-- The Keeping Room. Relics are the lantern-only tier, so the only way the
	-- story arrives is by somebody taking the dangerous choice.
	local MU = LEVELS[4]
	finish(F.Museum, "StorageFloor", CORE_X1, W, 120, Z1, MU, C.dark)
	wall(
		F.Museum,
		"MuseumWest",
		"X",
		CORE_X1,
		Z0,
		Z1,
		MU,
		HEADS[4],
		C.stone,
		{ { 30, 30 + DOOR_W } }
	)
	wall(
		F.Museum,
		"MuseumNorth",
		"Z",
		120,
		CORE_X1,
		W,
		MU,
		HEADS[4],
		C.stone,
		{ { 20, 20 + DOOR_W } }
	)
	coffers(F.Museum, "MuseumCeiling", CORE_X1, W, Z0, Z1, HEADS[4], 44, C.plaster, true)
	baseband(F.Museum, "MuseumBase", CORE_X1, W, Z0, 120, MU)
	-- Three lamps in a line at x = 20 lit a stripe up the middle of a floor
	-- 180 studs wide. Both the Keeping Room and the storage room get a grid.
	lightRoom(
		F.Museum,
		"MuseumLamp",
		CORE_X1,
		W,
		Z0,
		120,
		HEADS[4],
		{ Brightness = 0.95, Range = 34 }
	)
	lightRoom(
		F.Museum,
		"StorageLamp",
		CORE_X1,
		W,
		120,
		Z1,
		HEADS[4],
		{ Brightness = 0.8, Range = 34 }
	)
	-- The plinths stand on the HUB floor, not up here. Shelving a relic is
	-- something a keeper does between runs, and up here it cost 568 studs of
	-- walking for 112 of distance - a detour of 5.1, by far the worst thing in
	-- the building and most of what Chris meant by "lots of nooks and crannies
	-- that are awkward". Everything else in the hall walks at 1.0 to 1.2.
	--
	-- The museum keeps the burnt case below, and keeps being worth the climb
	-- for its own sake. It just stops being on the critical path.
	for i = 0, 3 do
		local x = 36 + i * 16
		box(F.Fixtures, `Plinth_{i}`, x, x + 12, B, B + 4, 44, 54, C.trim, Enum.Material.Concrete)
		local slot = box(
			F.Fixtures,
			`RelicSlot_{i}`,
			x + 1,
			x + 11,
			B + 4,
			B + 4.4,
			45,
			53,
			C.brass,
			Enum.Material.Metal
		)
		slot.CanCollide = false
		slot:SetAttribute("Slot", i)
		CollectionService:AddTag(slot, "RelicPlinth")
		local plate = box(
			F.Fixtures,
			`Plaque_{i}`,
			x + 1,
			x + 11,
			B + 1.6,
			B + 2.2,
			43.2,
			43.8,
			C.brass,
			Enum.Material.Metal
		)
		label(plate, "UNNAMED", Enum.NormalId.Front, 18)
	end
	-- The case that can never be filled: the order's earliest records burned
	-- long before any flood, so nothing from a run goes in here. The oldest
	-- things in the room are the ones nobody can name, and they stay that way.
	box(F.Fixtures, "BurntCase", 96, 112, MU, MU + 1, 30, 70, C.trim, Enum.Material.Concrete)
	box(
		F.Fixtures,
		"BurntCaseGlass",
		96,
		112,
		MU + 1,
		MU + 7,
		30,
		70,
		C.glass,
		Enum.Material.Glass,
		0.72
	)
	local burnt = box(
		F.Fixtures,
		"BurntCasePlaque",
		94.8,
		95.4,
		MU + 2,
		MU + 5,
		34,
		66,
		C.timber,
		Enum.Material.WoodPlanks
	)
	label(burnt, "THE YEARS BEFORE THE FIRE — NOTHING RECOVERED", Enum.NormalId.Left, 16)

	-- ------------------------------------------------- the north wings
	-- Chris: "do the north wings on the upper floors next."
	--
	-- Same fault as the back hall, three floors up and three times over. West
	-- of the core and north of the stair, every upper level carried 76 by 138
	-- studs of bare slab - floor you could walk onto, outside every room, with
	-- nothing on it. Lit in an earlier commit, which made them lit empty
	-- rooms.
	--
	-- Built as one loop rather than three hand-written rooms. The structure is
	-- identical on every floor - the core is in the same place on all of them
	-- - so the only thing that varies is what the room is for, and that is the
	-- table below. Three copies of this written out by hand is three places
	-- for the next change to the core to go stale.
	--
	-- Each room is entered from the core landing through one door, centred on
	-- the landing's own width so the walk off the stair continues straight.
	do
		local WING_WALL = 100 -- the room's south wall, clear of stair and lift
		local DOOR_AT = -80 -- door runs DOOR_AT .. DOOR_AT + DOOR_W

		for i = 2, #LEVELS do
			local lvl, ceil = LEVELS[i], HEADS[i]
			local folder = if i == 2 then F.Ballroom elseif i == 3 then F.Lodge else F.Museum

			wall(folder, `WingWall_{i}`, "Z", WING_WALL, CORE_X0, CORE_X1, lvl, ceil, C.stone, {
				{ DOOR_AT, DOOR_AT + DOOR_W },
			})
			coffers(folder, `WingCeiling_{i}`, CORE_X0, CORE_X1, WING_WALL, Z1, ceil, 40, C.plaster)
			baseband(folder, `WingBase_{i}`, CORE_X0, CORE_X1, WING_WALL, Z1, lvl)
			for j = 0, 1 do
				column(folder, `WingColumn_{i}_{j}`, -108 + j * 40, 140 + j * 50, lvl, ceil, 8)
			end
		end

		-- Level two, over the ballroom: the Muster Room. Where a crew forms up
		-- before going out - benches down both walls, pegs for kit, and a
		-- board. The ballroom next door is for the town; this is for the
		-- keepers, and the hub had no room that was.
		do
			local lvl = LEVELS[2]
			finish(
				F.Ballroom,
				"MusterFloor",
				CORE_X0,
				CORE_X1,
				WING_WALL,
				Z1,
				lvl,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for _, x in ipairs({ CORE_X0 + 6, CORE_X1 - 12 }) do
				box(
					F.Fixtures,
					`MusterBench_{x}`,
					x,
					x + 6,
					lvl + 1.4,
					lvl + 2,
					112,
					216,
					C.timber,
					Enum.Material.WoodPlanks
				)
				box(
					F.Fixtures,
					`MusterPegs_{x}`,
					x,
					x + 2,
					lvl + 9,
					lvl + 10,
					112,
					216,
					C.iron,
					Enum.Material.Metal
				)
			end
			local board = box(
				F.Fixtures,
				"MusterBoard",
				-100,
				-62,
				lvl + 5,
				lvl + 13,
				226,
				227,
				C.board,
				Enum.Material.WoodPlanks
			)
			label(board, "THE MUSTER ROOM", Enum.NormalId.Front, 22)
		end

		-- Level three, beside the lodge rooms: the Workroom. Relics feed
		-- SpecialCrafting in data/loot.json and there was no bench in the
		-- building to do it at - the Keeping Room names a relic and the
		-- workshop downstairs is for Supplies. A lodge keeps its preparation
		-- room on the lodge floor, so this is where that goes.
		do
			local lvl = LEVELS[3]
			finish(
				F.Lodge,
				"WorkroomFloor",
				CORE_X0,
				CORE_X1,
				WING_WALL,
				Z1,
				lvl,
				C.gothic,
				Enum.Material.Slate
			)
			local bench = box(
				F.Fixtures,
				"RelicBench",
				-112,
				-96,
				lvl,
				lvl + 3.4,
				120,
				190,
				C.timber,
				Enum.Material.WoodPlanks
			)
			box(
				F.Fixtures,
				"RelicBenchTop",
				-114,
				-94,
				lvl + 3.4,
				lvl + 3.9,
				120,
				190,
				C.brass,
				Enum.Material.Metal
			)
			label(bench, "THE WORKROOM", Enum.NormalId.Right, 18)
			CollectionService:AddTag(bench, "RelicBench")
			for j = 0, 2 do
				local z0 = 124 + j * 30
				box(
					F.Fixtures,
					`WorkroomRack_{j}`,
					-70,
					-56,
					lvl,
					lvl + 12,
					z0,
					z0 + 20,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			box(
				F.Fixtures,
				"WorkroomStool",
				-90,
				-84,
				lvl,
				lvl + 2.4,
				150,
				156,
				C.timber,
				Enum.Material.WoodPlanks
			)
		end

		-- Level four, beside the Keeping Room: the Archive. The museum names a
		-- relic; this is where the naming is written down and kept. Shelving,
		-- a reading desk under the lamp, and a map table.
		do
			local lvl = LEVELS[4]
			finish(
				F.Museum,
				"ArchiveFloor",
				CORE_X0,
				CORE_X1,
				WING_WALL,
				Z1,
				lvl,
				C.timber,
				Enum.Material.WoodPlanks
			)
			for j = 0, 3 do
				local z0 = 116 + j * 28
				box(
					F.Fixtures,
					`ArchiveShelf_{j}`,
					-120,
					-106,
					lvl,
					lvl + 13,
					z0,
					z0 + 20,
					C.timber,
					Enum.Material.WoodPlanks
				)
			end
			local desk = box(
				F.Fixtures,
				"ArchiveDesk",
				-92,
				-62,
				lvl,
				lvl + 3,
				136,
				160,
				C.timber,
				Enum.Material.WoodPlanks
			)
			label(desk, "THE ARCHIVE", Enum.NormalId.Top, 18)
			box(
				F.Fixtures,
				"ArchiveChair",
				-80,
				-74,
				lvl,
				lvl + 2.6,
				164,
				170,
				C.timber,
				Enum.Material.WoodPlanks
			)
			box(
				F.Fixtures,
				"ArchiveMapTable",
				-100,
				-60,
				lvl + 2.6,
				lvl + 3.2,
				190,
				216,
				C.cloth,
				Enum.Material.Fabric
			)
		end
	end

	M.Parent = Workspace
	return M
end

return LanternholdBuilder
