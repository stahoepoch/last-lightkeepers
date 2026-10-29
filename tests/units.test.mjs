// Units guard.
//
// `DECISIONS.md` row 61: `data/*.json` stays in Roblox studs, Unreal works in
// centimetres, and `Source/LLK/Public/LLKUnits.h` is the SINGLE conversion
// authority for every spatial value - movement, enemy ranges, interaction
// distances, light radii. No raw conversion anywhere else.
//
// That is a rule, and a rule with nothing checking it is a convention. The
// failure it prevents is quiet in both directions: a forgotten conversion
// makes a 34-stud sight range into 34 cm, which reads as "the enemy is
// blind"; a hand-rolled one makes a second source of truth for a constant
// that must only ever have one.
//
// This runs with no engine, so it gates every push under the two-tier CI rule
// rather than waiting for somebody to open the editor.
//
//   node tests/units.test.mjs

import { readFileSync, readdirSync, statSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join, relative } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const sourceRoot = join(root, "Source", "LLK");
const authority = join("Source", "LLK", "Public", "LLKUnits.h");

let failures = 0;
let checks = 0;
const check = (name, condition, detail) => {
  checks += 1;
  if (condition) console.log(`  ok   ${name}`);
  else {
    failures += 1;
    console.log(`  FAIL ${name}${detail ? ` — ${detail}` : ""}`);
  }
};

const walk = (dir) => {
  const out = [];
  for (const entry of readdirSync(dir)) {
    const full = join(dir, entry);
    if (statSync(full).isDirectory()) out.push(...walk(full));
    else if (/\.(h|cpp)$/.test(entry)) out.push(full);
  }
  return out;
};

const files = walk(sourceRoot);
check("Source/LLK has files to scan", files.length > 0, `${files.length} found`);

// ---- 1. the authority exists and still says what row 61 says it says ------

const unitsPath = join(root, authority);
const units = readFileSync(unitsPath, "utf8");

check(
  "LLKUnits.h declares the conversion",
  /StudsToCm\s*=\s*28(\.f)?\s*;/.test(units),
  "row 61 names this file as the single authority",
);
check(
  "LLKUnits.h offers a speed helper",
  /SpeedFromStuds/.test(units),
  "callers must not reach for the raw constant",
);
check(
  "LLKUnits.h offers a distance helper",
  /DistanceFromStuds/.test(units),
  "ranges, radii and reach are the M2 cases",
);

// ---- 2. nobody else converts by hand ---------------------------------------
//
// The shape to catch is the literal 28 used as a multiplier or divisor. A 28
// that is a position, a count or an index is not a conversion and is not this
// test's business, which is why the pattern requires an arithmetic operator.

const rawConversion = /(?:\*\s*28(?:\.0*f?)?\b)|(?:\b28(?:\.0*f?)?\s*\*)|(?:\/\s*28(?:\.0*f?)?\b)/;

const offenders = [];
for (const file of files) {
  const rel = relative(root, file);
  if (rel === authority) continue;
  const text = readFileSync(file, "utf8");
  text.split(/\r?\n/).forEach((line, i) => {
    // A comment explaining the rule is not a breach of it.
    const code = line.replace(/\/\/.*$/, "").replace(/\/\*.*?\*\//g, "");
    if (rawConversion.test(code)) offenders.push(`${rel}:${i + 1} — ${line.trim()}`);
  });
}

check(
  "no raw stud conversion outside LLKUnits.h",
  offenders.length === 0,
  offenders.length ? offenders.join(" | ") : undefined,
);

// ---- 3. the one caller that exists today actually uses the helper ----------
//
// Narrow on purpose. This is not a census of every future call site; it is
// proof that the authority is wired to something, so the guard above is
// protecting a live path rather than an unused header.

const characterPath = join(root, "Source", "LLK", "Private", "Characters", "LLKCharacter.cpp");
const character = readFileSync(characterPath, "utf8");
check(
  "ALLKCharacter converts through LLKUnits",
  /LLKUnits::SpeedFromStuds\(/.test(character),
  "speeds come out of data in studs and must not reach MaxWalkSpeed unconverted",
);

// ---- 4. the data really is still in studs ----------------------------------
//
// If somebody re-authors the JSON in centimetres, row 61 has been reversed and
// the conversion above becomes a 28x bug. walkSpeed is the canary because 16
// is Roblox's default humanoid walk speed - the tell that settled O9.

const lantern = JSON.parse(readFileSync(join(root, "data", "lantern.json"), "utf8"));
const walkStuds = lantern.movement?.walkSpeed;
check(
  "data/lantern.json is still authored in studs",
  typeof walkStuds === "number" && walkStuds > 0 && walkStuds < 100,
  `walkSpeed is ${walkStuds}; a centimetre value would be in the hundreds, and row 61 says it stays studs`,
);

console.log(`\n${checks - failures}/${checks} passed`);
process.exit(failures ? 1 : 0);
