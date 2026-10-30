// The map-readiness contract's prose and its code, kept in agreement.
//
// `LLK.Harness.MapReadiness` tests the judgement and `docs/MAP-READINESS.md`
// explains it to Chris, which leaves one gap neither of them can close: the
// document and the code drifting apart. A check code renamed in C++ and not in
// the document produces a log line nobody can look up, and a document row with
// no code behind it is a promise about a check that does not run.
//
// Precedent: `escalation.test.mjs` guards the threshold EFFECT sentences the
// same way, and that guard exists because `MakeFallbackRules` shipped
// thresholds with no effect text and a test caught it only after the fact.
//
// This file reads source and prose. It is deliberately not clever about either
// - substring checks, because the failure it is guarding against is a word
// changing in one place, and a parser would be a second thing to get wrong.

import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const read = (...p) => readFileSync(join(root, ...p), "utf8");

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

const doc = read("docs", "MAP-READINESS.md");
const header = read("Source", "LLK", "Public", "Harness", "LLKMapReadiness.h");
const impl = read("Source", "LLK", "Private", "Harness", "LLKMapReadiness.cpp");
const plan = read("Source", "LLK", "Public", "Harness", "LLKStoreFloorPlan.h");
const enemies = JSON.parse(read("data", "enemies.json"));

// ---- 1. every check code is documented, named, and reachable -----------
//
// The enum is the authority. Each value has to appear in the document's table,
// in NameOf's switch, and nowhere as a code the document invented.
const codes = [
  ...header.matchAll(/^\t(\w+)(?:,|$)/gm),
]
  .map((m) => m[1])
  // The enum block is the only place in that header with bare identifiers at
  // one tab followed by a comma, but filter to the ones NameOf knows about so
  // a future struct member cannot quietly join the list.
  .filter((name) => impl.includes(`ELLKMapCheck::${name}`));

check(
  "the check enum was found at all",
  codes.length >= 14,
  `found ${codes.length} codes, expected at least 14 — the parse above may have broken`,
);

for (const code of codes) {
  check(`${code} appears in docs/MAP-READINESS.md`, doc.includes(code));
  check(
    `${code} has a name in NameOf`,
    impl.includes(`return TEXT("${code}")`),
    "a code with no name prints as Unnamed and cannot be looked up",
  );
}

// And the reverse: a table row naming a code that does not exist.
//
// Read out of the table's third column rather than out of the prose. The first
// version of this matched every backticked identifier in the file and reported
// `NavMeshBoundsVolume` - an engine class, correctly named - as a missing
// check code. A guard that cries wolf about correct prose gets switched off.
const tableCodes = new Set();
for (const line of doc.split("\n")) {
  if (!line.startsWith("| ")) continue;
  const columns = line.split("|").map((c) => c.trim());
  if (columns.length < 5) continue;
  for (const m of columns[3].matchAll(/`(\w+)`/g)) tableCodes.add(m[1]);
}

check(
  "the requirements table was parsed",
  tableCodes.size >= 14,
  `found ${tableCodes.size} codes in the table's third column`,
);

for (const code of tableCodes) {
  check(
    `the table's ${code} is a real check code`,
    codes.includes(code),
    "the document promises a check the code does not have",
  );
}

// ---- 2. the numbers the contract rests on ------------------------------
const sweeperLit = enemies.Sweepers?.aggroRadiusLit;
check(
  "Sweepers still author a lit aggro radius",
  typeof sweeperLit === "number" && sweeperLit > 0,
  "the occlusion check's 'cover that matters' range is derived from it",
);
check(
  `the cover range in LLKMapReadiness.cpp is the lit notice range (${sweeperLit})`,
  impl.includes(`CoverMattersWithinStuds = ${sweeperLit}.f`),
  "geometry further than a keeper can be noticed from is not cover for the keeper being hunted",
);
check(
  `the document states the lit notice range as ${sweeperLit}`,
  doc.includes(String(sweeperLit)),
);

const planLimit = plan.match(/MaxSightlineStuds = ([\d.]+)f/)?.[1];
check(
  "the floor plan still authors a sightline limit",
  planLimit !== undefined,
  "requirement 7 is checked against it",
);
check(
  `the document's sightline limit matches the plan's (${planLimit})`,
  doc.includes(`(${parseFloat(planLimit)})`),
  "the one number Chris tunes by eye, stated twice and allowed to disagree",
);
check(
  "the sightline limit is under the lit notice range",
  parseFloat(planLimit) < sweeperLit,
  "a legal sightline longer than the notice range means being hunted from outside your own vision",
);

// ---- 3. eye height, which is the number a sight break must beat --------
//
// Stated in the document, in the header, and used in the implementation. Three
// places, because it is derived from two engine defaults and nobody will
// re-derive it when a capsule changes.
check("eye height is 152cm in the implementation", impl.includes("EyeHeightCm = 152.f"));
check("and 152 in the document", doc.includes("152"));
check(
  "and the document says which two numbers it came from",
  doc.includes("64") && doc.includes("88"),
  "an unexplained 152 is a number nobody can check against a changed capsule",
);

const shelf = plan.match(/ShelfHeightStuds = ([\d.]+)f/)?.[1];
check(
  `the plan's shelving (${shelf} studs) clears eye height`,
  parseFloat(shelf) * 28 > 152,
  "shelving at or below 152cm reads as cover and occludes nothing",
);

const counter = plan.match(/CounterHeightStuds = ([\d.]+)f/)?.[1];
check(
  `and the plan's counters (${counter} studs) deliberately do not`,
  parseFloat(counter) * 28 < 152,
  "the floor needs both, or 'get behind something' has one answer",
);

// ---- 4. the actor tags the document tells Chris to type ----------------
for (const tag of ["LLK.EnemySpawn", "LLK.TestLane.Open", "LLK.TestLane.Blocked"]) {
  check(`the tag ${tag} matches between document and code`, doc.includes(tag) && impl.includes(`TEXT("${tag}")`));
}

// ---- 5. the handoff stays findable ------------------------------------
check(
  "the NavigationSystem handoff is marked in the code",
  impl.includes("HANDOFF(Codex): NavigationSystem"),
  "the one line that turns RouteUnverified into a real answer",
);
check(
  "and written out in the document",
  doc.includes("NavigationSystem") && doc.includes("LLK.Build.cs"),
);

// ---- 6. the console command the document tells Chris to type ----------
check(
  "LLK.CheckMap is the command both the document and the code register",
  doc.includes("LLK.CheckMap") && impl.includes('TEXT("LLK.CheckMap")'),
);

console.log(`\n${checks} checks, ${failures} failed`);
process.exit(failures === 0 ? 0 : 1);
