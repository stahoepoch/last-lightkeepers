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

// ---- THE ALLOWLIST, which is what actually enforces the rule ------------
//
// **Codex found the hole in the first version of this guard.** It scanned only
// `LLKVitalsComponent`, so a future Sweeper in its own file could have written
// `Storm->AddSpike(60, TEXT("Knockdown"))` and passed. And `FName Reason`
// accepted any text at all: a named reason is good logging and no
// enforcement whatsoever.
//
// The fix was to make the mistake inexpressible rather than detectable:
// `AddSpike` now takes a typed `ELLKStormSpike`, so there is no way to spell
// "Knockdown". Which means the thing worth guarding is no longer a list of
// files - it is the ENUM'S CONTENTS. Any failure-triggered escalation now
// requires adding an entry here first, wherever the caller lives, and this
// check is what makes that a deliberate act rather than a quiet one.
{
  const stormPath = "Source/LLK/Public/Storm/LLKStormChargeSubsystem.h";
  const full = join(root, stormPath);
  const src = existsSync(full) ? readFileSync(full, "utf8") : "";
  const clean = stripComments(src);

  check(
    "AddSpike takes a typed cause, not free text",
    /AddSpike\s*\(\s*ELLKStormSpike\s+Cause\s*\)/.test(clean),
    "a free-text reason logs beautifully and enforces nothing - an enemy could " +
      "name its spike anything and the type system would not care",
  );

  check(
    "and no longer accepts an FName reason anywhere",
    !/AddSpike\s*\([^)]*FName/.test(clean),
    "the old signature surviving as an overload would reopen the hole",
  );

  // The allowlist itself. Both entries are things a crew CHOSE and were
  // priced for; anything else arriving here is the rule being broken.
  const enumBlock = clean.match(/enum class ELLKStormSpike\s*:\s*uint8\s*\{([^}]*)\}/);
  check("the spike allowlist exists as an enum", enumBlock !== null);

  if (enumBlock) {
    const entries = enumBlock[1]
      .split(",")
      .map((e) => e.trim())
      .filter((e) => e.length > 0);

    const ALLOWED = ["ExtractionCall", "AnchorRevive"];
    check(
      `the allowlist is exactly ${ALLOWED.join(" and ")}`,
      entries.length === ALLOWED.length && ALLOWED.every((a) => entries.includes(a)),
      `found: ${entries.join(", ") || "nothing"}. Every entry must be something the ` +
        `crew chose and was priced for. Adding one is a design decision about row 64 ` +
        `line three - escalation comes from choices, never from failing - so update ` +
        `this guard deliberately or not at all`,
    );

    // Named explicitly, because these are the words somebody reaches for when
    // they want the night to react to a knockdown.
    for (const forbidden of ["Knockdown", "Downed", "Contained", "Death", "Wipe", "Failure"]) {
      check(
        `the allowlist has no entry for ${forbidden}`,
        !entries.some((e) => e.toLowerCase().includes(forbidden.toLowerCase())),
        "escalation triggered by failure is punishment the player cannot diagnose",
      );
    }
  }

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
