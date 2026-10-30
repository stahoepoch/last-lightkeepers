// Enemy data invariants. M2.4's data half.
//
// `LLK.Enemies.Perception` tests the RULE; this file tests the DATA the rule
// reads. The split matters because the rule's correctness is about senses and
// light, while the data's correctness is about six families staying internally
// consistent while Chris retunes them - and a C++ test that hardcoded all six
// stat lines would fail on every retune, which trains people to edit the test
// instead of reading it.
//
// What this guards, and why each one is here rather than imagined:
//
//   1. **"Absent means absent" has to stay unambiguous.** `FLLKEnemyDef` uses
//      `TOptional` so a family without `aggroRadiusLit` is distinguishable
//      from one with a zero. That only works while no family AUTHORS a zero
//      for a sense it has - the moment one does, `LLKPerception`'s
//      `WithinStuds` guard treats it as unreachable and the two cases become
//      the same thing again.
//   2. **One sense per family.** `SenseReaching` resolves ties by enum order
//      and its comment promises that no family in the data has two senses
//      that can both fire. That promise is checkable, so it is checked: if a
//      family ever gets two, the precedence becomes a design decision
//      somebody has to make rather than one the function already made.
//   3. **Row 64 line three, reaching into the roster.** Escalation may never
//      be caused by failure. `Glimmers.tagAddsPressure` is 0, and being
//      tagged is something that HAPPENS to a crew rather than something they
//      chose - so a non-zero value here would be failure feeding the meter.
//      It would also need an `ELLKStormSpike` entry, which
//      `escalation.test.mjs` guards, so this is the same rule caught one step
//      earlier and in the place somebody would actually edit.
//   4. **Row 16: every enemy telegraphs.** `audioTelegraphSeconds` is
//      universal and a zero there is a design bug, not a quiet enemy.
//   5. **Row 7: the Warden stays unkillable and senseless.** The one family
//      whose absences are load-bearing.
//
//   node tests/enemies.test.mjs

import { readFileSync, existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");

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

const data = JSON.parse(readFileSync(join(root, "data/enemies.json"), "utf8"));
const families = Object.entries(data).filter(
  ([key, value]) => !key.startsWith("_") && value && typeof value === "object",
);

// The roster's size is a guardrail with a documented history - `_scopeNote`
// records that it moved from four to five to six, each time by Chris's
// approval. So this asserts the count is deliberate, not that it is six
// forever: a seventh is fine, it just has to arrive with the note updated.
check(
  "the roster has the six families the data claims",
  families.length === 6,
  `found ${families.length}: ${families.map(([k]) => k).join(", ")}. ` +
    `_scopeNote says the number moves only by Chris's approval`,
);

// ---- the senses, and which fields constitute one -------------------------
//
// Mirrors LLKPerception::SenseReaching exactly. A sense is the SET of fields
// the rule requires, not any one of them - which is why Sweepers need both
// radii and Stockers need both the range and the cone.
const SENSES = {
  LitProximity: ["aggroRadiusDark", "aggroRadiusLit"],
  SightCone: ["sightRange", "sightConeDegrees"],
  Scan: ["scanRange"],
  Wake: ["wakeRange"],
};

for (const [name, family] of families) {
  // ---- 4. row 16 ---------------------------------------------------------
  check(
    `${name} telegraphs before sight`,
    typeof family.audioTelegraphSeconds === "number" && family.audioTelegraphSeconds > 0,
    "row 16 is an accessibility requirement; a zero here is a design bug, " +
      "not a quiet enemy",
  );

  // ---- 1. no zero stands in for an absence ------------------------------
  const senseFields = new Set(Object.values(SENSES).flat());
  for (const field of senseFields) {
    if (!(field in family)) continue;
    check(
      `${name}.${field} is a real value, not a zero standing in for absence`,
      typeof family[field] === "number" && family[field] > 0,
      "FLLKEnemyDef uses TOptional so absence is expressible; a zero makes " +
        "'this family has no such sense' and 'this sense reaches nothing' the " +
        "same state again",
    );
  }

  // ---- 2. one sense per family ------------------------------------------
  const present = Object.entries(SENSES)
    .filter(([, fields]) => fields.every((f) => typeof family[f] === "number"))
    .map(([sense]) => sense);

  if (name === "Warden") {
    // ---- 5. row 7 --------------------------------------------------------
    check(
      "the Warden has no sense at all - it converges, it does not notice",
      present.length === 0,
      `found ${present.join(", ")}. Row 7 and row 54: it arrives on its own ` +
        `meter and converges on the brightest position. A radius here is the ` +
        `first step to making it a boss`,
    );
    check(
      "and the Warden is still unkillable",
      family.killable === false,
      "row 7. If it ever becomes killable the meter stops meaning anything",
    );
    continue;
  }

  check(
    `${name} has exactly one sense that can fire`,
    present.length === 1,
    `found ${present.length}: ${present.join(", ") || "none"}. ` +
      `LLKPerception::SenseReaching resolves ties by enum order and documents ` +
      `that no family needs it to; two senses makes that precedence a design ` +
      `decision somebody has to make deliberately`,
  );
}

// ---- 3. row 64 line three, in the roster ---------------------------------
{
  const glimmers = data.Glimmers ?? {};
  check(
    "being tagged does not feed zone pressure",
    glimmers.tagAddsPressure === 0,
    "row 64 line three: escalation comes from choices the crew can identify, " +
      "never from failing. A tag happens TO a crew. A non-zero value here " +
      "would also need an ELLKStormSpike entry, which escalation.test.mjs " +
      "guards - this is the same rule caught where somebody would edit it",
  );
  check(
    "and a tag does not hand the Warden a permanent waypoint",
    glimmers.tagGivesWardenPermanentWaypoint === false,
    "row 7 keeps the Warden an escalation rather than a punishment; a " +
      "permanent waypoint earned by being scanned is failure compounding " +
      "itself with no decision left to make",
  );
}

// ---- the relationship the floor plan is built on -------------------------
//
// `LLKStoreFloorPlan`'s one hard rule is that no sightline exceeds the range a
// LIT keeper is noticed from, and `balance.test.mjs` pins the lantern's reach
// against the same number. This is the third leg: the asymmetry itself.
{
  const sweepers = data.Sweepers ?? {};
  check(
    "a lit keeper is noticed from further than a dark one",
    sweepers.aggroRadiusLit > sweepers.aggroRadiusDark,
    "this asymmetry IS the lantern decision. Inverted or equal, light costs " +
      "nothing and the whole game stops teaching",
  );
  check(
    "and by enough to be felt rather than measured",
    sweepers.aggroRadiusLit >= sweepers.aggroRadiusDark * 2,
    "a lit radius a few studs larger is a difference a player cannot notice, " +
      "which is a trade nobody learns to make",
  );
}

// ---- ROW 16: the brain cannot skip the telegraph ------------------------
//
// Chris, 2026-10-30, on why the Sweeper comes before the siren: it "answers
// the real question: whether extraction is a tense decision or twenty seconds
// of waiting." It only answers that fairly if the announcement cannot be
// skipped, which is row 16 - sound before sight, as a build requirement.
//
// `LLK.Enemies.SweeperBrain` asserts the behaviour. This asserts what a
// behavioural test cannot: that the brain's own code has no path from noticing
// to moving which does not pass through the announcement, and that it stays
// world-free so the encounter can be judged before a map exists.
{
  const stripBrainComments = (src) =>
    src.replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/[^\n]*/g, "");

  const brainPath = "Source/LLK/Private/Enemies/LLKSweeperBrain.cpp";
  const brainFull = join(root, brainPath);
  const brain = existsSync(brainFull) ? stripBrainComments(readFileSync(brainFull, "utf8")) : "";

  check("the Sweeper brain exists", brain.length > 0, brainPath);

  check(
    "ROW 16: noticing a keeper enters Telegraph",
    /State = ELLKSweeperState::Telegraph;/.test(brain),
    "a Sweeper that went straight to Pursue would be a fair fight with an " +
      "unfair presentation",
  );
  check(
    "ROW 16: and Pursue is only ever entered from Telegraph or Search",
    (brain.match(/State = ELLKSweeperState::Pursue;/g) ?? []).length === 2,
    "two: the telegraph finishing, and a search re-acquiring. A third would " +
      "be a path from noticing to chasing with no announcement in between",
  );
  check(
    "ROW 16: the telegraph length is authored rather than a constant",
    /Def\.AudioTelegraphSeconds/.test(brain),
    "audioTelegraphSeconds is Chris's number, per family (row 55)",
  );

  for (const forbidden of ["UWorld", "GetWorld", "AActor", "MoveTo", "NavigationSystem"]) {
    check(
      `the brain never names ${forbidden}`,
      !brain.includes(forbidden),
      "the pawn measures and moves; the brain decides. The same split " +
        "LLKMapReadiness and LLKLanternFeel use",
    );
  }
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read data/enemies.json and Source/LLK/Public/Enemies/LLKPerception.h.\n");
  process.exit(1);
}
console.log("");
