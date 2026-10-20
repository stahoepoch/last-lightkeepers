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

// ---- 6. regressions the first playtest found -------------------------------
//
// These three cost a whole playtest between them and all three read as fine on
// a review, so they are pinned here rather than trusted to one.
{
  const byName = new Map(files.map((f) => [f.name, f]));
  const runState = byName.get("RunStateService");
  const interaction = byName.get("InteractionService");
  const loot = byName.get("LootService");
  const enemy = byName.get("EnemyService");

  if (runState) {
    // A solo knockdown used to wipe the run inside the same call, so the
    // Anchor Revive could never be reached at all.
    const knockDown = runState.src.slice(
      runState.src.indexOf("function RunStateService.knockDown"),
      runState.src.indexOf("local function stand"),
    );
    check(
      "knockDown defers the wipe to canAnchorRevive",
      knockDown.includes("canAnchorRevive") &&
        knockDown.indexOf("canAnchorRevive") < knockDown.indexOf("RunStateService.wipe()"),
      "without this a solo player is hard-wiped on their first knockdown and R is dead code",
    );
    check(
      "snapshot carries the Anchor Revive reason",
      runState.src.includes("yourCanAnchorRevive") && runState.src.includes("yourAnchorRevivesLeft"),
      "the client cannot explain a refused R without these",
    );
  }

  // Every gameplay tag must be watched, not scanned once: the generated map is
  // tagged after the services start.
  for (const [tag, file, owner] of [
    ["TAG_LOOT", interaction, "InteractionService"],
    ["TAG_EXTRACTION", interaction, "InteractionService"],
    ["TAG_RELAY", loot, "LootService"],
    ["SweeperSpawn", enemy, "EnemyService"],
  ]) {
    check(
      `${owner} watches ${tag} for late tags`,
      file !== undefined && new RegExp(`GetInstanceAddedSignal[(][^)]*${tag}`).test(file.src),
      `a map tagged after startup leaves ${tag} unwired`,
    );
  }

  if (enemy) {
    const init = enemy.src.slice(enemy.src.indexOf("function EnemyService.init"));
    check(
      "EnemyService.init spawns Sweepers",
      init.includes("spawnSweepersAtTaggedPoints"),
      "the spawn function existed but was never called, so nothing could knock a player down",
    );
  }

  const server = files.find((f) => f.rel.endsWith("server/init.server.luau"));
  if (server) {
    const buildAt = server.src.indexOf("FloodedMallBuilder.Build()");
    const interactionsAt = server.src.indexOf("InteractionService.init()");
    check(
      "server builds the tagged mall before interactions initialize",
      buildAt >= 0 && interactionsAt > buildAt,
      "Rojo clears Workspace.Map.GeneratedFloodedMall on sync; starting services first leaves no prompts",
    );
  }
}

// ---- 7. relic visibility is per-player, not crew-wide ----------------------
{
  const byName = new Map(files.map((f) => [f.name, f]));
  const interaction = byName.get("InteractionService");
  const relic = byName.get("RelicVisibilityController");

  if (interaction) {
    const attach = interaction.src.slice(
      interaction.src.indexOf("local function attachLootPrompt"),
      interaction.src.indexOf("local function attachExtractionPrompt"),
    );
    // The crew-wide toggle is the bug: it showed relics to dark players
    // standing next to a lit teammate.
    check(
      "InteractionService does not gate relic prompts on activeLanterns",
      !attach.includes("activeLanterns"),
      "a server-side crew-wide toggle cannot be per-player; relic visibility belongs on the client",
    );
    check(
      "relic prompts are still created disabled",
      /visibility == "lanternOnly"[\s\S]{0,120}prompt[.]Enabled = false/.test(attach),
      "a relic prompt must start hidden, or it is visible to everyone until a client disables it",
    );
  }

  check("RelicVisibilityController exists", relic !== undefined, "nothing enables relic prompts per viewer");
  if (relic) {
    check(
      "RelicVisibilityController keys off the per-viewer lantern",
      relic.src.includes("yourLanternOn"),
      "activeLanterns is crew-wide; only yourLanternOn answers \"is mine lit\"",
    );
  }
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  process.exit(1);
}
console.log("");
