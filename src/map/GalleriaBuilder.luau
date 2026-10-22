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
local RaidDirector = require(script.Parent:WaitForChild("RaidDirector"))

local GalleriaBuilder = {}

-- The first pass used 26 studs per plan unit and literal department-store
-- footprints. In play that made a 1,983 by 1,344-stud arena: a teammate could
-- call an exit from the far end and everyone else simply had no chance to get
-- there. This is a raid space, not a survey drawing. Twenty studs preserves
-- the recognizable dogleg and two-storey atrium while making a cross-mall push
-- a decision rather than dead travel.
local S = 20

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
	-- Was 232,228,217. A near-white floor is right for a lit shopping mall and
	-- wrong for this one: with GlobalShadows back on it takes the full sky and
	-- blows out, which is most of what "the lighting is terrible" was. This is
	-- the same terrazzo gone damp and unpolished, and it gives the lamps
	-- something to read against. One constant to put back if it reads too dark.
	tile = Color3.fromRGB(166, 161, 150),
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
-- The dogleg needs enough room for a dangerous deep push, but not a literal
-- one-to-one copy of a twelve-hundred-foot building. The prior arms plus
-- anchor-store footprints made the walk to a called extraction longer than its
-- arrival window. These retain the long west arm and angled south-east arm at
-- compact, playable scale.
local ARMS = {
	{ a = BEND, b = Vector2.new(-21, 0) }, -- west, to Boscov's
	{ a = BEND, b = Vector2.new(15, 15) }, -- south-east, to Sears
}

-- Invented tenants. Chris: "we should change the names of almost all the
-- stores so it's not copyright or whatever" - right call, and it applies to the
-- anchors hardest, because a department store name is the most protected thing
-- in a mall. The building is what makes this the Galleria; the signage does not
-- have to be, and a greybox is the cheap moment to get this right.
local L1_NAMES = {
	"VITA-MAX NUTRITION",
	"PAGE & HEARTH BOOKS",
	"STRIDE ATHLETIC",
	"HALLOWELL JEWELERS",
	"ODDITY",
	"SOLE & CENT SHOES",
	"GLIMMER ACCESSORIES",
	"SIGNAL & WIRE",
	"TWIST PRETZEL CO.",
	"WAXWORKS MUSIC",
	"GLOW ROLLER RINK",
	"ISLAND PLATE",
	"MISTER JUNE'S",
	"TRANSISTOR ALLEY",
	"D. BARROW BOOKSELLERS",
	"KEEPSAKE & CO.",
}
local L2_NAMES = {
	"THE STONE OVEN",
	"JACOBY'S TAP",
	"CELLULAR POINT",
	"MERIDIAN APPAREL",
	"OLYMPIA GYROS",
	"VESUVIO PIZZA",
	"ORCHARD CONFECTIONS",
	"DARK BREWS",
	"VALLEY COLLEGE SHOP",
	"ARCADE OF THRONES",
	"ROSALINDA'S",
	"SENTINEL'S KEEP",
	"VALLEY INK",
	"A.O.N. WRESTLING",
	"SWEATRX",
	"MORNING STAR",
}

-- Scope contract: the mall has 128 inline storefront shells, but only three
-- per concourse face and level are true interiors. That makes 24 inline raid
-- spaces, plus the eight floors across the four department stores. The rest
-- remain shuttered frontage: a big mall needs scale, not 100 empty rooms.
local PLAYABLE_BAY_INDICES = { [2] = true, [8] = true, [13] = true }

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

--- Pattern on a floor is paint, not a kerb. These strips sat 0.3 studs proud
--- of the concourse and collidable, so walking the mall meant stepping up and
--- down over every band in it - Chris: "floor looked uneven too". It is the
--- same mistake the extraction pads made: anything laid on a floor has to stop
--- being geometry. Sunk flush and left non-colliding, with a sliver showing so
--- it does not z-fight the slab it lies on.
local function paint(p: Part): Part
	p.CanCollide = false
	p.CanTouch, p.CanQuery = false, false
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
	-- Deliberately NOT inside Workspace.Map. Rojo owns that source-module
	-- subtree and replaces its children during sync; runtime geometry belongs
	-- beside it so a sync cannot delete the active mall.
	local map = Workspace
	local old = map:FindFirstChild("ClaudeGalleria")
	if old then
		old:Destroy()
	end

	local M = Instance.new("Model")
	M.Name = "ClaudeGalleria"
	local F: { [string]: Folder } = {}
	for _, n in ipairs({
		"Floors",
		"Shops",
		"Structure",
		"Anchors",
		"Roof",
		"Court",
		"Escalators",
		"Fixtures",
		"Gameplay",
	}) do
		local f = Instance.new("Folder")
		f.Name, f.Parent = n, M
		F[n] = f
	end

	-- A flat disc, for the court and the fountain. A Part cylinder is extruded
	-- along its own X, so it has to be stood on end to lie down.
	local function disc(
		parent: Instance,
		name: string,
		radius: number,
		y: number,
		h: number,
		colour: Color3,
		mat: Enum.Material?,
		trans: number?
	): Part
		local p = part(
			parent,
			name,
			Vector3.new(h, radius * 2 * S, radius * 2 * S),
			CFrame.new(0, y, 0) * CFrame.Angles(0, 0, math.pi / 2),
			colour,
			mat,
			trans
		)
		p.Shape = Enum.PartType.Cylinder
		return p
	end

	-- The columns are round and salmon. With the teal trusses that is most of
	-- what makes a photograph of this place recognisable; square posts read as
	-- any mall anywhere. Takes world studs, not model units.
	local function pillar(
		folder: Instance,
		name: string,
		wx: number,
		wz: number,
		y: number,
		h: number,
		dia: number,
		colour: Color3
	): Part
		local p = part(
			folder,
			name,
			Vector3.new(h, dia, dia),
			CFrame.new(wx, y, wz) * CFrame.Angles(0, 0, math.pi / 2),
			colour,
			Enum.Material.Concrete
		)
		p.Shape = Enum.PartType.Cylinder
		return p
	end

	-- Escalators are one sloped deck with treads drawn on it, not a stack of
	-- steps. Discrete step boxes do not produce a navmesh a Sweeper can path
	-- over - that was learned on the other mall's stairs, where
	-- PathfindingService returned NoPath at every attempt.
	-- A dressed figure in a shop window. Half set dressing, half foreshadowing:
	-- the Window Walkers were display mannequins, so a mall with none in its
	-- windows is missing both its life and its threat.
	local function mannequin(
		folder: Instance,
		name: string,
		wx: number,
		wz: number,
		yaw: number,
		y: number
	)
		local rot = CFrame.Angles(0, -yaw, 0)
		part(
			folder,
			`{name}_Base`,
			Vector3.new(3.4, 0.5, 3.4),
			CFrame.new(wx, y + 0.25, wz) * rot,
			C.shop,
			Enum.Material.Metal
		)
		part(
			folder,
			`{name}_Leg`,
			Vector3.new(1, 2.2, 1),
			CFrame.new(wx, y + 1.5, wz) * rot,
			C.fascia,
			Enum.Material.SmoothPlastic
		)
		part(
			folder,
			`{name}_Body`,
			Vector3.new(2.4, 2.6, 1.6),
			CFrame.new(wx, y + 3.9, wz) * rot,
			C.fascia,
			Enum.Material.SmoothPlastic
		)
		local head = part(
			folder,
			`{name}_Head`,
			Vector3.new(1.3, 1.3, 1.3),
			CFrame.new(wx, y + 5.8, wz) * rot,
			C.fascia,
			Enum.Material.SmoothPlastic
		)
		head.Shape = Enum.PartType.Ball
	end

	local ESC_W = 14
	-- (x1, z1) is the foot and (x2, z2) is the head. The yaw is taken from that
	-- vector rather than passed in, because the deck's pitch always lifts its
	-- own +X end: given the arm's yaw instead, a run asked to climb back down
	-- the arm was built climbing the other way, and its landing ended up at the
	-- foot, hanging a storey above the floor with the deck rising away from it.
	-- That is why nothing on level two could be reached after the rescale.
	local function escalator(
		folder: Instance,
		name: string,
		x1: number,
		z1: number,
		x2: number,
		z2: number,
		base: number
	)
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
			-- Flush with the deck, not standing on it. The treads were 0.45
			-- studs proud and non-colliding, so you walked the smooth deck
			-- underneath while your legs passed through every visible step -
			-- Chris: "clipping on escalators". They have to be either real
			-- geometry you stand on or paint; a raised ghost step is the one
			-- thing they cannot be. Paint is right here, because real steps on
			-- a 27 degree ramp make the climb jerky.
			local tread = part(
				folder,
				`Tread_{name}_{k}`,
				Vector3.new(1.2, 0.5, ESC_W - 1),
				cf * CFrame.new(t, 0.78, 0),
				C.teal,
				Enum.Material.Metal
			)
			tread.CanCollide, tread.CanTouch, tread.CanQuery = false, false, false
		end
		for sgn = -1, 1, 2 do
			-- 3.6 studs, not 8. A player is five studs tall, so the old
			-- balustrade stood higher than their head and the escalator read
			-- as a trench. A handrail is about a metre - call it 0.7 of a
			-- player. Chris: "the mall is still weird with the structures
			-- being out of proportion, escalators and whatnot."
			part(
				folder,
				`Balustrade_{name}_{sgn}`,
				Vector3.new(hyp, 3.6, 1.4),
				cf * CFrame.new(0, 2.3, sgn * (ESC_W / 2 + 0.7)),
				C.teal,
				Enum.Material.Metal
			)
		end

		-- Emergency lighting lives on the escape routes, and a stair is the
		-- first of them. One at each end of every run, which is also what
		-- makes the escalators findable now that the floor between them is
		-- dark.
		for _, endSign in ipairs({ -1, 1 }) do
			local lampAt = cf * CFrame.new(endSign * hyp / 2, 3.2, 0)
			local can = part(
				folder,
				`EscLight_{name}_{endSign}`,
				Vector3.new(3, 1, ESC_W - 2),
				lampAt,
				C.shop,
				Enum.Material.Metal
			)
			can.CanCollide, can.CanQuery, can.CanTouch = false, false, false
			local light = Instance.new("PointLight")
			light.Color = Color3.fromRGB(216, 232, 236)
			light.Brightness, light.Range, light.Shadows = 0.55, 30, false
			light.Parent = can
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
		{ arm = 1, side = 1, at = Vector2.new(-11, 0) },
		{ arm = 1, side = 1, at = Vector2.new(-5, 0) },
		{ arm = 2, side = -1, at = seUnit * 7 },
	}

	-- The food court span, shared with the bay loop so the shopfronts inside
	-- it are built as food units rather than clothes shops.
	-- Where the food court sits, hoisted out of the arm loop because the
	-- escalators are placed before it and have to keep off it. They did not,
	-- and it cost both escalators on this arm: the deck is laid across the
	-- same well the runs rise through, and at a span of six units centred on
	-- the arm it covered both heads. The runs climbed into the underside of
	-- the slab and surfaced nowhere, so the upper south-east arm had no way
	-- down at all. Nothing said so - the geometry was all present and the
	-- slab simply won.
	local FC_FRAC, FC_LEN = 0.49, 6.0
	local fcFrom, fcTo, fcArm = math.huge, -math.huge, 2
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

		part(
			F.Floors,
			`Hall{ai}_L1`,
			Vector3.new(len * S, 2, HALL * 2 * S),
			CFrame.new(midX * S, -1, midZ * S) * rot,
			C.tile,
			Enum.Material.Pebble
		)
		for side = -1, 1, 2 do
			local bx, bz = at(0, (HALL + WELL) / 2 * side)
			part(
				F.Floors,
				`Balcony{ai}_{side}`,
				Vector3.new(len * S, 2, BAL * S),
				CFrame.new(bx * S, F2F - 1, bz * S) * rot,
				C.tile,
				Enum.Material.Pebble
			)
			-- The edge band seen from the floor below: a cream soffit under the
			-- balcony slab, which is most of what reads as this mall from L1.
			local ex, ez = at(0, WELL * side)
			part(
				F.Structure,
				`Soffit{ai}_{side}`,
				Vector3.new(len * S, 5, 1.6),
				CFrame.new(ex * S, F2F - 4.5, ez * S) * rot,
				C.fascia
			)
		end

		-- Two escalators per arm, on opposite sides. Each sits in the well with
		-- its upper edge flush to the balcony edge, so the top of the run lands
		-- on the balcony with nothing to bridge.
		--
		-- The arm carrying the food court takes its pair nearer the ends. At
		-- 0.3 and 0.68 both heads landed under the food court deck and both
		-- runs were impassable - measured, not guessed: 514 to 787 studs
		-- walked for an 81 stud straight line, against 86 on the identical
		-- runs on the west arm. Keeping the food court where the photographs
		-- put it, over the middle of the arm and open to the court, and moving
		-- the stairs out from under it is the way round that costs nothing:
		-- the alternative was halving the food court to fit between them.
		local fracs = if ai == fcArm then { 0.14, 0.86 } else { 0.3, 0.68 }
		for k, frac in ipairs(fracs) do
			local centre = -len / 2 + len * frac
			-- 2.8 plan units of horizontal run, not 2.0. The pitch is
			-- atan2(F2F, run), so at 2.0 these climbed at 38.7 degrees - a
			-- real escalator is 30, and the department store pairs already
			-- were, because their run comes off the 3.2 unit void. Only the
			-- concourse pairs were steep, which is why they read wrong next
			-- to the ones inside the shops.
			local run = 2.8
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
			part(
				F.Floors,
				`Landing{ai}_{k}`,
				Vector3.new(0.95 * S, 2, WELL * 2 * S),
				CFrame.new(hx * S, F2F - 1, hz * S) * rot,
				C.tile,
				Enum.Material.Pebble
			)
			-- The far end of the landing is a two storey drop into the well.
			local ex, ez = at(head + sense * 0.5, 0)
			part(
				F.Structure,
				`LandingRail{ai}_{k}`,
				Vector3.new(1, 3.4, WELL * 2 * S),
				CFrame.new(ex * S, F2F + 1.7, ez * S) * rot,
				C.fascia
			)
			part(
				F.Structure,
				`LandingCap{ai}_{k}`,
				Vector3.new(1.8, 1.2, WELL * 2 * S),
				CFrame.new(ex * S, F2F + 4, ez * S) * rot,
				C.teal,
				Enum.Material.Metal
			)
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
		if ai == fcArm then
			-- The food court. Chris: "The mall still feels dead and has no real
			-- food court", with photographs of the Galleria's own - a proper
			-- room on the upper level, counters down one side, a sea of tables
			-- out over the court, and a chequered floor. The old one was six
			-- tables on a strip of decking and read as a landing with chairs.
			local fcAt = -len / 2 + len * FC_FRAC
			local fcLen = FC_LEN
			fcFrom, fcTo = fcAt - fcLen / 2, fcAt + fcLen / 2
			local fx, fz = at(fcAt, 0)
			part(
				F.Floors,
				"FoodCourtDeck",
				Vector3.new(fcLen * S, 2, WELL * 2 * S),
				CFrame.new(fx * S, F2F - 1, fz * S) * rot,
				C.tile,
				Enum.Material.Pebble
			)
			for side = -1, 1, 2 do
				table.insert(
					landings,
					{ arm = ai, side = side, from = fcFrom - 0.1, to = fcTo + 0.1 }
				)
			end
			for e = -1, 1, 2 do
				local ex, ez = at(fcAt + e * (fcLen / 2 + 0.03), 0)
				part(
					F.Structure,
					`FCRail_{e}`,
					Vector3.new(1, 3.4, WELL * 2 * S),
					CFrame.new(ex * S, F2F + 1.7, ez * S) * rot,
					C.fascia
				)
				part(
					F.Structure,
					`FCCap_{e}`,
					Vector3.new(1.8, 1.2, WELL * 2 * S),
					CFrame.new(ex * S, F2F + 4, ez * S) * rot,
					C.teal,
					Enum.Material.Metal
				)
			end

			-- The chequered floor, which is most of what a food court looks
			-- like from the balcony opposite.
			local cols = math.floor(fcLen * S / 13)
			local rows = math.floor(WELL * 2 * S / 13)
			for i = 0, cols - 1 do
				for j = 0, rows - 1 do
					if (i + j) % 2 == 0 then
						local a0 = fcAt - fcLen / 2 + (i + 0.5) * 13 / S
						local o0 = -WELL + (j + 0.5) * 13 / S
						local tx, tz = at(a0, o0)
						paint(
							part(
								F.Floors,
								`FCTile_{i}_{j}`,
								Vector3.new(13, 0.4, 13),
								CFrame.new(tx * S, F2F - 0.18, tz * S) * rot,
								C.shop,
								Enum.Material.Pebble
							)
						)
					end
				end
			end

			-- Tables for a crowd. Scaled to a player rather than to the room:
			-- a top at three studs, which is a table, not a counter you cannot
			-- see over.
			local seatRows = 7
			for i = 0, seatRows - 1 do
				for _, off in ipairs({ -0.34, 0.34 }) do
					local t = fcAt - fcLen / 2 + fcLen * ((i + 0.5) / seatRows)
					local tx, tz = at(t, off)
					part(
						F.Fixtures,
						`FCPedestal_{i}_{off}`,
						Vector3.new(3, 2.6, 3),
						CFrame.new(tx * S, F2F + 1.3, tz * S) * rot,
						C.teal,
						Enum.Material.Metal
					)
					part(
						F.Fixtures,
						`FCTable_{i}_{off}`,
						Vector3.new(10, 0.8, 10),
						CFrame.new(tx * S, F2F + 3, tz * S) * rot,
						C.fascia,
						Enum.Material.WoodPlanks
					)
					for c = -1, 1, 2 do
						local sx, sz = at(t + c * 0.26, off)
						part(
							F.Fixtures,
							`FCSeat_{i}_{off}_{c}`,
							Vector3.new(5, 0.6, 5),
							CFrame.new(sx * S, F2F + 1.7, sz * S) * rot,
							C.column,
							Enum.Material.WoodPlanks
						)
						part(
							F.Fixtures,
							`FCSeatLeg_{i}_{off}_{c}`,
							Vector3.new(1.6, 1.7, 1.6),
							CFrame.new(sx * S, F2F + 0.85, sz * S) * rot,
							C.teal,
							Enum.Material.Metal
						)
						part(
							F.Fixtures,
							`FCBack_{i}_{off}_{c}`,
							Vector3.new(0.8, 3, 5),
							CFrame.new((sx + c * 0.09) * S, F2F + 3.4, sz * S) * rot,
							C.column,
							Enum.Material.WoodPlanks
						)
					end
				end
			end
		end

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
					part(
						F.Structure,
						`RailPanel{ai}_{side}_{i}`,
						Vector3.new(wlen, 3.4, 0.9),
						CFrame.new(rx * S, F2F + 1.7, rz * S) * rot,
						C.fascia
					)
					part(
						F.Structure,
						`RailCap{ai}_{side}_{i}`,
						Vector3.new(wlen, 1.2, 1.8),
						CFrame.new(rx * S, F2F + 4, rz * S) * rot,
						C.teal,
						Enum.Material.Metal
					)
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
				part(
					F.Shops,
					`Shell{ai}_{side}_{si}`,
					Vector3.new(slen * S, F2F * 2, 2),
					CFrame.new(sx * S, F2F, sz * S) * rot,
					C.shell,
					Enum.Material.Concrete
				)
			end

			-- The corridor itself: one unbroken floor and ceiling per level,
			-- with a jamb each side.
			for ci, c in ipairs(corridors) do
				local cmid, clen = (c.lo + c.hi) / 2, c.hi - c.lo
				for lvl = 1, 2 do
					local y = (lvl - 1) * F2F
					local px, pz = at(cmid, (HALL + DEPTH / 2) * side)
					part(
						F.Shops,
						`PortalFloor_{ai}_{side}_{ci}_{lvl}`,
						Vector3.new(clen * S, 2, DEPTH * S),
						CFrame.new(px * S, y - 1, pz * S) * rot,
						C.tile,
						Enum.Material.Pebble
					)
					part(
						F.Shops,
						`PortalCeil_{ai}_{side}_{ci}_{lvl}`,
						Vector3.new(clen * S, 1, DEPTH * S),
						CFrame.new(px * S, y + STORE_H, pz * S) * rot,
						C.fascia
					)
					for e = -1, 1, 2 do
						local jx, jz = at(cmid + e * clen / 2, (HALL + DEPTH / 2) * side)
						part(
							F.Shops,
							`PortalJamb_{ai}_{side}_{ci}_{lvl}_{e}`,
							Vector3.new(2, STORE_H, DEPTH * S),
							CFrame.new(jx * S, y + STORE_H / 2, jz * S) * rot,
							C.fascia
						)
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
					local playable = PLAYABLE_BAY_INDICES[i] == true
					local cx, cz = at(along, (HALL + DEPTH / 2) * side)

					-- A bay is a room, not a block. Most are shuttered, which is
					-- what a dead mall looks like and also means the ones you can
					-- walk into are the ones worth walking into.
					part(
						F.Shops,
						`BayFloor_{tag}`,
						Vector3.new(bw * S, 1, DEPTH * S),
						CFrame.new(cx * S, y - 0.5, cz * S) * rot,
						C.tile,
						Enum.Material.Pebble
					)
					part(
						F.Shops,
						`BayCeil_{tag}`,
						Vector3.new(bw * S, 1, DEPTH * S),
						CFrame.new(cx * S, y + STORE_H, cz * S) * rot,
						C.shop
					)
					for e = -1, 1, 2 do
						local dx, dz = at(along + e * bw / 2, (HALL + DEPTH / 2) * side)
						part(
							F.Shops,
							`Divider_{tag}_{e}`,
							Vector3.new(1.5, STORE_H, DEPTH * S),
							CFrame.new(dx * S, y + STORE_H / 2, dz * S) * rot,
							C.shop,
							Enum.Material.Slate
						)
					end

					local fx, fz = at(along, (HALL + 0.08) * side)
					-- Inside the food court span and on its level, a shopfront
					-- is a servery: an open counter under a menu board, not a
					-- shutter. These are the units the smell comes from.
					if
						playable
						and ai == fcArm
						and lvl == 2
						and along > fcFrom
						and along < fcTo
					then
						part(
							F.Shops,
							`Counter_{tag}`,
							Vector3.new(bw * S, 7, 3),
							CFrame.new(fx * S, y + 3.5, fz * S) * rot,
							C.fascia,
							Enum.Material.Marble
						)
						part(
							F.Shops,
							`CounterFront_{tag}`,
							Vector3.new(bw * S, 1, 3.6),
							CFrame.new(fx * S, y + 7.4, fz * S) * rot,
							C.teal,
							Enum.Material.Metal
						)
						local menu = part(
							F.Shops,
							`Menu_{tag}`,
							Vector3.new(bw * S * 0.8, 9, 0.8),
							CFrame.new(fx * S, y + STORE_H - 11, fz * S) * rot,
							C.shop
						)
						label(
							menu,
							L2_NAMES[((i + ai * 3) % #L2_NAMES) + 1],
							if side == 1 then Enum.NormalId.Front else Enum.NormalId.Back
						)
						local fascia2 = part(
							F.Shops,
							`Fascia_{tag}`,
							Vector3.new(bw * S, 4.5, 1.4),
							CFrame.new(fx * S, y + STORE_H - 2.5, fz * S) * rot,
							C.fascia
						)
						label(
							fascia2,
							L2_NAMES[((i + ai * 3) % #L2_NAMES) + 1],
							if side == 1 then Enum.NormalId.Front else Enum.NormalId.Back
						)
						part(
							F.Shops,
							`Trim_{tag}`,
							Vector3.new(bw * S, 1.2, 1.8),
							CFrame.new(fx * S, y + STORE_H - 5.2, fz * S) * rot,
							C.teal
						)
						continue
					end
					if playable then
						local doorW = 14
						local paneW = (bw * S - doorW) / 2
						for e = -1, 1, 2 do
							local px, pz =
								at(along + e * (doorW / 2 + paneW / 2) / S, (HALL + 0.08) * side)
							part(
								F.Shops,
								`Glass_{tag}_{e}`,
								Vector3.new(paneW, STORE_H - 5, 1),
								CFrame.new(px * S, y + (STORE_H - 5) / 2, pz * S) * rot,
								C.glass,
								Enum.Material.Glass,
								0.45
							)
							-- A figure behind the glass, facing out.
							local mx, mz =
								at(along + e * (doorW / 2 + paneW / 2) / S, (HALL + 0.55) * side)
							mannequin(F.Fixtures, `Dummy_{tag}_{e}`, mx * S, mz * S, ang, y)
						end

						-- The inside of the shop. Every open bay was a lit
						-- shopfront in front of eighty-three studs of nothing -
						-- Chris: "all the places look empty", and he was looking
						-- into a hundred of these. Shuttered bays stay empty
						-- because you cannot see into them.
						--
						-- No light in here, and this is the second time that has
						-- been decided. A fitting per bay was added because
						-- MapAudit listed the bay interiors as the last unlit
						-- floor left - and Chris walked the result: "the mall
						-- lighting is too bright now."
						--
						-- Chasing unlit floor to zero was the wrong target for
						-- this map. The Lanternhold is a refuge and wants none;
						-- the Galleria is the raid, and the dark inside a shop
						-- is what the lantern is for. A hundred bays each
						-- bleeding a fitting through its own shopfront - every
						-- light here has Shadows off - is most of what turned
						-- the concourse milky.
						do
							local function fit(
								nm: string,
								dAlong: number,
								dAcross: number,
								sw: number,
								h: number,
								sd: number,
								col: Color3,
								mat: Enum.Material?
							)
								local px, pz = at(along + dAlong, (HALL + dAcross) * side)
								local p = part(
									F.Shops,
									`{nm}_{tag}`,
									Vector3.new(sw * S, h, sd * S),
									CFrame.new(px * S, y + h / 2, pz * S) * rot,
									col,
									mat
								)
								p.CanCollide = true
								return p
							end
							-- Shelving against the back wall and down one side.
							fit(
								"ShelfBack",
								0,
								DEPTH - 0.18,
								bw * 0.92,
								13,
								0.34,
								C.shop,
								Enum.Material.Metal
							)
							fit(
								"ShelfSide",
								-bw * 0.4,
								DEPTH * 0.56,
								0.22,
								11,
								DEPTH * 0.6,
								C.shop,
								Enum.Material.Metal
							)
							-- A till counter set back from the glass, and two
							-- free-standing units in the middle of the floor.
							fit(
								"Till",
								bw * 0.26,
								0.72,
								bw * 0.4,
								4.6,
								0.3,
								C.fascia,
								Enum.Material.WoodPlanks
							)
							for r = 0, 1 do
								fit(
									`Unit{r}`,
									(r - 0.5) * bw * 0.44,
									DEPTH * (0.38 + r * 0.22),
									bw * 0.3,
									if r == 0 then 8 else 4.4,
									0.26,
									if r == 0 then C.fascia else C.teal,
									Enum.Material.Metal
								)
							end
						end
					else
						part(
							F.Shops,
							`Shutter_{tag}`,
							Vector3.new(bw * S, STORE_H - 5, 1.2),
							CFrame.new(fx * S, y + (STORE_H - 5) / 2, fz * S) * rot,
							C.shop,
							Enum.Material.DiamondPlate
						)
					end

					local fascia = part(
						F.Shops,
						`Fascia_{tag}`,
						Vector3.new(bw * S, 4.5, 1.4),
						CFrame.new(fx * S, y + STORE_H - 2.5, fz * S) * rot,
						C.fascia
					)
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
					label(
						fascia,
						`{list[((i + ai * 3) % #list) + 1]}   {n}`,
						if side == 1 then Enum.NormalId.Front else Enum.NormalId.Back
					)
					part(
						F.Shops,
						`Trim_{tag}`,
						Vector3.new(bw * S, 1.2, 1.8),
						CFrame.new(fx * S, y + STORE_H - 5.2, fz * S) * rot,
						C.teal
					)
				end
			end
		end

		-- Salmon columns and teal trusses: most of the recognition.
		local cols = math.floor(len / 1.9)
		for i = 0, cols do
			local along = -len / 2 + len * (i / cols)
			for side = -1, 1, 2 do
				local cx, cz = at(along, (HALL - 0.18) * side)
				pillar(
					F.Structure,
					`Column{ai}_{i}_{side}`,
					cx * S,
					cz * S,
					F2F - 1,
					F2F * 2 - 2,
					10,
					C.column
				)
				-- The teal uplighter bowl near the head of each column.
				part(
					F.Structure,
					`Uplight{ai}_{i}_{side}`,
					Vector3.new(15, 3, 15),
					CFrame.new(cx * S, F2F * 2 - 11, cz * S),
					C.teal,
					Enum.Material.Metal
				)
			end
			local tx, tz = at(along, 0)
			part(
				F.Roof,
				`Truss{ai}_{i}`,
				Vector3.new(2.2, 2.2, HALL * 2 * S),
				CFrame.new(tx * S, F2F * 2 + 10, tz * S) * rot,
				C.teal,
				Enum.Material.Metal
			)
		end

		-- Street furniture, at a size a player reads. All of this was two to
		-- three times too big - bench seats seven feet off the floor, kiosks
		-- seventeen feet tall. Chris, walking it: "some of the stuff ... is
		-- huge and not to scale with players." It is also denser now, because
		-- an empty corridor reads as a corridor however well it is lit.
		local props = math.max(3, math.floor(len / 1.25))
		for i = 0, props - 1 do
			local t = -len / 2 + len * ((i + 0.5) / props)
			-- Keep the middle clear at the ends, where the department store
			-- doors are, and under the escalators, whose feet land on this
			-- floor. A kiosk had parked itself squarely in Boscov's entrance.
			local clear = math.abs(t) < len / 2 - 1.8
			for _, l in ipairs(landings) do
				if l.arm == ai and t > l.from - 0.6 and t < l.to + 0.6 then
					clear = false
				end
			end
			if clear then
				local px, pz = at(t, 0)
				local kind = i % 4
				if kind == 0 then
					part(
						F.Fixtures,
						`Planter{ai}_{i}`,
						Vector3.new(26, 2.6, 11),
						CFrame.new(px * S, 1.3, pz * S) * rot,
						C.column,
						Enum.Material.Concrete
					)
					part(
						F.Fixtures,
						`PlanterRim{ai}_{i}`,
						Vector3.new(27, 0.7, 12),
						CFrame.new(px * S, 2.9, pz * S) * rot,
						C.fascia,
						Enum.Material.Concrete
					)
					part(
						F.Fixtures,
						`Shrub{ai}_{i}`,
						Vector3.new(21, 4.5, 8),
						CFrame.new(px * S, 5, pz * S) * rot,
						Color3.fromRGB(74, 112, 72),
						Enum.Material.Grass
					)
				elseif kind == 1 then
					for b = -1, 1, 2 do
						local bx, bz = at(t, b * 0.28)
						part(
							F.Fixtures,
							`Bench{ai}_{i}_{b}`,
							Vector3.new(16, 0.7, 4),
							CFrame.new(bx * S, 1.7, bz * S) * rot,
							C.fascia,
							Enum.Material.WoodPlanks
						)
						part(
							F.Fixtures,
							`BenchBack{ai}_{i}_{b}`,
							Vector3.new(16, 2.6, 0.7),
							CFrame.new((bx + b * 0.07) * S, 3.2, bz * S) * rot,
							C.fascia,
							Enum.Material.WoodPlanks
						)
						for e = -1, 1, 2 do
							part(
								F.Fixtures,
								`BenchLeg{ai}_{i}_{b}_{e}`,
								Vector3.new(0.9, 1.7, 3.4),
								CFrame.new((bx + e * 0.26) * S, 0.85, bz * S) * rot,
								C.teal,
								Enum.Material.Metal
							)
						end
					end
				elseif kind == 2 then
					part(
						F.Fixtures,
						`Kiosk{ai}_{i}`,
						Vector3.new(30, 4, 16),
						CFrame.new(px * S, 2, pz * S) * rot,
						C.fascia
					)
					part(
						F.Fixtures,
						`KioskTop{ai}_{i}`,
						Vector3.new(32, 0.6, 18),
						CFrame.new(px * S, 4.3, pz * S) * rot,
						C.teal,
						Enum.Material.Metal
					)
					for e = -1, 1, 2 do
						part(
							F.Fixtures,
							`KioskPost{ai}_{i}_{e}`,
							Vector3.new(0.8, 5, 0.8),
							CFrame.new((px + e * 0.52) * S, 7, pz * S) * rot,
							C.teal,
							Enum.Material.Metal
						)
					end
					part(
						F.Fixtures,
						`KioskRoof{ai}_{i}`,
						Vector3.new(34, 0.8, 20),
						CFrame.new(px * S, 9.6, pz * S) * rot,
						C.teal,
						Enum.Material.Metal
					)
				else
					for e = -1, 1, 2 do
						local bx, bz = at(t, e * 0.3)
						local bin = part(
							F.Fixtures,
							`Bin{ai}_{i}_{e}`,
							Vector3.new(3, 3.4, 3),
							CFrame.new(bx * S, 1.7, bz * S) * rot,
							C.shop,
							Enum.Material.Metal
						)
						bin.Shape = Enum.PartType.Cylinder
						bin.CFrame = CFrame.new(bx * S, 1.7, bz * S)
							* rot
							* CFrame.Angles(0, 0, math.pi / 2)
					end
					local dx, dz = at(t, 0)
					part(
						F.Fixtures,
						`DirectoryPost{ai}_{i}`,
						Vector3.new(1, 3, 1),
						CFrame.new(dx * S, 1.5, dz * S) * rot,
						C.teal,
						Enum.Material.Metal
					)
					local board = part(
						F.Fixtures,
						`Directory{ai}_{i}`,
						Vector3.new(9, 5, 0.8),
						CFrame.new(dx * S, 5.2, dz * S) * rot,
						C.shop
					)
					label(board, "GALLERIA  DIRECTORY", Enum.NormalId.Front)
					label(board, "GALLERIA  DIRECTORY", Enum.NormalId.Back)
				end
			end
		end

		-- The floor has a pattern in every photograph of this place: a dark
		-- border along the shopfronts and bands across the walk. Flat colour on
		-- a flat floor is what makes a greybox read as a car park.
		for side = -1, 1, 2 do
			local bx, bz = at(0, (HALL - 0.12) * side)
			paint(
				part(
					F.Floors,
					`FloorBorder{ai}_{side}`,
					Vector3.new(len * S, 0.4, 5),
					CFrame.new(bx * S, -0.18, bz * S) * rot,
					C.shop,
					Enum.Material.Pebble
				)
			)
		end
		local bands = math.floor(len / 2.2)
		for i = 0, bands - 1 do
			local bx, bz = at(-len / 2 + len * ((i + 0.5) / bands), 0)
			paint(
				part(
					F.Floors,
					`FloorBand{ai}_{i}`,
					Vector3.new(5, 0.4, HALL * 2 * S),
					CFrame.new(bx * S, -0.18, bz * S) * rot,
					C.shop,
					Enum.Material.Pebble
				)
			)
		end

		-- Banners hung off the trusses. Cheap, and they fill the volume that
		-- otherwise reads as empty air above the shopfronts.
		local banners = math.floor(len / 3.4)
		for i = 0, banners - 1 do
			for side = -1, 1, 2 do
				local bx, bz = at(-len / 2 + len * ((i + 0.5) / banners), (HALL - 0.45) * side)
				part(
					F.Fixtures,
					`Banner{ai}_{i}_{side}`,
					Vector3.new(0.6, 16, 11),
					CFrame.new(bx * S, F2F * 2 - 4, bz * S) * rot,
					if i % 2 == 0 then C.teal else C.column,
					Enum.Material.Fabric
				)
			end
		end

		-- The mall had no light fixtures at all - every bit of it was lit by the
		-- sky, which is why it read as glare rather than as a room. Chris: "the
		-- lighting is terrible". These are the emergency circuit, not the shop
		-- lighting: a dim warm pool every few bays at balcony height, hung low
		-- enough to light the walk and leave the ceiling dark. Deliberately
		-- sparse and deliberately weak, because the lantern is the light that
		-- is supposed to matter and a well-lit mall takes that away.
		-- One every 2.1 plan units. The old code said 1.6 but actually emitted
		-- one every 2.8, leaving long black stretches between pools of light.
		-- Still the same weak 0.85 lamp - the answer to a dark patch is another pool of
		-- light, never a brighter one, and never Ambient.
		-- The upper concourse had no furniture of any kind. The street
		-- furniture loop above only ever ran at ground level, so the balcony
		-- was a bare ring you walked round past shopfronts - half the mall's
		-- walkable length with nothing on it. Benches face the void so you sit
		-- looking down into the court, which is what people actually do.
		do
			local seats = math.max(3, math.floor(len / 2.1))
			for k = 0, seats - 1 do
				local t = -len / 2 + len * ((k + 0.7) / seats)
				local onLanding = false
				for _, l in ipairs(landings) do
					if l.arm == ai and t > l.from - 0.35 and t < l.to + 0.35 then
						onLanding = true
					end
				end
				if not onLanding then
					for sideU = -1, 1, 2 do
						local ux, uz = at(t, (HALL - BAL / 2) * sideU)
						if k % 2 == 0 then
							part(
								F.Fixtures,
								`UpBench{ai}_{k}_{sideU}`,
								Vector3.new(15, 0.7, 4),
								CFrame.new(ux * S, F2F + 1.7, uz * S) * rot,
								C.fascia,
								Enum.Material.WoodPlanks
							)
							part(
								F.Fixtures,
								`UpBenchBack{ai}_{k}_{sideU}`,
								Vector3.new(15, 2.6, 0.7),
								CFrame.new((ux + sideU * 0.07) * S, F2F + 3.2, uz * S) * rot,
								C.fascia,
								Enum.Material.WoodPlanks
							)
							for e = -1, 1, 2 do
								part(
									F.Fixtures,
									`UpBenchLeg{ai}_{k}_{sideU}_{e}`,
									Vector3.new(0.9, 1.7, 3.4),
									CFrame.new((ux + e * 0.24) * S, F2F + 0.85, uz * S) * rot,
									C.teal,
									Enum.Material.Metal
								)
							end
						else
							part(
								F.Fixtures,
								`UpPlanter{ai}_{k}_{sideU}`,
								Vector3.new(12, 2.6, 9),
								CFrame.new(ux * S, F2F + 1.3, uz * S) * rot,
								C.column,
								Enum.Material.Concrete
							)
							part(
								F.Fixtures,
								`UpShrub{ai}_{k}_{sideU}`,
								Vector3.new(9, 4.2, 7),
								CFrame.new(ux * S, F2F + 4.6, uz * S) * rot,
								Color3.fromRGB(74, 112, 72),
								Enum.Material.Grass
							)
						end
					end
				end
			end
		end

		-- Emergency spacing. Chris: "imagine just the emergency lights are
		-- on... like fire escape, stairwells". A building on emergency power
		-- does not light its concourse, it lights the way out of it, so these
		-- go from one every 2.1 plan units to one every 5 - and the dark
		-- between them is the point, as DONTFUCKUPTHELIGHTS.md says.
		local lamps = math.max(2, math.floor(len / 5))
		for i = 0, lamps - 1 do
			local lx, lz = at(-len / 2 + len * ((i + 0.5) / lamps), 0)
			local can = part(
				F.Fixtures,
				`Downlight{ai}_{i}`,
				Vector3.new(7, 1.2, 7),
				CFrame.new(lx * S, F2F + 14, lz * S) * rot,
				C.shop,
				Enum.Material.Metal
			)
			-- A plain dark fitting with a light in it: no glowing lens, no
			-- emissive panel. Chris: "no fucking glowing shit, basic ass
			-- lighting". The fixture is scenery, the PointLight does the work.
			can.CanCollide, can.CanQuery = false, false
			local light = Instance.new("PointLight")
			light.Color = Color3.fromRGB(255, 232, 200)
			-- Below the house lamp now: these are the few fittings still fed
			-- by the emergency circuit, not the mall's working lighting.
			light.Brightness, light.Range, light.Shadows = 0.6, 42, false
			light.Parent = can
		end

		-- A second row, under the balconies, lighting level one.
		--
		-- The downlights above hang at F2F + 14 - forty six studs over the
		-- level one floor - because the middle of the concourse is open to the
		-- roof and there is nothing nearer to fix them to. Range is a sphere,
		-- so by the time one of those reaches the tile it has about twenty
		-- eight studs of spread left and only directly underneath itself.
		-- MapAudit put most of Hall1_L1 and Hall2_L1 in the dark while the
		-- balconies read fine, which is exactly that shape of fault: the
		-- ground floor was lit by a thin stripe down the middle.
		--
		-- These hang under the balcony soffit, over the walking lanes and the
		-- shopfronts, which is where a mall puts them anyway.
		local underPitch = 150
		local underCount = math.max(2, math.round(len * S / underPitch))
		for i = 0, underCount - 1 do
			for _, side in ipairs({ -1, 1 }) do
				local ux, uz =
					at(-len / 2 + len * ((i + 0.5) / underCount), (HALL - BAL / 2) * side)
				local can = part(
					F.Fixtures,
					`UnderLight{ai}_{i}_{side}`,
					Vector3.new(6, 1.2, 6),
					CFrame.new(ux * S, F2F - 6, uz * S) * rot,
					C.shop,
					Enum.Material.Metal
				)
				can.CanCollide, can.CanQuery, can.CanTouch = false, false, false
				local light = Instance.new("PointLight")
				light.Color = Color3.fromRGB(255, 232, 200)
				-- Dimmer than the roof downlights and shorter: this row exists
				-- because the ground floor had almost nothing, not to match
				-- the level above it.
				light.Brightness, light.Range, light.Shadows = 0.5, 38, false
				light.Parent = can
			end
		end

		part(
			F.Roof,
			`Ridge{ai}`,
			Vector3.new(len * S, 2.6, 2.6),
			CFrame.new(midX * S, F2F * 2 + 24, midZ * S) * rot,
			C.teal,
			Enum.Material.Metal
		)
	end

	-- The court.
	-- The rim is a skirt below the floor, not a disc level with it. Sized to
	-- the same top it would z-fight the floor across the whole court, which
	-- is the striping that showed up the moment the court was first built.
	disc(F.Court, "CourtRim", COURT_OUT + 0.06, -1, 2.4, C.teal, Enum.Material.Metal)
	disc(F.Court, "CourtFloor", COURT_OUT, -1 + LIFT, 2, C.tile, Enum.Material.Pebble)

	-- The court had no light of its own. The arm downlights stop where the
	-- arms stop, and the court is the one place every route crosses and where
	-- the crew spawns - it was the single largest patch of unlit floor in the
	-- mall. A ring under the level two balcony arcs, outside the open well so
	-- the fittings have something to hang from.
	for i = 0, 5 do
		local th = 2 * math.pi * (i + 0.5) / 6
		local cp = Vector2.new(math.cos(th), math.sin(th)) * ((COURT_WELL + COURT_OUT) / 2)
		local can = part(
			F.Fixtures,
			`CourtLight_{i}`,
			Vector3.new(6, 1.2, 6),
			CFrame.new(cp.X * S, F2F - 6, cp.Y * S),
			C.shop,
			Enum.Material.Metal
		)
		can.CanCollide, can.CanQuery, can.CanTouch = false, false, false
		local light = Instance.new("PointLight")
		light.Color = Color3.fromRGB(255, 232, 200)
		-- The court stays the one place that reads as lit: it is the spawn
		-- and the crossroads. Six, not the original eight - but at range 42
		-- and four of them the ring reached neither the fountain nor the
		-- spawn pad, both of which measured 0.00. A ring hung 23 studs up
		-- spends most of its reach on the drop.
		light.Brightness, light.Range, light.Shadows = 0.6, 56, false
		light.Parent = can
	end

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
			part(
				F.Court,
				`CourtBalcony_{i}`,
				Vector3.new(w * S, 2, (COURT_OUT - COURT_WELL) * S),
				CFrame.new(p.X * S, F2F - 1 + LIFT + jitter, p.Y * S) * yaw,
				C.tile,
				Enum.Material.Pebble
			)
			-- The court is wider than the concourse, so the arms' outer walls do
			-- not reach round it. Without this the court is open to the sky on
			-- both flanks and you can see straight out of the building.
			local wp = Vector2.new(math.cos(th), math.sin(th)) * (COURT_OUT + 0.1)
			part(
				F.Court,
				`CourtWall_{i}`,
				Vector3.new(w * S, F2F * 2, 2),
				CFrame.new(wp.X * S, F2F, wp.Y * S) * yaw,
				C.shell,
				Enum.Material.Concrete
			)
			-- No rail where the lift lands, or it arrives at level two and lets
			-- you off against a fence.
			local toLift = math.abs(((math.deg(th) - 225 + 180) % 360) - 180)
			if toLift >= 14 then
				local r = Vector2.new(math.cos(th), math.sin(th)) * COURT_WELL
				local railW = 2 * math.pi * COURT_WELL / RING * S * 1.25
				part(
					F.Court,
					`CourtRailPanel_{i}`,
					Vector3.new(railW, 3.4, 0.9),
					CFrame.new(r.X * S, F2F + 1.7 + LIFT + jitter, r.Y * S) * yaw,
					C.fascia
				)
				part(
					F.Court,
					`CourtRailCap_{i}`,
					Vector3.new(railW, 1.2, 1.8),
					CFrame.new(r.X * S, F2F + 4 + LIFT + jitter, r.Y * S) * yaw,
					C.teal,
					Enum.Material.Metal
				)
			end
		end
	end

	-- Two bridges across the well mouths, so the upper level goes all the way
	-- round the court. Chris: "the circular area I can't walk the whole way
	-- around the center." He was right - the ring is built in two arcs because
	-- each arm's well opens into the court, and without a crossing the loop is
	-- a horseshoe. Real malls bridge exactly here. They are narrow on purpose:
	-- the void is the point, and a bridge should read as a crossing rather than
	-- as a floor.
	for ai, arm in ipairs(ARMS) do
		local dir = (arm.b - arm.a).Unit
		local ang = math.atan2(dir.Y, dir.X)
		local rot = CFrame.Angles(0, -ang, 0)
		local r = (COURT_WELL + COURT_R) / 2
		local c = dir * r
		part(
			F.Court,
			`CourtBridge_{ai}`,
			Vector3.new(1.0 * S, 2, 2.0 * S),
			CFrame.new(c.X * S, F2F - 1 + LIFT, c.Y * S) * rot,
			C.tile,
			Enum.Material.Pebble
		)
		-- Both long edges look down into the void.
		for e = -1, 1, 2 do
			local edge = dir * (r + e * 0.5)
			part(
				F.Court,
				`CourtBridgePanel_{ai}_{e}`,
				Vector3.new(1, 3.4, 2.0 * S),
				CFrame.new(edge.X * S, F2F + 1.7 + LIFT, edge.Y * S) * rot,
				C.fascia
			)
			part(
				F.Court,
				`CourtBridgeCap_{ai}_{e}`,
				Vector3.new(1.8, 1.2, 2.0 * S),
				CFrame.new(edge.X * S, F2F + 4 + LIFT, edge.Y * S) * rot,
				C.teal,
				Enum.Material.Metal
			)
		end
	end

	disc(F.Court, "FountainBasin", 0.5, 3, 8, C.teal, Enum.Material.Marble)
	disc(F.Court, "FountainWater", 0.44, 6.9, 0.8, C.glass, Enum.Material.Glass, 0.35)
	part(
		F.Court,
		"FountainStem",
		Vector3.new(6, 15, 6),
		CFrame.new(0, 10, 0),
		C.column,
		Enum.Material.Concrete
	)

	-- The court is the middle of the mall and held a fountain, a lift and
	-- nothing else - Chris: "it's empty everywhere", and this is the room
	-- everyone passes through. A ring of benches facing the water, planters
	-- between them, and bins: what actually sits round a mall fountain.
	do
		local RING = 1.55 -- model units out from the centre, clear of the basin
		local SEATS = 8
		for k = 0, SEATS - 1 do
			local a = (k / SEATS) * math.pi * 2
			local bx, bz = math.cos(a) * RING * S, math.sin(a) * RING * S
			local face = CFrame.new(bx, 0, bz) * CFrame.Angles(0, -a + math.pi / 2, 0)
			if k % 2 == 0 then
				part(
					F.Fixtures,
					`CourtBench_{k}`,
					Vector3.new(16, 0.7, 4),
					face * CFrame.new(0, 1.7, 0),
					C.fascia,
					Enum.Material.WoodPlanks
				)
				part(
					F.Fixtures,
					`CourtBenchBack_{k}`,
					Vector3.new(16, 2.6, 0.7),
					face * CFrame.new(0, 3.2, 1.6),
					C.fascia,
					Enum.Material.WoodPlanks
				)
				for e = -1, 1, 2 do
					part(
						F.Fixtures,
						`CourtBenchLeg_{k}_{e}`,
						Vector3.new(0.9, 1.7, 3.4),
						face * CFrame.new(e * 6.5, 0.85, 0),
						C.teal,
						Enum.Material.Metal
					)
				end
			else
				part(
					F.Fixtures,
					`CourtPlanter_{k}`,
					Vector3.new(13, 2.6, 13),
					face * CFrame.new(0, 1.3, 0),
					C.column,
					Enum.Material.Concrete
				)
				part(
					F.Fixtures,
					`CourtPlanterRim_{k}`,
					Vector3.new(14, 0.7, 14),
					face * CFrame.new(0, 2.9, 0),
					C.fascia,
					Enum.Material.Concrete
				)
				part(
					F.Fixtures,
					`CourtShrub_{k}`,
					Vector3.new(10, 5, 10),
					face * CFrame.new(0, 5.2, 0),
					Color3.fromRGB(74, 112, 72),
					Enum.Material.Grass
				)
			end
		end
		for k = 0, 3 do
			local a = (k / 4) * math.pi * 2 + math.pi / 8
			local bx, bz = math.cos(a) * 2.5 * S, math.sin(a) * 2.5 * S
			local bin = part(
				F.Fixtures,
				`CourtBin_{k}`,
				Vector3.new(3, 3.4, 3),
				CFrame.new(bx, 1.7, bz),
				C.shop,
				Enum.Material.Metal
			)
			bin.Shape = Enum.PartType.Cylinder
			bin.CFrame = CFrame.new(bx, 1.7, bz) * CFrame.Angles(0, 0, math.pi / 2)
		end
	end

	-- The same deck carried round the court, open in the middle so the court
	-- keeps its skylight. The opening has to be at least as wide as the court
	-- void, or the atrium is roofed over just outside the rail and the fountain
	-- sits under a ceiling.
	for i = 0, 19 do
		local th = 2 * math.pi * (i + 0.5) / 20
		local rr = (COURT_WELL + 0.1 + COURT_OUT + 0.2) / 2
		local p = Vector2.new(math.cos(th), math.sin(th)) * rr
		local w = 2 * math.pi * rr / 20 * 1.4
		part(
			F.Roof,
			`CourtDeck_{i}`,
			Vector3.new(w * S, 2, (COURT_OUT + 0.2 - COURT_WELL - 0.1) * S),
			CFrame.new(p.X * S, F2F * 2 + 1 + (i % 2) * 0.05, p.Y * S)
				* CFrame.Angles(0, -(th + math.pi / 2), 0),
			C.shell,
			Enum.Material.Concrete
		)
	end

	-- The clock. It is in half the photographs of this place, standing at the
	-- edge of the court where both arms can see it.
	do
		local a = math.pi * 0.78
		local kx, kz = math.cos(a) * (COURT_WELL + 0.5) * S, math.sin(a) * (COURT_WELL + 0.5) * S
		pillar(F.Court, "ClockPost", kx, kz, F2F * 0.95, F2F * 1.9, 11, C.column)
		local face = part(
			F.Court,
			"ClockFace",
			Vector3.new(6, 26, 26),
			CFrame.new(kx, F2F * 1.85, kz) * CFrame.Angles(0, 0, math.pi / 2),
			C.fascia
		)
		face.Shape = Enum.PartType.Cylinder
		part(
			F.Court,
			"ClockRim",
			Vector3.new(4, 30, 30),
			CFrame.new(kx, F2F * 1.85, kz) * CFrame.Angles(0, 0, math.pi / 2),
			C.teal,
			Enum.Material.Metal
		).Shape =
			Enum.PartType.Cylinder
	end

	-- A dark band of tile round the fountain, the way the floor pattern rings
	-- it in the photographs.
	disc(
		F.Court,
		"FountainBand",
		0.95,
		-1 + LIFT + 0.05,
		2,
		Color3.fromRGB(58, 60, 64),
		Enum.Material.Pebble
	)

	disc(
		F.Roof,
		"CourtSkylight",
		COURT_OUT - 0.15,
		F2F * 2 + 20,
		1.2,
		C.glass,
		Enum.Material.Glass,
		0.55
	)
	-- Stepped, not stacked: eight full-diameter bars at one height all
	-- cross at the hub and fight there. A couple of studs of rise each turns
	-- the crossing into a lattice instead.
	for i = 0, 7 do
		part(
			F.Roof,
			`CourtTruss_{i}`,
			Vector3.new(COURT_OUT * 2 * S, 2.2, 2.2),
			CFrame.new(0, F2F * 2 + 17 + i * 0.25, 0) * CFrame.Angles(0, -math.pi * i / 8, 0),
			C.teal,
			Enum.Material.Metal
		)
	end

	-- A glass lift in the court. Chris asked for one by the fountain; it is
	-- scenery for now, in that the car does not travel - making it carry a
	-- player properly is gameplay code, not geometry, and a lift that looks
	-- rideable and is not would be worse than no lift at all.
	do
		local lx, lz = -1.3, -1.3
		local shaft = 1.0
		-- Two openings, and they face along the radius rather than along the
		-- axes: in toward the fountain, where you board at court level, and out
		-- toward the ring, where you step off at level two. The first version
		-- was glazed on all four sides - "the elevator does nothing" was
		-- literally true before any code existed - and the second opened the
		-- wrong side, so the car arrived upstairs behind glass.
		for k, a in ipairs({ math.rad(135), math.rad(315) }) do
			local ox, oz = math.cos(a) * shaft / 2, math.sin(a) * shaft / 2
			part(
				F.Court,
				`LiftGlass_{k}`,
				Vector3.new(shaft * S, F2F + 14, 2),
				CFrame.new((lx + ox) * S, (F2F + 14) / 2, (lz + oz) * S)
					* CFrame.Angles(0, -(a + math.pi / 2), 0),
				C.glass,
				Enum.Material.Glass,
				0.6
			)
		end
		for i = 0, 3 do
			local a = math.pi / 2 * i
			part(
				F.Court,
				`LiftPost_{i}`,
				Vector3.new(3, F2F + 16, 3),
				CFrame.new(
					(lx + math.cos(a) * shaft * 0.72) * S,
					(F2F + 16) / 2,
					(lz + math.sin(a) * shaft * 0.72) * S
				),
				C.teal,
				Enum.Material.Metal
			)
		end
		-- The car. LiftService drives it between these two heights; the map
		-- states where they are rather than the service guessing.
		-- The car's deck is flush with the court floor at the bottom and with
		-- the ring at the top. Parked two studs proud of the tile it looked
		-- fine and could not be walked onto at all: the navmesh will not climb
		-- a step that size, so the lift was unreachable before it was unwired.
		local car = part(
			F.Court,
			"LiftCar",
			Vector3.new(shaft * S - 5, 2, shaft * S - 5),
			CFrame.new(lx * S, -1 + LIFT, lz * S),
			C.shell,
			Enum.Material.Metal
		)
		car:SetAttribute("LowY", -1 + LIFT)
		car:SetAttribute("HighY", F2F - 1 + LIFT)
		CollectionService:AddTag(car, "Lift")
		part(
			F.Court,
			"LiftHead",
			Vector3.new(shaft * S + 5, 4, shaft * S + 5),
			CFrame.new(lx * S, F2F + 16, lz * S),
			C.teal,
			Enum.Material.Metal
		)
	end

	-- Department stores. They were solid brick blocks, which is what Chris hit:
	-- "i can't go into any of the dept stores, but when i can, they should also
	-- be highly populated, those are the primary areas, they should be huge,
	-- like a real dept. store. with escalators in there too."
	--
	-- So each one is now a building: two floors, a perimeter wall with a real
	-- opening onto the mall, its own escalator pair through a void in the upper
	-- slab, a column grid and rows of fixtures. The old fourteen-by-fifteen-unit
	-- boxes were true department-store acreage, but too much empty traversal for
	-- this raid. These are compact anchors: substantial rooms to loot and evade
	-- in, not a second mall hidden behind every entrance.
	local WALL_T = 3 / S -- wall thickness, in model units
	local DOORW = 2.2 -- mall entrance width, in model units
	local DOOR_H = 26

	local function department(
		name: string,
		cx: number,
		cz: number,
		w: number,
		d: number,
		yaw: number,
		front: string,
		doors: { number },
		doorsUp: { number }
	)
		local H = F2F * 2
		local rot = CFrame.Angles(0, -yaw, 0)
		local cos, sin = math.cos(yaw), math.sin(yaw)
		-- Local plan coordinates, so each store can be written as though it were
		-- square to the page and then turned to meet whichever arm it serves.
		local function at(lx: number, lz: number): (number, number)
			return cx + lx * cos - lz * sin, cz + lx * sin + lz * cos
		end
		local function box(
			folder: Instance,
			nm: string,
			lx: number,
			lz: number,
			sx: number,
			sz: number,
			y: number,
			h: number,
			colour: Color3,
			mat: Enum.Material?,
			trans: number?
		): Part
			local wx, wz = at(lx, lz)
			return part(
				folder,
				nm,
				Vector3.new(sx * S, h, sz * S),
				CFrame.new(wx * S, y, wz * S) * rot,
				colour,
				mat,
				trans
			)
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
		box(
			F.Anchors,
			`Floor2_{name}_W`,
			-(w + vw) / 4,
			0,
			(w - vw) / 2,
			d,
			F2F - 1,
			2,
			C.tile,
			Enum.Material.Pebble
		)
		box(
			F.Anchors,
			`Floor2_{name}_E`,
			(w + vw) / 4,
			0,
			(w - vw) / 2,
			d,
			F2F - 1,
			2,
			C.tile,
			Enum.Material.Pebble
		)
		box(
			F.Anchors,
			`Floor2_{name}_N`,
			0,
			-(d + vd) / 4,
			vw,
			(d - vd) / 2,
			F2F - 1,
			2,
			C.tile,
			Enum.Material.Pebble
		)
		box(
			F.Anchors,
			`Floor2_{name}_S`,
			0,
			(d + vd) / 4,
			vw,
			(d - vd) / 2,
			F2F - 1,
			2,
			C.tile,
			Enum.Material.Pebble
		)
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
					box(
						F.Anchors,
						`VoidRail_{name}_X{sgn}_{math.floor(lo * 100)}`,
						sgn * vw / 2,
						(lo + hi) / 2,
						0.06,
						hi - lo,
						F2F + 1.75,
						3.5,
						C.teal,
						Enum.Material.Metal
					)
				end
			end
			box(
				F.Anchors,
				`VoidRail_{name}_Z{sgn}`,
				0,
				sgn * vd / 2,
				vw,
				0.06,
				F2F + 1.75,
				3.5,
				C.teal,
				Enum.Material.Metal
			)
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
				local gaps = if isFront
					then band[3] :: { number }
					elseif isBack and bi == 1 then { 0 }
					else {}
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
						box(
							F.Anchors,
							`Wall_{name}_{axis}{sgn}_{bi}_{i}`,
							sgn * w / 2,
							mid,
							WALL_T,
							rlen,
							(y0 + y1) / 2,
							y1 - y0,
							C.brick,
							Enum.Material.Brick
						)
					else
						box(
							F.Anchors,
							`Wall_{name}_{axis}{sgn}_{bi}_{i}`,
							mid,
							sgn * d / 2,
							rlen,
							WALL_T,
							(y0 + y1) / 2,
							y1 - y0,
							C.brick,
							Enum.Material.Brick
						)
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
			box(
				F.Anchors,
				`ExitApron_{name}`,
				ox,
				oz,
				if isX then 2.4 else 3.0,
				if isX then 3.0 else 2.4,
				-1,
				2,
				C.shell,
				Enum.Material.Concrete
			)
			-- The marker does not carry anyone: a raised slab this thin is not
			-- picked up as ground, and every route that ended standing on it
			-- came back NoPath while the apron an inch away was reachable. It
			-- is paint on the floor, and the apron underneath is the floor.
			local pad = box(
				F.Anchors,
				`Exit_{name}`,
				ox,
				oz,
				1.5,
				1.5,
				0.3,
				0.6,
				C.teal,
				Enum.Material.Metal
			)
			pad.CanCollide = false
			pad:SetAttribute("Store", name)
			CollectionService:AddTag(pad, "ExtractCandidate")

			-- A light over the door. Measured with the emergency scheme in
			-- and every one of the four exits came out at 0.00 lux, which is
			-- backwards: the exit is the one fitting a building on emergency
			-- power always keeps. Cold white, because that is what an exit
			-- fitting is and it reads as different from the warm house lamps.
			--
			-- NOTE the argument order. department()'s own box() takes
			-- (lx, lz, sx, sz, y, h) in plan units with y and h in studs, not
			-- the corner pairs the Lanternhold helper of the same name takes.
			-- The first version of this used the other one and put four
			-- fittings nowhere at all, which the audit caught because the
			-- exits stayed at zero.
			local exitCan = box(
				F.Anchors,
				`ExitLight_{name}`,
				ox,
				oz,
				0.3,
				0.3,
				13,
				1.2,
				C.shop,
				Enum.Material.Metal
			)
			exitCan.CanCollide, exitCan.CanQuery, exitCan.CanTouch = false, false, false
			local exitLight = Instance.new("PointLight")
			exitLight.Color = Color3.fromRGB(206, 230, 236)
			exitLight.Brightness, exitLight.Range, exitLight.Shadows = 0.8, 40, false
			exitLight.Parent = exitCan
		end

		box(
			F.Anchors,
			`Parapet_{name}`,
			0,
			0,
			w + 0.12,
			d + 0.12,
			H + 5,
			6,
			C.shell,
			Enum.Material.Concrete
		)

		-- Sign band, mounted on the parapet above the roofline rather than on
		-- the face. It used to sit at 35 to 57 studs, which is exactly where
		-- the upper doorway is - Chris, after walking it: "signs are blocking
		-- entry to major anchor stores". A department store's sign belongs over
		-- the roof anyway.
		do
			local isX = front:sub(2) == "X"
			local sgn = if front:sub(1, 1) == "+" then 1 else -1
			local bandLen = (if isX then d else w) * 0.5
			local band
			if isX then
				band = box(
					F.Anchors,
					`Sign_{name}`,
					sgn * (w / 2 + 0.06),
					0,
					0.08,
					bandLen,
					H + 11,
					20,
					C.teal
				)
			else
				band = box(
					F.Anchors,
					`Sign_{name}`,
					0,
					sgn * (d / 2 + 0.06),
					bandLen,
					0.08,
					H + 11,
					20,
					C.teal
				)
			end
			for _, f in ipairs({
				Enum.NormalId.Front,
				Enum.NormalId.Back,
				Enum.NormalId.Left,
				Enum.NormalId.Right,
			}) do
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
					pillar(
						F.Structure,
						`DeptColumn_{name}_{i}_{j}`,
						px * S,
						pz * S,
						H / 2,
						H,
						10,
						C.column
					)
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
							box(
								F.Fixtures,
								`Rack_{name}_{lvl}_{r}_{c}`,
								lx,
								lz,
								w / 5.4,
								0.28,
								y + (if tall then 7 else 3.5),
								if tall then 14 else 7,
								if tall then C.shop else C.fascia,
								Enum.Material.Metal
							)
						end
					end
				end
			end
		end

		-- Light inside the store. Every anchor had none at all: the concourse
		-- downlights stop at the shop front, so all four were unlit sheds -
		-- and the only relic caches in the map are in them, and a relic is
		-- `visibility: lanternOnly` in data/loot.json. Chris walked it and
		-- reported the two halves of that as separate faults: "its still a
		-- little too dark" and "i haven't gotten any relics".
		--
		-- Deliberately weaker than the concourse downlight's 0.85. A store
		-- should read as dimmer than the mall it opens onto: lit enough to
		-- cross and to see a rack, never enough to make the lantern pointless,
		-- because the lantern is what makes the relics visible at all. Plain
		-- metal can, no lens, no Neon, no beam - see DONTFUCKUPTHELIGHTS.md.
		--
		-- Two numbers here were got wrong twice before the audit settled them,
		-- and both are worth keeping written down.
		--
		-- The grid comes off the store's own size rather than being a fixed
		-- three by three. At three by three the pools stood 73 studs apart in
		-- a 220 by 240 room and MapAudit put 30% of the mall's standable floor
		-- in the dark, nearly all of it inside the four anchors.
		--
		-- And the fittings hang at 17 rather than tight under the 27 stud
		-- ceiling. Range is a sphere, so mounting height is spent out of the
		-- same budget as floor coverage: at 27 up, a 58 stud range had only
		-- about 52 left to spread sideways once it reached the floor, and the
		-- corners got nothing. Dropping the fitting ten studs buys more floor
		-- than ten studs of range would, and it does it without the light
		-- reaching further through the walls - with Shadows off, which they
		-- are, every one of these shines straight through the shopfront into
		-- the concourse, so a bigger range is not free.
		-- Two a floor. This was a sixteen fitting grid, which lit a 220 by
		-- 240 shop floor end to end - the opposite of a building running on
		-- emergency power, and most of why Chris said the mall was too
		-- bright twice over. What is left reads as the circuit that stayed
		-- on, and the rest of the store is the lantern's job.
		local lightPitch = 150
		local nx = math.max(1, math.round(w * S / lightPitch))
		local nz = math.max(1, math.round(d * S / lightPitch))
		for lvl = 0, 1 do
			for a = 0, nx - 1 do
				for b = 0, nz - 1 do
					local lx = -w / 2 + w * ((a + 0.5) / nx)
					local lz = -d / 2 + d * ((b + 0.5) / nz)
					local wx, wz = at(lx, lz)
					local can = part(
						F.Fixtures,
						`DeptLight_{name}_{lvl}_{a}_{b}`,
						Vector3.new(7, 1.2, 7),
						CFrame.new(wx * S, lvl * F2F + 17, wz * S) * rot,
						C.shop,
						Enum.Material.Metal
					)
					can.CanCollide, can.CanQuery, can.CanTouch = false, false, false
					local light = Instance.new("PointLight")
					light.Color = Color3.fromRGB(255, 226, 196)
					-- 0.45 at range 40, down from 0.7 at 54. The grid and the
					-- low mount stay - those fixed real dark corners - but at
					-- 54 the pools merged into one flood across the whole
					-- shop floor, which is the thing DONTFUCKUPTHELIGHTS.md
					-- says not to do: a lamp lights its own patch and the dark
					-- between patches is the point.
					light.Brightness, light.Range, light.Shadows = 0.5, 42, false
					light.Parent = can
				end
			end
		end

		-- A store with racks in it and nothing else is a warehouse. Chris:
		-- "144 parts across four department stores is why they feel empty."
		-- The racks were never the problem - every cache and every Sweeper post
		-- was out on the arms, and the insides had no light of their own, so
		-- there was no reason to go in and nothing to see if you did. These are
		-- meant to be the rooms the run is about.
		--
		-- Where a marker may stand, decided geometrically rather than by a
		-- spatial query, because the store is built before the model is
		-- parented into Workspace and a query here would see an empty world.
		--
		-- This replaces two hand-typed quarter positions, and the reason is
		-- worth keeping. At fourteen by fifteen units every relic cache in
		-- Boscov's and Sayers landed INSIDE a structural column, so the
		-- post-pass destroyed all eight and the two anchors that carry the
		-- extractions held no relics whatsoever - measured in the live build,
		-- and the whole of Chris's "i haven't gotten any relics". The cause was
		-- a coincidence of fractions: 0.26 of the span is the quarter line, and
		-- a store whose column grid happens to come out four by four puts a
		-- column on exactly that line. Penhurst and Bellevue were fine by luck.
		-- Compacting the stores to a three-by-three grid stopped it by luck
		-- too, which is not the same as fixing it. Nothing below stands on a
		-- number that has to keep missing a grid by hand.
		local function clearSpot(lx: number, lz: number): boolean
			if onAisle(lx, lz) then
				return false
			end
			-- The void and the escalator apron either side of it: the same
			-- exclusion the column and rack loops use.
			if math.abs(lx) < vw / 2 + 1.4 and math.abs(lz) < vd / 2 + 0.6 then
				return false
			end
			-- A marker has to clear a column centre by the pillar's half width
			-- plus the 2.4 stud radius the post-pass rejects on, with a little
			-- margin. Columns are ten studs square.
			local colClear = (5 + 2.4 + 2) / S
			for i = 0, gx do
				for j = 0, gz do
					local cl = -w / 2 + w * (i / gx)
					local cd = -d / 2 + d * (j / gz)
					if math.abs(lx - cl) < colClear and math.abs(lz - cd) < colClear then
						return false
					end
				end
			end
			-- And the rack grid, tested against the same spans the rack loop
			-- lays down. A rack skipped for being on an aisle is treated as
			-- present, which only costs candidates, never correctness.
			local rows = math.max(3, math.floor(d / 1.1))
			local rackZ = 0.28 / 2 + 2.4 / S + 0.1
			local rackX = w / 5.4 / 2 + 2.4 / S + 0.1
			for r = 0, rows - 1 do
				local rz = -d / 2 + d * ((r + 0.5) / rows)
				if math.abs(lz - rz) < rackZ then
					for c = 0, 3 do
						local rx = -w / 2 + w * ((c + 0.5) / 4)
						if math.abs(lx - rx) < rackX then
							return false
						end
					end
				end
			end
			return true
		end

		-- Pick `count` clear spots, spread out. A relic in each quarter of the
		-- shop floor is a reason to walk the store; three in one aisle is a
		-- reason to walk its perimeter and leave. Greedy farthest-point, which
		-- is deterministic - two builds of the same mall put the relics in the
		-- same places, so a route learned is a route that holds.
		local function spreadSpots(count: number): { { number } }
			local pool = {}
			local STEPS = 19
			for a = 0, STEPS - 1 do
				for b = 0, STEPS - 1 do
					local lx = -w * 0.44 + (w * 0.88) * (a / (STEPS - 1))
					local lz = -d * 0.44 + (d * 0.88) * (b / (STEPS - 1))
					if clearSpot(lx, lz) then
						table.insert(pool, { lx, lz })
					end
				end
			end
			local picked = {}
			while #picked < count and #pool > 0 do
				local bestI, bestScore = 1, -math.huge
				for i, c in ipairs(pool) do
					local score
					if #picked == 0 then
						-- Start in a corner, not in the middle of the floor.
						score = c[1] * c[1] + c[2] * c[2]
					else
						score = math.huge
						for _, q in ipairs(picked) do
							local dx, dz = c[1] - q[1], c[2] - q[2]
							score = math.min(score, dx * dx + dz * dz)
						end
					end
					if score > bestScore then
						bestI, bestScore = i, score
					end
				end
				table.insert(picked, table.remove(pool, bestI))
			end
			if #picked < count then
				warn(`[Galleria] {name}: {#picked} of {count} marker spots clear`)
			end
			return picked
		end

		-- Two relics a floor and one extra Sweeper a floor, all off one spread
		-- so a relic and the robot standing over it are never handed the same
		-- coordinate. Relics come first: a store short of clear floor loses a
		-- Sweeper post before it loses a reason to go in.
		--
		-- Two a floor is the count the old hand-placed version intended and
		-- never delivered. Three was tried and overshot: with the upper
		-- concourse caches the mall came out 28 relics against 20 of everything
		-- else, and data/loot.json calls relics "rare caches" - a relic you
		-- trip over is not worth lighting a lantern for, and lighting the
		-- lantern is the whole point of them.
		local marks = spreadSpots(6)
		for lvl = 0, 1 do
			local y = lvl * F2F
			for i = 1, 2 do
				local m = marks[(lvl * 2) + i]
				if m then
					local wx, wz = at(m[1], m[2])
					local cache = part(
						F.Gameplay,
						`Cache_{name}_{lvl}_{i}`,
						Vector3.new(2.6, 1.25, 2.1),
						CFrame.new(wx * S, y + 3, wz * S) * rot,
						C.teal,
						Enum.Material.Metal
					)
					cache.CanCollide = false
					cache:SetAttribute("Tier", "Relics")
					cache:SetAttribute("PlacementTier", "Relics")
					CollectionService:AddTag(cache, "Loot")
				end
			end

			-- One Sweeper a floor, deep in the back but off the rear door
			-- aisle: that door is the one the extraction picker may choose, and
			-- a robot standing in it is a wall.
			local px, pz = at(w / 4, d / 2 - 2.2)
			local post = part(
				F.Gameplay,
				`SweeperSpawn_{name}_{lvl}`,
				Vector3.new(4, 4, 4),
				CFrame.new(px * S, y + 3, pz * S),
				C.teal,
				Enum.Material.Metal
			)
			post.CanCollide, post.Transparency = false, 1
			CollectionService:AddTag(post, "SweeperSpawn")

			-- And one out on the floor, between the player and the relics,
			-- because a single robot in the stockroom is a store you can loot
			-- at your leisure. Chris: "its not threatening at all".
			local extra = marks[5 + lvl]
			if extra then
				local ex, ez = at(extra[1], extra[2])
				local inner = part(
					F.Gameplay,
					`SweeperSpawn_{name}_{lvl}_Floor`,
					Vector3.new(4, 4, 4),
					CFrame.new(ex * S, y + 3, ez * S),
					C.teal,
					Enum.Material.Metal
				)
				inner.CanCollide, inner.Transparency = false, 1
				CollectionService:AddTag(inner, "SweeperSpawn")
			end
		end

		-- Figures standing on the shop floor, not only behind the windows. The
		-- Window Walkers were display mannequins, so a department store with
		-- none in it is missing its dressing and its threat at once.
		for i = 0, 5 do
			local lx = -w / 2 + w * ((i % 3 + 0.5) / 3)
			local lz = -d / 2 + d * ((math.floor(i / 3) + 0.35) / 2)
			if
				not onAisle(lx, lz)
				and not (math.abs(lx) < vw / 2 + 1 and math.abs(lz) < vd / 2 + 1)
			then
				local wx, wz = at(lx, lz)
				mannequin(F.Fixtures, `Dummy_{name}_{i}`, wx * S, wz * S, yaw + i, 0)
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
	department(
		"BOSWELL'S",
		ARMS[1].b.X - 11 / 2,
		0,
		11,
		12,
		0,
		"+X",
		{ 0 },
		{ -(HALL + WELL) / 2, (HALL + WELL) / 2 }
	)

	-- Sears closes the south-east arm the same way, squared to it.
	local sears = ARMS[2].b + se * (12 / 2)
	department(
		"SAYERS",
		sears.X,
		sears.Y,
		12,
		11,
		seAng,
		"-X",
		{ 0 },
		{ -(HALL + WELL) / 2, (HALL + WELL) / 2 }
	)

	-- JCPenney and Bon-Ton both sit on the NORTH side, on opposite arms, with
	-- the bend between them. Three earlier versions had one or both on the
	-- south, which is what Chris kept seeing as wrong: on the aerial there is
	-- open ground between them right at the court, and putting them on the same
	-- flank closed that gap up.
	-- These stay mid-arm. They were pushed out once, to clear the extraction
	-- picker's "at least 70% as far from the start as the farthest candidate"
	-- filter, and it was the wrong move twice over: it only got them from 398
	-- and 349 studs to 436 and 376 against a 464 cutoff, so they still did not
	-- qualify, and it put Bellevue close enough to Sayers to break sixteen
	-- relic-to-exit routes. An exit that is nearly as far away as the arm end
	-- is not a nearer exit anyway. The filter is the thing that was wrong, and
	-- it is fixed where it lives, in LootService.
	department("PENHURST", -8, -(EDGE + 10 / 2), 12, 10, 0, "+Z", { -3, 3 }, { -3, 3 })

	local bt = BEND + se * 7 + seNE * (EDGE + 9 / 2)
	department("BELLEVUE", bt.X, bt.Y, 10, 9, seAng, "+Z", { 0 }, { 0 })

	-- ------------------------------------------------------------- gameplay
	-- Everything a run needs, emitted by the mall the run actually happens in.
	-- All of this used to come from FloodedMallBuilder: the spawn, the loot,
	-- the Sweeper posts, the relay and the extractions lived in Codex's
	-- generated mall, so launching a raid dropped you into that mall whatever
	-- the crew was meant to be raiding. Chris: "starting the raid still sends
	-- me to codex's old mall". Deriving these from this builder's own arms is
	-- what lets the old one go.
	do
		local G = F.Gameplay

		local function marker(name: string, mx: number, mz: number, y: number): Part
			local p = part(
				G,
				name,
				Vector3.new(4, 4, 4),
				CFrame.new(mx * S, y, mz * S),
				C.teal,
				Enum.Material.Metal
			)
			p.CanCollide, p.Transparency = false, 0.55
			return p
		end

		-- Spawn in the court rather than at a door: the crew arrives where
		-- every arm is in sight and has to choose a direction.
		local spawn = Instance.new("SpawnLocation")
		spawn.Name = "GalleriaSpawn"
		spawn.Size = Vector3.new(12, 1, 12)
		spawn.CFrame = CFrame.new(0, 1.5, 2.2 * S)
		spawn.Anchored, spawn.Neutral, spawn.Transparency = true, true, 0.55
		-- Not an active spawn point. onLaunch teleports the crew here itself,
		-- and leaving this enabled let Roblox choose between the mall and the
		-- hub at random when a character respawned.
		spawn.Enabled = false
		spawn.Parent = G
		-- The extraction picker measures "far from the start" from a part
		-- tagged RunStart, and only falls back to hunting Workspace for the
		-- first SpawnLocation it happens to find if nothing carries the tag.
		-- Lanternhold's HallSpawn is also a SpawnLocation in Workspace and sits
		-- six thousand studs away, so which one that search landed on decided
		-- whether the picker measured from the mall or from the hub - and from
		-- the hub every exit is the same distance away and the filter means
		-- nothing. Saying it here costs one tag and removes the coin flip.
		CollectionService:AddTag(spawn, "RunStart")

		-- The relay deliberately is not in the court. A centre-court bank is
		-- something a crew trips over while taking the efficient route; banking
		-- then becomes free insurance rather than a choice. This one is upstairs
		-- on the west balcony: visible from the court, but a real trip up an
		-- escalator and off the direct route to either arm-ending extraction.
		--
		-- These attributes are map contract, not bank logic. Gameplay owns the
		-- transfer: it must require a lit lantern for its whole hold and bank all
		-- nearby Lightkeepers together. Keeping the intent on the physical relay
		-- prevents a later prompt rewrite from quietly turning it back into a
		-- solo, instant safe-room action.
		local westArm = ARMS[1]
		local westDirection = (westArm.b - westArm.a).Unit
		local westNormal = Vector2.new(-westDirection.Y, westDirection.X)
		local westMidpoint = (westArm.a + westDirection * COURT_R + westArm.b) / 2
		local relayPoint = westMidpoint + westNormal * ((HALL + WELL) / 2)
		local relay = part(
			G,
			"RelayBank_WestBalcony",
			Vector3.new(5, 4, 5),
			CFrame.new(relayPoint.X * S, F2F + 2, relayPoint.Y * S),
			C.teal,
			Enum.Material.Metal
		)
		relay:SetAttribute("InteractionLabel", "Bank Haul — Lanterns Required")
		relay:SetAttribute("RequiresLanternLit", true)
		relay:SetAttribute("BanksCrewInRange", true)
		CollectionService:AddTag(relay, "RelayBank")

		local relaySign = part(
			G,
			"RelayBankSign",
			Vector3.new(22, 5, 1),
			CFrame.new(relayPoint.X * S, F2F + 10, relayPoint.Y * S),
			C.shop,
			Enum.Material.Metal
		)
		relaySign.CanCollide, relaySign.CanTouch, relaySign.CanQuery = false, false, false
		label(relaySign, "RELAY ANNEX  •  LANTERNS REQUIRED", Enum.NormalId.Front)
		label(relaySign, "RELAY ANNEX  •  LANTERNS REQUIRED", Enum.NormalId.Back)

		-- Loot down both arms, and two upstairs so the balcony is worth the
		-- stairs rather than scenery you look at from below.  Their final tier
		-- is intentionally assigned by LootService once tonight's active exit
		-- is selected.  Do not hand-place "rare" near one fixed door: either
		-- endpoint can be the way home on a given raid.
		--
		-- So the names say WHERE, never WHAT. They used to say what, and a run
		-- that picked Boswell's as the exit left a part called
		-- "Relic_SayersEnd" holding RestorationCrates and "Relic_BoswellsEnd"
		-- holding Supplies - correct behaviour, unreadable in the explorer.
		-- The tier in this table is only the fallback for a run with no active
		-- extraction at all.
		-- Generated along the arms rather than hand-listed. The previous mall
		-- began with only 29 tagged finds across a 1,983-by-1,344 footprint, and
		-- runtime validation dropped more. It looked and played empty. Compact
		-- spacing here guarantees something to discover during a push while the
		-- final collision check still rejects markers inside kiosks or scenery.
		--
		-- Offsets are the walking lane, not the centreline: the middle of a
		-- concourse carries the kiosks, planters and benches. Upstairs they sit
		-- at 1.15 units off centre, on the balcony, because the centreline at
		-- that height is the well the escalators climb through.
		local placements = {}
		for ai, arm in ipairs(ARMS) do
			local dir = (arm.b - arm.a).Unit
			local nrm = Vector2.new(-dir.Y, dir.X)
			local len = (arm.b - arm.a).Magnitude
			local k, t = 0, COURT_R + 2.2
			while t < len - 1.6 do
				local side = if k % 2 == 0 then 1 else -1
				local gp = arm.a + dir * t + nrm * (0.95 * side)
				table.insert(placements, { `Cache_A{ai}_{k}`, gp.X, gp.Y, 3, "Components" })
				if k % 2 == 0 then
					local up = arm.a + dir * t + nrm * (1.15 * -side)
					-- A relic must be encountered during an ordinary route, not be
					-- hidden exclusively in a department-store back room. The lantern
					-- gate still controls whether it can be taken; this makes that
					-- decision visible on the first run.
					local upperTier = if k % 4 == 0 then "Relics" else "Supplies"
					table.insert(
						placements,
						{ `Cache_A{ai}_{k}_Up`, up.X, up.Y, F2F + 3, upperTier }
					)
				end
				t += 1.9
				k += 1
			end
		end
		for _, d in ipairs(placements) do
			local loot = marker(d[1] :: string, d[2] :: number, d[3] :: number, d[4] :: number)
			loot.Size = Vector3.new(2.6, 1.25, 2.1)
			loot.Material, loot.Transparency, loot.CanCollide = Enum.Material.Metal, 0, false
			loot:SetAttribute("Tier", d[5] :: string) -- preview/default before a run ranks it
			loot:SetAttribute("PlacementTier", d[5] :: string)
			CollectionService:AddTag(loot, "Loot")
		end

		-- A spawn point is a coordinate, not a prop. These used the same visible
		-- marker as the loot and the Sweepers were arriving inside a translucent
		-- teal cube - Chris: "did they spawn in little boxes? lol". The loot
		-- markers stay visible because you are meant to find and press them;
		-- nobody is meant to see one of these.
		-- Also off the centreline, and for the same reason the loot is: the
		-- middle of the concourse is where the kiosks stand, and SweeperSpawn_4
		-- was inside Kiosk2_14. A Sweeper that spawns inside a kiosk is stuck
		-- in it. Offsets are along each arm's normal, 0.9 units into the
		-- walking lane.
		-- Sparse posts made the mall read as a museum: in the live build sixteen
		-- Sweepers were dispersed over nearly two million square studs. Posts
		-- are closer now, but alternate lanes so an encounter is a navigation
		-- problem rather than a wall across the route.
		local posts = 0
		for ai, arm in ipairs(ARMS) do
			local dir = (arm.b - arm.a).Unit
			local nrm = Vector2.new(-dir.Y, dir.X)
			local len = (arm.b - arm.a).Magnitude
			local k, t = 0, COURT_R + 3.6
			while t < len - 2 do
				local side = if k % 2 == 0 then -1 else 1
				local p = arm.a + dir * t + nrm * (1.0 * side)
				posts += 1
				local post = marker(`SweeperSpawn_{ai}_{k}`, p.X, p.Y, 3)
				post.Transparency = 1
				CollectionService:AddTag(post, "SweeperSpawn")
				t += 2.8
				k += 1
			end
		end

		-- All four anchor-store back doors keep ExtractCandidate, but only two
		-- can be real extractions: data/loot.json defines LoadingDock and
		-- FreightLift and nothing else, and LootService.beginRun drops any
		-- candidate whose ExtractionId is not in that config - tag a door
		-- "Exit_BOSWELL'S" and it is silently never selectable. Balance data is
		-- Chris's call, so the map bends to it rather than the other way round.
		-- The two arm-closing anchors take them, which puts the pair as far
		-- apart as the plan allows; beginRun then picks whichever lies far from
		-- the spawn.
		-- Four doors, not two. Chris: "calling an extract gives no time to make
		-- it there". With only the two arm-closing anchors carrying ids, the
		-- active exit was always at the end of an arm, so the worst case was
		-- the full dogleg - 1,367 studs, which is 59 seconds sprinting and 102
		-- laden against the twenty and thirty second windows in data/loot.json.
		-- Penhurst and Bellevue sit mid-arm on the north flank, one per arm,
		-- and halve that.
		--
		-- The ids are reused rather than invented: data/loot.json defines
		-- LoadingDock and FreightLift and nothing else, and beginRun silently
		-- drops a candidate whose id is not in that config. Balance data is
		-- Chris's call, so the map bends to it. Each arm gets one of each, so
		-- whichever end the picker chooses the run home is sometimes the
		-- reliable dock and sometimes the breaker-gated lift, rather than one
		-- character per direction.
		local EXITS = {
			["BOSWELL'S"] = "LoadingDock",
			["PENHURST"] = "FreightLift",
			["BELLEVUE"] = "LoadingDock",
			["SAYERS"] = "FreightLift",
		}
		for _, p in ipairs(F.Anchors:GetChildren()) do
			if p:IsA("BasePart") and string.sub(p.Name, 1, 5) == "Exit_" then
				local id = EXITS[string.sub(p.Name, 6)]
				if id then
					p:SetAttribute("ExtractionId", id)
					if id == "FreightLift" then
						p:SetAttribute("RequiresBreaker", true)
					end
					CollectionService:AddTag(p, "Extraction")
				end
			end
		end
	end

	M.Parent = map

	-- Every generated cache and Sweeper post, checked against the world it
	-- actually landed in. Generating positions from the arm geometry is right,
	-- but the arms carry kiosks, planters, columns and escalator landings, and
	-- a marker inside one of those is loot you cannot reach or a Sweeper that
	-- spawns stuck. Hand-placed ones had both faults twice over, so nothing is
	-- trusted.
	--
	-- A failing marker is now NUDGED before it is given up on. Dropping it
	-- outright was throwing away density the generators had correctly asked
	-- for: in the live build nine markers died here, six of them caches, and
	-- the mall Chris walked was the one left over - "the mall is empty as far
	-- as enemies and components and stuff". A planter is a couple of studs
	-- wide and the concourse either side of it is clear floor, so the right
	-- answer to a cache inside one is to slide it out, not to delete it. Only
	-- a marker with nowhere clear within ten studs is dropped.
	do
		local params = RaycastParams.new()
		params.FilterType = Enum.RaycastFilterType.Exclude
		params.FilterDescendantsInstances = { F.Gameplay }

		-- Named `point` rather than `at`, because `at` is the plan-to-world
		-- helper every store uses and shadowing it here reads as a bug.
		local function standsClear(point: Vector3): boolean
			local floor =
				workspace:Raycast(point + Vector3.new(0, 6, 0), Vector3.new(0, -22, 0), params)
			if floor == nil or (point.Y - floor.Position.Y) >= 8 then
				return false
			end
			for _, other in ipairs(workspace:GetPartBoundsInRadius(point, 2.4)) do
				if other.CanCollide and other:IsDescendantOf(M) and other.Parent ~= F.Gameplay then
					return false
				end
			end
			return true
		end

		-- Tried in order, so a marker moves as little as it has to. Eight
		-- bearings at four studs, then the same eight at eight: far enough to
		-- clear a planter or a column, near enough that the marker is still
		-- where the generator meant it to be.
		local nudges: { Vector3 } = {}
		for _, radius in ipairs({ 4, 8 }) do
			for step = 0, 7 do
				local angle = (step / 8) * math.pi * 2
				table.insert(
					nudges,
					Vector3.new(math.cos(angle) * radius, 0, math.sin(angle) * radius)
				)
			end
		end

		local dropped, kept, moved = 0, 0, 0
		for _, marker in ipairs(F.Gameplay:GetChildren()) do
			local tagged = CollectionService:HasTag(marker, "Loot")
				or CollectionService:HasTag(marker, "SweeperSpawn")
			if marker:IsA("BasePart") and tagged then
				if standsClear(marker.Position) then
					kept += 1
				else
					local rescued = false
					for _, offset in ipairs(nudges) do
						local candidate = marker.Position + offset
						if standsClear(candidate) then
							marker.CFrame = marker.CFrame + offset
							kept += 1
							moved += 1
							rescued = true
							break
						end
					end
					if not rescued then
						dropped += 1
						marker:Destroy()
					end
				end
			end
		end
		M:SetAttribute("MarkersKept", kept)
		M:SetAttribute("MarkersDropped", dropped)
		M:SetAttribute("MarkersNudged", moved)
	end

	-- A landing rail is there because the far end of a landing is normally a
	-- two storey drop into the well. On the south-east arm it is not: the food
	-- court deck butts straight onto the landing, so the rail stood as a wall
	-- across the only way in and pathfinding could not reach the food court at
	-- all. Decide it from the world instead of from a span the builder would
	-- have to keep in step by hand - a rail with solid floor on BOTH sides at
	-- its own height is not guarding anything.
	do
		local params = RaycastParams.new()
		params.FilterType = Enum.RaycastFilterType.Exclude
		params.FilterDescendantsInstances = { F.Structure, F.Fixtures, F.Gameplay, F.Escalators }
		local removed = 0
		for _, rail in ipairs(F.Structure:GetChildren()) do
			if rail:IsA("BasePart") and string.sub(rail.Name, 1, 11) == "LandingRail" then
				local solid = 0
				for s = -1, 1, 2 do
					local probe = (rail.CFrame * CFrame.new(s * 6, 0, 0)).Position
					local hit = workspace:Raycast(
						probe + Vector3.new(0, 7, 0),
						Vector3.new(0, -14, 0),
						params
					)
					if hit and math.abs(hit.Position.Y - F2F) < 1.5 then
						solid += 1
					end
				end
				if solid == 2 then
					local cap = F.Structure:FindFirstChild(`LandingCap{string.sub(rail.Name, 12)}`)
					if cap then
						cap:Destroy()
					end
					rail:Destroy()
					removed += 1
				end
			end
		end
	end

	-- Nothing stands on an escalator landing.
	--
	-- Measured, not guessed: all four escalators on the angled south-east arm
	-- were impassable, while the four identical ones on the west arm were
	-- fine. The food court deck sits on the upper south-east arm, and its
	-- tables, seats, pedestals and front rail had been laid straight over both
	-- escalator heads - a probe ten studs off the top landing came down on
	-- FCBack rather than on floor. Pathfinding would not use those runs at
	-- all, so getting off the upper south-east arm meant walking back to the
	-- court, down an escalator by Bellevue, and out along the whole arm again:
	-- 1,094 studs walked for a 416 stud straight line. That is most of Chris's
	-- "calling an extract gives no time to make it there", and the whole of
	-- the older "clipping on escalators, i cant get through on the top
	-- levels".
	--
	-- The fix is a rule rather than a fix to the food court, because the food
	-- court is only the thing that happened to land there first. The landing
	-- zones come off each deck's own geometry, so an escalator that moves takes
	-- its clearance with it, and the check below proves every run is walkable
	-- before the mall ships.
	do
		-- The zone is the WHOLE run plus a landing at each end, and candidates
		-- are tested on their bounds rather than their centres. Two earlier
		-- versions of this got both of those wrong and left the escalators
		-- just as sealed. Testing centres missed the food court's front rail
		-- and cap, which are single long parts whose centres sit well off the
		-- landing while their length lies across it. Clearing only beyond the
		-- ramp ends missed the rest: the seats and pedestals were standing on
		-- the upper third of the ramp itself, nineteen studs along a run whose
		-- half length is twenty five, so a walker had to climb fourteen studs
		-- onto a chair to use the escalator at all.
		--
		-- Fixtures go without question - furniture has no business on a moving
		-- stair. From Structure only the rails and caps are taken, by name,
		-- because that folder also holds the soffits and columns holding the
		-- building up and a blanket sweep here would quietly punch holes in it.
		local params = OverlapParams.new()
		params.FilterType = Enum.RaycastFilterType.Include
		params.FilterDescendantsInstances = { F.Fixtures, F.Structure }
		params.MaxParts = 500

		local STRUCTURE_MAY_GO = { "FCRail", "FCCap", "LandingRail", "LandingCap" }
		local function removable(item: BasePart): boolean
			if item:IsDescendantOf(F.Fixtures) then
				return true
			end
			for _, prefix in ipairs(STRUCTURE_MAY_GO) do
				if string.sub(item.Name, 1, #prefix) == prefix then
					return true
				end
			end
			return false
		end

		local cleared = 0
		for _, deck in ipairs(F.Escalators:GetChildren()) do
			if deck:IsA("BasePart") and string.sub(deck.Name, 1, 5) == "Deck_" then
				-- In the deck's own frame, so a rotated run takes its
				-- clearance with it: the full length of the ramp plus a
				-- landing's reach at each end, a shoulder either side, and
				-- headroom that starts just above the tread surface so the
				-- ramp and the floors under it are never in the zone.
				local reach = 24
				local zoneAt = deck.CFrame * CFrame.new(0, 15, 0)
				local probe = Instance.new("Part")
				probe.Size = Vector3.new(deck.Size.X + reach * 2, 26, deck.Size.Z + 8)
				probe.CFrame = zoneAt
				probe.Anchored, probe.CanCollide, probe.CanQuery = true, false, true
				probe.Transparency = 1
				probe.Parent = workspace
				for _, item in ipairs(workspace:GetPartsInPart(probe, params)) do
					if item.CanCollide and removable(item) then
						item:Destroy()
						cleared += 1
					end
				end
				probe:Destroy()
			end
		end
		M:SetAttribute("LandingsCleared", cleared)

		-- And prove it, with the test that actually found the fault. An earlier
		-- proof asked only whether a path existed from one end of the ramp to
		-- the other, and it passed on all sixteen runs while four of them were
		-- still impassable - because a path existed, the long way round. What
		-- matters is whether going THROUGH the run is the short way: the
		-- sealed arm 2 escalators measured 514 to 787 studs for an 81 stud
		-- straight line, against 86 on the identical runs on arm 1.
		local PathfindingService = game:GetService("PathfindingService")
		local sealed = 0
		for _, deck in ipairs(F.Escalators:GetChildren()) do
			if deck:IsA("BasePart") and string.sub(deck.Name, 1, 5) == "Deck_" then
				local half = deck.Size.X / 2
				-- A point out on the floor past each end, dropped onto
				-- whatever surface is really there rather than assumed.
				local function standing(offset: number): Vector3
					local raw = (deck.CFrame * CFrame.new(offset, 6, 0)).Position
					local hit =
						workspace:Raycast(raw + Vector3.new(0, 14, 0), Vector3.new(0, -40, 0))
					return if hit then hit.Position + Vector3.new(0, 3, 0) else raw
				end
				local a, b = standing(half + 22), standing(-(half + 22))
				local path = PathfindingService:CreatePath({
					AgentRadius = 2.5,
					AgentHeight = 5,
					AgentCanJump = false,
				})
				local ok = pcall(function()
					path:ComputeAsync(a, b)
				end)
				local walked = 0
				if ok and path.Status == Enum.PathStatus.Success then
					local waypoints = path:GetWaypoints()
					for i = 2, #waypoints do
						walked += (waypoints[i].Position - waypoints[i - 1].Position).Magnitude
					end
				end
				local straight = (a - b).Magnitude
				-- Twice the straight line is generous for a stair you walk
				-- straight down. Anything past it is a detour, which means the
				-- run is not being used.
				if walked == 0 or walked > straight * 2 then
					sealed += 1
					warn(
						`[Galleria] escalator {deck.Name} is not the way through: {math.floor(
							walked
						)} studs walked for {math.floor(straight)} straight`
					)
				end
			end
		end
		M:SetAttribute("EscalatorsSealed", sealed)
	end

	RaidDirector.Apply(M)
	return M
end

return GalleriaBuilder
