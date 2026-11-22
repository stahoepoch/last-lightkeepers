// A sense that exists in a struct and is never filled in.
//
// `LLKHearingTest` proves the arithmetic and the brain rules: one second of
// travel, standing still is silent, sound starts a search and never a pursuit.
// All of it is true of a game where nothing can hear the player at all,
// because a C++ test instantiates the senses itself. `FLLKSweeperSenses` could
// carry `HeardNoisePosition` forever with no pawn ever setting it, every
// assertion green, and the keeper as inaudible as they were before any of this
// was written. That is this project's recurring defect, arriving for the sixth
// time: a correct system over an absent thing, with a clean log.
//
// So this file checks the two halves C++ structurally cannot:
//
//   1. **Something actually fills the sense**, and the brain actually reads
//      it. The broadcast-with-no-subscriber shape, which is why
//      `cues.test.mjs` exists.
//   2. **The derived radii still line up with the JSON they were derived
//      from.** `LLKHearing.h` argues that a sprint carries marginally further
//      than a Sweeper sees in the dark and nowhere near as far as it sees in
//      the light. Those are claims about `data/lantern.json` and
//      `data/enemies.json` together. A C++ constant copied out of them is a
//      copy that drifts, and both files have been globally rescaled twice.
//
//   node tests/hearing.test.mjs

import { existsSync, readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");

let checks = 0;
let failures = 0;

function check(name, ok, why) {
  checks += 1;
  if (ok) {
    console.log(`  ok   ${name}`);
  } else {
    failures += 1;
    console.log(`  FAIL ${name}`);
    if (why) console.log(`       ${why}`);
  }
}

const read = (p) => (existsSync(p) ? readFileSync(p, "utf8") : "");

const HDR = join(root, "Source", "LLK", "Public", "Enemies", "LLKHearing.h");
const PAWN = join(root, "Source", "LLK", "Private", "Enemies", "LLKSweeper.cpp");
const BRAIN = join(root, "Source", "LLK", "Private", "Enemies", "LLKSweeperBrain.cpp");
const SENSES = join(root, "Source", "LLK", "Public", "Enemies", "LLKSweeperBrain.h");

const hdr = read(HDR);
const pawn = read(PAWN);
const brain = read(BRAIN);
const senses = read(SENSES);

const lantern = JSON.parse(read(join(root, "data", "lantern.json")));
const enemies = JSON.parse(read(join(root, "data", "enemies.json")));

console.log("\nBeing heard: the sense, the wiring, and the radii it was derived from\n");

// ---- the wiring ----------------------------------------------------------

check(
  "the hearing module is in the repo",
  hdr.length > 0,
  "Source/LLK/Public/Enemies/LLKHearing.h is the model and the argument for it",
);

check(
  "the senses can carry a sound",
  /TOptional<FVector>\s+HeardNoisePosition/.test(senses),
  "FLLKSweeperSenses has no HeardNoisePosition, so either this sense was " +
    "removed or it was renamed and every guard below is passing on nothing",
);

// THE check. A sense nothing fills is the defect this file exists for.
check(
  "something actually fills it",
  /Senses\.HeardNoisePosition\s*=/.test(pawn),
  "nothing in LLKSweeper.cpp ever sets Senses.HeardNoisePosition. The field " +
    "exists, LLKHearingTest passes on senses it builds itself, and no machine " +
    "in the running game can hear the keeper - which is the exact defect this " +
    "sense was added to remove, looking identical in logs to a working one",
);

check(
  "and the pawn includes the module that decides it",
  /#include "Enemies\/LLKHearing\.h"/.test(pawn),
  "without the include the pawn cannot be calling NoiseReaching, so whatever " +
    "fills the sense is computing its own answer - two definitions of audible",
);

check(
  "and the brain reads it",
  /Senses\.HeardNoisePosition\.IsSet\(\)/.test(brain),
  "the brain never asks whether it heard anything, so the pawn is measuring " +
    "a sound that changes no decision",
);

check(
  "and the investigation is actually reported",
  /Step\.bNoiseInvestigated\s*=\s*true/.test(brain),
  "bNoiseInvestigated is declared and never set, so the one event a playtest " +
    "needs to see - a machine leaving its route for a reason the player " +
    "caused - is invisible",
);

// Vertical motion is the bug this shape invites: a keeper falling down a
// stairwell broadcasting a sprint they did not run.
//
// **This guard moved, and it moved because it failed.** The keeper's noise
// used to be computed inside `ALLKSweeper::Observe`, beside a loop that asked
// every `ILLKNoiseSource` in the world - two code paths for one question. The
// climb fell through the gap between them: a mantle moves the capsule
// directly, so velocity reads zero, and the component that knew the real
// answer was a *component*, invisible to a loop that casts actors. The keeper
// is now a noise source like everything else.
const character = read(
  join(root, "Source", "LLK", "Private", "Characters", "LLKCharacter.cpp"));

check(
  "the keeper's own noise is measured from ground speed only",
  /GetVelocity\(\)\.Size2D\(\)/.test(character)
    && !/GetVelocity\(\)\.Size\(\)/.test(character),
  "the keeper's speed must be read with Size2D. Size includes vertical " +
    "motion, so a fall down an escalator well would broadcast as a sprint " +
    "and a jump would be the loudest thing in the game",
);

check(
  "and the keeper reports it as a noise source, not as a special case",
  /ALLKCharacter::GetCurrentNoise/.test(character)
    && !/GetVelocity\(\)\.Size2D\(\)/.test(pawn),
  "either the keeper is not an ILLKNoiseSource, or the Sweeper still " +
    "computes their noise itself. Two code paths for 'what can I hear' is " +
    "what hid the climb's silence: anything that is not an actor, or not " +
    "velocity-driven, falls down the gap between them",
);

// ---- the anti-harassment rule --------------------------------------------
//
// Textual, and deliberately narrow: it asserts the trigger sits in the Patrol
// arm and nowhere else. Slice 3 refuses endless harassment outright, and the
// rule that prevents it is that a sound cannot extend a search.
{
  const patrolArm = brain.indexOf("case ELLKSweeperState::Patrol:");
  const stockArm = brain.indexOf("case ELLKSweeperState::Stock:");
  const trigger = brain.indexOf("if (bHeard)");
  check(
    "hearing triggers from patrol only",
    trigger > patrolArm && trigger < stockArm && patrolArm >= 0 && stockArm > 0,
    "the `if (bHeard)` transition is outside the Patrol arm. Triggering from " +
      "Search lets a sprinting keeper refresh the clock and be followed across " +
      "the store, which ROADMAP.md slice 3 refuses; triggering from Stock means " +
      "sound interrupts the filing job, whose authored rule is that being SEEN " +
      "is how you stop it",
  );
  check(
    "and it is the only place a sound changes state",
    (brain.match(/if \(bHeard\)/g) || []).length === 1,
    "more than one bHeard transition means the rule above is not the whole " +
      "story and this guard no longer describes the behaviour",
  );
}

// ---- the radii, against the files they were derived from -----------------

const seconds = Number((hdr.match(/SecondsOfTravel\s*=\s*([0-9.]+)f/) || [])[1]);
check(
  `the carry interval is readable from the header (${seconds}s)`,
  Number.isFinite(seconds) && seconds > 0,
  "could not find SecondsOfTravel in LLKHearing.h, so nothing below is " +
    "checking the real model",
);

const pace = lantern.keeper ?? lantern.movement ?? lantern;
const gait = (name) => {
  const direct = pace?.[name];
  if (typeof direct === "number") return direct;
  // Fall back to a shallow search, so a reshuffle of lantern.json's sections
  // fails loudly on the value rather than silently on the path.
  for (const value of Object.values(lantern)) {
    if (value && typeof value === "object" && typeof value[name] === "number") {
      return value[name];
    }
  }
  return undefined;
};

const walk = gait("walkSpeed");
const sprint = gait("sprintSpeed");
const crouch = gait("crouchSpeed");

check(
  `the authored gaits are readable (crouch ${crouch}, walk ${walk}, sprint ${sprint} studs/s)`,
  [walk, sprint, crouch].every((v) => typeof v === "number" && v > 0),
  "walkSpeed / sprintSpeed / crouchSpeed could not be found in " +
    "data/lantern.json, so the radii below are not being checked against " +
    "anything",
);

const sweeper = (enemies.enemies ?? enemies).Sweepers
  ?? Object.values(enemies.enemies ?? enemies).find((e) => e?.family === "Sweepers");
const dark = sweeper?.aggroRadiusDark;
const lit = sweeper?.aggroRadiusLit;

check(
  `the Sweeper's notice radii are readable (dark ${dark}, lit ${lit} studs)`,
  typeof dark === "number" && typeof lit === "number",
  "aggroRadiusDark / aggroRadiusLit could not be found for Sweepers in " +
    "data/enemies.json",
);

if ([walk, sprint, crouch, dark, lit, seconds].every((v) => typeof v === "number")) {
  const radius = (studsPerSecond) => studsPerSecond * seconds;

  // The mechanic. If this stops being true, sprinting in the dark is free
  // again and the sense has quietly stopped mattering.
  check(
    `a sprint carries past what it sees in the dark (${radius(sprint)} > ${dark} studs)`,
    radius(sprint) > dark,
    "the loudest gait no longer carries further than the Sweeper's dark " +
      "notice radius, so running past one in an unlit aisle costs nothing " +
      "again. Either a pace value fell, aggroRadiusDark rose, or " +
      "SecondsOfTravel was lowered - LLKHearing.h's argument needs rewriting " +
      "with this number in it",
  );

  // The thesis guard. Light must stay the dominant way to be found.
  check(
    `and nowhere near what it sees in the light (${radius(sprint)} < ${lit} studs)`,
    radius(sprint) < lit,
    "the loudest noise a keeper can make now reaches as far as the Sweeper " +
      "sees in the light, which makes sound competitive with light as a way " +
      "to be found. The lantern is the game's central gamble and a new sense " +
      "must not quietly overturn it",
  );

  // Crouch has to be worth the 84 cm/s it costs.
  check(
    `a crouch is genuinely quiet (${radius(crouch)} studs, under a third of ${dark})`,
    radius(crouch) * 3 <= dark,
    "crouching no longer buys meaningful quiet, so the deliberate slow mode " +
      "data/lantern.json describes as 'for tight shops and careful approach' " +
      "costs speed and returns nothing acoustically",
  );

  check(
    "and each gear is audibly a gear change",
    radius(walk) >= radius(crouch) * 1.5 && radius(sprint) >= radius(walk) * 1.5,
    "two gaits now sound close enough to be one. The player has three speeds " +
      "and needs three acoustic signatures for the choice to be readable",
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("A keeper can be inaudible in play while every C++ test about");
  console.log("hearing passes. That is what these checks are for.\n");
  process.exit(1);
}
console.log("");
