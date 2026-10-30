// Burden-source guard. M3.
//
// **Two rules, and the second is the one worth a file.**
//
// 1. The authored numbers are coherent: a per-point penalty, a cap at or above
//    it, and both small enough that `loot.json`'s own note stays true -
//    *"ordinary loot must not make a raid feel sluggish."*
//
// 2. **Light never changes a keeper's speed.** Chris, 2026-10-29, when the
//    carrying question came up: *"Do not add a lantern speed penalty yet."*
//    That is easy to honour today and very easy to break later, because the
//    movement path and the lantern now live on the same actor and the obvious
//    place to put an encumbrance is the function that already multiplies a
//    speed. A unit test cannot catch it - a lantern-aware speed would simply
//    return a different number and every burden assertion would still pass -
//    so this is an inspection guard, the same shape as `escalation.test.mjs`.
//
// The reason the rule exists is worth keeping beside it. Rows 72 and 73 make
// light cost visibility and machine attention and nothing else; a speed
// penalty on top would be the third price for one decision, and the one a
// player would feel without being able to name.
//
//   node tests/burden.test.mjs

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

const stripComments = (src) =>
  src.replace(/\/\*[\s\S]*?\*\//g, "").replace(/\/\/[^\n]*/g, "");

const read = (rel) => {
  const full = join(root, rel);
  return existsSync(full) ? readFileSync(full, "utf8") : null;
};

// ---- 1. the authored numbers ---------------------------------------------
{
  const loot = JSON.parse(readFileSync(join(root, "data/loot.json"), "utf8"));
  const burden = loot.burden ?? {};

  check(
    "a per-point penalty is authored",
    typeof burden.speedPenaltyPerPoint === "number" && burden.speedPenaltyPerPoint > 0,
    "without one, burden is off and a haul is weightless - which is the state " +
      "every run before M3 was in",
  );
  check(
    "and a cap",
    typeof burden.maxSpeedPenalty === "number" && burden.maxSpeedPenalty > 0,
    "the cap is what lets a crew be greedy without becoming unable to run",
  );
  check(
    "the cap is at least one point's worth",
    burden.maxSpeedPenalty >= burden.speedPenaltyPerPoint,
    "a cap below the first point makes every relic cost the same, which is a " +
      "flat tax rather than a decision",
  );

  // ROW 8's decision needs the haul to be felt and not endured. These two
  // bounds are the sanity rails on his own note: a quarter of a keeper's
  // speed is not "a small movement penalty".
  check(
    "and the whole burden stays small enough to be a nudge",
    burden.maxSpeedPenalty <= 0.25,
    "loot.json's own note: ordinary loot must not make a raid feel sluggish",
  );
  check(
    "while still being perceptible",
    burden.speedPenaltyPerPoint >= 0.01,
    "a penalty nobody can feel is a system nobody plays with",
  );

  // The cap in relics, which is what the readout tells a player.
  const capAt = Math.ceil(burden.maxSpeedPenalty / burden.speedPenaltyPerPoint);
  check(
    `the cap is reached at a sane number of relics (${capAt})`,
    capAt >= 2 && capAt <= 8,
    "one relic reaching the cap is a flat tax; twenty is a cap nobody meets",
  );
}

// ---- 2. ROW 72/73: light costs visibility and attention, never speed -----
{
  const character = stripComments(read("Source/LLK/Private/Characters/LLKCharacter.cpp") ?? "");
  const burdenImpl = stripComments(read("Source/LLK/Private/Loot/LLKBurden.cpp") ?? "");
  const burdenHeader = stripComments(read("Source/LLK/Public/Loot/LLKBurden.h") ?? "");

  check(
    "the burden rule exists at all",
    burdenImpl.length > 0 && burdenHeader.length > 0,
    "Source/LLK/Private/Loot/LLKBurden.cpp",
  );

  // The rule layer cannot see a lantern, a light level or the clock. As with
  // extraction, the point is that the wrong thing is inexpressible rather
  // than merely absent.
  for (const forbidden of [
    "Lantern",
    "LightField",
    "Intensity",
    "ZonePressure",
    "LLKPerception",
  ]) {
    check(
      `ROW 72: the burden rule never names ${forbidden}`,
      !burdenImpl.includes(forbidden) && !burdenHeader.includes(forbidden),
      "burden is weight. Light's price is visibility and machine attention, " +
        "and a speed penalty on top would be a third cost for one decision",
    );
  }

  // And the movement path itself: the function that multiplies a speed must
  // not reach for the lantern's state. Sliced to ApplyMovementSpeed rather
  // than searched across the file, because this actor legitimately owns a
  // lantern and wires it up elsewhere.
  const at = character.indexOf("void ALLKCharacter::ApplyMovementSpeed()");
  const body = at < 0 ? "" : character.slice(at, character.indexOf("\n}", at));
  check(
    "the speed function was found",
    body.length > 0,
    "ApplyMovementSpeed is the single place a speed is assigned; if it moved, " +
      "this guard has to move with it",
  );
  for (const forbidden of ["Lantern", "IsLit", "Intensity"]) {
    check(
      `ROW 72: and it never reads ${forbidden}`,
      !body.includes(forbidden),
      "Chris, 2026-10-29: \"Do not add a lantern speed penalty yet.\" This is " +
        "the function somebody would add it to",
    );
  }

  // Burden is applied to every speed, not only walking: a keeper must not be
  // able to sprint away from a cost they accepted.
  check(
    "the haul slows walking, sprinting and crouching alike",
    /MaxWalkSpeedCrouched = CrouchSpeedCms \* Burden/.test(body)
      && /MaxWalkSpeed = \(bSprinting \? SprintSpeedCms : WalkSpeedCms\) \* Burden/.test(body),
    "a sprint that ignored the haul would make burden a rounding error",
  );
}

// ---- 3. the count that feeds it has one writer --------------------------
{
  const interactor = stripComments(
    read("Source/LLK/Private/Loot/LLKInteractorComponent.cpp") ?? "");

  // Burden derives from the carried count, so a count that changed without
  // announcing it would be a keeper whose speed silently disagreed with their
  // haul until something else happened to re-apply it.
  const writes = (interactor.match(/CarriedCount\s*=\s*/g) ?? []).length;
  check(
    "the carried count is written in one place",
    writes === 1,
    `found ${writes}. Every other writer is a change that never reached the ` +
      `movement path`,
  );
  check(
    "and that place announces it",
    /OnCarriedCountChanged\.Broadcast/.test(interactor),
    "on the server as well as through the OnRep: a listen server's own keeper " +
      "never receives its own replication, and that keeper is whoever is hosting",
  );
}

// ---- 4. the readout explains it, because row 5 says so ------------------
{
  const hud = stripComments(read("Source/LLK/Private/UI/LLKDebugHUD.cpp") ?? "");

  check(
    "the haul and its cost are shown together",
    /carrying %d/.test(hud) && /slower/.test(hud),
    "row 5's shape: the cause beside the consequence. A keeper who is quietly " +
      "8% slower blames the controls",
  );
  check(
    "and the readout reads the same rule the movement does",
    /LLKBurden::PenaltyFraction/.test(hud),
    "a HUD computing its own percentage is a second authority, and the one " +
      "that drifts is whichever nobody is reading",
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("Read DECISIONS.md rows 8, 72 and 73 before changing an assertion here.\n");
  process.exit(1);
}
console.log("");
