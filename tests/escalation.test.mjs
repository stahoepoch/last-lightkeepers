// Escalation-source guard.
//
// **The rule.** `Storm Charge` may rise from things the crew CHOSE - light,
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
];

// What "reaching the meter" looks like in this codebase.
const ESCALATION_SYMBOLS = [
  "AddSpike",
  "ULLKStormChargeSubsystem",
  "LLKStormChargeSubsystem.h",
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
      `${entry.why}. Storm Charge must never rise from a knockdown: escalation ` +
        `triggered by failure is punishment the player cannot diagnose. If the ` +
        `night genuinely needs to react to a containment, it must read the state ` +
        `and slow down, not be told to speed up`,
    );
  }
}

// ---- and the meter's own side: every spike is a named, chosen cause ------
//
// AddSpike takes a Reason and logs it, so every acceleration in the game is
// traceable to something the crew can argue about afterwards. That
// traceability IS Chris's fifth pillar, so it is worth pinning.
{
  const stormPath = "Source/LLK/Public/Storm/LLKStormChargeSubsystem.h";
  const full = join(root, stormPath);
  const src = existsSync(full) ? readFileSync(full, "utf8") : "";

  check(
    "AddSpike still requires a named reason",
    /AddSpike\s*\(\s*float\s+Amount\s*,\s*FName\s+Reason\s*\)/.test(stripComments(src)),
    "an unnamed spike is an unexplained acceleration, which is the thing the " +
      "fairness rule forbids",
  );

  check(
    "and the header still records why failure may not feed the meter",
    /only ever rises|row 6/i.test(src),
    "the reasoning has to survive beside the rule, or the next reader restores " +
      "the shortcut",
  );
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
