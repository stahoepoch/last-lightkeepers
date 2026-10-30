// Telemetry-source guard. M4's instrument.
//
// **One rule, and it is the only one that matters: a metric may never become
// an input.** `docs/LANES.md` makes playtest instrumentation mine and names
// the risk it covers - *"Row 3's question answered with opinions instead of
// data"* - and the way an instrument like this goes wrong is not by measuring
// badly. It goes wrong by being read: a rule that consults how many relics a
// crew is carrying, or how lit their run has been, is a difficulty system
// nobody designed and nobody can see.
//
// Chris's fifth replayability pillar forbids exactly that: never secretly
// rubber-band, the player must always see why danger rose. `escalation.test.mjs`
// enforces it for the clock. This enforces it for the measurement, which is
// the other half - the clock cannot be fed by a rule, and the instrument that
// watches the clock cannot feed one either.
//
// **The three properties:**
//
//   1. No gameplay module includes `LLKRunMetrics.h` or
//      `LLKRunTelemetrySubsystem.h`. Only the telemetry's own files and tests.
//   2. The telemetry subsystem calls no setter on anything it observes. It
//      binds delegates and reads getters.
//   3. The metrics layer is pure - no world, no actor, no file - so what every
//      number means is checkable in a test rather than in a playtest.
//
//   node tests/telemetry.test.mjs

import { readFileSync, existsSync, readdirSync } from "node:fs";
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

const stripComments = (src) =>
  src.replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/[^\n]*/g, "");

const read = (rel) => {
  const full = join(root, rel);
  return existsSync(full) ? readFileSync(full, "utf8") : null;
};

const METRICS_HEADER = "Source/LLK/Public/Run/LLKRunMetrics.h";
const METRICS_IMPL = "Source/LLK/Private/Run/LLKRunMetrics.cpp";
const TELEMETRY_IMPL = "Source/LLK/Private/Run/LLKRunTelemetrySubsystem.cpp";

for (const rel of [METRICS_HEADER, METRICS_IMPL, TELEMETRY_IMPL]) {
  check(`${rel} exists`, read(rel) !== null, "the guard names a file that is not there");
}

// ---- 1. nothing in the game reads the instrument ------------------------
{
  const sourceFiles = [];
  const walk = (dir) => {
    for (const entry of readdirSync(dir, { withFileTypes: true })) {
      const full = join(dir, entry.name);
      if (entry.isDirectory()) walk(full);
      else if (/\.(cpp|h)$/.test(entry.name)) {
        sourceFiles.push({
          rel: full.slice(root.length + 1).split("\\").join("/"),
          src: readFileSync(full, "utf8"),
        });
      }
    }
  };
  walk(join(root, "Source"));

  check("the module was walked", sourceFiles.length > 20, `found ${sourceFiles.length} files`);

  // Its own two files, and the tests that check the arithmetic. Everything
  // else reading this header is a rule that can see the measurement.
  const allowed = new Set([
    METRICS_HEADER,
    METRICS_IMPL,
    TELEMETRY_IMPL,
    "Source/LLK/Public/Run/LLKRunTelemetrySubsystem.h",
    "Source/LLK/Private/Tests/LLKRunMetricsTest.cpp",
  ]);

  const readers = [];
  for (const { rel, src } of sourceFiles) {
    if (allowed.has(rel)) continue;
    if (/LLKRunMetrics\.h|LLKRunTelemetrySubsystem\.h|LLKRunMetrics::|ULLKRunTelemetrySubsystem/
      .test(stripComments(src))) {
      readers.push(rel);
    }
  }

  check(
    "no gameplay module can see the instrument",
    readers.length === 0,
    `${readers.join(", ")}. A rule that reads a metric is secret rubber-banding: ` +
      `Chris's fifth replayability pillar says the player must always see why ` +
      `danger rose`,
  );
}

// ---- 2. the observer observes ------------------------------------------
{
  const telemetry = stripComments(read(TELEMETRY_IMPL) ?? "");

  // It binds. That is how it learns anything.
  check(
    "it subscribes to events that already exist",
    (telemetry.match(/AddDynamic\(/g) ?? []).length >= 8,
    "every number comes from a delegate some other system already broadcast",
  );

  // And it must not act. These are the verbs that would make an instrument a
  // participant - named rather than globbed, so adding one is deliberate.
  for (const forbidden of [
    "CallExtraction",
    "CancelExtraction",
    "AddDistressPulse",
    "AddOrRefreshDistressPulse",
    "GoDown",
    "SetCarriedCount",
    "SpawnActor",
    "Destroy(",
    "SetRequirementSatisfied",
    "RegisterZone",
  ]) {
    check(
      `it never calls ${forbidden.replace("(", "")}`,
      !telemetry.includes(forbidden),
      "an instrument that can act is not an instrument. It reads getters and " +
        "binds delegates, and that is all",
    );
  }

  // The one thing it polls is the stall, because a condition has no delegate.
  check(
    "ROW 75: it polls the stall, which is a condition rather than an event",
    /bHoldStalled/.test(telemetry),
    "how often the dock emptied and for how long is the number no self-report " +
      "would ever produce accurately",
  );

  // Writes under Saved/, which nobody's lane owns and nothing commits.
  check(
    "it writes to Saved/ rather than anywhere tracked",
    /ProjectSavedDir\(\)/.test(telemetry) && /LLKTelemetry/.test(telemetry),
    "an instrument that filled a tracked folder would dirty a working tree " +
      "once per run",
  );
}

// ---- 3. the metrics layer is pure --------------------------------------
{
  const metrics = stripComments(read(METRICS_IMPL) ?? "");
  const header = stripComments(read(METRICS_HEADER) ?? "");
  const both = `${metrics}\n${header}`;

  for (const forbidden of ["UWorld", "GetWorld", "AActor", "FFileHelper", "FPaths"]) {
    check(
      `the metrics layer never names ${forbidden}`,
      !both.includes(forbidden),
      "the arithmetic is a function of a list of events, so every number's " +
        "definition is checkable in a test rather than in a playtest",
    );
  }

  // The three questions M4 is judged on have to be answerable from the
  // summary, or the instrument measures something else.
  for (const field of ["LitFraction", "RelicsSecured", "CancelCount", "ClimaxFraction"]) {
    check(
      `the summary carries ${field}`,
      header.includes(field),
      "M4's gate: do they kill their lanterns, do they bank, do they argue " +
        "for one more room",
    );
  }

  // Row 8, as arithmetic rather than as a caller's promise.
  check(
    "ROW 8: a wipe keeps nothing, enforced in the summary",
    /CrewWiped:[\s\S]{0,400}?RelicsSecured = 0;/.test(metrics),
    "only extraction secures a haul, and the instrument must not be the one " +
      "place that forgets it",
  );
}

// ---- 4. nothing identifying goes in the file ---------------------------
{
  const metrics = read(METRICS_IMPL) ?? "";
  const telemetry = read(TELEMETRY_IMPL) ?? "";

  for (const forbidden of [
    "FPlatformProcess::UserName",
    "ComputerName",
    "GetLoginId",
    "UniqueNetId",
    "PlayerName",
  ]) {
    check(
      `the file records nothing identifying (${forbidden})`,
      !metrics.includes(forbidden) && !telemetry.includes(forbidden),
      "a run file describes a run, not a person",
    );
  }
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read docs/LANES.md on playtest instrumentation, and ROADMAP.md on M4.\n");
  process.exit(1);
}
console.log("");
