// Static wiring checks for the Luau.
//
// The project has no Luau toolchain, so nothing here is typechecked. These
// checks target the specific failure classes that would otherwise only show up
// as a runtime error on the first Studio run:
//
//   1. a Remotes.get("Name") that no remote is created for
//   2. a Config.<Section> whose Balance JSON module doesn't exist
//   3. a Config.<Section>.<key> path that isn't in the JSON  (nil index)
//   4. a WaitForChild("Module") with no matching file          (infinite yield)
//   5. a Service.method() call that nothing defines            (nil call)
//
// Regex over Luau, not a parser. It can miss things; it does not invent them.
//
//   node tests/wiring.test.mjs

import { readFileSync, readdirSync, existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join, basename, sep } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");

let failures = 0;
let checks = 0;
const fail = (msg) => {
  failures += 1;
  console.log(`  FAIL ${msg}`);
};
const ok = (msg) => console.log(`  ok   ${msg}`);
const check = (name, condition, detail) => {
  checks += 1;
  if (condition) ok(name);
  else fail(`${name}${detail ? ` — ${detail}` : ""}`);
};

// ---- collect sources -------------------------------------------------------

function walk(dir) {
  const out = [];
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const p = join(dir, entry.name);
    if (entry.isDirectory()) out.push(...walk(p));
    else if (entry.name.endsWith(".luau")) out.push(p);
  }
  return out;
}

const stripSuffix = (name) => name.replace(/[.](client|server)?[.]?luau$/, "");

const files = walk(join(root, "src")).map((path) => ({
  path,
  rel: path.slice(root.length + 1).split(sep).join("/"),
  name: stripSuffix(basename(path)),
  src: readFileSync(path, "utf8"),
}));

const all = (re, text) => [...text.matchAll(re)];

// ---- 1. remote names -------------------------------------------------------

console.log("\nRemotes");
{
  const remotesFile = files.find((f) => f.rel.endsWith("shared/Remotes.luau"));
  if (!remotesFile) {
    fail("src/shared/Remotes.luau not found");
  } else {
    // Index-based rather than regex: the EVENTS table spans lines and contains
    // braces in comments, which a lazy regex gets wrong in both directions.
    const start = remotesFile.src.indexOf("local EVENTS = {");
    const end = start >= 0 ? remotesFile.src.indexOf("\n}", start) : -1;
    const block = start >= 0 && end > start ? remotesFile.src.slice(start, end) : "";
    const declared = new Set(all(/"([^"]+)"/g, block).map((m) => m[1]));
    check("EVENTS list is non-empty", declared.size > 0);

    const used = new Map();
    for (const f of files) {
      for (const m of all(/Remotes[.]get[(]"([^"]+)"[)]/g, f.src)) {
        if (!used.has(m[1])) used.set(m[1], new Set());
        used.get(m[1]).add(f.rel);
      }
    }
    for (const [name, where] of used) {
      check(`remote "${name}" is created`, declared.has(name), `used in ${[...where].join(", ")}`);
    }
    for (const name of declared) {
      if (!used.has(name)) console.log(`  note declared remote "${name}" is never used`);
    }
  }
}

// ---- 2 & 3. Config sections and key paths ----------------------------------

console.log("\nConfig against data/*.json");
{
  const configFile = files.find((f) => f.rel.endsWith("shared/Config.luau"));
  if (!configFile) {
    fail("src/shared/Config.luau not found");
  } else {
    const varToJson = new Map(
      all(/local (\w+) = require[(]Balance:WaitForChild[(]"([^"]+)"[)][)]/g, configFile.src).map((m) => [m[1], m[2]]),
    );
    const sectionToVar = new Map(all(/^\s+(\w+) = (\w+),$/gm, configFile.src).map((m) => [m[1], m[2]]));

    const sections = new Map();
    for (const [section, varName] of sectionToVar) {
      const jsonName = varToJson.get(varName);
      if (!jsonName) continue;
      const jsonPath = join(root, "data", `${jsonName}.json`);
      checks += 1;
      if (!existsSync(jsonPath)) {
        fail(`Config.${section} -> data/${jsonName}.json is missing`);
        continue;
      }
      ok(`Config.${section} -> data/${jsonName}.json exists`);
      sections.set(section, JSON.parse(readFileSync(jsonPath, "utf8")));
    }

    const configFns = new Set(all(/function Config[.](\w+)/g, configFile.src).map((m) => m[1]));

    // Sections defined as Luau tables in Config.luau itself (e.g. Config.Run),
    // not loaded from JSON. Valid roots; their keys aren't checkable from here.
    const configTables = new Set(all(/^Config[.](\w+) = [{]/gm, configFile.src).map((m) => m[1]));

    const seen = new Set();
    for (const f of files) {
      for (const m of all(/\bConfig[.]([A-Za-z_]\w*(?:[.][A-Za-z_]\w*)*)/g, f.src)) {
        const parts = m[1].split(".");
        if (configFns.has(parts[0]) || configTables.has(parts[0])) continue;
        const key = `${f.rel}|${m[1]}`;
        if (seen.has(key)) continue;
        seen.add(key);

        if (!sections.has(parts[0])) {
          check(`Config.${parts[0]} is a real section`, false, `in ${f.rel}`);
          continue;
        }
        let node = sections.get(parts[0]);
        let walked = parts[0];
        let broke = false;
        for (const part of parts.slice(1)) {
          if (node === null || typeof node !== "object" || Array.isArray(node)) break;
          if (!(part in node)) {
            check(`Config.${m[1]}`, false, `${walked} has no "${part}" — ${f.rel}`);
            broke = true;
            break;
          }
          node = node[part];
          walked += `.${part}`;
        }
        if (!broke) {
          checks += 1;
          ok(`Config.${m[1]}`);
        }
      }
    }
  }
}

// ---- 4. WaitForChild module resolution ------------------------------------

console.log("\nModule resolution");
{
  const byName = new Map(files.map((f) => [f.name, f.rel]));
  const dataNames = new Set(
    readdirSync(join(root, "data"))
      .filter((n) => n.endsWith(".json"))
      .map((n) => n.replace(/[.]json$/, "")),
  );
  for (const f of files) {
    for (const m of all(/require[(](?:\w+|script[.]Parent):WaitForChild[(]"([^"]+)"[)][)]/g, f.src)) {
      const target = m[1];
      checks += 1;
      if (byName.has(target) || dataNames.has(target)) {
        ok(`require "${target}" resolves (${f.rel})`);
      } else {
        fail(`require "${target}" has no matching file — ${f.rel}`);
      }
    }
  }
}

// ---- 5. cross-module method calls -----------------------------------------

console.log("\nCross-module calls");
{
  const serviceFiles = files.filter((f) => /Service|Remotes/.test(f.name));
  const exported = new Map();
  for (const f of serviceFiles) {
    const names = new Set();
    for (const m of all(/function (\w+)[.](\w+)/g, f.src)) if (m[1] === f.name) names.add(m[2]);
    for (const m of all(/^\s*(\w+)[.](\w+)\s*=/gm, f.src)) if (m[1] === f.name) names.add(m[2]);
    exported.set(f.name, names);
  }

  for (const f of files) {
    for (const [mod, names] of exported) {
      if (f.name === mod) continue;
      const requiresIt = new RegExp(`require[(][^)]*WaitForChild[(]"${mod}"[)][)]`).test(f.src);
      if (!requiresIt) continue;
      const used = new Set(all(new RegExp(`\\b${mod}[.](\\w+)`, "g"), f.src).map((m) => m[1]));
      for (const name of used) {
        check(`${mod}.${name}`, names.has(name), `not defined in ${mod} — called from ${f.rel}`);
      }
    }
  }
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  process.exit(1);
}
console.log("");
