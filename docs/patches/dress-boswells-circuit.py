# The dressing pass for LLK_Boswells_FirstRun, as shipped in d399e40.
#
# Banked here on 2026-11-04 because it lived only in a session-specific
# scratchpad, and this is the re-runnable half of that commit: it wipes every
# actor whose label starts with DRESS_ before placing anything, so it can be
# re-run against a changed circuit without duplicating, and it never touches
# an actor authored by hand.
#
# That property is not decoration. slice.py, the pass before this one, was not
# idempotent and ran twice: it squared a scale factor (0.838 = 0.9153^2) and
# left a duplicate set of _N actors behind. Anything that edits this map
# should be safe to run again.
#
# HOW TO RUN IT
#   1. Open the editor with Content/LLK_Boswells_FirstRun as the loaded level.
#      Scene tools answer about the OPEN level, so the wrong map open means
#      the wrong map dressed.
#   2. Run `ModelContextProtocol.StartServer` in the editor console.
#   3. Hand the whole of this file as the `script` argument to
#      ProgrammaticToolset.execute_tool_script. It defines run() and returns a
#      dict of before/after route probes, widths, and the save result.
#   4. Not during PIE. During PIE the scene tools answer about the UEDPIE_0_
#      duplicate; read freely there, write nothing.
#
# WHAT IT NEEDS
#   Content/Warehouse_Big_Pack/, which is gitignored licensed art. See
#   docs/ASSET-DEPENDENCIES.md and run `node tests/asset-deps.test.mjs` first:
#   absent exits 0 and names the Fab listing, partially present fails.
#
# WHAT THE NUMBERS MEAN
#   docs/FINDINGS-BOSWELLS-DRESSING-PASS.md. The short version: the racking is
#   106 cm deep against the cube gondolas' 65, the rack's OUTER face is flush
#   at x +/-392.5 so all 41 cm of the difference comes out of the central
#   aisle rather than the 167.5 cm side aisle, and the escape gap measures
#   200.0 cm at the shelf face and 194.1 cm at the front post line.
#
#   Only json, math, datetime, copy, re and time are importable in this
#   sandbox, and the dicts it returns are _StrictDict: .get() with a default
#   raises TypeError, so use [] with an `in` check.
#
# Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>

import json

SCENE = "editor_toolset.toolsets.scene.SceneTools"
ACTOR = "editor_toolset.toolsets.actor.ActorTools"
ASSET = "editor_toolset.toolsets.asset.AssetTools"
B = "/Game/Warehouse_Big_Pack/Meshes/"

# Everything placed here is labelled DRESS_* and wiped first, so the pass is
# re-runnable and nothing authored by hand is ever touched.
PREFIX = "DRESS_"

# Measured offstage before placing (sizes in cm, unrotated):
#   upright   9.0 x 106.0 x 250.0      beam35  349.5 x 106.4 x 19.3 (base z -19.1)
#   deck    113.0 x 106.3 x   3.4      lampB    17.7 x 131.0 x 69.1 (base z -69.1)
#   dockopen 400 x 62 x 463.8 (base z -150)
#
# Racking runs along y here, so every rack piece takes yaw 90: its local x
# becomes world y and its 106 cm depth becomes world x.
#
# The depth is the one number that governs this pass. The cube gondolas are
# 65 deep (x -392.5..-327.5); real racking is 106. Putting the rack's OUTER
# face flush at x -392.5 leaves the 167.5 cm side aisle exactly as measured
# and pushes the extra 41 cm into the centre aisle, which has it to spare.
RACK_OUTER = 392.5
RACK_DEPTH = 106.0
RACK_CX = RACK_OUTER - RACK_DEPTH / 2.0      # 339.5
SEG_CY = 275.0                               # segment centres at y -275 / +275
SEG_HALF = 175.0

def spawn(asset, label, loc, yaw=0.0, scale=(1.0, 1.0, 1.0)):
    a = execute_tool(SCENE + ".add_to_scene_from_asset", json.dumps({
        "asset_path": asset, "name": label,
        "xform": {"location": {"x": loc[0], "y": loc[1], "z": loc[2]},
                  "rotation": {"pitch": 0.0, "yaw": yaw, "roll": 0.0},
                  "scale": {"x": scale[0], "y": scale[1], "z": scale[2]}}}))["returnValue"]
    execute_tool(ACTOR + ".set_label", json.dumps({"actor": a, "label": label}))
    return a

def shot(s, e):
    return execute_tool(SCENE + ".trace_world", json.dumps({
        "start": {"x": s[0], "y": s[1], "z": s[2]},
        "end": {"x": e[0], "y": e[1], "z": e[2]}}))["returnValue"]

def clear(s, e):
    d = shot(s, e)
    return "clear" if d is None else ("blocked at %.1f" % d)

def route():
    return {
      "escape west through the gap":   clear([0, 0, 120], [-500, 0, 120]),
      "escape east through the gap":   clear([0, 0, 120], [500, 0, 120]),
      "centre aisle west face y -300": clear([0, -300, 120], [-500, -300, 120]),
      "centre aisle east face y -300": clear([0, -300, 120], [500, -300, 120]),
      "side aisle W along its run":    clear([-470, -400, 120], [-470, 450, 120]),
      "crossing W into toys y 430":    clear([-400, 430, 120], [-1300, 430, 120]),
      "crossing E to counter y 350":   clear([400, 350, 120], [1300, 350, 120]),
      "spawn -> dock sightline":       clear([0, -750, 160], [0, 780, 160]),
      "patrol A->C along x 0":         clear([0, -200, 120], [0, 650, 120]),
      "centre aisle -> dock mouth":    clear([0, 450, 120], [0, 760, 120]),
      "gap clear at head height W":    clear([-360, -90, 255], [-360, 90, 255]),
    }

def widths():
    out = {}
    for y in (-300, -200, 300, 400):
        w = shot([0, y, 120], [-600, y, 120])
        e = shot([0, y, 120], [600, y, 120])
        out["centre aisle at y %d" % y] = (
            None if (w is None or e is None) else round(w + e, 1))
    for y in (-300, 300):
        inn = shot([-470, y, 120], [-200, y, 120])
        out_ = shot([-470, y, 120], [-800, y, 120])
        out["side aisle W at y %d" % y] = (
            None if (inn is None or out_ is None) else round(inn + out_, 1))
    # The authored 200 cm escape gap, measured along y at the rack face.
    for x in (-300, 300):
        s = shot([x, -400, 120], [x, 400, 120])
        out["gap edge from south at x %d" % x] = None if s is None else round(-400 + s, 1)
    return out

def run():
    before_route = route()
    before_widths = widths()

    # ---- wipe anything this pass placed previously --------------------------
    wiped = 0
    for a in execute_tool(SCENE + ".find_actors",
                          json.dumps({"name": "", "tag": "", "collision_channels": []}))["returnValue"]:
        lb = execute_tool(ACTOR + ".get_label", json.dumps({"actor": a}))["returnValue"]
        if lb.startswith(PREFIX):
            execute_tool(SCENE + ".remove_from_scene", json.dumps({"actor": a}))
            wiped += 1

    placed = []

    # ---- 1. racking over the four gondola segments -------------------------
    for side, sx in (("W", -1.0), ("E", 1.0)):
        for seg, sy in (("S", -1.0), ("N", 1.0)):
            cx = sx * RACK_CX
            cy = sy * SEG_CY
            tag = "%s%s" % (side, seg)
            # uprights at the two ends of the 350 run
            for i, dy in enumerate((-SEG_HALF, SEG_HALF)):
                placed.append(spawn(B + "Racking/SM_Upright_Frame",
                                    PREFIX + "Rack_Upright_%s_%d" % (tag, i),
                                    (cx, cy + dy, 0.0), yaw=90.0))
            # load beams at two shelf heights
            for i, z in enumerate((80.0, 170.0)):
                placed.append(spawn(B + "Racking/SM_Load_Beam_3_5m",
                                    PREFIX + "Rack_Beam_%s_%d" % (tag, i),
                                    (cx, cy, z), yaw=90.0))
            # decking on each level, three panels per run
            for i, dy in enumerate((-116.7, 0.0, 116.7)):
                for j, z in enumerate((80.0, 170.0)):
                    placed.append(spawn(B + "Racking/SM_Decking_Steel",
                                        PREFIX + "Rack_Deck_%s_%d_%d" % (tag, i, j),
                                        (cx, cy + dy, z), yaw=90.0))
            # a rack sign on the aisle face, at the segment's outer end
            placed.append(spawn(B + "Racking/SM_Sign",
                                PREFIX + "Rack_Sign_%s" % tag,
                                (cx - sx * (RACK_DEPTH / 2.0), cy + sy * SEG_HALF * 0.6, 0.0),
                                yaw=90.0))

    # ---- 2. unlit fixture housings over the existing lights ----------------
    # The lights stay as they are and keep doing the lighting; these are the
    # bodies that let the player read where light comes from. Hung so the
    # housing's underside sits just below the 586 ceiling, with the authored
    # spot at z 560 inside it.
    for i, x in enumerate((-900.0, -300.0, 300.0, 900.0)):
        placed.append(spawn(B + "Lamps/SM_Lamp_Indoor_B",
                            PREFIX + "Fixture_Aisle_%d" % (i + 1), (x, 0.0, 586.0)))
    for i, (x, y) in enumerate(((-650.0, -250.0), (100.0, 0.0), (850.0, -250.0))):
        placed.append(spawn(B + "Lamps/SM_Lamp_Indoor_C",
                            PREFIX + "Fixture_Emergency_%d" % (i + 1), (x, y, 586.0)))

    # ---- 3. the dock, which has never been reached -------------------------
    # 400 wide, 464 tall, modelled 150 below its origin, so z 150 puts its
    # base on the floor. Set into the north wall at y 894.
    placed.append(spawn(B + "Dock/SM_Dock_Open", PREFIX + "Dock_Opening",
                        (0.0, 888.0, 150.0)))
    for i, x in enumerate((-170.0, 170.0)):
        placed.append(spawn(B + "Dock/SM_Wheel_Stopper_2m",
                            PREFIX + "Dock_Stopper_%d" % (i + 1), (x, 700.0, 0.0)))
    placed.append(spawn(B + "Dock/SM_Parking_Pole", PREFIX + "Dock_Pole_1", (-240.0, 640.0, 0.0)))
    placed.append(spawn(B + "Dock/SM_Parking_Pole", PREFIX + "Dock_Pole_2", (240.0, 640.0, 0.0)))
    placed.append(spawn(B + "Handling_Equipment/SM_Pallet_Truck",
                        PREFIX + "Dock_PalletTruck", (210.0, 780.0, 0.0), yaw=-20.0))

    # ---- 4. crates and pallet dressing, kept out of the route --------------
    DRESS = [
      ("Crates/SM_Crate_110x80x50cm", "Crate_StockBay_1", (470.0, 300.0, 0.0), 12.0),
      ("Crates/SM_Crate_100x120x120cm", "Crate_StockBay_2", (430.0, 200.0, 0.0), -8.0),
      ("Pallets/SM_Pallets_Pile_1", "Pallets_StockBay", (520.0, 420.0, 0.0), 0.0),
      ("Crates/SM_Crate_Stack_1", "Crate_DockSide", (-330.0, 760.0, 0.0), 0.0),
      ("Pallets/SM_Wooden_Pallet", "Pallet_Dock", (-120.0, 690.0, 0.0), 25.0),
      ("Handling_Equipment/SM_Cart_Boxes_1", "Cart_ToyAisle", (-1050.0, 560.0, 0.0), 40.0),
      ("Cardboard_Boxes/SM_Box_Cardboard_Pile_1", "Boxes_BackWall", (-1150.0, -700.0, 0.0), 0.0),
      ("Crates/SM_Crate_50x60x50cm", "Crate_Endcap_W", (-360.0, 620.0, 100.0), 0.0),
      ("Crates/SM_Crate_50x60x50cm", "Crate_Endcap_E", (360.0, 620.0, 100.0), 0.0),
      ("Handling_Equipment/SM_Hand_Trolley", "Trolley_Service", (1020.0, 300.0, 0.0), 90.0),
    ]
    for asset, name, loc, yaw in DRESS:
        placed.append(spawn(B + asset, PREFIX + name, loc, yaw=yaw))

    after_route = route()
    after_widths = widths()
    saved = execute_tool(ASSET + ".save_assets",
                         json.dumps({"asset_paths": ["/Game/LLK_Boswells_FirstRun"]}))["returnValue"]

    return {"wiped": wiped, "placed": len(placed),
            "route_before": before_route, "route_after": after_route,
            "widths_before": before_widths, "widths_after": after_widths,
            "saved": saved}
