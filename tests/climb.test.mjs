// A verb that finds ledges nobody can reach, on a key nobody pressed.
//
// `LLKClimbTest` proves the rules and `LLK.World.ClimbChain` proves a real
// child gets onto a real counter. Between them they cover a lot. What neither
// can see:
//
//   1. **Whether the key is wired.** The whole verb hangs off one line in
//      `OnJumpStarted`. Remove it and every test above still passes, every
//      trace still works, and no player can climb anything - the purest form
//      of this project's recurring defect.
//   2. **Whether the keeper has the component at all.** `ALLKCharacter`
//      creating it is a constructor line, and a C++ test that builds its own
//      character would not notice its absence.
//   3. **Whether the numbers still sort the store.** That the counter in
//      `LLKStoreFloorPlan` is climbable and the shelf is not is a claim about
//      three constants in three files, and the world test asserts it with
//      the plan's own values - so a change that moved BOTH the plan and the
//      ceiling together would keep passing while the store stopped sorting.
//      This reads the floor plan and the body and checks the gap directly.
//
//   node tests/climb.test.mjs

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

const FEEL = read(join(root, "Source", "LLK", "Public", "Characters", "LLKClimbFeel.h"));
const COMP = read(join(root, "Source", "LLK", "Private", "Characters", "LLKClimbComponent.cpp"));
const COMPH = read(join(root, "Source", "LLK", "Public", "Characters", "LLKClimbComponent.h"));
const CHAR = read(join(root, "Source", "LLK", "Private", "Characters", "LLKCharacter.cpp"));
const PLAN = read(join(root, "Source", "LLK", "Public", "Harness", "LLKStoreFloorPlan.h"));
const LANTERN = read(join(root, "Source", "LLK", "Private", "Lantern", "LLKLanternFeel.cpp"));

console.log("\nClimbing: the key, the component, and whether the store still sorts\n");

// ---- the wiring ----------------------------------------------------------

check(
  "the climb rules are in the repo",
  /MaxLedgeHeightCm/.test(FEEL),
  "Source/LLK/Public/Characters/LLKClimbFeel.h is the model and the argument",
);

check(
  "the keeper is given a climb component",
  /CreateDefaultSubobject<ULLKClimbComponent>/.test(CHAR),
  "ALLKCharacter never creates one, so no keeper in any level can climb - " +
    "and a C++ test that builds its own character would not notice",
);

// THE check for this slice.
check(
  "jump tries a mantle before it jumps",
  /Climb\s*!=\s*nullptr\s*&&\s*Climb->TryClimb\(\)/.test(CHAR),
  "nothing calls TryClimb from the input path. The traces work, the rules " +
    "are right, the component ticks, and the verb is unreachable because no " +
    "key reaches it",
);

check(
  "and still jumps when there is nothing to climb",
  (() => {
    const tryAt = CHAR.indexOf("Climb->TryClimb()");
    const jumpAt = CHAR.indexOf("\tJump();");
    return tryAt > 0 && jumpAt > tryAt;
  })(),
  "the fallback to ACharacter::Jump has to come after the mantle attempt, " +
    "or the player either never jumps or never climbs",
);

check(
  "the server re-traces rather than trusting the client",
  /ServerStartClimb_Implementation[\s\S]{0,600}FindLedge\(\)/.test(COMP),
  "the server takes the client's word that a ledge exists, so a client " +
    "could place itself on top of anything. Every other server entry point " +
    "in this project re-resolves",
);

check(
  "the room above the ledge is checked before the climb, not during",
  /OverlapBlockingTestByChannel/.test(COMP),
  "without it a mantle onto a surface with a shelf just above finishes with " +
    "the capsule inside the shelf, and the engine ejects the keeper " +
    "somewhere nobody chose - the crate-in-a-recess defect again",
);

check(
  "the climb reports its own noise",
  /ULLKClimbComponent::GetCurrentNoise/.test(COMP)
    && /ILLKNoiseSource/.test(COMPH),
  "a mantle moves the capsule with SetActorLocation, so the keeper's " +
    "velocity is zero throughout. Without this the most exposed second in " +
    "the chapter is the quietest thing in it",
);

check(
  "and the keeper passes it on",
  /Climb->GetCurrentNoise/.test(CHAR),
  "ALLKCharacter::GetCurrentNoise ignores the climb, so the component's " +
    "noise reaches nothing - a component is not an actor and the Sweeper's " +
    "scan iterates actors",
);

check(
  "the step-height floor comes from the movement component",
  /MaxStepHeight/.test(COMP),
  "a hard-coded floor here is a second opinion about what walking can " +
    "already step over, and every kerb in the store becomes a mantle",
);

// ---- and whether the store still sorts -----------------------------------

{
  const num = (src, name) => {
    const m = src.match(new RegExp(`${name}\\s*=\\s*([0-9.]+)f`));
    return m ? Number(m[1]) : undefined;
  };

  const studsToCm = 28;
  const counter = num(PLAN, "CounterHeightStuds");
  const shelf = num(PLAN, "ShelfHeightStuds");
  const height = num(LANTERN, "KeeperHeightCm");

  // Eye = capsule half-height + camera offset, both scaled from the one
  // authored keeper height. Recomputed here rather than read, so this
  // notices a change to any of the three.
  const scale = height / 176;
  const eye = (88 + 64) * scale;

  check(
    `the authored numbers are readable (counter ${counter}, shelf ${shelf} studs, keeper ${height} cm)`,
    [counter, shelf, height].every((v) => Number.isFinite(v)),
    "could not read CounterHeightStuds / ShelfHeightStuds / KeeperHeightCm, " +
      "so nothing below is checking the real store",
  );

  if ([counter, shelf, height].every((v) => Number.isFinite(v))) {
    const counterCm = counter * studsToCm;
    const shelfCm = shelf * studsToCm;

    check(
      `a counter is reachable (${counterCm.toFixed(0)} cm under a ${eye.toFixed(0)} cm eye)`,
      counterCm <= eye,
      "the store's counters are now above the climb ceiling, so the verb " +
        "does nothing anywhere in Boswell's. Either the counters rose or the " +
        "keeper shrank",
    );
    check(
      `and a shelf is not (${shelfCm.toFixed(0)} cm over it)`,
      shelfCm > eye,
      "the shelves are now climbable, which removes the reason the push " +
        "verb exists and lets a player onto the top of every gondola in the " +
        "store",
    );
    check(
      "and there is real headroom between the two, not a coincidence",
      shelfCm - eye > 20 && eye - counterCm > 20,
      "the climb ceiling now sits within 20 cm of a counter or a shelf, so " +
        "a small retune of either would flip what the store allows. The " +
        "sorting should be comfortable, not marginal",
    );
  }
}

// ---- the acoustic vocabulary --------------------------------------------

{
  const climb = Number((FEEL.match(/ClimbNoiseStuds\s*=\s*([0-9.]+)f/) || [])[1]);
  const push = Number(
    (read(join(root, "Source", "LLK", "Public", "World", "LLKPushFeel.h"))
      .match(/PushNoiseStuds\s*=\s*([0-9.]+)f/) || [])[1]);

  check(
    `walk 5 < climb ${climb} < sprint 12.5 < push ${push}`,
    climb > 5 && climb < 12.5 && push > 12.5,
    "the four verbs have to be distinguishable by ear or a player cannot " +
      "judge what they just did. This ordering is the chapter's whole " +
      "acoustic vocabulary",
  );

  // Documented consequence, asserted so it stays deliberate: a climb is
  // quieter than a Sweeper's dark sight, so its sound only matters behind
  // cover. LLK.World.ClimbChain section 4 is built on exactly this.
  check(
    `and a climb is quieter than dark sight (${climb} < 12), so it only matters behind cover`,
    climb < 12,
    "a climb now carries further than a Sweeper can see in the dark, which " +
      "makes it an audible commitment in the open. That may be wanted, but " +
      "it changes what ClimbChain section 4 is testing and the argument in " +
      "LLKClimbFeel.h needs rewriting with it",
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("A climb verb can be unreachable in play, or useless in this");
  console.log("particular store, while every C++ test about it passes.\n");
  process.exit(1);
}
console.log("");
