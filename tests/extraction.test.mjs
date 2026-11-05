// Extraction-source guard. M3.
//
// **The rule this protects.** Row 73: *"Clock means weather alone... A radio
// call cannot accelerate the weather."* Extraction is the single most tempting
// place to break that, because the call used to pay 180 into the meter and the
// JSON key that paid it is still sitting in `zone-pressure.json` waiting for
// Codex's importer to move. Somebody wiring the climax up could read
// `extractionCall`, find three fields, and use all three.
//
// **Why a source guard rather than a unit test**, which is the same argument
// `escalation.test.mjs` makes and worth restating because it is the whole
// reason this file exists. `LLK.Run.Extraction` asserts the state machine's
// behaviour thoroughly, but it cannot assert that the state machine never
// learns about the clock: a pure function given no clock will never touch one,
// so the test would pass just as happily on a version that took a
// `ULLKZonePressureSubsystem*`. The constraint is architectural - the rule
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

import { readFileSync, existsSync, readdirSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join, sep } from "node:path";

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

// Section 6 walks the runtime module rather than naming files, because the
// caller it is looking for does not exist yet and so has no name to name.
function walkCpp(dir, out = []) {
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const full = join(dir, entry.name);
    if (entry.isDirectory()) walkCpp(full, out);
    else if (entry.name.endsWith(".cpp")) out.push(full);
  }
  return out;
}

const PURE_HEADER = "Source/LLK/Public/Run/LLKExtraction.h";
const PURE_IMPL = "Source/LLK/Private/Run/LLKExtraction.cpp";
const SUBSYSTEM_HEADER = "Source/LLK/Public/Run/LLKExtractionSubsystem.h";
const SUBSYSTEM_IMPL = "Source/LLK/Private/Run/LLKExtractionSubsystem.cpp";
const TEST = "Source/LLK/Private/Tests/LLKExtractionTest.cpp";
const ZONE_TEST = "Source/LLK/Private/Tests/LLKExtractionZoneTest.cpp";
const READOUT_IMPL = "Source/LLK/Private/UI/LLKExtractionReadout.cpp";
const HUD_IMPL = "Source/LLK/Private/UI/LLKDebugHUD.cpp";

for (const rel of [PURE_HEADER, PURE_IMPL, SUBSYSTEM_HEADER, SUBSYSTEM_IMPL, TEST, ZONE_TEST, READOUT_IMPL, HUD_IMPL]) {
  check(`${rel} exists`, read(rel) !== null, "the guard names a file that is not there");
}

// ---- 1. the rule layer cannot reach the clock ---------------------------
{
  const header = stripComments(read(PURE_HEADER) ?? "");
  const impl = stripComments(read(PURE_IMPL) ?? "");
  const both = `${header}\n${impl}`;

  for (const forbidden of [
    "LLKZonePressureSubsystem",
    "ULLKZonePressureData",
    "StormChargeSpike",
    "AddSpike",
    "GetPressure",
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
    /AddOrRefreshDistressPulse/.test(impl),
    "the rig arrives with its lamps lit; the building notices the light. " +
      "Row 72 left light allowed to do exactly this and nothing more",
  );

  // ROW 75, Chris's lock on the pause: "the siren, rig lights, and converged
  // patrols persist, so stepping away buys survival time, not a free reset or
  // a safer retry." That is a rule about a light's DURATION, and no pure
  // function can hold it - a hold that pauses has no knowable length, so a
  // pulse lit once for siren-plus-hold seconds would lapse underneath a long
  // pause and make hiding the safe play. The light has to be re-armed while
  // the operation runs, and the shape of that is checkable here.
  check(
    "ROW 75: the rig's light is re-armed while the extraction is underway",
    /IsUnderway\(\)\s*\)\s*\{\s*RefreshRigLight\(\);/.test(impl),
    "including while the hold is stalled: a crew that steps off the route has " +
      "not ended the operation, and the dock is still lit",
  );
  check(
    "ROW 75: and its duration is never computed from the siren plus the hold",
    !/SirenSecondsRemaining\s*\+\s*State\.HoldSecondsRequired/.test(impl),
    "that was the original version and it was wrong: a paused hold outlives " +
      "it, the dock goes dark mid-extraction, and stepping away becomes the " +
      "safe play rather than a survival cost",
  );
  check(
    "ROW 75: one named pulse rather than one per frame",
    /RigLightSourceName\(\)/.test(impl),
    "an unowned pulse re-armed every tick is sixty lights a second, each one " +
      "walked by the Warden's brightest-position search",
  );
}

// ---- 2b. the subsystem does no geometry of its own ---------------------
//
// Chris, 2026-10-30: the zone actor *"should own only the trigger volume and
// which keepers are currently inside, feeding the state machine's pause/advance
// rule."* For one day the subsystem measured a radius itself, and the reason
// that had to go is not tidiness: a sphere cannot describe a bay mouth, and two
// places doing geometry is how they come to disagree about who is in the bay.
{
  const impl = stripComments(read(SUBSYSTEM_IMPL) ?? "");
  const header = stripComments(read(SUBSYSTEM_HEADER) ?? "");

  for (const forbidden of ["DistSquared", "RadiusCm *", "FVector::Dist"]) {
    check(
      `the subsystem measures no distances of its own (${forbidden.trim()})`,
      !impl.includes(forbidden),
      "the zone owns the volume and answers the question; the subsystem asks " +
        "it, the way perception asks an ILLKVisibilitySource",
    );
  }

  check(
    "and the radius-based route is gone rather than left beside it",
    !impl.includes("FLLKBoardingRoute") && !header.includes("FLLKBoardingRoute"),
    "a placeholder kept next to its replacement is a second way to measure",
  );

  // ROW 75's rule, and the reason the zone reports two numbers: a downed
  // keeper is in the bay and holds nothing, because row 74 is explicit that no
  // driver comes inside to collect anybody.
  check(
    "ROW 75: the hold reads who is STANDING in the zone",
    /CountKeepersStanding\(\)/.test(impl),
    "CountKeepersInside is geometry and belongs to a readout; only standing " +
      "keepers hold a boarding route",
  );
  check(
    "and never advances on a bay full of downed keepers",
    !/ObserveBoardingRoute[\s\S]{0,400}CountKeepersInside/.test(impl),
    "row 74: nobody comes into the building to collect anybody",
  );
  check(
    "one zone per exit, with the second refused rather than replacing the first",
    /is a second zone for/.test(impl),
    "replacing would make the answer depend on which zone began play last",
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

  const zoneTest = read(ZONE_TEST) ?? "";
  check(
    "the zone's occupancy test exists",
    zoneTest.includes('"LLK.Run.ExtractionZone"'),
    "entering twice, a destroyed keeper and a downed keeper are three ways a " +
      "hold could advance with an empty bay",
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

// ---- 4b. the words live in one place, and the HUD only draws them ------
//
// Chris, 2026-10-30: *"It must not become another top-left debug paragraph...
// Put the actionable prompt in-world at the zone; reserve centre-screen
// messaging for the active climax."* The readout is a debug HUD today and his
// real widget later, so the wording and the rules about when to show it live
// in `LLKExtractionReadout` - otherwise the real widget starts by
// reimplementing them from a screenshot.
{
  const readout = stripComments(read(READOUT_IMPL) ?? "");
  const hud = stripComments(read(HUD_IMPL) ?? "");

  for (const line of ["RECOVERY INCOMING", "HOLD THE DOCK", "RECOVERY HOLDING", "SECURED"]) {
    check(
      `the readout owns the words "${line}"`,
      readout.includes(line),
      "one authority for what extraction says",
    );
    check(
      `and the HUD does not spell "${line}" itself`,
      !hud.includes(line),
      "a HUD that builds its own strings is a second authority, and the one " +
        "that drifts is whichever nobody is reading",
    );
  }

  check(
    "the prompt is the only thing drawn at the zone",
    readout.includes("Hold [E]") && !hud.includes("Hold [E]"),
  );

  // The centre slot is for the climax only, so an idle run fills neither slot.
  // Sliced to the Idle branch's own `return` rather than matched with a
  // character budget: the branches sit next to each other once comments are
  // stripped, and a window wide enough to cover one reaches into the next.
  const idleAt = readout.indexOf("case ELLKExtractionPhase::Idle:");
  const idleBranch = idleAt < 0
    ? ""
    : readout.slice(idleAt, readout.indexOf("return Readout;", idleAt));
  check(
    "ROW 75: an idle run says nothing centre screen",
    idleBranch.length > 0 && !idleBranch.includes("Readout.Centre"),
    "a permanent line about extraction is the top-left paragraph by another route",
  );

  // And the HUD's prompt agrees with what E actually does, by reading the
  // same focus the key branches on rather than searching again.
  check(
    "the prompt is suppressed when E would do something else",
    /GetFocusedCasualty\(\) == nullptr[\s\S]{0,200}GetFocused\(\) == nullptr/.test(hud),
    "a casualty and a relic both outrank the bay, in the interactor and on " +
      "screen, or the prompt offers a key that lifts a brooch",
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

  const siren = JSON.parse(readFileSync(join(root, "data/zone-pressure.json"), "utf8"))
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

// ---- 6. an authored requirement needs a way to be satisfied -------------
//
// **The defect this exists for, found 2026-11-05.** `ULLKExtractionSubsystem::
// SetRequirementSatisfied` has exactly one caller in the whole project and it
// is the console command `LLK.Extraction.Satisfy`. So `FreightLift`, which
// authors `requires: SubBasementBreakerRestored`, refuses forever with
// `RequirementNotMet` and there is no path through the game that clears it.
// Section 5 above checks that an exit authors a hold and a payout; nothing
// checked that its gate can ever open, which is the difference between an exit
// that costs something and an exit that is not there.
//
// **Why an allowlist rather than a plain assertion.** The defect is real and
// open as `HANDOFFS.md` row M14, and Chris has not decided whether the second
// exit belongs in the first slice at all - so a bare check would land a red
// suite and gate every commit on a design decision that is his. The shape
// `cues.test.mjs` already uses is the honest one: a known gap is listed WITH
// its reason, so the silence is a recorded decision, and **the list itself
// fails when it goes stale** in either direction.
//
// Three ways this goes red, which is what makes it worth having:
//
//   1. A new exit authors `requires` and nobody listed it. That is the next
//      `FreightLift`, caught at authoring time instead of in a playtest.
//   2. A listed exit becomes satisfiable - somebody built the interaction and
//      the entry is now a lie about the code.
//   3. A listed exit stops authoring `requires`, or stops existing. Deleting
//      the `requires` is the tempting wrong fix, because the 1.6x payout is
//      priced against that cost, so this notices it.
{
  const UNSATISFIABLE = new Map([
    [
      "FreightLift",
      "HANDOFFS.md M14. Nothing in the run can restore the sub-basement " +
        "breaker; the fiction has no mechanism yet. Closed by building that " +
        "interaction or by Chris ruling the deep exit out of the first slice " +
        "- NOT by deleting the requires, which is what the 1.6x pays for",
    ],
  ]);

  const loot = JSON.parse(readFileSync(join(root, "data/loot.json"), "utf8"));
  const exits = loot.extraction ?? {};

  // Any gameplay caller at all, which is deliberately coarser than "a caller
  // that sets THIS requirement": the flag is an `FName` assembled at runtime,
  // so tying a call site to an exit by inspection would be a guess. One real
  // caller outside the subsystem is the thing that is false today, and it is
  // the thing whose arrival should prune this list.
  const OWNER = "LLKExtractionSubsystem.cpp";
  const callers = walkCpp(join(root, "Source/LLK/Private"))
    .filter((f) => !f.includes(`${sep}Tests${sep}`) && !f.endsWith(OWNER))
    .filter((f) => /SetRequirementSatisfied\s*\(/.test(stripComments(readFileSync(f, "utf8"))))
    .map((f) => f.slice(f.indexOf("Source")));

  const gated = Object.keys(exits).filter((id) => typeof exits[id].requires === "string");

  check(
    "every gated exit is either satisfiable or listed as not",
    gated.every((id) => callers.length > 0 || UNSATISFIABLE.has(id)),
    `${gated.filter((id) => callers.length === 0 && !UNSATISFIABLE.has(id)).join(", ")} ` +
      "authors a `requires` that no gameplay code can satisfy, and is not on " +
      "the known-gap list. An exit whose gate cannot open is not an exit",
  );

  check(
    "and the known-gap list has not gone stale against the code",
    !(callers.length > 0 && UNSATISFIABLE.size > 0),
    `SetRequirementSatisfied now has gameplay callers (${callers.join(", ")}), ` +
      "so the entries in UNSATISFIABLE are a lie about the code. Re-check " +
      "each gated exit by hand and prune the list",
  );

  for (const id of UNSATISFIABLE.keys()) {
    check(
      `and ${id} is still a gated exit`,
      typeof exits[id]?.requires === "string",
      `${id} no longer authors a requires, so this entry describes nothing. ` +
        "If the gate was deleted rather than built, that is the wrong fix: " +
        "read the entry's own reason",
    );
  }
}

// The far side of the convergence chain, which these tests could not see.
//
// Rows 74 and 75 above guard the extraction subsystem's half: the rig plants
// `AddOrRefreshDistressPulse` under one `RigLightSourceName()`. But that
// pulse only converges anybody because the gameplay light field walks
// DistressPulses alongside lanterns - and that loop lives in the STORM
// subsystem, a file none of the checks above open.
//
// So the link that would silently end the climax of every run is the one
// nothing was reading. Nothing in `Source/LLK/Private/Enemies/` consults the
// extraction call at all; patrols come because the dock is lit. Drop the loop
// and the rig still lights, every check above still passes, and no patrol
// ever arrives.
//
// **Checked as one distinctive line rather than by slicing the function.**
// Two earlier drafts of this passed with the link deliberately broken: the
// first regex spanned the whole file and matched a `DistressPulses` belonging
// to a different function, and the second tried to cut GetLitLightSources out
// with a lazy `\n}` - but this file reads comment-stripped source, so the
// brace it found was not the one the raw file has. The loop header below
// occurs exactly once in the project and states the whole invariant on its
// own.
{
  const storm = stripComments(
    read("Source/LLK/Private/Storm/LLKZonePressureSubsystem.cpp") ?? "",
  );

  check(
    "ROW 74: the light field still carries distress pulses to the enemies",
    /for\s*\(\s*const\s+FLLKDistressPulse\s*&\s*\w+\s*:\s*DistressPulses\s*\)/.test(storm),
    "the gameplay light field no longer walks DistressPulses, so an " +
      "extraction's rig light exists and nothing can perceive it. Every " +
      "run's twenty-second hold becomes standing still, with every other " +
      "extraction check green",
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read DECISIONS.md rows 73 and 74 before changing an assertion here.\n");
  process.exit(1);
}
console.log("");
