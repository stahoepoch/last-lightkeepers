--!strict
-- Flooded Mall vertical-slice builder.
-- Run from Studio's command bar after Rojo sync:
--   require(workspace.Map.FloodedMallBuilder).Build()
-- The generated model is intentionally isolated in Workspace.Map.GeneratedFloodedMall
-- so rebuilding never deletes hand-authored map work.

local CollectionService = game:GetService("CollectionService")
local Lighting = game:GetService("Lighting")
local Workspace = game:GetService("Workspace")

local FloodedMallBuilder = {}

local MAP_NAME = "GeneratedFloodedMall"

local Colors = {
	concrete = Color3.fromRGB(51, 57, 62),
	darkConcrete = Color3.fromRGB(25, 29, 35),
	tealSteel = Color3.fromRGB(32, 107, 112),
	galleryWhite = Color3.fromRGB(204, 211, 206),
	glass = Color3.fromRGB(130, 196, 213),
	stormBlue = Color3.fromRGB(42, 123, 255),
	stormDeep = Color3.fromRGB(13, 35, 75),
	amber = Color3.fromRGB(255, 154, 48),
	planter = Color3.fromRGB(54, 72, 55),
	foliage = Color3.fromRGB(79, 119, 82),
	storeDark = Color3.fromRGB(32, 35, 42),
	storeAccent = Color3.fromRGB(90, 65, 50),
}

local function part(
	parent: Instance,
	name: string,
	size: Vector3,
	cframe: CFrame,
	color: Color3,
	material: Enum.Material
): Part
	local instance = Instance.new("Part")
	instance.Name = name
	instance.Anchored = true
	instance.Size = size
	instance.CFrame = cframe
	instance.Color = color
	instance.Material = material
	instance.TopSurface = Enum.SurfaceType.Smooth
	instance.BottomSurface = Enum.SurfaceType.Smooth
	instance.Parent = parent
	return instance
end

local function invisibleMarker(parent: Instance, name: string, position: Vector3): Part
	local marker =
		part(parent, name, Vector3.new(3, 3, 3), CFrame.new(position), Color3.new(1, 1, 1), Enum.Material.SmoothPlastic)
	marker.Transparency = 1
	marker.CanCollide = false
	marker.CanTouch = false
	marker.CanQuery = false
	return marker
end

local function sign(parent: Instance, label: string, position: Vector3, facing: CFrame, width: number, color: Color3)
	local board = part(
		parent,
		"Sign_" .. label:gsub("[^%w]", ""),
		Vector3.new(width, 3, 0.35),
		facing + position,
		color,
		Enum.Material.Neon
	)
	board.CanCollide = false
	local gui = Instance.new("SurfaceGui")
	gui.Face = Enum.NormalId.Front
	gui.AlwaysOnTop = true
	gui.SizingMode = Enum.SurfaceGuiSizingMode.PixelsPerStud
	gui.PixelsPerStud = 36
	gui.Parent = board
	local text = Instance.new("TextLabel")
	text.Size = UDim2.fromScale(1, 1)
	text.BackgroundTransparency = 1
	text.Text = label
	text.Font = Enum.Font.GothamBold
	text.TextColor3 = Color3.fromRGB(235, 239, 235)
	text.TextScaled = true
	text.Parent = gui
end

local function pointLight(parent: Instance, color: Color3, brightness: number, range: number)
	local light = Instance.new("PointLight")
	light.Color = color
	light.Brightness = brightness
	light.Range = range
	light.Shadows = true
	light.Parent = parent
end

local function truss(parent: Instance, x: number)
	-- A tall teal portal frame: a direct visual cue for the glass-gabled Galleria.
	for _, side in ipairs({ -1, 1 }) do
		part(
			parent,
			"TealTruss",
			Vector3.new(2, 34, 2),
			CFrame.new(x, 20, side * 55),
			Colors.tealSteel,
			Enum.Material.Metal
		)
	end
	local left = Vector3.new(x, 37, -55)
	local apex = Vector3.new(x, 51, 0)
	local right = Vector3.new(x, 37, 55)
	for _, pair in ipairs({ { left, apex }, { apex, right } }) do
		local a = pair[1] :: Vector3
		local b = pair[2] :: Vector3
		local length = (a - b).Magnitude
		part(
			parent,
			"TealGable",
			Vector3.new(2, 2, length),
			CFrame.lookAt((a + b) / 2, b),
			Colors.tealSteel,
			Enum.Material.Metal
		)
	end
end

local function storefront(
	parent: Instance,
	name: string,
	position: Vector3,
	width: number,
	depth: number,
	facing: number
)
	local shell = Instance.new("Model")
	shell.Name = name
	shell.Parent = parent
	local cframe = CFrame.new(position) * CFrame.Angles(0, math.rad(facing), 0)
	part(
		shell,
		"Floor",
		Vector3.new(width, 1, depth),
		cframe * CFrame.new(0, 0, -depth / 2),
		Colors.storeDark,
		Enum.Material.Concrete
	)
	part(
		shell,
		"BackWall",
		Vector3.new(width, 12, 1),
		cframe * CFrame.new(0, 6, -depth),
		Colors.storeDark,
		Enum.Material.Concrete
	)
	part(
		shell,
		"LeftWall",
		Vector3.new(1, 12, depth),
		cframe * CFrame.new(-width / 2, 6, -depth / 2),
		Colors.storeDark,
		Enum.Material.Concrete
	)
	part(
		shell,
		"RightWall",
		Vector3.new(1, 12, depth),
		cframe * CFrame.new(width / 2, 6, -depth / 2),
		Colors.storeDark,
		Enum.Material.Concrete
	)
	local awning = part(
		shell,
		"ClosedShutter",
		Vector3.new(width - 2, 5, 0.5),
		cframe * CFrame.new(0, 5, -0.1),
		Colors.storeAccent,
		Enum.Material.Metal
	)
	awning.CanCollide = false
	sign(
		shell,
		name:gsub("_", " "):upper(),
		position + cframe.LookVector * -0.4 + Vector3.new(0, 10, 0),
		cframe,
		math.max(8, width - 3),
		Colors.tealSteel
	)
end

local function planter(parent: Instance, name: string, position: Vector3, size: Vector3)
	local base = part(parent, name, size, CFrame.new(position), Colors.planter, Enum.Material.Slate)
	local soil = part(
		parent,
		name .. "_Planting",
		Vector3.new(size.X - 1, 0.5, size.Z - 1),
		CFrame.new(position + Vector3.new(0, size.Y / 2 + 0.25, 0)),
		Colors.foliage,
		Enum.Material.Grass
	)
	soil.CanCollide = false
	for index = 1, math.max(1, math.floor(size.X / 7)) do
		local offset = -size.X / 2 + index * (size.X / (math.floor(size.X / 7) + 1))
		local trunk = part(
			parent,
			name .. "_Tree",
			Vector3.new(1.2, 6, 1.2),
			CFrame.new(position + Vector3.new(offset, size.Y / 2 + 3.3, 0)),
			Color3.fromRGB(80, 61, 42),
			Enum.Material.Wood
		)
		trunk.CanCollide = false
		local crown = part(
			parent,
			name .. "_Canopy",
			Vector3.new(5, 5, 5),
			CFrame.new(position + Vector3.new(offset, size.Y / 2 + 7, 0)),
			Colors.foliage,
			Enum.Material.Grass
		)
		crown.Shape = Enum.PartType.Ball
		crown.CanCollide = false
	end
	return base
end

local function loot(parent: Instance, name: string, position: Vector3, tier: string, amount: number)
	local marker = invisibleMarker(parent, name, position)
	marker:SetAttribute("Tier", tier)
	marker:SetAttribute("Amount", amount)
	CollectionService:AddTag(marker, "Loot")
	return marker
end

local function addStormLighting()
	Lighting.Technology = Enum.Technology.Future
	Lighting.ClockTime = 2
	Lighting.Brightness = 1.1
	Lighting.Ambient = Color3.fromRGB(8, 11, 20)
	Lighting.OutdoorAmbient = Color3.fromRGB(10, 24, 52)
	Lighting.EnvironmentDiffuseScale = 0.3
	Lighting.EnvironmentSpecularScale = 0.65
	local atmosphere = Lighting:FindFirstChild("FloodedMallAtmosphere") or Instance.new("Atmosphere")
	atmosphere.Name = "FloodedMallAtmosphere"
	atmosphere.Color = Color3.fromRGB(102, 151, 190)
	atmosphere.Decay = Color3.fromRGB(20, 42, 89)
	atmosphere.Density = 0.28
	atmosphere.Glare = 0.15
	atmosphere.Haze = 1.8
	atmosphere.Parent = Lighting
	local bloom = Lighting:FindFirstChild("FloodedMallBloom") or Instance.new("BloomEffect")
	bloom.Name = "FloodedMallBloom"
	bloom.Intensity = 0.45
	bloom.Size = 34
	bloom.Threshold = 1.2
	bloom.Parent = Lighting
end

function FloodedMallBuilder.Build(): Model
	local map = Workspace:FindFirstChild("Map")
	if not map then
		map = Instance.new("Folder")
		map.Name = "Map"
		map.Parent = Workspace
	end
	local old = map:FindFirstChild(MAP_NAME)
	if old then
		old:Destroy()
	end

	addStormLighting()
	local mall = Instance.new("Model")
	mall.Name = MAP_NAME
	mall:SetAttribute("BuildVersion", 1)
	mall.Parent = map

	local architecture = Instance.new("Folder")
	architecture.Name = "Architecture"
	architecture.Parent = mall
	local objectives = Instance.new("Folder")
	objectives.Name = "Objectives"
	objectives.Parent = mall
	local markers = Instance.new("Folder")
	markers.Name = "GameplayMarkers"
	markers.Parent = mall

	-- A broad two-level mall: this is built for readable mobile navigation first.
	part(
		architecture,
		"MainConcourse",
		Vector3.new(250, 2, 112),
		CFrame.new(0, 0, 0),
		Colors.concrete,
		Enum.Material.Concrete
	)
	part(
		architecture,
		"UpperGalleryWest",
		Vector3.new(84, 2, 24),
		CFrame.new(-72, 14, 0),
		Colors.concrete,
		Enum.Material.Concrete
	)
	part(
		architecture,
		"UpperGalleryEast",
		Vector3.new(84, 2, 24),
		CFrame.new(72, 14, 0),
		Colors.concrete,
		Enum.Material.Concrete
	)
	part(architecture, "RoofGlass", Vector3.new(244, 1, 108), CFrame.new(0, 52, 0), Colors.glass, Enum.Material.Glass).Transparency =
		0.58

	for x = -110, 110, 22 do
		truss(architecture, x)
	end
	for _, z in ipairs({ -45, 45 }) do
		local rail = part(
			architecture,
			"UpperGalleryRail",
			Vector3.new(84, 3, 0.6),
			CFrame.new(0, 16, z),
			Colors.galleryWhite,
			Enum.Material.Metal
		)
		rail.CanCollide = true
	end

	-- The old mall's shop shell language: distinct districts, no copied businesses.
	storefront(architecture, "Beacon_Pharmacy", Vector3.new(-79, 1, -44), 26, 20, 0)
	storefront(architecture, "The_Circuit_Arcade", Vector3.new(-35, 1, -44), 26, 20, 0)
	storefront(architecture, "Northstar_Outfitters", Vector3.new(35, 1, -44), 30, 20, 0)
	storefront(architecture, "Hearth_Food_Hall", Vector3.new(82, 1, -44), 30, 20, 0)
	storefront(architecture, "Miller_Home", Vector3.new(-82, 1, 44), 30, 20, 180)
	storefront(architecture, "Floodmark_Books", Vector3.new(-38, 1, 44), 26, 20, 180)
	storefront(architecture, "Cinema_Seven", Vector3.new(40, 1, 44), 28, 20, 180)
	storefront(architecture, "Anchor_Department", Vector3.new(86, 1, 44), 30, 20, 180)

	-- Sunken Rain Garden: Galleria scale with a Richland-style indoor park as its heart.
	part(
		architecture,
		"RainGardenBasin",
		Vector3.new(62, 7, 62),
		CFrame.new(0, -3, 0),
		Colors.darkConcrete,
		Enum.Material.Concrete
	)
	local water = part(
		architecture,
		"Stormwater",
		Vector3.new(58, 0.45, 58),
		CFrame.new(0, 0.15, 0),
		Colors.stormDeep,
		Enum.Material.Glass
	)
	water.Transparency = 0.28
	water.CanCollide = false
	pointLight(water, Colors.stormBlue, 1.4, 45)
	for _, data in ipairs({
		{ Vector3.new(-25, 2, 0), Vector3.new(12, 3, 38) },
		{ Vector3.new(25, 2, 0), Vector3.new(12, 3, 38) },
		{ Vector3.new(0, 2, -24), Vector3.new(28, 3, 9) },
		{ Vector3.new(0, 2, 24), Vector3.new(28, 3, 9) },
	}) do
		planter(architecture, "RainGardenPlanter", data[1] :: Vector3, data[2] :: Vector3)
	end
	-- Ramps deliberately preserve the alternate ground route for accessibility and enemies.
	for _, x in ipairs({ -39, 39 }) do
		local ramp = part(
			architecture,
			"GardenRamp",
			Vector3.new(18, 1.2, 34),
			CFrame.new(x, -1.5, 0) * CFrame.Angles(0, 0, math.rad(if x < 0 then -10 else 10)),
			Colors.concrete,
			Enum.Material.Concrete
		)
		ramp.CanCollide = true
	end
	local waterfall = part(
		architecture,
		"StormfallWaterFeature",
		Vector3.new(15, 18, 8),
		CFrame.new(0, 7, -4),
		Colors.stormBlue,
		Enum.Material.Glass
	)
	waterfall.Transparency = 0.35
	waterfall.CanCollide = false
	pointLight(waterfall, Colors.stormBlue, 3, 38)
	local boulder = part(
		architecture,
		"StormfallBoulder",
		Vector3.new(19, 8, 14),
		CFrame.new(0, 3, 7),
		Colors.darkConcrete,
		Enum.Material.Slate
	)
	boulder.Shape = Enum.PartType.Ball

	-- Safe banking is visibly useful but exposed from the entire upper gallery.
	local relay = part(
		objectives,
		"RelayBank_RainGarden",
		Vector3.new(10, 12, 10),
		CFrame.new(0, 6, 22),
		Colors.amber,
		Enum.Material.Metal
	)
	relay:SetAttribute("InteractionLabel", "Bank Haul")
	CollectionService:AddTag(relay, "RelayBank")
	pointLight(relay, Colors.amber, 3.5, 42)
	sign(objectives, "LIGHTKEEPER RELAY", Vector3.new(0, 14, 22), CFrame.Angles(0, math.rad(180), 0), 14, Colors.amber)

	-- Reliable early exit, and a deeper exit that asks the player to commit.
	local dock = part(
		objectives,
		"Extraction_LoadingDock",
		Vector3.new(25, 1, 30),
		CFrame.new(-108, 1.5, -25),
		Colors.tealSteel,
		Enum.Material.Metal
	)
	dock:SetAttribute("ExtractionId", "LoadingDock")
	CollectionService:AddTag(dock, "Extraction")
	sign(objectives, "LOADING DOCK // EXIT", Vector3.new(-108, 10, -39), CFrame.new(), 20, Colors.tealSteel)
	local lift = part(
		objectives,
		"Extraction_FreightLift",
		Vector3.new(25, 1, 25),
		CFrame.new(106, -2.5, 25),
		Colors.storeAccent,
		Enum.Material.Metal
	)
	lift:SetAttribute("ExtractionId", "FreightLift")
	lift:SetAttribute("RequiresBreaker", true)
	CollectionService:AddTag(lift, "Extraction")
	sign(objectives, "FREIGHT LIFT // 20 SEC", Vector3.new(106, 7, 39), CFrame.new(), 19, Colors.amber)

	local spawn = Instance.new("SpawnLocation")
	spawn.Name = "FloodedMallSpawn"
	spawn.Size = Vector3.new(10, 1, 10)
	spawn.CFrame = CFrame.new(-112, 4, -16)
	spawn.Anchored = true
	spawn.Neutral = true
	spawn.Transparency = 0.2
	spawn.Color = Colors.tealSteel
	spawn.Parent = objectives

	-- Markers are intentionally visible only to services; meshes/UI can replace them later.
	loot(markers, "Supplies_Dock", Vector3.new(-94, 3, -18), "Supplies", 2)
	loot(markers, "Supplies_Pharmacy", Vector3.new(-78, 3, -26), "Supplies", 2)
	loot(markers, "Components_Pharmacy", Vector3.new(-79, 3, -35), "Components", 1)
	loot(markers, "Components_Arcade", Vector3.new(-35, 3, -30), "Components", 2)
	loot(markers, "Components_Cinema", Vector3.new(39, 3, 27), "Components", 1)
	loot(markers, "Relic_Waterfall", Vector3.new(0, 3, -11), "Relics", 1)
	loot(markers, "Relic_UpperGallery", Vector3.new(71, 17, 0), "Relics", 1)
	loot(markers, "RestorationCrate_FoodHall", Vector3.new(82, 3, -25), "RestorationCrates", 1)
	for index, position in ipairs({
		Vector3.new(-98, 3, 20),
		Vector3.new(-21, 3, 30),
		Vector3.new(24, 3, -29),
		Vector3.new(91, 3, 21),
	}) do
		local sweeper = invisibleMarker(markers, "SweeperSpawn_" .. index, position)
		CollectionService:AddTag(sweeper, "SweeperSpawn")
	end

	return mall
end

return FloodedMallBuilder
