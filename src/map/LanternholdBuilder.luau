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
local CORE_X0, CORE_X1 = -126, -72 -- the stair and lift core, west side
local CORE_Z0, CORE_Z1 = 16, 92

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
}
local WARM = Color3.fromRGB(255, 214, 160)

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

local function box(parent: Instance, name: string, x0: number, x1: number, y0: number, y1: number, z0: number, z1: number, colour: Color3, mat: Enum.Material?, trans: number?): Part
	return part(parent, name, Vector3.new(x1 - x0, y1 - y0, z1 - z0), CFrame.new((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2), colour, mat, trans)
end

--- A wall with doorways cut in it, each with a lintel over it.
local function wall(parent: Instance, name: string, axis: string, at: number, from: number, to: number, y0: number, y1: number, colour: Color3, gaps: { { number } })
	local head = math.min(y0 + DOOR_H, y1)
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
				box(parent, `{name}_{i}`, at - h, at + h, y0, y1, r[1], r[2], colour, Enum.Material.Concrete)
			else
				box(parent, `{name}_{i}`, r[1], r[2], y0, y1, at - h, at + h, colour, Enum.Material.Concrete)
			end
		end
	end
	for i, g in ipairs(cuts) do
		if y1 > head then
			if axis == "X" then
				box(parent, `{name}_Head_{i}`, at - h, at + h, head, y1, g[1], g[2], colour, Enum.Material.Concrete)
			else
				box(parent, `{name}_Head_{i}`, g[1], g[2], head, y1, at - h, at + h, colour, Enum.Material.Concrete)
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

local function lamp(parent: Instance, name: string, x: number, y: number, z: number, lit: boolean): Part
	local glass = part(parent, name, Vector3.new(3, 4.5, 3), CFrame.new(x, y, z), if lit then WARM else C.dark, Enum.Material.Glass, 0.35)
	part(parent, `{name}_Bracket`, Vector3.new(1, 1, 3.5), CFrame.new(x, y + 2.6, z), C.brass, Enum.Material.Metal)
	if lit then
		local light = Instance.new("PointLight")
		light.Color, light.Brightness, light.Range = WARM, 1.5, 42
		-- Shadows off everywhere. A lantern that casts them strobes as you walk
		-- past the posts, which is the flicker Chris hit in the mall.
		light.Shadows = false
		light.Parent = glass
	end
	return glass
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
local function flight(parent: Instance, name: string, x0: number, x1: number, zLow: number, zHigh: number, yLow: number, yHigh: number)
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
		local tread = part(parent, `{name}_Tread_{k}`, Vector3.new(x1 - x0 - 1, 0.4, 1), cf * CFrame.new(0, 1.2, t), C.stone, Enum.Material.Concrete)
		tread.CanCollide = false
	end
	-- One handrail, on the outer side only. Railed on both, the flight is a
	-- walled corridor and the rail on the landing side stands between the top
	-- of the stair and the floor it arrives at.
	part(parent, `{name}_Rail`, Vector3.new(0.8, 3.4, hyp), cf * CFrame.new(-((x1 - x0) / 2 - 0.4), 2.6, 0), C.brass, Enum.Material.Metal)

	local function pad(nm: string, y: number, za: number, zb: number)
		box(parent, nm, x0, x1, y - 2, y, math.min(za, zb), math.max(za, zb), C.trim, Enum.Material.Concrete)
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
	for _, n in ipairs({ "Ground", "Shell", "Core", "Basement", "Ballroom", "Lodge", "Museum", "Fixtures", "Markers" }) do
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
	for i, lvl in ipairs(LEVELS) do
		if i == 1 then
			box(F.Shell, `Slab_{i}`, -W, W, lvl - 2, lvl, Z0, Z1, C.board, Enum.Material.WoodPlanks)
		else
			box(F.Shell, `Slab_{i}_E`, CORE_X1, W, lvl - 2, lvl, Z0, Z1, C.board, Enum.Material.WoodPlanks)
			box(F.Shell, `Slab_{i}_W`, -W, CORE_X0, lvl - 2, lvl, Z0, Z1, C.board, Enum.Material.WoodPlanks)
			box(F.Shell, `Slab_{i}_S`, CORE_X0, CORE_X1, lvl - 2, lvl, Z0, CORE_Z0, C.board, Enum.Material.WoodPlanks)
			box(F.Shell, `Slab_{i}_N`, CORE_X0, CORE_X1, lvl - 2, lvl, CORE_Z1, Z1, C.board, Enum.Material.WoodPlanks)
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
			box(F.Shell, `Pilaster_{i}_{x}`, x - 2, x + 2, 0, TOP, z - 5, z + 5, C.stone, Enum.Material.Concrete)
		end
		if i < 8 then
			for _, x in ipairs({ -W - 1, W + 1 }) do
				-- One tall recessed slot per bay: in the photograph the glazing
				-- runs most of the height rather than floor by floor.
				box(F.Shell, `Glazing_{i}_{x}`, x - 1, x + 1, 30, TOP - 16, z + 7, z + 22, C.glass, Enum.Material.Glass, 0.45)
			end
		end
	end
	box(F.Shell, "Frieze", -W - 4, W + 4, TOP - 14, TOP - 8, Z0 - 4, Z1 + 4, C.trim, Enum.Material.Concrete)
	box(F.Shell, "Parapet", -W - 4, W + 4, TOP, TOP + 8, Z0 - 4, Z1 + 4, C.stone, Enum.Material.Concrete)
	box(F.Shell, "Tower", -W - 10, -W + 16, 0, TOP + 22, Z0 + 6, Z0 + 44, C.stone, Enum.Material.Concrete)

	local name = box(F.Shell, "NameStone", -40, 40, 15, 23, Z0 - 3.2, Z0 - 2, C.trim, Enum.Material.Concrete)
	label(name, "LANTERNHOLD", Enum.NormalId.Front)

	-- ------------------------------------------------------------- the core
	-- Stairs and the lift in one well, open the whole height. From the basement
	-- you can see three unlit landings above you.
	for i = 2, #LEVELS do
		local up = i % 2 == 0
		flight(F.Core, `Stair_{i}`, CORE_X0 + 6, CORE_X0 + 32, if up then CORE_Z0 + 8 else CORE_Z1 - 8, if up then CORE_Z1 - 8 else CORE_Z0 + 8, LEVELS[i - 1], LEVELS[i])
	end
	wall(F.Core, "CoreEast", "X", CORE_X1, CORE_Z0, CORE_Z1, 0, TOP, C.stone, {})
	for i, lvl in ipairs(LEVELS) do
		box(F.Core, `CoreLanding_{i}`, CORE_X0 + 32, CORE_X1, lvl - 2, lvl, CORE_Z0, CORE_Z1, C.board, Enum.Material.WoodPlanks)
	end

	local LIFT_X0, LIFT_X1 = CORE_X1 + 6, CORE_X1 + 34
	local LIFT_Z0, LIFT_Z1 = CORE_Z0 + 10, CORE_Z0 + 38
	wall(F.Core, "LiftWallW", "X", LIFT_X0, LIFT_Z0, LIFT_Z1, 0, TOP, C.trim, {})
	wall(F.Core, "LiftWallE", "X", LIFT_X1, LIFT_Z0, LIFT_Z1, 0, TOP, C.trim, {})
	wall(F.Core, "LiftWallN", "Z", LIFT_Z1, LIFT_X0, LIFT_X1, 0, TOP, C.trim, {})
	local car = box(F.Core, "LiftCar", LIFT_X0 + 2, LIFT_X1 - 2, -2, 0, LIFT_Z0 + 2, LIFT_Z1 - 2, C.trim, Enum.Material.Metal)
	car:SetAttribute("Stops", "0,26,64,92")
	CollectionService:AddTag(car, "Lift")

	-- ------------------------------------------------------------- basement
	local B = LEVELS[1]
	wall(F.Basement, "LandingNorth", "Z", 86, CORE_X1, W, B, HEADS[1], C.stone, { { -30, -2 } })
	-- Stops at the core. Carried across it, this ceiling capped the stair well
	-- and the first flight rose straight into its underside, which is why no
	-- floor above the basement could be reached.
	box(F.Basement, "LandingCeiling", CORE_X1, W, HEADS[1], HEADS[1] + 2, Z0, 86, C.timber, Enum.Material.WoodPlanks)
	for i = -1, 1, 2 do
		for j = 0, 1 do
			lamp(F.Basement, `LandingLamp_{i}_{j}`, i * 54, HEADS[1] - 5, 26 + j * 36, true)
		end
	end

	-- The Lamp Wall: one lantern per Lightkeeper out, tagged and numbered.
	box(F.Fixtures, "LampWallFace", W - WALL_T - 3, W - WALL_T, B, B + 20, 14, 70, C.timber, Enum.Material.WoodPlanks)
	local plaque = box(F.Fixtures, "LampWallPlaque", W - WALL_T - 4, W - WALL_T - 3.2, B + 21, B + 27, 24, 60, C.timber, Enum.Material.WoodPlanks)
	label(plaque, "KEEPERS OUT", Enum.NormalId.Left, 30)
	for i = 0, 7 do
		local g = lamp(F.Fixtures, `KeeperLamp_{i}`, W - WALL_T - 6, B + 11, 18 + i * 7, i % 3 ~= 2)
		g:SetAttribute("Slot", i)
		CollectionService:AddTag(g, "LampWallSlot")
	end

	-- The Community Kitchen and, behind it, the food room. This is what the
	-- floor was for, which is why the hall has a kitchen before an armoury.
	wall(F.Basement, "KitchenNorth", "Z", 150, -W, CORE_X1, B, HEADS[1], C.stone, { { -96, -68 } })
	box(F.Basement, "KitchenCeiling", -W, CORE_X1, HEADS[1], HEADS[1] + 2, CORE_Z1, 150, C.timber, Enum.Material.WoodPlanks)
	for i = 0, 1 do
		lamp(F.Basement, `KitchenLamp_{i}`, -98, HEADS[1] - 5, 102 + i * 30, true)
	end
	for i = 0, 1 do
		local x0 = -120 + i * 42
		box(F.Fixtures, `Table_{i}`, x0, x0 + 34, B + 2.2, B + 3, 98, 130, C.timber, Enum.Material.WoodPlanks)
		for _, z in ipairs({ 101, 127 }) do
			box(F.Fixtures, `TableLeg_{i}_{z}`, x0 + 3, x0 + 5, B, B + 2.2, z - 1, z + 1, C.timber, Enum.Material.WoodPlanks)
		end
		for _, dx in ipairs({ -4, 38 }) do
			box(F.Fixtures, `Bench_{i}_{dx}`, x0 + dx - 2, x0 + dx + 2, B + 1.4, B + 2, 100, 128, C.timber, Enum.Material.WoodPlanks)
		end
	end
	box(F.Fixtures, "Servery", -124, -60, B, B + 3, 138, 146, C.timber, Enum.Material.WoodPlanks)
	box(F.Fixtures, "ServeryTop", -126, -58, B + 3, B + 3.6, 137, 147, C.trim, Enum.Material.Concrete)
	local hatch = box(F.Fixtures, "KitchenContribution", -104, -80, B + 3.6, B + 4, 139, 145, C.brass, Enum.Material.Metal)
	hatch.CanCollide = false
	label(hatch, "HAND IT IN", Enum.NormalId.Top, 24)
	CollectionService:AddTag(hatch, "KitchenContribution")
	local roll = box(F.Fixtures, "ClubBoard", -W + WALL_T + 3.2, -W + WALL_T + 4, B + 4, B + 16, 96, 132, C.timber, Enum.Material.WoodPlanks)
	label(roll, "THE GOOD CHEER CLUB", Enum.NormalId.Right, 30)
	CollectionService:AddTag(roll, "ClubBoard")

	-- Your workshop corner, where visitors walk past it.
	-- East of the core. It was sitting in the stairwell, across the foot of the
	-- first flight.
	box(F.Fixtures, "WorkBench", -64, -32, B + 2.4, B + 3.2, 20, 30, C.timber, Enum.Material.WoodPlanks)
	for _, x in ipairs({ -61, -35 }) do
		box(F.Fixtures, `WBLeg_{x}`, x - 1, x + 1, B, B + 2.4, 24, 26, C.timber, Enum.Material.WoodPlanks)
	end
	local vice = box(F.Fixtures, "Vice", -38, -34, B + 3.2, B + 4.6, 23, 27, C.brass, Enum.Material.Metal)
	CollectionService:AddTag(vice, "WorkshopBench")
	local shop = box(F.Markers, "WorkshopPlot", -66, -30, B, B + 0.4, 16, 40, C.cloth, Enum.Material.Fabric)
	shop.CanCollide = false
	CollectionService:AddTag(shop, "WorkshopPlot")

	local departure = box(F.Markers, "Departure", -16, 16, B, B + 0.4, -26, -6, C.brass, Enum.Material.Metal)
	departure.CanCollide = false
	label(departure, "TO THE ROAD", Enum.NormalId.Top, 26)
	CollectionService:AddTag(departure, "Departure")
	local ret = box(F.Markers, "ReturnPoint", -16, 16, B, B + 0.4, 12, 32, C.brass, Enum.Material.Metal)
	ret.CanCollide = false
	CollectionService:AddTag(ret, "ReturnPoint")

	box(F.Basement, "StoreFloorDark", -W, CORE_X1, B - 2, B, 150, Z1, C.dark, Enum.Material.WoodPlanks)

	-- ------------------------------------------------------------- ballroom
	-- Eight thousand square feet, built and unlit, with its chandeliers fitted
	-- and never switched on. From the landing you see the whole volume and a
	-- rope across the doorway.
	local BR = LEVELS[2]
	box(F.Ballroom, "BallroomFloor", CORE_X1, W, BR - 2, BR, Z0, Z1, C.dark, Enum.Material.WoodPlanks)
	wall(F.Ballroom, "BallroomWest", "X", CORE_X1, Z0, Z1, BR, HEADS[2], C.stone, { { 30, 30 + DOOR_W * 2 } })
	box(F.Ballroom, "BallroomCeiling", CORE_X1, W, HEADS[2], HEADS[2] + 2, Z0, Z1, C.dark, Enum.Material.WoodPlanks)
	for i = 0, 4 do
		local z = 30 + i * 42
		part(F.Ballroom, `DeadChandelier_{i}`, Vector3.new(14, 2, 14), CFrame.new(20, HEADS[2] - 10, z), C.dark, Enum.Material.Metal)
		part(F.Ballroom, `DeadChain_{i}`, Vector3.new(0.8, 10, 0.8), CFrame.new(20, HEADS[2] - 4, z), C.dark, Enum.Material.Metal)
	end
	box(F.Ballroom, "BallroomBar", CORE_X1 - 1, CORE_X1 + 1, BR, BR + 3.4, 30, 30 + DOOR_W * 2, C.brass, Enum.Material.Metal)
	local bn = box(F.Ballroom, "BallroomNotice", CORE_X1 - 1.6, CORE_X1 - 1, BR + 4, BR + 8, 32, 56, C.timber, Enum.Material.WoodPlanks)
	label(bn, "THE BALLROOM — NOT YET LIT", Enum.NormalId.Left, 18)

	-- ------------------------------------------------------------- lodge
	-- The two lodge rooms, shut and dark and visible through their doorways.
	-- The Egyptian room in ochre; the Norman Gothic chapel in grey stone with
	-- timber beams and a slate floor. These are the rooms worth lighting.
	local LD = LEVELS[3]
	box(F.Lodge, "EgyptFloor", CORE_X1, W, LD - 2, LD, Z0, 120, C.egypt, Enum.Material.Slate)
	box(F.Lodge, "GothicFloor", CORE_X1, W, LD - 2, LD, 120, Z1, C.gothic, Enum.Material.Slate)
	wall(F.Lodge, "LodgeSpine", "Z", 120, CORE_X1, W, LD, HEADS[3], C.stone, {})
	wall(F.Lodge, "LodgeWest", "X", CORE_X1, Z0, Z1, LD, HEADS[3], C.stone, { { 40, 40 + DOOR_W }, { 150, 150 + DOOR_W } })
	box(F.Lodge, "LodgeCeiling", CORE_X1, W, HEADS[3], HEADS[3] + 2, Z0, Z1, C.dark, Enum.Material.WoodPlanks)
	for i = 0, 3 do
		local z = 24 + i * 24
		box(F.Lodge, `EgyptColumn_{i}`, 30, 38, LD, HEADS[3], z, z + 8, C.egypt, Enum.Material.Concrete)
	end
	for i = 0, 3 do
		local z = 136 + i * 22
		box(F.Lodge, `GothicBeam_{i}`, CORE_X1, W, HEADS[3] - 6, HEADS[3] - 3, z, z + 4, C.timber, Enum.Material.WoodPlanks)
	end
	for _, spec in ipairs({ { "Egypt", 40, "THE EGYPTIAN ROOM — SHUT" }, { "Gothic", 150, "THE CHAPEL — SHUT" } }) do
		local nm, z, text = spec[1] :: string, spec[2] :: number, spec[3] :: string
		box(F.Lodge, `{nm}Bar`, CORE_X1 - 1, CORE_X1 + 1, LD, LD + 3.4, z, z + DOOR_W, C.brass, Enum.Material.Metal)
		local n = box(F.Lodge, `{nm}Notice`, CORE_X1 - 1.6, CORE_X1 - 1, LD + 4, LD + 8, z, z + DOOR_W, C.timber, Enum.Material.WoodPlanks)
		label(n, text, Enum.NormalId.Left, 16)
	end

	-- ------------------------------------------------------------- museum
	-- The Keeping Room. Relics are the lantern-only tier, so the only way the
	-- story arrives is by somebody taking the dangerous choice.
	local MU = LEVELS[4]
	box(F.Museum, "MuseumFloor", CORE_X1, W, MU - 2, MU, Z0, 120, C.board, Enum.Material.WoodPlanks)
	box(F.Museum, "StorageFloor", CORE_X1, W, MU - 2, MU, 120, Z1, C.dark, Enum.Material.WoodPlanks)
	wall(F.Museum, "MuseumWest", "X", CORE_X1, Z0, Z1, MU, HEADS[4], C.stone, { { 30, 30 + DOOR_W } })
	wall(F.Museum, "MuseumNorth", "Z", 120, CORE_X1, W, MU, HEADS[4], C.stone, { { 20, 20 + DOOR_W } })
	box(F.Museum, "MuseumCeiling", CORE_X1, W, HEADS[4], HEADS[4] + 2, Z0, Z1, C.timber, Enum.Material.WoodPlanks)
	for i = 0, 2 do
		lamp(F.Museum, `MuseumLamp_{i}`, 20, HEADS[4] - 5, 30 + i * 34, true)
	end
	for i = 0, 3 do
		local z = 26 + i * 24
		box(F.Fixtures, `Plinth_{i}`, 60, 72, MU, MU + 4, z, z + 10, C.trim, Enum.Material.Concrete)
		local slot = box(F.Fixtures, `RelicSlot_{i}`, 61, 71, MU + 4, MU + 4.4, z + 1, z + 9, C.brass, Enum.Material.Metal)
		slot.CanCollide = false
		slot:SetAttribute("Slot", i)
		CollectionService:AddTag(slot, "RelicPlinth")
		local plate = box(F.Fixtures, `Plaque_{i}`, 59.2, 59.8, MU + 1.6, MU + 3.4, z + 1, z + 9, C.brass, Enum.Material.Metal)
		label(plate, "UNNAMED", Enum.NormalId.Left, 18)
	end
	-- The case that can never be filled: the order's earliest records burned
	-- long before any flood, so nothing from a run goes in here. The oldest
	-- things in the room are the ones nobody can name, and they stay that way.
	box(F.Fixtures, "BurntCase", 96, 112, MU, MU + 1, 30, 70, C.trim, Enum.Material.Concrete)
	box(F.Fixtures, "BurntCaseGlass", 96, 112, MU + 1, MU + 7, 30, 70, C.glass, Enum.Material.Glass, 0.72)
	local burnt = box(F.Fixtures, "BurntCasePlaque", 94.8, 95.4, MU + 2, MU + 5, 34, 66, C.timber, Enum.Material.WoodPlanks)
	label(burnt, "THE YEARS BEFORE THE FIRE — NOTHING RECOVERED", Enum.NormalId.Left, 16)

	M.Parent = Workspace
	return M
end

return LanternholdBuilder
