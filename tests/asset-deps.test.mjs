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

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("The pack is here but incomplete, which is the case that costs an");
  console.log("evening: the map half-renders and nothing explains it. Re-copy the");
  console.log(`missing folders from ${SOURCE}.\n`);
  process.exit(1);
}
console.log("");
