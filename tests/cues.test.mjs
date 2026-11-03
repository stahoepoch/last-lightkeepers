// A broadcast with no subscriber is silence that compiles.
//
// Found 2026-11-02, the hard way. `ALLKSweeper::OnTelegraph` was declared,
// broadcast, and bound by nothing from the day it landed — so row 16's
// "every enemy telegraphs with sound before sight", which is LOCKED and is an
// accessibility requirement rather than polish, was unmet *in play* and nobody
// could see it. Both halves of the mechanism existed: a declaration and a
// `Broadcast` call read as finished code to anyone opening the file, and the
// compiler has no opinion about a delegate nobody subscribed to.
//
// Nothing in this project could have reported that. This file is what reports
// it. It is the same shape as `VerifyTable` comparing row names instead of row
// contents (`HANDOFFS.md` X2): a cheap identity check cannot stand in for a
// comparison that has to look inside.
//
// What it asserts, for every `BlueprintAssignable` delegate in the runtime
// module:
//
//   1. if it is broadcast, something in C++ binds it — or it is listed in
//      SILENT below with a reason, so silence is a decision and not an
//      accident;
//   2. nothing in SILENT has quietly gained a subscriber, because an
//      allowlist nobody prunes becomes a stale document, which is this
//      project's other recurring failure (`ADVERSARY.md`, "the wrong
//      vintage").
//
// **What this file does NOT cover, stated so nobody over-trusts it.** It sees
// `BlueprintAssignable` delegates and nothing else. The same failure —
// produced, asserted by tests, logged, and never consumed by anything a player
// feels — happens to plain fields too, and there is a live instance:
// `bStumbleLanded` is set by `LLKSweeperBrain`, asserted twice in
// `LLKSweeperBrainTest`, and read exactly once, by a `UE_LOG` at
// `LLKSweeper.cpp:646`. The keeper never actually stumbles. This file cannot
// see that — a bool on a struct has no declaration shape to key on — and
// `HANDOFFS.md` X4 tracks it by hand instead.
//
// A guard believed to cover more than it does is the same failure one level
// up, so: delegates only, and the census line at the end is the honest scope.
//
// Regex over C++, not a parser. It can miss a binding; it does not invent one.
// Bindings inside `Source/LLK/Private/Tests/` do not count — a test harness
// subscribing to a delegate makes it no more audible to a player.
//
//   node tests/cues.test.mjs

import { readFileSync, readdirSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");

let failures = 0;
let checks = 0;
const ok = (msg) => console.log(`  ok   ${msg}`);
const check = (name, condition, detail) => {
  checks += 1;
  if (condition) ok(name);
  else {
    failures += 1;
    console.log(`  FAIL ${name}${detail ? ` — ${detail}` : ""}`);
  }
};

// Delegates that are broadcast and deliberately have no C++ subscriber yet.
// Each entry is a claim that someone looked and decided, not that nobody
// checked. Remove an entry the moment something binds it — the test enforces
// that, so the list cannot rot quietly.
const SILENT = {
  OnDistressPulse:
    "The Anchor Revive's 'wakes the room' pulse (row 73). It is already a " +
    "gameplay source in the light field, so it has a mechanical effect " +
    "without a cue — the player pays for the revive and cannot perceive " +
    "what they spent. Needs an authored cue; tracked for Chris.",
  OnExtractionFailed:
    "Extraction failing is the run ending badly, with nothing announcing it " +
    "but the HUD state. Needs an authored cue; tracked for Chris.",
  // The three relic-read delegates below share one reason, and it is a real
  // one rather than an excuse: `ULLKRelicComponent` already drives a custom
  // depth stencil (1 unidentified / 2 read-but-dark / 3 takeable / 0 taken,
  // NOTES-FOR-CHRIS item 2) on every legibility and affordance change. That is
  // the authored channel for this state. A delegate would be a second one.
  //
  // Two of them were found by this file rather than by the grep that preceded
  // it: `OnInteractableChanged` and `OnLegibilityChanged` are bound *only* in
  // `LLKRelicTest.cpp`, so a search for `.AddDynamic` reports them as wired
  // while a player hears nothing. That is why bindings inside
  // `Private/Tests/` are excluded above — a harness subscribing to a delegate
  // is the test-shaped version of the bug this file exists to catch.
  OnFocusChanged:
    "Relic focus. Carried by the stencil states; see the note above.",
  OnInteractableChanged:
    "A relic becoming takeable or not. Carried by stencil 2 -> 3, which " +
    "NOTES-FOR-CHRIS item 2 calls the pair most worth authoring. Bound only " +
    "in LLKRelicTest.cpp, which does not count.",
  OnLegibilityChanged:
    "A relic becoming readable. Carried by stencil 1 -> 2. Bound only in " +
    "LLKRelicTest.cpp, which does not count.",
  OnReviveCancelled:
    "A revive interrupted. The player caused it or suffered it and gets no " +
    "confirmation either way. Needs an authored cue; tracked for Chris.",
};

// ---- collect sources -------------------------------------------------------

function walk(dir, out = []) {
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const full = join(dir, entry.name);
    if (entry.isDirectory()) {
      if (entry.name === "Intermediate" || entry.name === "Binaries") continue;
      walk(full, out);
    } else if (/\.(cpp|h)$/.test(entry.name)) {
      out.push(full);
    }
  }
  return out;
}

const files = walk(join(root, "Source", "LLK"));
const isTest = (f) => f.includes(join("Private", "Tests"));

// ---- find every BlueprintAssignable delegate member -------------------------

const declared = new Map(); // name -> declaring file
for (const file of files.filter((f) => f.endsWith(".h"))) {
  const text = readFileSync(file, "utf8");
  const re = /UPROPERTY\s*\([^)]*BlueprintAssignable[^)]*\)\s*(?:\/\/[^\n]*\n\s*)*([A-Za-z_][A-Za-z0-9_]*)\s+(On[A-Za-z0-9_]*)\s*;/g;
  let m;
  while ((m = re.exec(text)) !== null) declared.set(m[2], file);
}

check(
  "the runtime module declares BlueprintAssignable delegates",
  declared.size > 0,
  "found none, which means this scan is broken rather than that the project has none",
);

// ---- which are broadcast, and which are bound -------------------------------

const broadcast = new Set();
const bound = new Map(); // name -> [files]
for (const file of files.filter((f) => f.endsWith(".cpp"))) {
  const text = readFileSync(file, "utf8");
  for (const name of declared.keys()) {
    if (new RegExp(`\\b${name}\\s*\\.\\s*Broadcast\\s*\\(`).test(text)) {
      broadcast.add(name);
    }
    if (
      !isTest(file) &&
      new RegExp(`\\b${name}\\s*\\.\\s*(AddDynamic|AddUniqueDynamic)\\s*\\(`).test(text)
    ) {
      if (!bound.has(name)) bound.set(name, []);
      bound.get(name).push(file.slice(root.length + 1));
    }
  }
}

// Control: if nothing anywhere is bound, the binding regex is broken and every
// result below is a false alarm. A check that cannot fail for its own reasons
// is a check nobody can trust.
check(
  "the binding scan can see bindings at all",
  bound.size > 0,
  "no delegate anywhere has a subscriber, so the scan is broken, not the project",
);

// ---- 1. every broadcast delegate is heard, or declared silent ---------------

for (const name of [...broadcast].sort()) {
  if (bound.has(name)) {
    ok(`${name} is broadcast and something binds it`);
    checks += 1;
    continue;
  }
  check(
    `${name} is broadcast and nothing binds it`,
    Object.prototype.hasOwnProperty.call(SILENT, name),
    "a delegate nobody subscribes to is silence that compiles. Bind it, or " +
      "add it to SILENT in this file with the reason it is allowed to be quiet",
  );
}

// ---- 2. the allowlist has not gone stale ------------------------------------

for (const name of Object.keys(SILENT).sort()) {
  check(
    `SILENT still describes ${name}`,
    !bound.has(name),
    `it now has a subscriber (${(bound.get(name) || []).join(", ")}), so remove ` +
      "it from SILENT — an allowlist nobody prunes is a stale document",
  );
  check(
    `SILENT does not list a delegate that no longer exists (${name})`,
    declared.has(name),
    "it is not declared BlueprintAssignable anywhere, so this entry is dead",
  );
}

// ---- report -----------------------------------------------------------------

console.log(
  `\n${checks - failures}/${checks} passed` + (failures ? `\n${failures} FAILED` : ""),
);
console.log(
  `  ${declared.size} assignable delegates, ${broadcast.size} broadcast, ` +
    `${bound.size} with a C++ subscriber, ${Object.keys(SILENT).length} declared silent`,
);
if (failures) process.exit(1);
