// Extraction-source guard. M3.
//
// **The rule this protects.** Row 73: *"Clock means weather alone... A radio
// call cannot accelerate the weather."* Extraction is the single most tempting
// place to break that, because the call used to pay 180 into the meter and the
// JSON key that paid it is still sitting in `storm-charge.json` waiting for
// Codex's importer to move. Somebody wiring the climax up could read
// `extractionCall`, find three fields, and use all three.
//
// **Why a source guard rather than a unit test**, which is the same argument
// `escalation.test.mjs` makes and worth restating because it is the whole
// reason this file exists. `LLK.Run.Extraction` asserts the state machine's
// behaviour thoroughly, but it cannot assert that the state machine never
// learns about the clock: a pure function given no clock will never touch one,
// so the test would pass just as happily on a version that took a
// `ULLKStormChargeSubsystem*`. The constraint is architectural - the rule
// layer must be UNABLE to reach the meter - and architecture is checkable by
// inspection.
//
// **The three properties, in order of how much they would cost to lose:**
//
//   1. The pure layer cannot see the clock at all. No include, no subsystem,
//      no spike, and `FLLKExtractionAnnounce` carries two of `extractionCall`'s
//      three fields on purpose.
//   2. The subsystem may read the siren and the convergence flag, and may add
//      a distress pulse, and may NOT advance or charge the meter. A pulse is a
//      light source - a local consequence, which is exactly what row 72 left
//      light allowed to do.
//   3. The hold cannot resolve itself from an unmeasured route. The `TOptional`
//      is the mechanism; a `.Get(` with a default on that field would defeat
//      it silently, which is precisely the defect that was planted to prove
//      the in-engine test catches it.
//
//   node tests/extraction.test.mjs

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

// Comments stripped before matching, so this file's own explanation of the
// rule - and the headers' - cannot trip it. units.test.mjs's approach.
const stripComments = (src) =>
  src.replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/[^\n]*/g, "");

const read = (rel) => {
  const full = join(root, rel);
  if (!existsSync(full)) return null;
  return readFileSync(full, "utf8");
};

const PURE_HEADER = "Source/LLK/Public/Run/LLKExtraction.h";
const PURE_IMPL = "Source/LLK/Private/Run/LLKExtraction.cpp";
const SUBSYSTEM_HEADER = "Source/LLK/Public/Run/LLKExtractionSubsystem.h";
const SUBSYSTEM_IMPL = "Source/LLK/Private/Run/LLKExtractionSubsystem.cpp";
const TEST = "Source/LLK/Private/Tests/LLKExtractionTest.cpp";

for (const rel of [PURE_HEADER, PURE_IMPL, SUBSYSTEM_HEADER, SUBSYSTEM_IMPL, TEST]) {
  check(`${rel} exists`, read(rel) !== null, "the guard names a file that is not there");
}

// ---- 1. the rule layer cannot reach the clock ---------------------------
{
  const header = stripComments(read(PURE_HEADER) ?? "");
  const impl = stripComments(read(PURE_IMPL) ?? "");
  const both = `${header}\n${impl}`;

  for (const forbidden of [
    "LLKStormChargeSubsystem",
    "ULLKStormChargeData",
    "StormChargeSpike",
    "AddSpike",
    "GetCharge",
    "ZonePressure",
  ]) {
    check(
      `ROW 73: the extraction rules never name ${forbidden}`,
      !both.includes(forbidden),
      "the pure layer decides what extraction does; a rule that can see the " +
        "meter is a rule that can charge it, and the call may not move the weather",
    );
  }

  // No world either. The two-halves pattern, same as LLKMapReadiness and
  // LLKLanternFeel: a judgement function that touches a UWorld cannot be
  // exercised without a level, and this one has no level to be exercised in.
  for (const forbidden of ["UWorld", "GetWorld", "AActor", "UObject*"]) {
    check(
      `the extraction rules never name ${forbidden}`,
      !both.includes(forbidden),
      "the pure half measures nothing. ULLKExtractionSubsystem is the half " +
        "that is allowed to know a world exists",
    );
  }

  // The announce struct's omission is load-bearing, so it is asserted rather
  // than trusted to the header comment that explains it.
  const announce = header.slice(header.indexOf("struct FLLKExtractionAnnounce"));
  const announceBody = announce.slice(0, announce.indexOf("};"));
  check(
    "ROW 73: FLLKExtractionAnnounce carries the siren",
    /SirenSeconds/.test(announceBody),
  );
  check(
    "and the convergence flag",
    /bConvergeAllPatrols/.test(announceBody),
  );
  check(
    "ROW 73: and has nowhere to put a spike",
    !/[Ss]pike/.test(announceBody),
    "two of extractionCall's three fields, deliberately. A field is how the " +
      "retired rule would come back",
  );
}

// ---- 2. the subsystem's local consequence, and only that ----------------
{
  const impl = stripComments(read(SUBSYSTEM_IMPL) ?? "");

  check(
    "the subsystem reads the authored siren length",
    /ExtractionCall\.SirenSeconds/.test(impl),
    "8 seconds is Chris's number and belongs in data, not in C++",
  );
  check(
    "and the convergence flag",
    /ExtractionCall\.bConvergeAllPatrols/.test(impl),
  );
  check(
    "ROW 73: and never reads the retired spike beside them",
    !/StormChargeSpike/.test(impl),
    "the field still exists in the data asset until Codex's rename lands; " +
      "reading it is the regression this guard exists for",
  );

  for (const forbidden of ["AddSpike", "ApplySpike", "AdvanceForTest(", "Advance("]) {
    check(
      `ROW 73: the subsystem never calls Storm->${forbidden.replace("(", "")}`,
      !new RegExp(`Storm\\s*->\\s*${forbidden.replace("(", "\\(")}`).test(impl),
      "the only function that writes the meter is the clock's own Advance, " +
        "and the clock calls it from its own tick",
    );
  }

  // Row 74's convergence IS allowed, because a light is a local consequence.
  check(
    "ROW 74: convergence is spent as light rather than as weather",
    /AddDistressPulse/.test(impl),
    "the rig arrives with its lamps lit; the building notices the light. " +
      "Row 72 left light allowed to do exactly this and nothing more",
  );
}

// ---- 3. the hold cannot resolve from an unmeasured route ----------------
{
  const impl = stripComments(read(PURE_IMPL) ?? "");

  check(
    "the hold checks that somebody measured the route",
    /KeepersOnBoardingRoute\.IsSet\(\)/.test(impl),
    "unset is nobody measuring, which is the normal case while there is no " +
      "dock - and it must not behave like an empty route that still secures",
  );
  check(
    "and never defaults that measurement away",
    !/KeepersOnBoardingRoute\.Get\(/.test(impl),
    "a Get with a default turns 'nobody measured' into a number, which is " +
      "the exact defect this project plants to check its own tests",
  );
  check(
    "and the only way to secure is to reach the end of the hold",
    (impl.match(/ELLKExtractionPhase::Secured;/g) ?? []).length === 1,
    "one assignment, in one branch, after the hold has been held",
  );
}

// ---- 4. the behavioural half exists and says so ------------------------
{
  const test = read(TEST) ?? "";
  check(
    "the in-engine state machine test still exists",
    test.includes('"LLK.Run.Extraction"'),
    "this file guards architecture only. Deleting the automation test would " +
      "leave every rule about holds, cancels and wipes unchecked",
  );

  // The tuning decision the header flags, asserted so that flipping it is a
  // deliberate act with a failing test attached rather than a quiet edit.
  check(
    "and it pins the pause-never-rewind decision by name",
    /leaving the route costs time, never progress/.test(test),
    "if Chris wants a reset instead, this is the assertion that should fail " +
      "first and be changed on purpose",
  );
}

// ---- 5. the authored exits still say what the rules assume -------------
{
  const loot = JSON.parse(readFileSync(join(root, "data/loot.json"), "utf8"));
  const exits = loot.extraction ?? {};
  const ids = Object.keys(exits);

  check("there are exits to call", ids.length >= 2, "data/loot.json extraction");

  for (const id of ids) {
    const exit = exits[id];
    check(
      `${id} authors a hold`,
      typeof exit.arrivalSeconds === "number" && exit.arrivalSeconds > 0,
      "loot.json's own words: never zero, because an exit that costs nothing " +
        "is not a decision. LLKExtraction::Call refuses one as NoHoldAuthored",
    );
    check(
      `${id} authors a payout`,
      typeof exit.payoutModifier === "number" && exit.payoutModifier > 0,
    );
  }

  const siren = JSON.parse(readFileSync(join(root, "data/storm-charge.json"), "utf8"))
    .extractionCall;
  check(
    "the siren is authored and audible",
    typeof siren?.sirenSeconds === "number" && siren.sirenSeconds > 0,
    "the warning is the whole price of calling; a zero-second siren means the " +
      "rig arrives with no announcement at all",
  );
  check(
    "ROW 74: and the call still converges the building",
    siren?.convergeAllPatrols === true,
    "row 74 gave this its reason - a full recovery operation is the largest " +
      "light-and-motion event of the night",
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read DECISIONS.md rows 73 and 74 before changing an assertion here.\n");
  process.exit(1);
}
console.log("");
