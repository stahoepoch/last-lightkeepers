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

import { existsSync, readdirSync } from "node:fs";
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

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("The pack is here but incomplete, which is the case that costs an");
  console.log("evening: the map half-renders and nothing explains it. Re-copy the");
  console.log(`missing folders from ${SOURCE}.\n`);
  process.exit(1);
}
console.log("");
