--!strict
-- Lanternhold — the hub, and the lobby you arrive in.
--
-- Built from docs/LANTERNHOLD.md. Three ideas from that document do all the
-- work here, and none of them is decoration:
--
--   1. MOST OF THE HALL IS DARK, AND VISIBLY SO. Not missing - there, and
--      unlit. The order laid its cornerstone in 1931 and did not finish the
--      main room until 1948; they moved in with one hall done, got caught by
--      the Depression, and met in a side room for twenty years. So the Great
--      Hall is built, full size, and black. You can see into it from the
--      Landing and you cannot go in. That single arch answers "why am I going
--      back out there" every time a player turns round, with no dialogue and
--      almost no content.
--
--   2. THE LAMP WALL. One lantern per Lightkeeper currently out. Yours burns
--      here while you are away; it comes down when you extract and goes out in
--      front of the whole hall when you wipe. The hall is inhabited even when
--      nobody is standing in it.
--
--   3. YOU ARE SAFE BECAUSE YOU ARE FAR. The road arrives from the south out
--      of the farms, and the building is too big for the people in it.
--
-- The geometry here is in studs, not the model units the Galleria uses. That
-- mall was traced off a real floor plan and needed a scale factor; this is one
-- building and reads better in the units it is walked in.
--
-- Run from the command bar:
--   require(workspace.Map.LanternholdBuilder).Build()
-- Builds Workspace.Lanternhold. Deliberately NOT inside Workspace.Map:
-- FloodedMallBuilder clears every child of Map except itself on the first line
-- of its build, so anything parented there is destroyed when the server starts.

local CollectionService = game:GetService("CollectionService")
local Workspace = game:GetService("Workspace")

local LanternholdBuilder = {}

-- The plan. South is -Z: the road comes up from the farms and meets the hall.
local W = 130 -- the hall's half-width
local LANDING_Z0, LANDING_Z1 = 0, 80 -- the lobby
local HALL_Z1 = 230 -- the back wall
local SIDE_X = -40 -- divide: side room west of this, Great Hall east
local KEEP_Z = 136 -- the kitchen gives way to the Keeping Room here
local CORRIDOR_Z = 170 -- where the lit side room gives out to the dark

local FLOOR_Y = 0
local LANDING_H = 34
local GREAT_H = 58
local WALL_T = 4
local DOOR_H = 22

local C = {
	brick = Color3.fromRGB(150, 108, 92),
	stone = Color3.fromRGB(206, 198, 186),
	timber = Color3.fromRGB(120, 84, 56),
	board = Color3.fromRGB(168, 150, 128),
	dark = Color3.fromRGB(38, 36, 40),
	brass = Color3.fromRGB(198, 160, 92),
	glass = Color3.fromRGB(196, 214, 212),
	cloth = Color3.fromRGB(122, 74, 62),
	grass = Color3.fromRGB(94, 104, 74),
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

-- Everything in this file is an axis-aligned slab between two corners, which is
-- how a building is actually described. Bounds in, part out.
local function box(parent: Instance, name: string, x0: number, x1: number, y0: number, y1: number, z0: number, z1: number, colour: Color3, mat: Enum.Material?, trans: number?): Part
	return part(parent, name, Vector3.new(x1 - x0, y1 - y0, z1 - z0), CFrame.new((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2), colour, mat, trans)
end

-- A wall with doorways cut in it. `gaps` are ranges along the run; each gets a
-- lintel over it so an opening is a doorway and not a slot to the roof.
local function wall(parent: Instance, name: string, axis: string, at: number, from: number, to: number, y0: number, y1: number, colour: Color3, gaps: { { number } }, headY: number?)
	local head = headY or (y0 + DOOR_H)
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
	local half = WALL_T / 2
	for i, r in ipairs(runs) do
		if axis == "X" then
			box(parent, `{name}_{i}`, at - half, at + half, y0, y1, r[1], r[2], colour, Enum.Material.Brick)
		else
			box(parent, `{name}_{i}`, r[1], r[2], y0, y1, at - half, at + half, colour, Enum.Material.Brick)
		end
	end
	for i, g in ipairs(cuts) do
		if y1 > head then
			if axis == "X" then
				box(parent, `{name}_Lintel_{i}`, at - half, at + half, head, y1, g[1], g[2], colour, Enum.Material.Brick)
			else
				box(parent, `{name}_Lintel_{i}`, g[1], g[2], head, y1, at - half, at + half, colour, Enum.Material.Brick)
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
	l.TextSize = size or 40
	l.TextColor3, l.Text = Color3.fromRGB(240, 226, 200), text
	l.Parent = g
end

local function lamp(parent: Instance, name: string, x: number, y: number, z: number, lit: boolean): Part
	local glass = part(parent, name, Vector3.new(6, 9, 6), CFrame.new(x, y, z), if lit then WARM else C.dark, Enum.Material.Glass, 0.35)
	part(parent, `{name}_Bracket`, Vector3.new(2, 2, 7), CFrame.new(x, y + 5, z), C.brass, Enum.Material.Metal)
	if lit then
		local light = Instance.new("PointLight")
		light.Color, light.Brightness, light.Range = WARM, 1.4, 34
		-- Shadows off everywhere in this build. A lantern that casts shadows
		-- strobes as the player walks past the posts; Chris hit exactly that in
		-- the mall and his words were "there is a ton of flickering when i walk".
		light.Shadows = false
		light.Parent = glass
	end
	return glass
end

function LanternholdBuilder.Build(): Model
	local old = Workspace:FindFirstChild("Lanternhold")
	if old then
		old:Destroy()
	end

	local M = Instance.new("Model")
	M.Name = "Lanternhold"
	local F: { [string]: Folder } = {}
	for _, n in ipairs({ "Ground", "Shell", "Landing", "GreatHall", "SideRoom", "LampWall", "Fixtures", "Markers" }) do
		local f = Instance.new("Folder")
		f.Name, f.Parent = n, M
		F[n] = f
	end

	-- ---------------------------------------------------------------- outside
	-- Ground enough to arrive across, and the road up out of the farms. The
	-- approach is a ramp rather than steps: discrete steps do not generate a
	-- navmesh anything can path over, which is the fault that has kept the
	-- other map's stairs unusable since the first playtest.
	box(F.Ground, "Ground", -360, 360, -8, -2, -220, 330, C.grass, Enum.Material.Grass)
	-- Road, porch and hall floor all top out at the same height. The first
	-- version had a tilted approach slab bridging a one-stud rise and the road
	-- would not path to the Landing at all; a ramp solving a step that does not
	-- exist is just an obstacle laid across the front door.
	box(F.Ground, "Road", -34, 34, -2, FLOOR_Y, -220, -34, C.stone, Enum.Material.Concrete)
	box(F.Ground, "Porch", -70, 70, -2, FLOOR_Y, -34, LANDING_Z0, C.stone, Enum.Material.Concrete)

	-- ---------------------------------------------------------------- shell
	box(F.Shell, "Floor", -W, W, -2, FLOOR_Y, LANDING_Z0, HALL_Z1, C.board, Enum.Material.WoodPlanks)
	wall(F.Shell, "WallWest", "X", -W, LANDING_Z0, HALL_Z1, FLOOR_Y, GREAT_H, C.brick, {})
	wall(F.Shell, "WallEast", "X", W, LANDING_Z0, HALL_Z1, FLOOR_Y, GREAT_H, C.brick, {})
	wall(F.Shell, "WallNorth", "Z", HALL_Z1, -W, W, FLOOR_Y, GREAT_H, C.brick, {})
	-- The front, with the door in it.
	wall(F.Shell, "WallSouth", "Z", LANDING_Z0, -W, W, FLOOR_Y, LANDING_H, C.brick, { { -15, 15 } })
	box(F.Shell, "Cornice", -W - 4, W + 4, GREAT_H, GREAT_H + 6, LANDING_Z0 - 4, HALL_Z1 + 4, C.stone, Enum.Material.Concrete)
	box(F.Shell, "RoofGreat", -W, W, GREAT_H, GREAT_H + 2, LANDING_Z1, HALL_Z1, C.dark, Enum.Material.Slate)
	box(F.Shell, "RoofLanding", -W, W, LANDING_H, LANDING_H + 2, LANDING_Z0, LANDING_Z1, C.dark, Enum.Material.Slate)

	-- The name over the door, cut in stone.
	local stone = box(F.Shell, "NameStone", -46, 46, LANDING_H - 14, LANDING_H - 2, LANDING_Z0 - 3.2, LANDING_Z0 - 2, C.stone, Enum.Material.Concrete)
	label(stone, "LANTERNHOLD", Enum.NormalId.Front)

	-- ---------------------------------------------------------------- landing
	box(F.Landing, "Ceiling", -W, W, LANDING_H - 2, LANDING_H, LANDING_Z0, LANDING_Z1, C.timber, Enum.Material.WoodPlanks)
	for i = -1, 1, 2 do
		for j = 0, 1 do
			local x = i * 62
			local z = 22 + j * 34
			part(F.Landing, `Post_{i}_{j}`, Vector3.new(9, LANDING_H - 2, 9), CFrame.new(x, (LANDING_H - 2) / 2, z), C.timber, Enum.Material.WoodPlanks)
		end
	end
	-- Doors, standing open. You are expected.
	for i = -1, 1, 2 do
		box(F.Landing, `Door_{i}`, i * 15 - (if i > 0 then 0 else 14), i * 15 + (if i > 0 then 14 else 0), FLOOR_Y, FLOOR_Y + DOOR_H, LANDING_Z0 - 1, LANDING_Z0 + 1, C.timber, Enum.Material.WoodPlanks)
	end

	-- ---------------------------------------------------------------- the dark
	-- The Great Hall. Full size, built, and black. The arch is wide enough to
	-- show the whole volume from the Landing and barred low enough that nobody
	-- mistakes it for a way through.
	wall(F.GreatHall, "ArchWall", "Z", LANDING_Z1, SIDE_X, W, FLOOR_Y, GREAT_H, C.brick, { { 10, 100 } }, FLOOR_Y + 40)
	wall(F.GreatHall, "SideWall", "X", SIDE_X, LANDING_Z1, HALL_Z1, FLOOR_Y, GREAT_H, C.brick, {})
	box(F.GreatHall, "DarkFloor", SIDE_X, W, -2, FLOOR_Y, LANDING_Z1, HALL_Z1, C.dark, Enum.Material.WoodPlanks)
	-- Unlit fixtures hanging in it: the room is finished enough to have had
	-- lights fitted and never switched on.
	for i = 0, 3 do
		local z = LANDING_Z1 + 28 + i * 32
		part(F.GreatHall, `DeadChandelier_{i}`, Vector3.new(22, 3, 22), CFrame.new(45, GREAT_H - 16, z), C.dark, Enum.Material.Metal)
		part(F.GreatHall, `DeadChain_{i}`, Vector3.new(1.4, 16, 1.4), CFrame.new(45, GREAT_H - 7, z), C.dark, Enum.Material.Metal)
	end
	-- The bar across the arch, and a board saying why.
	-- A barrier you cannot walk under and cannot jump onto. The first version
	-- was a brass rail at thirteen studs, which is above head height: it looked
	-- like a barrier and pathfinding strolled straight underneath it into the
	-- Great Hall. Ten studs of solid balustrade stops that without closing the
	-- view - you stand back from it, and you see over it into the dark.
	box(F.GreatHall, "ArchBarrier", 10, 100, FLOOR_Y, FLOOR_Y + 10, LANDING_Z1 - 1.5, LANDING_Z1 + 1.5, C.timber, Enum.Material.WoodPlanks)
	box(F.GreatHall, "ArchRail", 9, 101, FLOOR_Y + 10.5, FLOOR_Y + 13, LANDING_Z1 - 2, LANDING_Z1 + 2, C.brass, Enum.Material.Metal)
	for _, x in ipairs({ 12, 98 }) do
		part(F.GreatHall, `ArchPost_{x}`, Vector3.new(3, 13, 3), CFrame.new(x, FLOOR_Y + 6.5, LANDING_Z1), C.brass, Enum.Material.Metal)
	end
	local notice = box(F.GreatHall, "ArchNotice", 38, 72, FLOOR_Y + 14, FLOOR_Y + 26, LANDING_Z1 - 2.2, LANDING_Z1 - 1.6, C.timber, Enum.Material.WoodPlanks)
	label(notice, "THE GREAT HALL — NOT YET LIT", Enum.NormalId.Front, 28)

	-- ---------------------------------------------------------------- the lit
	-- The side room: where the order actually met for twenty years, and where
	-- the Kitchen and the workshop corner go when they are built.
	wall(F.SideRoom, "SideDoorWall", "Z", LANDING_Z1, -W, SIDE_X, FLOOR_Y, GREAT_H, C.brick, { { -100, -70 } })
	box(F.SideRoom, "KitchenCeiling", -W, SIDE_X, LANDING_H - 2, LANDING_H, LANDING_Z1, KEEP_Z, C.timber, Enum.Material.WoodPlanks)
	for i = 0, 1 do
		lamp(F.SideRoom, `KitchenLamp_{i}`, -85, LANDING_H - 12, LANDING_Z1 + 18 + i * 30, true)
	end

	-- The Community Kitchen. v1's shared landmark and the Good Cheer Club's
	-- first project: long tables and a servery, church-basement energy. The
	-- hall has a kitchen before it has an armoury, which is the whole tone of
	-- the place - a town that has done this before and knows the drill.
	-- Two tables side by side across the room rather than stacked down it. The
	-- kitchen is only fifty-six studs deep; stacked, the second table's benches
	-- ran through the servery and out through the Keeping Room wall.
	for i = 0, 1 do
		local x0 = -122 + i * 42
		local x1 = x0 + 36
		local z = LANDING_Z1 + 22
		box(F.Fixtures, `Table_{i}`, x0, x1, FLOOR_Y + 9, FLOOR_Y + 11, z - 7, z + 7, C.timber, Enum.Material.WoodPlanks)
		for _, x in ipairs({ x0 + 5, x1 - 5 }) do
			part(F.Fixtures, `TableLeg_{i}_{x}`, Vector3.new(3, 9, 3), CFrame.new(x, FLOOR_Y + 4.5, z), C.timber, Enum.Material.WoodPlanks)
		end
		for _, dz in ipairs({ -12, 12 }) do
			box(F.Fixtures, `Bench_{i}_{dz}`, x0 + 3, x1 - 3, FLOOR_Y + 5, FLOOR_Y + 6.5, z + dz - 4, z + dz + 4, C.timber, Enum.Material.WoodPlanks)
		end
	end

	-- The servery along the far wall, and the hatch a crew hands its haul over
	-- at. Contributing is a counter you walk up to, not a menu: the Club has a
	-- ledger and a board, and this is where a restoration crate stops being
	-- yours and starts being the next room's lighting.
	box(F.Fixtures, "Servery", -124, -56, FLOOR_Y, FLOOR_Y + 12, KEEP_Z - 12, KEEP_Z - 4, C.timber, Enum.Material.WoodPlanks)
	box(F.Fixtures, "ServeryTop", -126, -54, FLOOR_Y + 12, FLOOR_Y + 13.2, KEEP_Z - 13, KEEP_Z - 3, C.stone, Enum.Material.Concrete)
	local hatch = box(F.Fixtures, "KitchenContribution", -108, -80, FLOOR_Y + 13.2, FLOOR_Y + 13.8, KEEP_Z - 13, KEEP_Z - 5, C.brass, Enum.Material.Metal)
	hatch.CanCollide = false
	label(hatch, "HAND IT IN", Enum.NormalId.Top, 26)
	CollectionService:AddTag(hatch, "KitchenContribution")

	-- Crates already handed in, stacked where they were put down. Evidence that
	-- other people have been doing this, which is most of what makes a hub feel
	-- lived in.
	for i, spec in ipairs({ { -124, 98, 0 }, { -124, 98, 9 }, { -112, 100, 0 }, { -124, 110, 0 } }) do
		local x, z, dy = spec[1], spec[2], spec[3]
		box(F.Fixtures, `HandedIn_{i}`, x - 7, x + 7, FLOOR_Y + dy, FLOOR_Y + dy + 9, z - 7, z + 7, C.cloth, Enum.Material.WoodPlanks)
	end

	-- ---------------------------------------------------------------- keeping
	-- The Keeping Room: the order's vault, and the game's lore engine. Relics
	-- are the lantern-only tier, so the only way the story arrives is by
	-- somebody taking the dangerous choice. Play it safe and you finish runs
	-- and learn nothing; burn light and the hall starts talking.
	wall(F.SideRoom, "KeepWall", "Z", KEEP_Z, -W, SIDE_X, FLOOR_Y, GREAT_H, C.brick, { { -100, -70 } })
	box(F.SideRoom, "KeepCeiling", -W, SIDE_X, LANDING_H - 2, LANDING_H, KEEP_Z, CORRIDOR_Z, C.timber, Enum.Material.WoodPlanks)
	lamp(F.SideRoom, "KeepLamp", -85, LANDING_H - 12, (KEEP_Z + CORRIDOR_Z) / 2, true)

	-- Shelved and named. Each plinth is tagged and numbered so the gameplay
	-- side can put a relic on one and write the plaque; the map only provides
	-- the shelf.
	-- Four plinths, two each side of the corridor door. Five evenly spaced
	-- across this wall stood shoulder to shoulder over the doorway with
	-- four-stud slots between them, and the dark corridor became unreachable.
	for i, x in ipairs({ -124, -110, -62, -48 }) do
		box(F.Fixtures, `Plinth_{i}`, x - 6, x + 6, FLOOR_Y, FLOOR_Y + 13, CORRIDOR_Z - 9, CORRIDOR_Z - 1, C.stone, Enum.Material.Concrete)
		local slot = box(F.Fixtures, `RelicSlot_{i}`, x - 5, x + 5, FLOOR_Y + 13, FLOOR_Y + 13.6, CORRIDOR_Z - 8, CORRIDOR_Z - 2, C.brass, Enum.Material.Metal)
		slot.CanCollide = false
		slot:SetAttribute("Slot", i)
		CollectionService:AddTag(slot, "RelicPlinth")
		local plate = box(F.Fixtures, `Plaque_{i}`, x - 6, x + 6, FLOOR_Y + 7, FLOOR_Y + 11, CORRIDOR_Z - 9.8, CORRIDOR_Z - 9.2, C.brass, Enum.Material.Metal)
		label(plate, "UNNAMED", Enum.NormalId.Front, 22)
	end

	-- The case that will never be filled. The order's earliest records burned
	-- long before any flood, so nothing brought back from a run can go in here.
	-- Building the empty case on purpose is the point: the oldest things in the
	-- room are the ones nobody can name, and they stay that way.
	box(F.Fixtures, "BurntCase", -126, -114, FLOOR_Y, FLOOR_Y + 3, KEEP_Z + 6, KEEP_Z + 28, C.stone, Enum.Material.Concrete)
	box(F.Fixtures, "BurntCaseGlass", -126, -114, FLOOR_Y + 3, FLOOR_Y + 20, KEEP_Z + 6, KEEP_Z + 28, C.glass, Enum.Material.Glass, 0.72)
	local burnt = box(F.Fixtures, "BurntCasePlaque", -113.8, -113.2, FLOOR_Y + 9, FLOOR_Y + 15, KEEP_Z + 8, KEEP_Z + 26, C.timber, Enum.Material.WoodPlanks)
	label(burnt, "THE YEARS BEFORE THE FIRE — NOTHING RECOVERED", Enum.NormalId.Right, 20)

	-- Past the side room it stops being finished. A corridor with no lamps in
	-- it, and a door at the end that does not open.
	wall(F.SideRoom, "CorridorWall", "Z", CORRIDOR_Z, -W, SIDE_X, FLOOR_Y, GREAT_H, C.brick, { { -100, -74 } })
	box(F.SideRoom, "CorridorCeiling", -W, SIDE_X, LANDING_H - 2, LANDING_H, CORRIDOR_Z, HALL_Z1, C.dark, Enum.Material.WoodPlanks)
	local shut = box(F.SideRoom, "ShutDoor", -100, -74, FLOOR_Y, FLOOR_Y + DOOR_H, HALL_Z1 - 3.2, HALL_Z1 - 2, C.timber, Enum.Material.WoodPlanks)
	label(shut, "NO LAMPS BEYOND THIS DOOR", Enum.NormalId.Front, 24)
	for i = 0, 2 do
		lamp(F.SideRoom, `DeadCorridorLamp_{i}`, -85, LANDING_H - 12, CORRIDOR_Z + 12 + i * 16, false)
	end

	-- ---------------------------------------------------------------- lamp wall
	-- One lantern per Lightkeeper out. The gameplay side binds a lamp to a
	-- player and puts it out when they wipe; the map's job is to provide the
	-- slots and make them legible, so each is tagged and numbered.
	box(F.LampWall, "LampWallFace", W - WALL_T - 3, W - WALL_T, FLOOR_Y, FLOOR_Y + 30, 8, 72, C.timber, Enum.Material.WoodPlanks)
	local plaque = box(F.LampWall, "LampWallPlaque", W - WALL_T - 4, W - WALL_T - 3.2, FLOOR_Y + 31, FLOOR_Y + 40, 20, 60, C.timber, Enum.Material.WoodPlanks)
	label(plaque, "KEEPERS OUT", Enum.NormalId.Left, 34)
	for i = 0, 7 do
		local z = 12 + i * 8
		-- Lit by default so the hall reads as inhabited on a first arrival;
		-- whoever wires this owns which are actually burning.
		local g = lamp(F.LampWall, `KeeperLamp_{i}`, W - WALL_T - 8, FLOOR_Y + 16, z, i % 3 ~= 2)
		g:SetAttribute("Slot", i)
		CollectionService:AddTag(g, "LampWallSlot")
	end

	-- The Good Cheer Club roll: the hub's contribution ledger, by name. The
	-- handoff requires landmark contributions to be credited and impossible to
	-- undo or monopolise, and a club roll is that requirement as furniture.
	local roll = box(F.Fixtures, "ClubBoard", -W + WALL_T + 3.2, -W + WALL_T + 4, FLOOR_Y + 10, FLOOR_Y + 34, 16, 64, C.timber, Enum.Material.WoodPlanks)
	label(roll, "THE GOOD CHEER CLUB", Enum.NormalId.Right, 34)
	CollectionService:AddTag(roll, "ClubBoard")

	-- ---------------------------------------------------------------- markers
	-- Everything the gameplay side needs to find, tagged rather than guessed at
	-- by position. None of these are wired here: this file is the map.
	local departure = box(F.Markers, "Departure", -22, 22, FLOOR_Y, FLOOR_Y + 0.6, -30, -6, C.brass, Enum.Material.Metal)
	departure.CanCollide = false -- a slab this thin reads as an obstacle, not ground
	label(departure, "TO THE ROAD", Enum.NormalId.Top, 30)
	CollectionService:AddTag(departure, "Departure")

	local ret = box(F.Markers, "ReturnPoint", -22, 22, FLOOR_Y, FLOOR_Y + 0.6, 14, 38, C.brass, Enum.Material.Metal)
	ret.CanCollide = false
	CollectionService:AddTag(ret, "ReturnPoint")

	-- The workshop corner, reserved and empty. Per-player and persistent when
	-- it is built; for now it is a marked floor so the space is not taken.
	local shop = box(F.Markers, "WorkshopPlot", -124, -76, FLOOR_Y, FLOOR_Y + 0.6, 10, 58, C.cloth, Enum.Material.Fabric)
	shop.CanCollide = false
	label(shop, "WORKSHOP", Enum.NormalId.Top, 30)
	CollectionService:AddTag(shop, "WorkshopPlot")

	-- The workshop corner. Per-player and persistent when it is wired; the map
	-- furnishes it and marks it. It sits in the Landing rather than off in a
	-- private room on purpose - the handoff sells belonging and identity, and
	-- neither works if nobody walks past your bench.
	box(F.Fixtures, "Bench", -124, -84, FLOOR_Y + 11, FLOOR_Y + 13, 16, 30, C.timber, Enum.Material.WoodPlanks)
	for _, x in ipairs({ -120, -88 }) do
		part(F.Fixtures, `BenchLeg_{x}`, Vector3.new(4, 11, 4), CFrame.new(x, FLOOR_Y + 5.5, 23), C.timber, Enum.Material.WoodPlanks)
	end
	local vice = box(F.Fixtures, "Vice", -92, -86, FLOOR_Y + 13, FLOOR_Y + 18, 20, 26, C.brass, Enum.Material.Metal)
	vice:SetAttribute("Station", "bench")
	CollectionService:AddTag(vice, "WorkshopBench")
	-- A rack of lantern glass, which is what this corner is for.
	box(F.Fixtures, "LensRack", -124, -100, FLOOR_Y + 18, FLOOR_Y + 32, 14, 16, C.timber, Enum.Material.WoodPlanks)
	for i = 0, 3 do
		part(F.Fixtures, `Lens_{i}`, Vector3.new(5, 5, 1.5), CFrame.new(-120 + i * 6, FLOOR_Y + 25, 15), C.glass, Enum.Material.Glass, 0.45)
	end
	part(F.Fixtures, "Stool", Vector3.new(7, 9, 7), CFrame.new(-104, FLOOR_Y + 4.5, 36), C.timber, Enum.Material.WoodPlanks)

	-- Light in the Landing itself, so arriving is warm. Four lamps, shadows off.
	for i = -1, 1, 2 do
		for j = 0, 1 do
			lamp(F.Landing, `LandingLamp_{i}_{j}`, i * 62, LANDING_H - 11, 22 + j * 34, true)
		end
	end

	M.Parent = Workspace
	return M
end

return LanternholdBuilder
