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
  // Functions of the capsule, not constants. See make_fixture_kit.py; the
  // keeper is a 145 cm boy as of 2026-11-15 and was a 176 cm adult before.
  const KEEPER_HEIGHT_CM = 145;
  const K = KEEPER_HEIGHT_CM / 176;
  const TRACE_CROUCHED_CM = 40 * K;
  const TRACE_STANDING_CM = 88 * K;
  const EYE_HEIGHT_CM = (88 + 64) * K;

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
  // Functions of the capsule, not constants. See make_fixture_kit.py; the
  // keeper is a 145 cm boy as of 2026-11-15 and was a 176 cm adult before.
  const KEEPER_HEIGHT_CM = 145;
  const K = KEEPER_HEIGHT_CM / 176;
  const TRACE_CROUCHED_CM = 40 * K;
  const TRACE_STANDING_CM = 88 * K;
  const EYE_HEIGHT_CM = (88 + 64) * K;

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
      (() => {
        // Must precede the placement, and also the PREFLIGHT: the preflight
        // fires traces of its own, so proving the instrument after it would
        // be checking the ruler after measuring with it.
        const preflight = /^[ \t]+preflight_the_fixtures\(world\)[ \t]*$/m
          .exec(src);
        const placement = src.indexOf('placed[spec["label"]] = place(spec)');
        return called !== null && preflight !== null && placement >= 0 &&
          called.index < preflight.index && called.index < placement;
      })(),
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

    // Two guards on the import script, for a defect that actually shipped.
    // 1ee88b8 was titled "imported, boxed and blocking Visibility"; a trace on
    // 2026-11-15 found two of the three fixtures missed at every height. The
    // cause was a hand-built KBoxElem written when StaticMeshEditorSubsystem
    // was absent - a struct the engine never cooks physics for, which reads
    // correct in every asset-level check and which sight goes straight
    // through. These two stop it coming back by the route it came.
    const importSrc = existsSync(
      join(root, "assets/models/boswells/import_fixtures.py"),
    )
      ? readFileSync(join(root, "assets/models/boswells/import_fixtures.py"), "utf8")
      : "";

    check(
      "the import script builds no collision by hand",
      importSrc.length > 0 && !/^\s*box\s*=\s*unreal\.KBoxElem\(\)/m.test(importSrc),
      "a KBoxElem written straight onto a body setup produces an asset with " +
        "box_elems 1, BlockAll and correct bounds that a Visibility trace " +
        "passes through. There is no headless way to cook collision - " +
        "BodySetup exposes no InvalidatePhysicsData and no " +
        "CreatePhysicsMeshes - so the only correct behaviour without an " +
        "editor is to refuse",
    );
    // Stated as a positive. The first version of this searched the success
    // log for "blocking Visibility" and asserted its absence, which broke two
    // ways: the regex could not step over the ")" inside len(FIXTURES), and a
    // bare phrase search would have fired on the docstring, where that phrase
    // legitimately appears quoting the commit this defect shipped in. Proving
    // a disclaimer present is both easier to get right and the thing actually
    // wanted.
    check(
      "the import script says what it did NOT prove",
      importSrc.length > 0 && /NOT PROVEN HERE/.test(importSrc),
      "box_elems counts a struct and collision_profile_name reads a string; " +
        "neither consults the physics scene. Every check in that script " +
        "passed on all three fixtures while two of them were not cover, so " +
        "its success message has to name the gap rather than leave the " +
        "reader to assume it was covered",
    );

    // Three guards on the acceptance bar itself, added after the same
    // question was shown to get different answers in different processes and
    // in one process minutes apart. Two wrong claims landed today on single
    // green observations, so "it passed" is no longer a thing this kit is
    // allowed to say from one trace.
    // ANCHOR NOTE. These two orderings were first anchored on
    // "for spec in PLACEMENTS:", which stopped meaning "where placement
    // starts" the moment preflight_the_fixtures grew a loop over the same
    // list higher up the file - so indexOf found the preflight and both
    // guards went green on the wrong line. Third anchor today to match text
    // that existed somewhere else for another reason. Anchored now on the
    // one statement that IS the placement, and if that line is ever renamed
    // these fail loudly rather than quietly passing.
    check(
      "a trace is fired more than once before its answer is believed",
      /^TRACE_REPEATS = ([2-9]|\d\d+)$/m.test(src),
      "TRACE_REPEATS is 1 or missing. A flaky result read once is " +
        "indistinguishable from a true one, and that is how both of today's " +
        "wrong claims were made",
    );
    check(
      "disagreement between repeats raises rather than being summarised",
      /raise RuntimeError\(\s*\n?\s*f?"UNSTABLE/.test(src) ||
        /"UNSTABLE:/.test(src),
      "repeats that disagree must be their own outcome. A majority vote " +
        "would convert \"this does not hold still\" into a clean boolean " +
        "and let the placement proceed on it, which is the single most " +
        "dangerous thing this file could do",
    );
    check(
      "every mesh is proven to block in empty space before anything is placed",
      (() => {
        // Two traps in one line, both hit earlier today, both rewritten here
        // rather than patched. /preflight_the_fixtures\(world\)/ also matches
        // its own "def" line, so the call could be deleted and the regex
        // still pass. And String.search returns -1 when it finds nothing, so
        // "-1 < indexOf(...)" is TRUE and a missing call reads as a correctly
        // ordered one - the same negative-sentinel trap as
        // add_simple_collisions returning -1 where "if not" waves it through.
        // Match the call explicitly, require it to exist, then compare.
        const call = /^[ \t]+preflight_the_fixtures\(world\)[ \t]*$/m.exec(src);
        const placement = src.indexOf('placed[spec["label"]] = place(spec)');
        return call !== null && placement >= 0 && call.index < placement;
      })(),
      "the instrument check proves the TRACE works, not that the ASSETS do " +
        "- and that is the exact gap two broken fixtures hid in. A mesh " +
        "that cannot block in empty space will not block on a shop floor",
    );

    // Two guards on the import ORDER, which is the only part of the Nanite
    // story that is load-bearing. Saving a Nanite-built asset with the flag
    // off was tested: a full editor traced BLOCK and saved, and a commandlet
    // then loaded that package and read clear. So the asset has to be BUILT
    // without Nanite, and that means deleting first and rebuilding - not
    // flipping a flag on something that already exists.
    check(
      "the import deletes each asset before re-importing it",
      importSrc.length > 0 &&
        /^[ \t]+delete_first\(stem\)[ \t]*$/m.test(importSrc) &&
        importSrc.search(/^[ \t]+delete_first\(stem\)/m) <
          importSrc.search(/^[ \t]+mesh = import_mesh\(stem\)/m),
      "replace_existing reuses the package and keeps its build settings, so " +
        "a re-import over a Nanite-built asset inherits the thing being " +
        "removed. Delete first or the rebuild is not clean",
    );
    check(
      "a Nanite flag written without a rebuild is refused, not shipped",
      importSrc.length > 0 &&
        /raise RuntimeError\([\s\S]{0,400}?set_nanite_settings is not/.test(
          importSrc,
        ),
      "writing the flag with no build is the route already proven to leave " +
        "collision broken. Producing assets that way and leaving the " +
        "pre-flight to notice is shipping the thing that looks done again",
    );

    // The fixtures must be closed solids. Measured on the committed files
    // 2026-11-15: they were not - the generator minted fresh vertices for
    // every corner of every face, so a six-sided box came out as six
    // disconnected quads and the merchandise bay was 252 separate shells
    // with not one shared edge between them.
    //
    // This does NOT explain why two of the three miss a sight trace: all
    // three were pathological in the same way and only two fail. It is wrong
    // on its own terms, which is reason enough - a fixture whose faces do not
    // know they are a solid is not a solid to anything that asks the question
    // geometrically, and these meshes exist to be asked exactly that.
    const manifold = (stem) => {
      const path = join(root, "assets/models/boswells", `${stem}.obj`);
      if (!existsSync(path)) return null;
      const edges = new Map();
      let normals = 0;
      for (const line of readFileSync(path, "utf8").split("\n")) {
        if (line.startsWith("vn ")) normals += 1;
        if (!line.startsWith("f ")) continue;
        const idx = line.trim().split(/\s+/).slice(1)
          .map((t) => Number(t.split("/")[0]));
        for (let i = 0; i < idx.length; i += 1) {
          const a = idx[i];
          const b = idx[(i + 1) % idx.length];
          const key = `${Math.min(a, b)}-${Math.max(a, b)}`;
          edges.set(key, (edges.get(key) || 0) + 1);
        }
      }
      let open = 0;
      let nonManifold = 0;
      for (const n of edges.values()) {
        if (n === 1) open += 1;
        if (n > 2) nonManifold += 1;
      }
      return { open, nonManifold, normals, edges: edges.size };
    };

    for (const stem of [
      "SM_BSW_FoldedGoodsTable",
      "SM_BSW_ServiceCounter",
      "SM_BSW_WallMerchBay",
    ]) {
      const m = manifold(stem);
      if (!m) continue;
      check(
        `${stem} is a closed solid with shared edges`,
        m.open === 0 && m.nonManifold === 0,
        `${m.open} boundary edge(s) and ${m.nonManifold} non-manifold out of ` +
          `${m.edges}. Unwelded faces mean every box is a loose set of quads ` +
          `rather than a solid`,
      );
      check(
        `${stem} carries flat normals, so welding did not round its edges`,
        m.normals > 0,
        "welded positions with no normals make the importer average across " +
          "every join and shade a box like a ball. Welding without normals " +
          "is a downgrade, not an upgrade",
      );
    }

    // Four guards on the collision-proxy design, chosen by Chris on
    // 2026-11-15 after the fault was localized to import_fixtures.py but not
    // fixed. Two of the three imported meshes are missed by a Visibility
    // trace everywhere, so cover comes from a hidden engine cube scaled to
    // the fixture's own measured bounds instead of from the mesh's body.
    //
    // It is a workaround over an undiagnosed fault. These guards exist
    // because the most likely way it goes wrong is not failing - it is
    // someone later "tidying" it back toward the shape that looks right and
    // does not work.
    const proxied = ["SM_BSW_FoldedGoodsTable", "SM_BSW_WallMerchBay"];

    check(
      "the two meshes that block nothing are given a collision proxy",
      proxied.every((m) => new RegExp(`PROXY_COVER[\\s\\S]{0,200}${m}`).test(src)),
      "PROXY_COVER names which fixtures cannot carry their own collision. " +
        "A fixture dropped from that set silently becomes scenery with no " +
        "cover, which is exactly the state this whole day was spent finding",
    );
    check(
      "a proxied mesh is set NoCollision, so one body owns the sightline",
      /"NoCollision" if spec\["mesh"\] in PROXY_COVER else "BlockAll"/.test(src),
      "if the mesh keeps BlockAll alongside its proxy there are two bodies " +
        "claiming the same cover. Today two of them block nothing, so the " +
        "claim is false - and the day the import bug is fixed they would " +
        "start colliding with no commit to point at",
    );
    check(
      "the proxy carries the re-run tag, so a second run replaces it",
      /def place_proxy[\s\S]{0,1600}?unreal\.Name\(TAG\)/.test(src),
      "clear_previous() finds its own work by that tag. An untagged proxy " +
        "survives the next run and a second one is stacked on it, so the " +
        "floor quietly accumulates invisible collision nobody placed",
    );
    check(
      "the proxy is centre-pivot while the fixtures stay base-pivot",
      /def place_proxy[\s\S]{0,900}?FLOOR_Z \+ spec\["height"\] \/ 2\.0/.test(src),
      "the engine cube's origin is its centre; the fixture meshes' origin " +
        "is their base. The two must be spawned differently, and making " +
        "them agree is the plausible tidy-up that buries a proxy half its " +
        "height in the deck while still passing every other check",
    );

    // The fixture heights live in THREE files - the generator builds them,
    // import_fixtures asserts them after import, and place_fixtures scales
    // the collision proxy from them. They went out of agreement the moment
    // the table was resized: the import still expected 80 and refused the
    // 69 it got, which is the check working, but only after a full editor
    // run. Comparing them here costs nothing and catches it before.
    const importHeights = Object.fromEntries(
      [...importSrc.matchAll(/\("(SM_BSW_\w+)",\s*([\d.]+)\)/g)]
        .map((m) => [m[1], Number(m[2])]),
    );
    for (const e of entries) {
      const expected = importHeights[e.mesh];
      check(
        `${e.mesh} is the same height to the importer and the placer`,
        expected !== undefined && Math.abs(expected - e.height) < 0.5,
        `import_fixtures expects ${expected} cm, place_fixtures places ` +
          `${e.height}. Whichever is wrong, the editor only finds out after ` +
          `booting, and the one that is silent is the placer - a proxy ` +
          `scaled to the wrong height is cover of the wrong size`,
      );
    }

    // Deliberately NOT checked here: whether each fixture actually blocks a
    // sight line where it stands. That is a question about 30 other solids on
    // the deck and the answer changes the moment anyone moves one, so it
    // belongs to the trace probes in place_fixtures.py, which refuse to save
    // the map when they fail. Re-implementing it from a copied actor list
    // would be a second source of truth that goes stale quietly.
  }
}

// ====================================================================
// The lantern census, and the two ways it could quietly stop working.
//
// It exists because a correct system with no voice cost two play sessions
// on 2026-11-15: the keeper had no lantern, every component said so, and
// nothing said the map had none to find. These guard the parts of that
// fix that are invisible to a compiler.
{
  const gmPath = join(root, "Source/LLK/Private/LLKTestGameMode.cpp");
  const gm = existsSync(gmPath) ? readFileSync(gmPath, "utf8") : "";

  check(
    "the game mode still has a lantern census",
    gm.includes("ReportLanternPickups"),
    "nothing reports how many pickups a world has, so a map with none " +
      "looks exactly like a map whose pickup is out of reach",
  );

  // Scheduled, not called inline. Actor BeginPlay order inside
  // NotifyBeginPlay is unspecified and the game mode is in that same pass,
  // so a direct call can count zero in a map that has one - a diagnostic
  // that is wrong about its own subject, which is worse than silence.
  const timerIdx = gm.indexOf("SetTimerForNextTick");
  const reportIdx = gm.indexOf("&ALLKTestGameMode::ReportLanternPickups");
  check(
    "the census is deferred a tick rather than called during BeginPlay",
    timerIdx >= 0 && reportIdx >= 0 && reportIdx - timerIdx < 200,
    "ReportLanternPickups must be bound to SetTimerForNextTick. Called " +
      "straight from BeginPlay it races every pickup's own BeginPlay and " +
      "can report an empty world that is not empty",
  );

  // BeginPlay returns early in any world with no zone-pressure subsystem.
  // Anything scheduled after that return is not scheduled everywhere, and
  // the symptom is a census that is silent in exactly some maps - which
  // reads as "no pickup here" rather than "nobody asked".
  const beginIdx = gm.indexOf("void ALLKTestGameMode::BeginPlay()");
  const earlyReturnIdx = gm.indexOf("Storm == nullptr");
  check(
    "and it is scheduled above the subsystem early-return",
    beginIdx >= 0 &&
      timerIdx >= 0 &&
      earlyReturnIdx >= 0 &&
      timerIdx > beginIdx &&
      timerIdx < earlyReturnIdx,
    "the census must be scheduled before BeginPlay's `Storm == nullptr` " +
      "return, or it never runs in a world without that subsystem and its " +
      "silence is indistinguishable from a map with no lantern",
  );

  // The empty-case message must not name the map that does have one. A map
  // name spelled into a log string is the same failure shape this suite
  // was built to catch: it keeps printing confidently after a rename.
  const bodyStart = gm.indexOf("void ALLKTestGameMode::ReportLanternPickups()");
  const bodyEnd = gm.indexOf("void ALLKTestGameMode::", bodyStart + 10);
  const body = bodyStart >= 0 && bodyEnd > bodyStart
    ? gm.slice(bodyStart, bodyEnd)
    : "";
  check(
    "the census names no map in its own log text",
    body.length > 200 && !/LLK_Boswells/.test(body),
    body.length > 200
      ? "the census spells a map name into a log string, which goes stale " +
        "silently on the next rename - it should describe what the player " +
        "will experience and let the map name come from GetMapName"
      : "could not find the census body to check, which means this guard " +
        "is passing on nothing",
  );
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
