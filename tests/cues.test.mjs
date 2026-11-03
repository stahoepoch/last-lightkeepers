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
// **The second census, added 2026-11-02, covers the other shape.** The first
// version of this file saw `BlueprintAssignable` delegates and nothing else,
// and said so — which was honest and was also a hole, because the same failure
// happens to plain fields. The live instance was `bStumbleLanded`: set by
// `LLKSweeperBrain`, asserted twice in the brain's own test, read exactly once
// by a `UE_LOG`, and never felt by the keeper. A delegate census structurally
// cannot see that.
//
// So the flags on `FLLKSweeperStep` get their own pass. They are the brain's
// output — each one exists to be acted on by the pawn exactly once — and they
// do have a declaration shape to key on. The rule is that the block guarding
// each flag must do something other than log, because a `UE_LOG` is how
// `bStumbleLanded` looked finished for as long as it did.
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

// ---- 3. the brain's output flags are acted on, not just logged --------------

// Flags on FLLKSweeperStep that the pawn is not expected to act on. Same
// contract as SILENT: an entry is a decision somebody made, and the test
// checks the entry is still true.
// The distinction that decides an entry here, because `bStumbleLanded` looked
// exactly like these for weeks and was not one of them: a flag belongs in
// UNACTED when it reports a transition the brain already owns, and does not
// when it names a consequence somebody outside the brain is supposed to feel.
// The stumble named a consequence — a speed the keeper pays — so a log was
// never enough. These four report.
const UNACTED = {
  bContactLanded:
    "The raw contact. `bStumbleLanded` and `bKnockedDown` are what the pawn " +
    "acts on; this one is the brain's own bookkeeping for the decay window, " +
    "and acting on it too would charge one touch twice.",
  bLostTheKeeper:
    "The brain has already returned itself to the route and the pawn follows " +
    "`Step.Move`, not this flag. Nothing outside the brain changes, so there " +
    "is nothing for the pawn to do but say so.",
  bStockAbandoned:
    "Deliberately a diagnostic rather than a cue, and the code says so: it " +
    "logs a Warning pointing at docs/MAP-READINESS.md, because an " +
    "unreachable relic is a map problem that should be findable as a warning " +
    "rather than as a Sweeper that quietly stopped filing.",
  bDwellStarted:
    "Verbose, and the action that matters is `bAdvancePatrolPoint`, which " +
    "the pawn does act on when the pause is spent. Starting a pause asks " +
    "nothing of the pawn.",
};

const brainHeader = readFileSync(
  join(root, "Source", "LLK", "Public", "Enemies", "LLKSweeperBrain.h"),
  "utf8",
);
const pawn = readFileSync(
  join(root, "Source", "LLK", "Private", "Enemies", "LLKSweeper.cpp"),
  "utf8",
);

// Only the flags declared on FLLKSweeperStep — the brain's output. The Mind's
// and the Senses' bools are inputs and state, and are not the pawn's to act on.
const stepStart = brainHeader.indexOf("struct FLLKSweeperStep");
const stepFlags = [
  ...brainHeader.slice(stepStart).matchAll(/\bbool\s+(b[A-Za-z0-9_]*)\s*=/g),
].map((m) => m[1]);

check(
  "the step struct declares output flags",
  stepFlags.length > 0,
  "found none, so this scan is broken rather than the brain being empty",
);

// Pull the braced block that each `if (Step.<flag>` guards, so the question is
// what the pawn DOES with the flag rather than whether it mentions it.
function guardedBlock(source, flag) {
  const at = source.search(new RegExp(`if\\s*\\(\\s*Step\\.${flag}\\b`));
  if (at === -1) return null;
  const open = source.indexOf("{", at);
  if (open === -1) return null;
  let depth = 0;
  for (let i = open; i < source.length; i += 1) {
    if (source[i] === "{") depth += 1;
    else if (source[i] === "}") {
      depth -= 1;
      if (depth === 0) return source.slice(open + 1, i);
    }
  }
  return null;
}

// Strip comments and every UE_LOG call; whatever survives is the action.
function actionIn(block) {
  return block
    .replace(/\/\*[\s\S]*?\*\//g, "")
    .replace(/\/\/[^\n]*/g, "")
    .replace(/UE_LOG\s*\([\s\S]*?\);/g, "")
    .replace(/\s+/g, "");
}

for (const flag of stepFlags) {
  const exempt = Object.prototype.hasOwnProperty.call(UNACTED, flag);
  const block = guardedBlock(pawn, flag);
  const acted = block !== null && actionIn(block).length > 0;

  if (exempt) {
    check(
      `UNACTED still describes ${flag}`,
      !acted,
      "the pawn now acts on it, so remove the entry — an allowlist nobody " +
        "prunes is a stale document",
    );
    continue;
  }

  check(
    `the pawn acts on ${flag}, not just logs it`,
    acted,
    block === null
      ? "no `if (Step." + flag + ")` in LLKSweeper.cpp at all — the brain " +
        "emits it and the pawn never reads it"
      : "the guarded block is nothing but a UE_LOG. That is exactly how " +
        "`bStumbleLanded` looked finished while the keeper never stumbled. " +
        "Act on it, or add it to UNACTED with the reason",
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
