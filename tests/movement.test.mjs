// Movement invariant test. The one rule Chris stated as a rule:
//
//   "The invariant stays: sprint must always escape a stumbled Sweeper. We
//   should not lower player speed alone and accidentally make the Sweeper
//   impossible."  - 2026-11-03
//
// **Why this file exists at all, when balance.test.mjs already reads these
// numbers.** Because the invariant was violated for two days by a version of
// itself that was checked. `LLKCharacter.h` carried the rule in a comment and
// enforced it against `Sweeper / sprint` - true for an empty-handed keeper,
// and an empty-handed keeper is not who the rule protects. The burden
// multiplier and the stumble multiplier COMPOUND (LLKCharacter.cpp:572), so
// the state that decides the rule is stumbled AND carrying, and in that state
// a keeper sprinted at 368 cm/s against a 392 cm/s Sweeper.
//
// A clean-state assertion of this invariant passes today and passed then. That
// is the whole lesson: the test has to be written in the WORST legal state, or
// it certifies the bug.
//
// **The properties, in order of what they cost to lose:**
//
//   1. The worst-state escape. Sprint x stumble x minimum burden must exceed
//      the Sweeper, with margin. This is the rule.
//   2. The compounding still compounds. Property 1's arithmetic is only the
//      right model while the two multipliers are multiplied together and
//      applied to sprint. Decouple them and this test would keep passing
//      while describing a game that no longer exists - so the source line is
//      guarded by inspection, the same argument extraction.test.mjs makes.
//   3. The gear ratios survive a rescale. The retune moved every speed by
//      k=0.5; sprint/walk and crouch/walk are what the gear change FEELS
//      like, and a future rescale that drifts them has changed the game
//      rather than its scale.
//   4. Stumble duration stays under contact recovery. A stumble that outlasts
//      recovery is re-applied before it expires, which is a permanent slow.
//
//   node tests/movement.test.mjs

import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const load = (name) => JSON.parse(readFileSync(join(root, "data", name), "utf8"));
const source = (rel) => readFileSync(join(root, "Source", "LLK", rel), "utf8");

const lantern = load("lantern.json");
const enemies = load("enemies.json");
const loot = load("loot.json");

const characterHeader = source("Public/Characters/LLKCharacter.h");
const characterBody = source("Private/Characters/LLKCharacter.cpp");
const unitsHeader = source("Public/LLKUnits.h");
const sweeperBrain = source("Private/Enemies/LLKSweeperBrain.cpp");

let failures = 0;
let checks = 0;

function check(name, condition, detail = "") {
  checks += 1;
  if (condition) {
    console.log(`  ok   ${name}`);
  } else {
    failures += 1;
    console.log(`  FAIL ${name}${detail ? ` — ${detail}` : ""}`);
  }
}

// Pull a float out of C++ rather than restating it here. A test that hardcodes
// the value it is checking is a fixture that hardcodes its outcome, and this
// project has produced three of those in one week.
function cppFloat(text, name, label) {
  const found = text.match(
    // `\.\d*` rather than `\.\d+`: UE writes trailing-dot literals like
    // `28.f` and `1.f`, and a regex that insists on a digit after the point
    // silently fails to find a constant that is sitting right there.
    new RegExp(`${name}\\s*=\\s*(-?\\d+(?:\\.\\d*)?)f?\\s*;`),
  );
  if (!found) {
    throw new Error(
      `could not read ${name} from ${label}. The constant was renamed or ` +
        `removed, and this test cannot assert the invariant without it - ` +
        `which is a failure, not a reason to skip.`,
    );
  }
  return Number(found[1]);
}

const studsToCm = cppFloat(unitsHeader, "StudsToCm", "LLKUnits.h");
const stumbleMultiplier = cppFloat(
  characterHeader,
  "FallbackStumbleMultiplier",
  "LLKCharacter.h",
);
const stumbleSeconds = cppFloat(
  characterHeader,
  "FallbackStumbleSeconds",
  "LLKCharacter.h",
);
const recoverySeconds = cppFloat(
  sweeperBrain,
  "FallbackPatrolDwellSeconds",
  "LLKSweeperBrain.cpp",
);

const cms = (studs) => studs * studsToCm;

const walk = cms(lantern.movement.walkSpeed);
const sprint = cms(lantern.movement.sprintSpeed);
const crouch = cms(lantern.movement.crouchSpeed);
const sweeper = cms(enemies.Sweepers.moveSpeed);

// The worst legal burden. Relics are the only tier carrying burden points, and
// the penalty caps - so the heaviest a keeper can legally be is the cap.
const { speedPenaltyPerPoint: perPoint, maxSpeedPenalty: cap } = loot.burden;
const minBurden = 1 - Math.min(cap, perPoint * Math.ceil(cap / perPoint));

console.log("\nThe worst legal state: stumbled, carrying a full haul");
{
  const worst = sprint * stumbleMultiplier * minBurden;
  const margin = worst - sweeper;

  check(
    "a stumbled keeper carrying a full haul still outruns a Sweeper",
    worst > sweeper,
    `sprint ${sprint.toFixed(1)} x stumble ${stumbleMultiplier} x burden ` +
      `${minBurden.toFixed(2)} = ${worst.toFixed(1)} cm/s against a Sweeper at ` +
      `${sweeper.toFixed(1)}. Short by ${Math.abs(margin).toFixed(1)} cm/s. ` +
      `This is a keeper who found the relics, picked them up, is running them ` +
      `to the dock, gets tripped, and cannot get away. Chris's rule is that ` +
      `sprint must ALWAYS escape a stumbled Sweeper`,
  );

  // 5% rather than "greater than zero". A margin of a few cm/s satisfies the
  // inequality and is not an escape anyone can feel or execute.
  check(
    "and by a margin a player can actually use, not a rounding error",
    worst > sweeper * 1.05,
    `margin is ${margin.toFixed(1)} cm/s (${((worst / sweeper - 1) * 100).toFixed(1)}%), ` +
      `under the 5% this asks for. The inequality holds but the escape is ` +
      `theoretical - at these speeds one stud is ${studsToCm} cm/s, so a ` +
      `margin thinner than that is inside the authoring resolution`,
  );

  check(
    "the stumble multiplier clears the burdened floor, not the empty-handed one",
    stumbleMultiplier > sweeper / (sprint * minBurden),
    `floor is ${(sweeper / (sprint * minBurden)).toFixed(4)} with a full haul ` +
      `and only ${(sweeper / sprint).toFixed(4)} empty-handed. The multiplier is ` +
      `${stumbleMultiplier}. Checking the empty-handed floor is the exact ` +
      `mistake that let 0.65 ship beneath a floor of 0.692`,
  );
}

console.log("\nThe states that were already fine, asserted so a fix cannot break them");
{
  check(
    "a clean sprint outruns a Sweeper",
    sprint > sweeper,
    `${sprint.toFixed(1)} vs ${sweeper.toFixed(1)}`,
  );
  check(
    "a stumbled sprint with empty hands outruns a Sweeper",
    sprint * stumbleMultiplier > sweeper,
    `${(sprint * stumbleMultiplier).toFixed(1)} vs ${sweeper.toFixed(1)}`,
  );
  check(
    "a fully burdened sprint outruns a Sweeper",
    sprint * minBurden > sweeper,
    `${(sprint * minBurden).toFixed(1)} vs ${sweeper.toFixed(1)}`,
  );
  check(
    "a stumbled walk does NOT outrun a Sweeper, so walking it off is no option",
    walk * stumbleMultiplier < sweeper,
    `${(walk * stumbleMultiplier).toFixed(1)} vs ${sweeper.toFixed(1)}. If this ` +
      `passes the stumble has stopped costing anything: the answer to contact ` +
      `would be to keep walking`,
  );
  check(
    "crouch does NOT outrun a Sweeper, so crouch is concealment and not escape",
    crouch < sweeper,
    `${crouch.toFixed(1)} vs ${sweeper.toFixed(1)}. Deliberate - see ` +
      `_crouchIsNotEscape in lantern.json. If crouch outran the machine there ` +
      `would be no reason to ever stand up`,
  );
}

console.log("\nThe model this test depends on");
{
  // Property 1's arithmetic is only correct while the game multiplies these
  // two together. If it ever takes the lower of the two, or applies one to a
  // different speed, every number above is wrong and silently so.
  check(
    "burden and stumble still compound into one multiplier",
    /GetBurdenMultiplier\(\)\s*\*\s*GetStumbleMultiplier\(\)/.test(characterBody),
    `LLKCharacter.cpp no longer multiplies the two multipliers together. ` +
      `Every figure this test reports assumes it does, so the test has ` +
      `stopped describing the game rather than caught a bug in it`,
  );

  check(
    "and that product is what sprint is scaled by",
    /MaxWalkSpeed\s*=\s*\(\s*bSprinting\s*\?\s*SprintSpeedCms\s*:\s*WalkSpeedCms\s*\)\s*\*\s*Burden/.test(
      characterBody,
    ),
    `sprint is no longer scaled by the compounded multiplier in ` +
      `LLKCharacter.cpp, so the worst-state figure above is not the speed a ` +
      `sprinting keeper actually reaches`,
  );

  check(
    "relics are still the only tier with burden",
    Object.values(loot.tiers ?? {}).filter((t) => (t?.burden ?? 0) > 0).length === 1,
    `the minimum burden multiplier above assumes relics alone carry points. ` +
      `If another tier gained burden, a keeper can get heavier than ` +
      `${minBurden.toFixed(2)} and the worst state this file tests is no ` +
      `longer the worst state`,
  );
}

console.log("\nThe whole roster, because a chase is a ratio and the file has six of them");
{
  // Added after the retune halved the player and the Sweeper and left the
  // other four alone. Glimmers sat at 504 cm/s and the Warden at 448 against
  // a 322 cm/s sprint: two enemies no player could ever get away from, from a
  // change that only touched the player and one machine. `balance.test.mjs`
  // caught it by a side door - Window Walkers must be slower than Sweepers -
  // which is luck, not coverage. This is the direct assertion.
  const roster = Object.entries(enemies).filter(
    ([, v]) => v && typeof v === "object" && typeof v.moveSpeed === "number",
  );

  check(
    "there is a roster to check",
    roster.length >= 6,
    `found ${roster.length} enemies with a moveSpeed; the roster is six`,
  );

  for (const [name, stats] of roster) {
    const speed = cms(stats.moveSpeed);
    check(
      `a clean sprint escapes ${name}`,
      speed < sprint,
      `${name} moves ${speed.toFixed(1)} cm/s against a sprint of ` +
        `${sprint.toFixed(1)}. Nothing in this game may be faster than a ` +
        `keeper who is running and carrying nothing - that is not a balance ` +
        `preference, it is the difference between a chase and a cutscene`,
    );
  }
}

console.log("\nGear ratios, which are what the scale change must not touch");
{
  const sprintOverWalk = lantern.movement.sprintSpeed / lantern.movement.walkSpeed;
  const crouchOverWalk = lantern.movement.crouchSpeed / lantern.movement.walkSpeed;

  // From 16/23/9, the pre-retune authored set. The retune halved all three, so
  // these are unchanged by construction; the point is that the NEXT rescale
  // has to say so out loud.
  check(
    "sprint is about 1.44x walk, as it was before the rescale",
    Math.abs(sprintOverWalk - 23 / 16) < 0.02,
    `sprint/walk is ${sprintOverWalk.toFixed(4)} against the authored ` +
      `${(23 / 16).toFixed(4)}. A rescale changes the scale; this ratio is the ` +
      `feel of the gear change, and drifting it is a design change wearing a ` +
      `tuning change's clothes`,
  );
  check(
    "crouch is about 0.56x walk, as it was before the rescale",
    Math.abs(crouchOverWalk - 9 / 16) < 0.02,
    `crouch/walk is ${crouchOverWalk.toFixed(4)} against the authored ` +
      `${(9 / 16).toFixed(4)}`,
  );
}

console.log("\nThe stumble cannot become permanent");
{
  check(
    "a stumble expires before contact can recur",
    stumbleSeconds < recoverySeconds,
    `stumble runs ${stumbleSeconds}s against a recovery window of ` +
      `${recoverySeconds}s. At or above it, each contact re-applies the slow ` +
      `before the last one expires and the keeper is permanently slowed`,
  );

  // Named rather than asserted: the authored contact cooldown is 1.5s and the
  // live window is FallbackPatrolDwellSeconds. That is Codex's to reconcile,
  // and a test here asserting 1.5 would fail on code that is working as
  // written.
  console.log(
    `  note the live recovery window is ${recoverySeconds}s, read from ` +
      `FallbackPatrolDwellSeconds, not from the authored 1.5s contact cooldown`,
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  process.exit(1);
}
console.log("");
