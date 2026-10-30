// Escalation-source guard.
//
// **The rule.** `Storm Pressure` may rise from things the crew CHOSE - light,
// theft, depth, alarms, elapsed time, the extraction call, the anchor revive.
// It must never rise from a knockdown or a containment, which are what happen
// when the crew gets it wrong.
//
// The reason is Creative Assembly's, from Alien: Isolation: no behaviour
// unlock there is ever keyed to a player death, because escalation triggered
// by failure is punishment the player cannot diagnose - they look for a
// cause, find none, and conclude the game is cheating. Chris's own fifth
// replayability pillar says the same thing in his words: never secretly
// rubber-band, the player must always see why danger rose. A knockdown that
// fed the meter would break his design using his own escalation system.
//
// **Why this is a source check and not a unit test, which is the interesting
// part.** `LLK.Storm.KnockdownDoesNotEscalate` asserts the behavioural half
// and asserts it well: going down moves neither meter, sixty seconds on the
// floor costs what sixty seconds standing costs, and containment actually
// *slows* the night because a downed keeper's lantern is forced off. But when
// the violation was planted deliberately - a `Storm->AddSpike(60)` inside the
// knockdown path - that test stayed green, because its keeper is built with
// `NewObject` and has no `UWorld`, so `GetWorld()` returned null and the
// spike was never reached. A test that cannot reach the code it forbids is
// not guarding it.
//
// The constraint is architectural: the knockdown path must not be able to
// talk to the meter at all. Architecture is checkable by inspection, which is
// what this file does and what `units.test.mjs` already does for raw stud
// conversions.
//
//   node tests/escalation.test.mjs

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

// Comments are stripped before searching, so the explanation of the rule
// cannot trip the rule. The same approach units.test.mjs takes.
const stripComments = (src) =>
  src.replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/[^\n]*/g, "");

// ---- the files that must not reach the meter -----------------------------
//
// Named explicitly rather than globbed, so adding a new knockdown-adjacent
// file is a deliberate decision to add it here too.
const FORBIDDEN_CALLERS = [
  {
    path: "Source/LLK/Private/Characters/LLKVitalsComponent.cpp",
    why: "the knockdown path itself - this is the file the planted violation went into",
  },
  {
    path: "Source/LLK/Public/Characters/LLKVitalsComponent.h",
    why: "its header, so a dependency cannot be declared here and used elsewhere",
  },
  {
    path: "Source/LLK/Private/Run/LLKRunStateSubsystem.cpp",
    why: "the crew-wipe path. A wipe is the purest failure in the game, so it is " +
      "the last thing that may charge the meter - the anchor revive costs charge " +
      "because continuing is a decision; a wipe is what happens when no decision " +
      "is left",
  },
  {
    path: "Source/LLK/Public/Run/LLKRunStateSubsystem.h",
    why: "its header, for the same reason",
  },
];

// What "reaching the meter" looks like in this codebase.
const ESCALATION_SYMBOLS = [
  "AddSpike",
  "ULLKZonePressureSubsystem",
  "LLKZonePressureSubsystem.h",
];

for (const entry of FORBIDDEN_CALLERS) {
  const full = join(root, entry.path);
  if (!existsSync(full)) {
    check(`${entry.path} exists`, false, "the guard names a file that is not there");
    continue;
  }

  const src = stripComments(readFileSync(full, "utf8"));
  for (const symbol of ESCALATION_SYMBOLS) {
    check(
      `${entry.path} does not reach the storm via ${symbol}`,
      !src.includes(symbol),
      `${entry.why}. Storm Pressure must never rise from a knockdown: escalation ` +
        `triggered by failure is punishment the player cannot diagnose. If the ` +
        `night genuinely needs to react to a containment, it must read the state ` +
        `and slow down, not be told to speed up`,
    );
  }
}

// ---- NO MECHANISM AT ALL, which is stronger than an allowlist -----------
//
// **This guard has been through three shapes and the third is the last one.**
//
// First it scanned `LLKVitalsComponent` for a spike call, and Codex found the
// hole: a future Sweeper in its own file could have written
// `Storm->AddSpike(60, TEXT("Knockdown"))` and passed, because the guard only
// looked in one place. And `FName Reason` accepted any text at all - a named
// reason is good logging and no enforcement whatsoever.
//
// Second it guarded the CONTENTS of a typed `ELLKStormSpike` allowlist, so
// failure-triggered escalation was inexpressible rather than merely
// detectable: there was no way to spell "Knockdown", wherever the caller
// lived.
//
// Third, and now: there is no allowlist, because there is no mechanism. Chris,
// 2026-10-29, row 73: *"Clock means weather alone... A radio call cannot
// accelerate the weather."* The extraction call was the last priced spike, and
// with it gone `Advance` is the only function in the project that writes the
// meter, and it takes nothing but a delta.
//
// So the thing to guard is an ABSENCE, which is the easiest kind of rule to
// keep and the hardest to notice breaking. Row 64 line three - escalation
// comes from choices the crew can identify, never from failing - used to need
// an argument about which causes were permitted. It now needs none: nothing a
// player does moves the clock, so failure cannot be a special case of
// anything.
{
  const stormHeader = "Source/LLK/Public/Storm/LLKZonePressureSubsystem.h";
  const stormImpl = "Source/LLK/Private/Storm/LLKZonePressureSubsystem.cpp";
  const headerFull = join(root, stormHeader);
  const implFull = join(root, stormImpl);
  const src = existsSync(headerFull) ? readFileSync(headerFull, "utf8") : "";
  const impl = existsSync(implFull) ? readFileSync(implFull, "utf8") : "";
  const clean = stripComments(src);
  const cleanImpl = stripComments(impl);

  check("the clock's header was found", src.length > 0, stormHeader);

  // 1. The mechanism is gone, not merely unused.
  for (const gone of ["AddSpike", "ApplySpike", "GetSpikeAmount", "ELLKStormSpike"]) {
    check(
      `ROW 73: ${gone} no longer exists in the clock's API`,
      !clean.includes(gone),
      "an unused spike entry point is a spike entry point; the rule is that there is no way to move the clock",
    );
  }
  check(
    "ROW 73: and no spike implementation survives either",
    !cleanImpl.includes("ApplySpike") && !cleanImpl.includes("ELLKStormSpike"),
    "a private implementation with no declaration is still a thing somebody re-declares",
  );

  // 2. Exactly one expression writes the meter, and it is time.
  const writes = (cleanImpl.match(/Pressure\s*\+=/g) ?? []).length;
  check(
    "ROW 73: exactly one expression adds to the meter",
    writes === 1,
    `found ${writes}. Every one of them is a way for something other than time to advance the night`,
  );
  check(
    "and that expression is the baseline times a delta",
    /Pressure \+= Rules->BaselineRatePerSecond \* DeltaSeconds;/.test(cleanImpl),
    "a second term in the accrual is the regression this whole guard exists for",
  );

  // 3. And nothing anywhere else reaches for the meter to raise it. Walked
  //    across the whole module rather than checked in one file, because that
  //    was the first version's hole.
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
  walk(join(root, "Source", "LLK"));

  check("the module was walked", sourceFiles.length > 20, `found ${sourceFiles.length} files`);

  let reachers = 0;
  for (const { rel, src: fileSrc } of sourceFiles) {
    if (rel.includes("/Storm/") || rel.includes("/Tests/")) continue;
    if (/(AddSpike|ApplySpike)/.test(stripComments(fileSrc))) {
      reachers += 1;
      console.log(`       ${rel}`);
    }
  }
  check(
    "ROW 73: nothing outside the clock tries to raise it",
    reachers === 0,
    `${reachers} file(s) listed above. Row 64 line three, enforced across the whole module rather than in one file`,
  );

  check(
    "and the header still records why nothing may feed the meter",
    /only ever rises|row 6/i.test(src),
    "the reasoning has to survive beside the rule, or the next reader restores " +
      "the shortcut",
  );
}

// ---- M2.3: the stage's words are the authored words ----------------------
//
// `zone-pressure.json` carries an `effect` sentence per threshold, and the
// debug readout prints it beside the stage so a player can name what changed
// - which is what M2.6 is judged on. The fallback rules in
// `LLKZonePressureSubsystem.cpp` carry the same sentences, because
// `LLK.Storm.NightStage` caught them being empty there: a fallback threshold
// with an id and a value but no consequence text is resolved-but-degenerate,
// the same shape as the lantern that resolved to a zero radius.
//
// Fixing it duplicated two strings, so this guards the duplicate. The words a
// designer tunes and the words a player reads have to be the same words, and
// a drifted fallback would show the old consequence on exactly the run where
// the asset failed to load - the run where somebody is already confused.
{
  const dataPath = "data/zone-pressure.json";
  const srcPath = "Source/LLK/Private/Storm/LLKZonePressureSubsystem.cpp";
  const data = JSON.parse(readFileSync(join(root, dataPath), "utf8"));
  const src = readFileSync(join(root, srcPath), "utf8");

  const thresholds = Array.isArray(data.thresholds) ? data.thresholds : [];
  check(
    "the authored data still has thresholds to escalate through",
    thresholds.length >= 2,
    `${dataPath} thresholds is what the night is made of`,
  );

  for (const threshold of thresholds) {
    check(
      `${threshold.id} has an authored effect sentence`,
      typeof threshold.effect === "string" && threshold.effect.trim().length > 0,
      "the readout prints this sentence; an empty one reads as a threshold " +
        "that changes nothing rather than as missing data",
    );

    if (typeof threshold.effect !== "string") continue;

    // Verbatim, including the full stop. Comments are NOT stripped here on
    // purpose: the fallback's copy lives in a string literal, and stripping
    // would not touch it, while the duplicate-detection is the whole point.
    check(
      `and the fallback rules carry ${threshold.id}'s sentence verbatim`,
      src.includes(threshold.effect),
      `${srcPath} must hold the same words as ${dataPath}, or the fallback ` +
        `path shows a consequence the data no longer describes`,
    );
  }
}

// ---- the behavioural half is covered elsewhere, and says so --------------
{
  const testPath = "Source/LLK/Private/Tests/LLKKnockdownEscalationTest.cpp";
  check(
    "the in-engine behavioural guard still exists alongside this one",
    existsSync(join(root, testPath)),
    `${testPath} asserts that containment slows the night rather than speeding ` +
      `it up. This file only guards the architecture; deleting that one would ` +
      `leave the property itself unchecked`,
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read the escalation section of reports/Retention and escalating dread.md.\n");
  process.exit(1);
}
console.log("");
