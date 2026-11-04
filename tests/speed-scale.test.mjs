// The movement scale lives in data/lantern.json. This finds every place that
// kept its own copy.
//
// Written 2026-11-04, auditing 7426625, which halved the whole movement scale.
// The authored data moved. Four constants in the tree did not, and each one is
// silent: nothing crashes, nothing logs, the number is simply wrong by exactly
// 2x and the symptom is a feel complaint rather than an error.
//
// This is the same shape as row X3 and as tests/cues.test.mjs — a value that
// is authoritative in one place and copied in another, where the copy keeps
// working after the original changes. The copies here are worse than most,
// because two of them are denominators in presentation maths, so their failure
// mode is "the lantern stopped feeling like a carried object" and nobody would
// grep for 448 to explain it.
//
//   node tests/speed-scale.test.mjs

import { existsSync, readFileSync, readdirSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join, relative } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const read = (p) => readFileSync(join(root, p), "utf8");

let failures = 0;
let checks = 0;
const check = (name, ok, detail) => {
  checks += 1;
  if (ok) console.log(`  ok   ${name}`);
  else {
    failures += 1;
    console.log(`  FAIL ${name}${detail ? `\n       ${detail}` : ""}`);
  }
};

// ---- the authority -------------------------------------------------------

const lantern = JSON.parse(read("data/lantern.json"));
const authored = {
  walk: lantern.movement.walkSpeed,
  sprint: lantern.movement.sprintSpeed,
  crouch: lantern.movement.crouchSpeed,
};

const units = read("Source/LLK/Public/LLKUnits.h");
const studsToCm = Number(/StudsToCm\s*=\s*([0-9.]+)f/.exec(units)?.[1]);

const cm = (studs) => studs * studsToCm;

console.log("\nThe authored scale, which is the only authority\n");
check(
  "data/lantern.json authors all three speeds as numbers",
  [authored.walk, authored.sprint, authored.crouch].every(
    (v) => typeof v === "number" && Number.isFinite(v) && v > 0,
  ),
  JSON.stringify(authored),
);
check(
  "LLKUnits::StudsToCm is readable",
  Number.isFinite(studsToCm) && studsToCm > 0,
  `parsed ${studsToCm}`,
);
console.log(
  `  note walk ${authored.walk} studs = ${cm(authored.walk)} cm/s, ` +
    `sprint ${authored.sprint} = ${cm(authored.sprint)}, ` +
    `crouch ${authored.crouch} = ${cm(authored.crouch)}`,
);

// ---- 1. fractional studs must survive the type chain ---------------------
//
// 7426625 authors sprint 11.5 and crouch 4.5. At these speeds one stud is 9%
// of sprint, too coarse to hold the escape margin, so the fractions are
// deliberate. They are only safe if every type between the JSON and the
// movement component is floating point. An int32 anywhere rounds 11.5 to 11
// and takes 14 cm/s off sprint without saying so.

console.log("\nFractional studs, and the type chain that has to carry them\n");

const hasFraction = [authored.walk, authored.sprint, authored.crouch].some(
  (v) => !Number.isInteger(v),
);
console.log(
  `  note the authored data ${hasFraction ? "DOES" : "does not"} use fractional studs, ` +
    `so this section ${hasFraction ? "is load-bearing today" : "is guarding against a future value"}`,
);

const lanternData = read("Source/LLK/Public/Data/LLKLanternData.h");
for (const field of ["WalkSpeed", "SprintSpeed", "CrouchSpeed"]) {
  const decl = new RegExp(`(\\w+)\\s+${field}\\s*=`).exec(lanternData)?.[1];
  check(
    `FLLKLanternDef::${field} is a float, not an integer`,
    decl === "float" || decl === "double",
    `declared as '${decl}' in Source/LLK/Public/Data/LLKLanternData.h`,
  );
}

const enemyDef = read("Source/LLK/Public/Data/LLKEnemyDef.h");
const moveSpeedType = /(\w+)\s+MoveSpeed\s*=/.exec(enemyDef)?.[1];
check(
  "FLLKEnemyDef::MoveSpeed is a float, so a fractional enemy speed is possible too",
  moveSpeedType === "float" || moveSpeedType === "double",
  `declared as '${moveSpeedType}'`,
);

const commandlet = read("Source/LLKEditor/Private/LLKImportDataCommandlet.cpp");
check(
  "the importer reads the movement speeds through the float helper, not the int one",
  /GetNumber\(Movement, TEXT\("sprintSpeed"\)/.test(commandlet) &&
    /GetNumber\(Movement, TEXT\("crouchSpeed"\)/.test(commandlet) &&
    !/GetInt\(Movement, TEXT\("(sprint|crouch|walk)Speed"\)/.test(commandlet),
  "GetNumber goes through double and casts to float; GetInt would truncate",
);
check(
  "the float helper itself is double-in, float-out",
  /bool GetNumber\([^)]*float& OutValue\)[\s\S]{0,200}?double Value = 0\.0;[\s\S]{0,200}?static_cast<float>\(Value\)/.test(
    commandlet,
  ),
  "GetNumber in LLKImportDataCommandlet.cpp",
);

// ---- 2. the importer's own verification must cover what changed ----------
//
// VerifyRuleAssets re-reads the generated DA_Lantern and compares it against
// the JSON it came from, which is exactly the right idea. It compares
// walkSpeed. It does not compare sprintSpeed or crouchSpeed — the two values
// 7426625 made fractional, and therefore the two most likely to be wrong.
//
// A generator that reports success while the thing it generated is wrong is
// the trap this repository keeps finding. Here it is inside the verifier.

console.log("\nThe importer verifies what it writes — or says it does\n");

const verifyBlock =
  /bool VerifyRuleAssets\([\s\S]*?\n\t\}/.exec(commandlet)?.[0] ?? "";
for (const field of ["walkSpeed", "sprintSpeed", "crouchSpeed"]) {
  check(
    `VerifyRuleAssets compares ${field} against the authored payload`,
    verifyBlock.includes(`TEXT("${field}")`),
    `${field} is written by ImportRuleAssets but never read back, so a bad ` +
      `value survives the import's own success report`,
  );
}

const enemyVerify = /bool VerifyTable\([\s\S]*?\n\t\}/.exec(commandlet)?.[0] ?? "";
check(
  "VerifyEnemyMoveSpeeds reloads DT_Enemies and compares every moveSpeed",
  /bool VerifyEnemyMoveSpeeds\([\s\S]*?FindRow<FLLKEnemyDef>[\s\S]*?Generated->MoveSpeed[\s\S]*?ExpectedMoveSpeed/.test(
    commandlet,
  ),
  "row names alone do not prove the generated MoveSpeed values survived JSON conversion",
);

// ---- 3. no hardcoded copy of the scale may diverge from the data ---------
//
// The real find. Two presentation denominators hold 448.f — the old walk
// speed — and both are used as `Speed / 448.f` to drive a 0..1 fraction. With
// walk now 224 a walking keeper produces 0.5 instead of 1.0, so the lantern
// sway and the carry bob run at half rate and half amplitude at walking pace.
// Nothing errors. The lantern just stops reading as a carried object.

console.log("\nHardcoded copies of the scale, which is where this bites\n");

const sourceFiles = (() => {
  const out = [];
  const walk = (dir) => {
    for (const e of readdirSync(join(root, dir), { withFileTypes: true })) {
      const p = `${dir}/${e.name}`;
      if (e.isDirectory()) walk(p);
      else if (/\.(cpp|h)$/.test(e.name)) out.push(p);
    }
  };
  if (existsSync(join(root, "Source"))) walk("Source");
  return out;
})();

// Every cm/s figure the OLD scale produced. A literal matching one of these
// is a copy that did not move when the data did.
const stale = new Map();
for (const [name, oldStuds] of [
  ["walk", authored.walk * 2],
  ["sprint", authored.sprint * 2],
  ["crouch", authored.crouch * 2],
]) {
  stale.set(cm(oldStuds), `${name} at the pre-7426625 scale (${oldStuds} studs)`);
}

const hits = [];
for (const file of sourceFiles) {
  // Tests may legitimately name an old value while describing a regression.
  if (file.includes("/Tests/")) continue;
  const lines = read(file).split("\n");
  lines.forEach((line, i) => {
    // Code only: a trailing or leading comment marker means prose, which is
    // stale documentation rather than a stale computation.
    const code = line.replace(/\/\/.*$/, "").replace(/\/\*[\s\S]*?\*\//g, "");
    if (/^\s*\*/.test(line) || !code.trim()) return;
    for (const [value, why] of stale) {
      const literal = new RegExp(`(?<![\\d.])${value}(\\.\\d*)?f?(?![\\d.])`);
      if (literal.test(code)) {
        hits.push({ file, line: i + 1, value, why, text: line.trim() });
      }
    }
  });
}

check(
  "no source file holds a cm/s literal from the superseded scale",
  hits.length === 0,
  hits.length === 0
    ? ""
    : hits
        .map(
          (h) =>
            `${relative(".", h.file)}:${h.line} holds ${h.value} — ${h.why}\n` +
            `         ${h.text}\n` +
            `         fix by reading the resolved speed (ALLKCharacter::GetWalkSpeed())\n` +
            `         rather than by editing the constant to 224`,
        )
        .join("\n       "),
);

// ---- 4. the declared fallback is a promise about the data ----------------
//
// ALLKCharacter logs LOUDLY when it runs on fallback speeds, and the comment
// beside that log says why: "gameplay reads the data rather than a constant
// that happens to match it today." The fallback is only honest while it
// matches. After a rescale that leaves it behind, a machine whose DA_Lantern
// does not resolve plays the whole game at the old scale and the warning it
// prints names numbers that are no longer anybody's intent.

console.log("\nThe declared fallback, which is a promise that it matches\n");

const character = read("Source/LLK/Public/Characters/LLKCharacter.h");
for (const [field, want, name] of [
  ["FallbackWalkStuds", authored.walk, "walk"],
  ["FallbackSprintStuds", authored.sprint, "sprint"],
  ["FallbackCrouchStuds", authored.crouch, "crouch"],
]) {
  const got = Number(
    new RegExp(`${field}\\s*=\\s*([0-9.]+)f`).exec(character)?.[1],
  );
  check(
    `${field} matches the authored ${name} speed`,
    Number.isFinite(got) && Math.abs(got - want) < 1e-6,
    `declared ${got} studs, data/lantern.json authors ${want} — ` +
      `a run on fallback moves at ${cm(got)} cm/s instead of ${cm(want)}`,
  );
}

// ---- report --------------------------------------------------------------

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("These are copies of a number that moved. The fix is not to edit");
  console.log("each copy to the new value — that only resets the clock until the");
  console.log("next rescale. It is to read the resolved speed, and to keep the");
  console.log("declared fallback equal to the authored data by construction.");
  console.log("See docs/FINDINGS-SPEED-SCALE-COPIES.md.\n");
  process.exit(1);
}
console.log("");
