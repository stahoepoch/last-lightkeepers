--!strict
-- The Johnstown Galleria, greybox, built from the mall's own floor plan.
--
-- Not a guess and not a generic mall. The mall publishes an interactive map at
-- johnstowngalleria.com/map/, which turns out to be a Verge3D export - a real
-- 3D model of the building. The suite table below is that model's geometry:
-- every tenant unit's centre and footprint, on the correct level, read out of
-- `galleria mall 2.gltf` and scaled into studs.
--
-- So the shape here is the building's shape. Worth stating plainly, because the
-- first version of this file was a cruciform with four corner anchors, which is
-- what you would reasonably assume from "1992 mall, four anchors at the
-- corners" - and it is wrong. The real Galleria is a long east-west concourse
-- with Boscov's closing the west end, JCPenney hanging off a short south spur,
-- and the east end bending north-east toward the far anchor pad, which a casino
-- occupies today.
--
-- Model units are metres-ish; SCALE converts to studs. At SCALE 26 the mall is
-- about 700 x 510 studs, which is large enough that one run cannot cover it.
--
-- Run from the command bar:
--   require(workspace.Map.GalleriaBuilder).Build()
-- It builds Workspace.Map.ClaudeGalleria and touches nothing else, so it can
-- sit beside FloodedMallBuilder's version for comparison.

local Workspace = game:GetService("Workspace")

local GalleriaBuilder = {}

local MAP_NAME = "ClaudeGalleria"

local SCALE = 26
local FLOOR_TO_FLOOR = 20
local L1_Y = 0
local L2_Y = L1_Y + FLOOR_TO_FLOOR
local SLAB = 2
local STORE_H = FLOOR_TO_FLOOR - 4

-- floor.l1: centre 2.4,-6.4 size 27 x 19.6
-- floor.l2: centre 2.3,-4.4 size 26.8 x 15.1
local FLOORS = {
	{ level = 1, x = 2.4, z = -6.4, w = 27.0, d = 19.6 },
	{ level = 2, x = 2.3, z = -4.4, w = 26.8, d = 15.1 },
}

local Colors = {
	floor = Color3.fromRGB(198, 192, 180),
	floorAlt = Color3.fromRGB(172, 165, 153),
	teal = Color3.fromRGB(32, 107, 112), -- the Galleria's framing colour
	glass = Color3.fromRGB(150, 196, 210),
	storefront = Color3.fromRGB(56, 58, 64),
	vacant = Color3.fromRGB(88, 86, 84),
	rail = Color3.fromRGB(196, 198, 202),
	shell = Color3.fromRGB(150, 146, 138),
	trim = Color3.fromRGB(118, 114, 108),
	ceiling = Color3.fromRGB(44, 46, 50),
}

-- The concourse spine, straight from the mall's model: the units sit in two
-- bands either side of it.
local CORRIDOR_Z = -4.4 * SCALE

-- Every tenant unit, from the mall's own model.
local SUITES = {
	{ "570", -15.4, -4.9, 8.6, 10.6, 1 }, -- Boscov's, the west anchor
	{ "164", -10.5, -8.0, 0.9, 0.9, 1 },
	{ "160", -10.5, -6.6, 0.9, 1.8, 1 },
	{ "172", -9.4, -2.8, 3.1, 3.0, 1 },
	{ "156", -9.0, -5.9, 0.4, 1.4, 1 },
	{ "154", -8.1, -5.9, 1.2, 1.4, 1 },
	{ "158", -7.9, -7.6, 2.6, 3.4, 1 },
	{ "150", -6.6, -7.9, 0.7, 1.3, 1 },
	{ "178", -5.3, -3.0, 5.2, 3.3, 1 },
	{ "148", -4.4, -6.3, 1.6, 1.7, 1 },
	{ "147", -4.2, -5.8, 1.2, 1.1, 1 },
	{ "146", -3.1, -6.2, 0.9, 2.0, 1 },
	{ "145", -2.2, -6.3, 0.9, 2.2, 1 },
	{ "179", -1.8, -2.8, 1.6, 3.0, 1 },
	{ "143", -1.2, -5.8, 0.8, 1.2, 1 },
	{ "141B", -1.2, -7.2, 0.8, 1.4, 1 },
	{ "180", -0.3, -2.8, 0.9, 3.0, 1 },
	{ "141A", -0.3, -7.0, 0.9, 1.1, 1 },
	{ "142", -0.3, -5.8, 0.9, 1.2, 1 },
	{ "181", 0.7, -2.8, 1.1, 3.0, 1 },
	{ "141", 0.7, -6.6, 0.9, 0.9, 1 },
	{ "140", 0.7, -5.7, 0.9, 0.9, 1 },
	{ "136", 1.7, -8.8, 3.4, 3.0, 1 },
	{ "182", 1.7, -3.0, 0.6, 2.7, 1 },
	{ "182A", 2.5, -2.8, 0.8, 3.0, 1 },
	{ "183", 3.4, -2.8, 0.9, 3.0, 1 },
	{ "130", 3.4, -8.2, 0.5, 3.0, 1 },
	{ "128", 4.4, -7.7, 1.1, 3.0, 1 },
	{ "184", 4.9, -2.8, 1.6, 2.3, 1 },
	{ "186", 5.3, -2.2, 1.2, 3.0, 1 },
	{ "126", 5.5, -7.0, 1.4, 3.0, 1 },
	{ "190", 5.7, -0.9, 0.8, 1.2, 1 },
	{ "189", 6.3, -1.9, 0.9, 0.9, 1 },
	{ "124", 6.6, -6.1, 1.8, 3.0, 1 },
	{ "188", 6.9, -2.7, 0.9, 0.7, 1 },
	{ "123", 7.2, -4.5, 0.6, 0.8, 1 },
	{ "122", 8.2, -5.3, 1.5, 2.6, 1 },
	{ "120", 8.8, -6.0, 1.2, 0.8, 1 },
	{ "196", 9.2, 0.3, 2.8, 3.0, 1 },
	{ "195", 10.9, 1.4, 0.8, 3.0, 1 },
	{ "116", 11.1, -3.5, 0.9, 2.6, 1 },
	{ "197", 11.6, 1.8, 0.7, 1.6, 1 },
	{ "114", 11.9, -2.7, 1.7, 3.1, 1 },
	{ "198", 12.5, 2.2, 1.1, 1.6, 1 },
	{ "112", 12.6, -2.0, 1.0, 3.1, 1 },
	{ "100", 14.3, -0.6, 1.1, 1.7, 1 },

	{ "260", -10.6, -2.9, 0.9, 1.3, 2 },
	{ "254", -10.3, -7.0, 0.9, 2.6, 2 },
	{ "261", -9.5, -3.7, 0.9, 0.6, 2 },
	{ "262", -9.0, -3.4, 1.0, 1.0, 2 },
	{ "252", -8.7, -8.1, 1.8, 3.5, 2 },
	{ "264", -8.3, -2.7, 1.4, 1.4, 2 },
	{ "250", -8.0, -7.9, 2.2, 3.5, 2 },
	{ "265", -7.7, -2.1, 1.0, 1.0, 2 },
	{ "266", -7.4, -1.4, 1.0, 1.0, 2 },
	{ "K1", -6.3, -3.6, 0.5, 0.6, 2 },
	{ "270", -5.3, -1.4, 1.0, 1.0, 2 },
	{ "271", -5.0, -2.1, 1.0, 1.0, 2 },
	{ "272", -4.6, -2.5, 1.1, 1.1, 2 },
	{ "249", -4.2, -6.2, 1.1, 1.6, 2 },
	{ "273", -4.1, -3.0, 1.0, 1.0, 2 },
	{ "248", -3.9, -6.3, 0.6, 1.4, 2 },
	{ "274", -3.6, -3.4, 0.6, 1.0, 2 },
	{ "K4", -3.5, -4.5, 0.3, 0.3, 2 },
	{ "246", -3.1, -6.4, 0.9, 1.6, 2 },
	{ "275", -2.9, -2.3, 0.9, 2.8, 2 },
	{ "276", -2.2, -2.2, 1.5, 2.8, 2 },
	{ "244", -2.2, -6.5, 0.9, 1.7, 2 },
	{ "243", -1.3, -7.3, 1.3, 0.7, 2 },
	{ "242", -0.9, -7.1, 1.8, 1.7, 2 },
	{ "277", -0.8, -2.6, 0.7, 2.6, 2 },
	{ "278A", 0.0, -2.6, 0.7, 2.6, 2 },
	{ "235", 0.5, -9.9, 1.6, 1.9, 2 },
	{ "278B", 0.7, -2.6, 0.7, 2.6, 2 },
	{ "240", 0.7, -6.0, 1.0, 1.3, 2 },
	{ "K3", 1.0, -5.1, 0.3, 0.9, 2 },
	{ "279", 1.6, -2.5, 0.7, 2.3, 2 },
	{ "230", 2.1, -9.4, 3.1, 3.0, 2 },
	{ "281", 2.4, -3.7, 0.5, 0.4, 2 },
	{ "280", 3.1, -2.9, 2.2, 2.7, 2 },
	{ "224", 3.7, -7.9, 2.5, 3.0, 2 },
	{ "282", 4.2, -2.3, 1.1, 1.9, 2 },
	{ "220", 5.2, -7.4, 2.3, 2.8, 2 },
	{ "283", 5.4, -1.9, 2.4, 2.8, 2 },
	{ "K2", 6.1, -4.5, 0.3, 0.3, 2 },
	{ "218", 6.2, -6.7, 2.4, 2.8, 2 },
	{ "284", 6.4, -1.8, 2.0, 2.6, 2 },
	{ "216", 7.0, -6.2, 2.1, 2.6, 2 },
	{ "214", 8.0, -4.9, 3.0, 2.9, 2 },
	{ "287", 8.8, -0.3, 3.0, 2.9, 2 },
	{ "289", 10.7, 1.1, 2.3, 3.1, 2 },
	{ "290", 11.1, 0.8, 1.1, 1.2, 2 },
	{ "206", 11.1, -2.6, 1.5, 1.2, 2 },
	{ "210", 11.2, -3.1, 2.6, 2.4, 2 },
	{ "292", 11.8, 1.6, 1.5, 1.6, 2 },
	{ "294", 12.1, 2.4, 1.6, 1.8, 2 },
	{ "200", 14.5, -2.0, 4.2, 4.1, 2 },
}

-- Real tenants, from the mall's directory. Flavour, but the right flavour.
local TENANTS: { [string]: string } = {
	["570"] = "BOSCOV'S",
	["112"] = "AAA", ["116"] = "BAAA SAAS FARM", ["130"] = "NAVY RECRUITING",
	["136"] = "MISTER JAY'S", ["140"] = "BEAUFETCH", ["141"] = "JK LAW",
	["142"] = "AMALFI TOURS", ["143"] = "GORGEOUS BY GOD", ["145"] = "AXOLOT A RAGE",
	["146"] = "GNC", ["147"] = "YUM NATURALS", ["148"] = "STATTECH",
	["154"] = "FOREVER NAILS", ["156"] = "ICE CREAM & BAKERY", ["158"] = "ISLAND CUISINE",
	["160"] = "APPLES AND BANANAS", ["172"] = "GLOW ROLLER RINK", ["180"] = "J&J COLLECTIBLES",
	["181"] = "AT&T", ["182"] = "CAMILLE'S", ["182A"] = "ROCK & ROLL STORE",
	["184"] = "CAT CLINIC", ["186"] = "TOYS FOR TOTS", ["188"] = "AUNTIE ANNE'S",
	["189"] = "AAUW BOOKS", ["196"] = "RARE COLLECTIBLES",
	["200"] = "A.O.N. WRESTLING", ["206"] = "WAYPOINT ATHLETICS", ["210"] = "ULTRASNAPS",
	["216"] = "COMMUNITY ROOM", ["218"] = "JOHNSTOWN FASHION", ["224"] = "ITALIAN OVEN",
	["230"] = "JAKE'S PUB & GRILL", ["235"] = "SENTINEL'S KEEP", ["240"] = "D.C. GOLDSMITHING",
	["242"] = "AXOLOT A LOVE", ["244"] = "MORNING STAR MASSAGE", ["246"] = "THE PILLAGE",
	["248"] = "STEVE'S BEES", ["249"] = "T-MOBILE", ["252"] = "MAURICES",
	["254"] = "ARMED FORCES", ["260"] = "ARCADE OF THRONES", ["262"] = "ANNELESSE'S",
	["264"] = "OPA! GREEK", ["266"] = "RAYNE'S BBQ", ["270"] = "NAPOLI PIZZA",
	["271"] = "SOUTHERN CITY", ["272"] = "ROSALINDA'S", ["274"] = "DEN OF SKILLZ",
	["275"] = "GALLERIA INK", ["277"] = "GARDNERS CANDIES", ["279"] = "DARK BREWS",
	["280"] = "PITT JOHNSTOWN", ["281"] = "SELF PAY MARKET", ["282"] = "MATTRESS",
	["289"] = "SWEATRX", ["290"] = "ULTRASNAPS PODCAST", ["292"] = "DB RECORDING",
	["294"] = "VETERAN'S",
	["K1"] = "AUNTIE ANNE'S", ["K2"] = "KIOSK", ["K3"] = "KIOSK", ["K4"] = "KIOSK",
}

-- Vertical circulation and doors, also from the model.
local STAIRS = {
	{ "Stair_Center", 1.9, -1.3 },
	{ "Stair_Court", 1.5, -11.2 },
	{ "Stair_West", -10.5, -1.0 },
	{ "Stair_Southwest", -8.0, -9.6 },
	{ "Stair_East", 15.3, -1.9 },
	{ "Stair_Northeast", 11.9, 3.3 },
}
local ENTRANCES = {
	{ "Entrance_A", -6.4, -0.9 },
	{ "Entrance_A2", -9.6, -8.9 },
	{ "Entrance_D", 4.0, -10.2 },
	{ "Entrance_D2", 14.8, -3.1 },
}

local function sx(u: number): number
	return u * SCALE
end

local function part(parent: Instance, name: string, size: Vector3, cf: CFrame, colour: Color3, material: Enum.Material?): Part
	local p = Instance.new("Part")
	p.Name = name
	p.Size = size
	p.CFrame = cf
	p.Color = colour
	p.Material = material or Enum.Material.Concrete
	p.Anchored = true
	p.TopSurface = Enum.SurfaceType.Smooth
	p.BottomSurface = Enum.SurfaceType.Smooth
	p.Parent = parent
	return p
end

--- A walking surface placed by its TOP face, because that is the number that
--- decides whether a player can step onto it.
local function floorSlab(parent: Instance, name: string, w: number, d: number, x: number, z: number, topY: number, colour: Color3): Part
	return part(parent, name, Vector3.new(w, SLAB, d), CFrame.new(x, topY - SLAB / 2, z), colour, Enum.Material.Pebble)
end

local function sign(parent: Instance, host: BasePart, text: string, faces: { Enum.NormalId })
	for _, face in ipairs(faces) do
		local gui = Instance.new("SurfaceGui")
		gui.Face = face
		gui.CanvasSize = Vector2.new(480, 110)
		gui.Parent = host
		local label = Instance.new("TextLabel")
		label.Size = UDim2.fromScale(1, 1)
		label.BackgroundTransparency = 1
		label.Font = Enum.Font.GothamBold
		label.TextScaled = true
		label.TextColor3 = Color3.fromRGB(238, 242, 246)
		label.Text = text
		label.Parent = gui
	end
end

--- Stairs the navmesh will climb. Treads for looks, one invisible ramp over
--- them so Roblox's pathfinder sees a continuous slope - a stack of boxes on
--- its own returns NoPath, which is what left the Sweepers stuck downstairs in
--- the other mall.
local function staircase(parent: Instance, name: string, x: number, z: number)
	local folder = Instance.new("Folder")
	folder.Name = name
	folder.Parent = parent

	local run, width, steps = 70, 22, 14
	local rise = FLOOR_TO_FLOOR / steps
	for i = 1, steps do
		local t = i / steps
		part(
			folder,
			`{name}_Step{i}`,
			Vector3.new(width, 1.2, run / steps + 0.4),
			CFrame.new(x, L1_Y + rise * t - 0.6, z - run / 2 + (run / steps) * (i - 0.5)),
			Colors.floorAlt,
			Enum.Material.Pebble
		)
	end
	local ramp = part(
		folder,
		`{name}_Ramp`,
		Vector3.new(width, 1, math.sqrt(run * run + FLOOR_TO_FLOOR * FLOOR_TO_FLOOR)),
		CFrame.new(x, L1_Y + FLOOR_TO_FLOOR / 2, z) * CFrame.Angles(math.atan2(FLOOR_TO_FLOOR, run), 0, 0),
		Colors.floorAlt
	)
	ramp.Transparency = 1

	-- A matching opening guard rail at the top.
	part(folder, `{name}_Rail`, Vector3.new(width + 4, 4, 1), CFrame.new(x, L2_Y + 2, z + run / 2 + 1), Colors.rail, Enum.Material.Metal)
end

function GalleriaBuilder.Build(): Model
	local map = Workspace:FindFirstChild("Map") or Workspace
	local old = map:FindFirstChild(MAP_NAME)
	if old then
		old:Destroy()
	end

	local model = Instance.new("Model")
	model.Name = MAP_NAME

	-- Floor plates, one per level, at their real extents.
	local plates = Instance.new("Folder")
	plates.Name = "Floors"
	plates.Parent = model
	for _, f in ipairs(FLOORS) do
		local y = if f.level == 1 then L1_Y else L2_Y
		floorSlab(plates, `Floor_L{f.level}`, sx(f.w), sx(f.d), sx(f.x), sx(f.z), y, Colors.floor)
	end

	-- Perimeter shell around the ground plate.
	local g = FLOORS[1]
	local shell = Instance.new("Folder")
	shell.Name = "Shell"
	shell.Parent = model
	local h = FLOOR_TO_FLOOR * 2 + 8
	local hw, hd = sx(g.w) / 2, sx(g.d) / 2
	part(shell, "WallNorth", Vector3.new(sx(g.w) + 6, h, 4), CFrame.new(sx(g.x), h / 2, sx(g.z) - hd), Colors.shell)
	part(shell, "WallSouth", Vector3.new(sx(g.w) + 6, h, 4), CFrame.new(sx(g.x), h / 2, sx(g.z) + hd), Colors.shell)
	part(shell, "WallWest", Vector3.new(4, h, sx(g.d) + 6), CFrame.new(sx(g.x) - hw, h / 2, sx(g.z)), Colors.shell)
	part(shell, "WallEast", Vector3.new(4, h, sx(g.d) + 6), CFrame.new(sx(g.x) + hw, h / 2, sx(g.z)), Colors.shell)

	-- Tenant units.
	local units = Instance.new("Folder")
	units.Name = "Suites"
	units.Parent = model
	for _, s in ipairs(SUITES) do
		local id, ux, uz, uw, ud, level = s[1] :: string, s[2] :: number, s[3] :: number, s[4] :: number, s[5] :: number, s[6] :: number
		local y = if level == 1 then L1_Y else L2_Y
		local name = TENANTS[id]
		local box = part(
			units,
			`Suite_{id}`,
			Vector3.new(sx(uw), STORE_H, sx(ud)),
			CFrame.new(sx(ux), y + STORE_H / 2, sx(uz)),
			if name then Color3.fromRGB(52, 54, 60) else Colors.vacant,
			Enum.Material.Slate
		)
		box:SetAttribute("Suite", id)
		box:SetAttribute("Level", level)

		-- The teal fascia goes on the SHOPFRONT, not over the unit. Capping each
		-- box with a teal lid turns the whole plan into teal blobs from above;
		-- a band on the face that meets the concourse is what the Galleria
		-- actually looks like, and it tells you which way a unit opens.
		--
		-- The mall is two bands of units either side of one spine, so a unit
		-- faces the spine. That is a heuristic, not a fact from the model, and
		-- it is the thing most likely to be wrong on an oddly-placed unit.
		local facing = if sx(uz) < CORRIDOR_Z then 1 else -1
		local fasciaDepth = 2
		part(
			units,
			`Header_{id}`,
			Vector3.new(sx(uw), 3.5, fasciaDepth),
			CFrame.new(sx(ux), y + STORE_H - 1.75, sx(uz) + facing * (sx(ud) / 2 + fasciaDepth / 2 - 0.2)),
			Colors.teal,
			Enum.Material.SmoothPlastic
		)
		-- A ceiling, so the unit reads as a room rather than a solid block.
		part(
			units,
			`Cap_{id}`,
			Vector3.new(sx(uw), 0.6, sx(ud)),
			CFrame.new(sx(ux), y + STORE_H + 0.3, sx(uz)),
			Colors.ceiling,
			Enum.Material.Concrete
		)
		sign(units, box, name or id, { if facing == 1 then Enum.NormalId.Front else Enum.NormalId.Back })
	end

	-- Anchors the model marks with logos rather than suites.
	local anchors = Instance.new("Folder")
	anchors.Name = "Anchors"
	anchors.Parent = model
	local function anchorBox(label: string, ux: number, uz: number, uw: number, ud: number)
		local box = part(
			anchors,
			`Anchor_{label}`,
			Vector3.new(sx(uw), FLOOR_TO_FLOOR * 2, sx(ud)),
			CFrame.new(sx(ux), FLOOR_TO_FLOOR, sx(uz)),
			Colors.shell,
			Enum.Material.Concrete
		)
		local band = part(
			anchors,
			`AnchorSign_{label}`,
			Vector3.new(sx(uw) * 0.7, 14, 2),
			CFrame.new(sx(ux), FLOOR_TO_FLOOR * 2 - 10, sx(uz) + sx(ud) / 2 + 1),
			Colors.teal,
			Enum.Material.SmoothPlastic
		)
		sign(anchors, band, label, { Enum.NormalId.Front, Enum.NormalId.Back })
		return box
	end
	anchorBox("JCPENNEY", -2.9, -13.5, 9.0, 4.5)
	anchorBox("CASINO", 16.9, 3.1, 10.0, 10.0)

	-- Vertical circulation.
	local circ = Instance.new("Folder")
	circ.Name = "Circulation"
	circ.Parent = model
	for _, s in ipairs(STAIRS) do
		staircase(circ, s[1] :: string, sx(s[2] :: number), sx(s[3] :: number))
	end
	local lift = part(circ, "Elevator", Vector3.new(26, FLOOR_TO_FLOOR * 2, 26), CFrame.new(sx(2.0), FLOOR_TO_FLOOR, sx(-5.1)), Colors.glass, Enum.Material.Glass)
	lift.Transparency = 0.5

	for _, e in ipairs(ENTRANCES) do
		part(circ, e[1] :: string, Vector3.new(34, 24, 6), CFrame.new(sx(e[2] :: number), 12, sx(e[3] :: number)), Colors.glass, Enum.Material.Glass).Transparency =
			0.45
	end

	-- The gabled skylight over the concourse spine. The pitched glass roof is
	-- the silhouette the building is actually known for.
	local roof = Instance.new("Folder")
	roof.Name = "Skylight"
	roof.Parent = model
	local roofY = L2_Y + FLOOR_TO_FLOOR
	local spineZ = sx(-4.4)
	local spineW = sx(26.8)
	for _, s in ipairs({ -1, 1 }) do
		local pane = part(
			roof,
			"SkylightPane",
			Vector3.new(spineW, 1.5, 150),
			CFrame.new(0, roofY + 16, spineZ + s * 60) * CFrame.Angles(math.rad(s * -22), 0, 0),
			Colors.glass,
			Enum.Material.Glass
		)
		pane.Transparency = 0.6
	end
	part(roof, "RidgeBeam", Vector3.new(spineW, 3, 3), CFrame.new(0, roofY + 46, spineZ), Colors.teal, Enum.Material.Metal)

	model.Parent = map
	return model
end

return GalleriaBuilder
