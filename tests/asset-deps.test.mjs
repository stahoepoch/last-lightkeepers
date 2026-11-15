// Marketplace art is a dependency, not source. This says so out loud.
//
// Content/Warehouse_Big_Pack/ is gitignored: it is licensed art, it is
// re-downloadable from Fab, and 302 of its 330 MB is 4K texture sets for a
// greybox store lit at 2 candelas. Versioning it would undo most of the 19 GB
// LFS reclaim of 2026-11-03 and make every clone and CI run pull it.
//
// The risk that creates is the one `docs/HANDOFFS.md` row X3 already records
// against `Content/Data/`: a gitignored dependency goes missing on a fresh
// checkout and the thing that depends on it reports success anyway. Unreal
// does not error on a missing mesh — it renders nothing and carries on. So a
// clone without this pack opens Boswell's with invisible racking and no dock,
// and nothing says why.
//
// This check draws the distinction that matters:
//
//   pack wholly absent    -> report plainly, exit 0. A fresh clone or CI box
//                            is not broken; it just does not have the art.
//   pack partially there  -> FAIL. That is a half-finished copy or a corrupt
//                            import, and it is the case that wastes an evening.
//
//   node tests/asset-deps.test.mjs

import { existsSync, readdirSync, readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const pack = join(root, "Content", "Warehouse_Big_Pack");

// The Fab listing, so the message can name what to download rather than
// leaving the reader to guess which pack "Warehouse_Big_Pack" came from.
const SOURCE = "Warehouse Big Pack (Fab) — in Chris's library as Warehouse_Big_Pack-ff6df3b8";

// What was imported on 2026-11-03, by folder and the count copied. These are
// the folders Boswell's draws on; if one is empty while others are full, the
// copy did not finish.
const EXPECTED = [
  ["Meshes/Racking", 10],
  ["Meshes/Lamps", 9],
  ["Meshes/Dock", 10],
  ["Meshes/Crates", 17],
  ["Meshes/Cardboard_Boxes", 25],
  ["Materials", 1],
  ["Blueprints", 1],
  ["Textures", 1],
];

let failures = 0;
let checks = 0;
const check = (name, ok, detail) => {
  checks += 1;
  if (ok) console.log(`  ok   ${name}`);
  else {
    failures += 1;
    console.log(`  FAIL ${name}${detail ? ` — ${detail}` : ""}`);
  }
};

const countAssets = (rel) => {
  const dir = join(pack, rel);
  if (!existsSync(dir)) return 0;
  const walk = (d) =>
    readdirSync(d, { withFileTypes: true }).reduce(
      (n, e) =>
        n + (e.isDirectory() ? walk(join(d, e.name)) : e.name.endsWith(".uasset") ? 1 : 0),
      0,
    );
  return walk(dir);
};

if (!existsSync(pack)) {
  console.log("\nWarehouse_Big_Pack is not present.\n");
  console.log("  This is not a failure. The pack is gitignored licensed art, so a");
  console.log("  fresh clone or a CI box will not have it. Boswell's will open with");
  console.log("  its racking, lamps and dock props missing, and Unreal will not say");
  console.log("  so — it renders nothing and carries on.\n");
  console.log(`  To get it: ${SOURCE}`);
  console.log("  Then copy Content/Warehouse_Big_Pack from the vault into this");
  console.log("  project's Content/. See docs/ASSET-DEPENDENCIES.md.\n");
  console.log("0/0 checked — dependency absent, nothing to verify\n");
  process.exit(0);
}

console.log("\nWarehouse_Big_Pack is present; checking the import is complete.\n");

for (const [rel, min] of EXPECTED) {
  const n = countAssets(rel);
  check(
    `${rel} has at least ${min} asset(s)`,
    n >= min,
    n === 0
      ? `empty or missing while the rest of the pack is here — a half-finished copy`
      : `found ${n}, expected at least ${min}`,
  );
}

// ---- the two cover fixtures are the height they were measured to be -----
//
// `docs/FINDINGS-KIDS-FLOOR-STEALTH.md` measured the live editor: sight is
// traced between capsule centres, 88 cm standing and 40 cm crouched, while the
// keeper's eye is at 152. The children's rolling racks block only 88-130, so
// crouching behind one makes the keeper MORE visible.
//
// `assets/models/boswells/make_fixture_kit.py` already refuses to write a
// fixture that breaks the rule. This checks the OBJ that actually landed,
// because the generator's guarantee only covers the run that produced it and
// these files are committed art, not build output.
console.log("\nCover fixtures");
{
  const TRACE_CROUCHED_CM = 40;
  const TRACE_STANDING_CM = 88;
  const EYE_HEIGHT_CM = 152;

  const topOf = (stem) => {
    const path = join(root, "assets/models/boswells", `${stem}.obj`);
    if (!existsSync(path)) return null;
    let low = Infinity;
    let high = -Infinity;
    for (const line of readFileSync(path, "utf8").split("\n")) {
      if (!line.startsWith("v ")) continue;
      const z = Number(line.trim().split(/\s+/)[3]);
      if (!Number.isFinite(z)) continue;
      if (z < low) low = z;
      if (z > high) high = z;
    }
    return { low, high };
  };

  const table = topOf("SM_BSW_FoldedGoodsTable");
  const counter = topOf("SM_BSW_ServiceCounter");

  check(
    "both cover fixtures exist as committed source meshes",
    table !== null && counter !== null,
    "run python assets/models/boswells/make_fixture_kit.py",
  );

  if (table && counter) {
    check(
      "the folded-goods table reaches the floor",
      table.low <= 0.01,
      `lowest vertex ${table.low} cm. A gap under a fixture is a sight line ` +
        `under it, and the crouched trace sits at ${TRACE_CROUCHED_CM} cm`,
    );
    check(
      "the folded-goods table hides a crouched keeper",
      table.high > TRACE_CROUCHED_CM,
      `tops at ${table.high} cm against a crouched trace at ${TRACE_CROUCHED_CM}`,
    );
    check(
      "and does NOT hide a standing one, so it is the fixture you duck behind",
      table.high < TRACE_STANDING_CM,
      `tops at ${table.high} cm against a standing trace at ${TRACE_STANDING_CM}. ` +
        `If it covers both, the floor has no fixture that teaches crouching`,
    );
    check(
      "the service counter reaches the floor",
      counter.low <= 0.01,
      `lowest vertex ${counter.low} cm`,
    );
    check(
      "the service counter hides a standing keeper",
      counter.high > TRACE_STANDING_CM,
      `tops at ${counter.high} cm against a standing trace at ${TRACE_STANDING_CM}`,
    );
    const bay = topOf("SM_BSW_WallMerchBay");
    if (bay) {
      check(
        "the merchandise bay reaches the floor",
        bay.low <= 0.01,
        `lowest vertex ${bay.low} cm`,
      );
      check(
        "the merchandise bay blocks above eye height, so it hides and blinds",
        bay.high > EYE_HEIGHT_CM,
        `tops at ${bay.high} cm against a ${EYE_HEIGHT_CM} cm eye. Its whole ` +
          `job is to be the cover you cannot see over; below this the floor ` +
          `has two kinds of cover instead of three`,
      );
    }

    check(
      "and stays below eye height, so the keeper can watch over it",
      counter.high < EYE_HEIGHT_CM,
      `tops at ${counter.high} cm against a ${EYE_HEIGHT_CM} cm eye. Cover you ` +
        `cannot see over is a wall, and the findings asked for this affordance ` +
        `to be made deliberate rather than lost`,
    );
  }

  // The racks must keep reading as open, and that is deliberately NOT checked
  // here. The obvious proxy - "no vertex reaches the floor" - fails on the
  // castors, which legitimately touch it, and a check that fires on correct
  // geometry teaches people to edit the test. Whether a fixture is see-through
  // is a question about air between solids at a height and an angle, and the
  // instrument that answers it is a trace in the live editor, which is what
  // FINDINGS-KIDS-FLOOR-STEALTH.md already used. Left to that.
}

console.log("\nCover placement");
{
  // The placement table in place_fixtures.py is the part of that script that
  // has to be right BEFORE an editor sees it. A wrong height field or a
  // centre-pivot z sinks a fixture into the deck, and the probe still passes,
  // because the half of the fixture above the floor still blocks the trace.
  // So it is checked here, with no engine, against the same committed OBJs.
  const TRACE_CROUCHED_CM = 40;
  const TRACE_STANDING_CM = 88;
  const EYE_HEIGHT_CM = 152;

  // The children's department footprint and the deck height, from the
  // commandlet read of the committed map on 2026-11-15.
  const DEPARTMENT = { xMin: -3900, xMax: -1100, yMin: -2050, yMax: -250 };
  const DECK_Z = 662.5;

  const objTop = (stem) => {
    const path = join(root, "assets/models/boswells", `${stem}.obj`);
    if (!existsSync(path)) return null;
    let low = Infinity;
    let high = -Infinity;
    for (const line of readFileSync(path, "utf8").split("\n")) {
      if (!line.startsWith("v ")) continue;
      const z = Number(line.trim().split(/\s+/)[3]);
      if (!Number.isFinite(z)) continue;
      if (z < low) low = z;
      if (z > high) high = z;
    }
    return { low, high };
  };

  const script = join(root, "assets/models/boswells/place_fixtures.py");
  const src = existsSync(script) ? readFileSync(script, "utf8") : "";

  check(
    "the placement script is committed",
    src.length > 0,
    "assets/models/boswells/place_fixtures.py is missing",
  );

  if (src) {
    const floorZ = Number((src.match(/^FLOOR_Z = ([\d.]+)/m) || [])[1]);
    check(
      "the deck height is the measured 662.5, not an approximation",
      floorZ === DECK_Z,
      `FLOOR_Z is ${floorZ}. All four slabs carrying the deck top out at ` +
        `662.500 exactly; "about 663" is half a centimetre of fixture buried`,
    );

    // Base-pivot: the spawn z must be a bare FLOOR_Z. The failure this catches
    // is the plausible one - reaching for Codex's centre-pivot cube
    // convention, which would bury these three by 40, 51 and 95 cm.
    //
    // This asserts the spawn call's shape and nothing more. It first also
    // asserted that "FLOOR_Z + <height>" appears nowhere in the file, which
    // fired on the correct script: the probes build their trace points at
    // FLOOR_Z + height by design, and must. A check that fails on correct
    // code teaches the next person to edit the test, so it is gone rather
    // than narrowed. The positive form already covers the real case - a
    // centre-pivot spawn cannot match a bare FLOOR_Z.
    check(
      "fixtures spawn at the floor, as base-pivot meshes require",
      /spec\["xy"\]\[1\],\s*FLOOR_Z\s*\)/.test(src),
      "the spawn z is not a bare FLOOR_Z. These meshes have their origin at " +
        "the base; FLOOR_Z + height/2 is the centre-pivot convention and " +
        "sinks each one half its own height into the deck",
    );

    const entries = [
      ...src.matchAll(
        /dict\(mesh="([^"]+)",\s*label="([^"]+)",\s*xy=\(([-\d.]+),\s*([-\d.]+)\),\s*yaw=([-\d.]+),\s*size=\(([\d.]+),\s*([\d.]+)\),\s*height=([\d.]+),\s*blocks=\[([^\]]*)\],\s*clear=\[([^\]]*)\]/g,
      ),
    ].map((m) => ({
      mesh: m[1],
      label: m[2],
      x: Number(m[3]),
      y: Number(m[4]),
      yaw: Number(m[5]),
      sx: Number(m[6]),
      sy: Number(m[7]),
      height: Number(m[8]),
      blocks: m[9],
      clear: m[10],
    }));

    check(
      "all three fixtures appear in the placement table",
      entries.length === 3,
      `parsed ${entries.length} placements. If the table was edited into a ` +
        `shape this regex does not read, every check below silently passes ` +
        `by testing nothing`,
    );

    const named = {
      TRACE_CROUCHED_CM,
      TRACE_STANDING_CM,
      EYE_HEIGHT_CM,
    };
    const heights = (list) =>
      list
        .split(",")
        .map((n) => named[n.trim()])
        .filter(Number.isFinite);

    for (const e of entries) {
      const obj = objTop(e.mesh);
      if (obj) {
        check(
          `${e.label} declares the height its mesh actually is`,
          Math.abs(obj.high - e.height) < 0.5,
          `the table says ${e.height} cm, the committed OBJ is ` +
            `${obj.high.toFixed(1)} cm. Every blocks/clear decision is ` +
            `derived from this number, so a drift here makes all of them lies`,
        );
      }

      const blocks = heights(e.blocks);
      const clears = heights(e.clear);

      for (const h of blocks) {
        check(
          `${e.label} is tall enough to block the ${h} cm trace it claims`,
          e.height > h,
          `${e.height} cm cannot block a trace at ${h} cm. The editor reports ` +
            `this as a probe failure only after spawning; catching it here ` +
            `costs nothing`,
        );
      }
      for (const h of clears) {
        check(
          `${e.label} is short enough to be clear at ${h} cm as it claims`,
          e.height < h,
          `${e.height} cm stands above a trace at ${h} cm, so "clear" is ` +
            `unsatisfiable and the height control built on it proves nothing`,
        );
      }
      check(
        `${e.label} accounts for all three trace heights exactly once`,
        blocks.length + clears.length === 3 &&
          new Set([...blocks, ...clears]).size === 3,
        `blocks and clear together name ${blocks.length + clears.length} of ` +
          `the three trace heights. A height in neither list is a height ` +
          `nothing checks, at a fixture whose whole purpose is that height`,
      );
    }

    // Fixtures must not be laid over one another. This is the exact fault
    // standing on this deck today - UPPER_BEDDING and UPPER_SEASONAL
    // interpenetrate three racks and the sales desk because a rack grid was
    // placed without checking what was already there - and it would be poor
    // to file that handoff row from inside the commit that repeats it.
    for (let i = 0; i < entries.length; i += 1) {
      for (let j = i + 1; j < entries.length; j += 1) {
        const a = entries[i];
        const b = entries[j];
        if (a.yaw !== 0 || b.yaw !== 0) {
          check(
            `${a.label} / ${b.label} overlap test understands their rotation`,
            false,
            "a placement is rotated and this check only compares " +
              "axis-aligned footprints. Extend it before trusting it, " +
              "rather than reading a pass that compared the wrong boxes",
          );
          continue;
        }
        const ox =
          Math.min(a.x + a.sx / 2, b.x + b.sx / 2) -
          Math.max(a.x - a.sx / 2, b.x - b.sx / 2);
        const oy =
          Math.min(a.y + a.sy / 2, b.y + b.sy / 2) -
          Math.max(a.y - a.sy / 2, b.y - b.sy / 2);
        check(
          `${a.label} and ${b.label} do not interpenetrate`,
          !(ox > 0 && oy > 0),
          `they overlap by ${ox.toFixed(1)} x ${oy.toFixed(1)} cm`,
        );
      }
    }

    for (const e of entries) {
      const x0 = e.x - e.sx / 2;
      const x1 = e.x + e.sx / 2;
      const y0 = e.y - e.sy / 2;
      const y1 = e.y + e.sy / 2;
      check(
        `${e.label} stands inside the children's department`,
        x0 >= DEPARTMENT.xMin && x1 <= DEPARTMENT.xMax &&
          y0 >= DEPARTMENT.yMin && y1 <= DEPARTMENT.yMax,
        `x ${x0.toFixed(0)}..${x1.toFixed(0)}, y ${y0.toFixed(0)}..` +
          `${y1.toFixed(0)} falls outside the department footprint ` +
          `x -3900..-1100, y -2050..-250`,
      );
    }

    // Three guards against the one failure mode this kit has already shipped
    // once: a call that reports success whether or not it did anything.
    // add_simple_collisions returns -1 without raising, and the import script
    // asserted against collision it had never added. Each of these was a live
    // fault in this script, found in review before it ran.
    check(
      "the map save's return value is checked",
      /if not unreal\.EditorLoadingAndSavingUtils\.save_map\(/.test(src),
      "save_current_level() and save_map() both return a bool and both can " +
        "refuse. Logging success unconditionally after one is the " +
        "add_simple_collisions fault again, one file along: the fixtures " +
        "would exist only in that process and vanish when it exits",
    );
    // Matches the CALL, not the definition: /prove_the_instrument\(world\)/
    // also matches "def prove_the_instrument(world):", so the first version
    // of this guard passed with the call deleted. A leading-whitespace anchor
    // and no trailing colon is the difference between checking a function
    // exists and checking it is used.
    const instrumentCall = /^[ \t]+prove_the_instrument\(world\)[ \t]*$/m;
    const called = instrumentCall.exec(src);
    check(
      "the trace instrument is proven against known geometry before spawning",
      called !== null && called.index < src.indexOf("for spec in PLACEMENTS:"),
      "every way the trace can break - a tuple result read for truthiness, a " +
        "None world, the wrong channel - gives a uniform answer, so the " +
        "probes cannot detect it. Two traces with opposite known answers can, " +
        "and they must run before anything is spawned",
    );
    check(
      "the re-run tag is read back after it is set",
      /if not actor\.actor_has_tag\(TAG\)/.test(src),
      "clear_previous() finds its own work by this tag. A tag that failed to " +
        "stick means the second run stacks three more fixtures on the first " +
        "three instead of replacing them, and the re-run guard is the thing " +
        "protecting the map from this script",
    );

    // Deliberately NOT checked here: whether each fixture actually blocks a
    // sight line where it stands. That is a question about 30 other solids on
    // the deck and the answer changes the moment anyone moves one, so it
    // belongs to the trace probes in place_fixtures.py, which refuse to save
    // the map when they fail. Re-implementing it from a copied actor list
    // would be a second source of truth that goes stale quietly.
  }
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("The pack is here but incomplete, which is the case that costs an");
  console.log("evening: the map half-renders and nothing explains it. Re-copy the");
  console.log(`missing folders from ${SOURCE}.\n`);
  process.exit(1);
}
console.log("");
