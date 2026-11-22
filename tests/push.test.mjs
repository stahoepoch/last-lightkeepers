// A verb that compiles, prompts, and moves nothing.
//
// `LLKPushTest` proves the kinematics and the noise economy: the rail is
// finite, facing is a gate, pushing is louder than sprinting, a fixture
// outranks the steps of whoever is pushing it. All of it is true of a game
// where no player can push anything, because a C++ test calls the arithmetic
// directly. It cannot see whether `ULLKInteractorComponent` ever resolves an
// `ILLKInteractable`, whether E reaches `BeginInteract`, whether releasing E
// reaches `EndInteract`, whether a Sweeper ever asks an `ILLKNoiseSource` what
// it can hear, or whether a single pushable exists in any level.
//
// Those are the five ways this slice arrives as the project's recurring
// defect - a correct system over an absent thing, with a clean log - and the
// last one is the likeliest, because placing an actor is somebody else's lane.
//
//   node tests/push.test.mjs

import { existsSync, readFileSync, readdirSync } from "node:fs";
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

const SEAM = join(root, "Source", "LLK", "Public", "World", "LLKInteractable.h");
const FEEL = join(root, "Source", "LLK", "Public", "World", "LLKPushFeel.h");
const FIXTURE = join(root, "Source", "LLK", "Private", "World", "LLKPushableFixture.cpp");
const INTERACTOR = join(
  root, "Source", "LLK", "Private", "Loot", "LLKInteractorComponent.cpp");
const SWEEPER = join(root, "Source", "LLK", "Private", "Enemies", "LLKSweeper.cpp");
const GAMEMODE = join(root, "Source", "LLK", "Private", "LLKTestGameMode.cpp");

const seam = read(SEAM);
const feel = read(FEEL);
const fixture = read(FIXTURE);
const interactor = read(INTERACTOR);
const sweeper = read(SWEEPER);
const gamemode = read(GAMEMODE);

console.log("\nPushing: the seam, the verb, the noise, and whether anything uses them\n");

// ---- the seam ------------------------------------------------------------

check(
  "the interactable seam is in the repo",
  /class LLK_API ILLKInteractable/.test(seam),
  "Source/LLK/Public/World/LLKInteractable.h is the thing that stops the " +
    "sixth hard-coded focus type",
);

check(
  "the interactor resolves interactables generically",
  /FocusedInteractable\s*=\s*ResolveInteractable\(\)/.test(interactor),
  "nothing assigns FocusedInteractable, so the seam exists and no object " +
    "ever gets focus through it - every prompt and every E press still goes " +
    "to one of the five hard-coded types",
);

check(
  "and E reaches BeginInteract through it",
  /ServerBeginInteractable\(/.test(interactor)
    && /Interactable->BeginInteract\(/.test(interactor),
  "the press never reaches the interface, so a focused pushable prompts and " +
    "does nothing when you press E",
);

check(
  "and releasing E reaches EndInteract",
  /ServerEndInteractable\(/.test(interactor)
    && /Interactable->EndInteract\(/.test(interactor),
  "without the release the fixture keeps moving after the player lets go, " +
    "which is a runaway object and the loudest bug this verb can have",
);

// The server must not take the client's word for what it is touching.
check(
  "the server re-resolves the press rather than trusting the client",
  /ResolveInteractable\(\)\s*!=\s*Candidate/.test(interactor),
  "ServerBeginInteractable acts on whatever actor the client named. A client " +
    "could then push an object on the other side of the store - the exact " +
    "rule ServerReleaseOpeningGrille already states for the grille",
);

// ...but the release must always land, which is the opposite rule.
check(
  "and does NOT re-resolve the release",
  !/ServerEndInteractable_Implementation[\s\S]{0,400}ResolveInteractable\(\)/.test(interactor),
  "a release that is re-resolved fails exactly when it matters - the keeper " +
    "has walked out of reach of the thing they were pushing, so the guard " +
    "refuses and the fixture never stops",
);

// ---- the verb ------------------------------------------------------------

check(
  "the pushable exists and implements both interfaces",
  /class LLK_API ALLKPushableFixture[\s\S]{0,200}ILLKInteractable[\s\S]{0,80}ILLKNoiseSource/.test(
    read(join(root, "Source", "LLK", "Public", "World", "LLKPushableFixture.h"))),
  "the fixture must be focusable AND audible; implementing only the first " +
    "gives a puzzle object that costs the player nothing to use",
);

check(
  "focus is re-checked every frame, not only at the press",
  /if \(!CanFocus\(Keeper\)\)/.test(fixture),
  "without the per-frame check a player starts a push, runs for cover, and " +
    "the fixture finishes the job and makes all the noise for them - the " +
    "whole cost of the verb is avoidable",
);

check(
  "travel is the only replicated state",
  /DOREPLIFETIME\(ALLKPushableFixture, Travel\)/.test(fixture)
    && /SetReplicateMovement\(false\)/.test(fixture),
  "a replicated transform alongside a replicated travel scalar is two " +
    "definitions of where the fixture is, and they disagree under packet loss",
);

check(
  "a zero push axis is reported rather than silently unpushable",
  /zero LocalPushAxis/.test(fixture),
  "an un-normalisable axis would present as a fixture that focuses, prompts " +
    "and never moves, with nothing in any log saying why",
);

// ---- the noise -----------------------------------------------------------

check(
  "a Sweeper actually asks world objects what they sound like",
  /ILLKNoiseSource/.test(sweeper) && /GetCurrentNoise\(/.test(sweeper),
  "the Sweeper only hears the keeper's own movement, so a fixture can grind " +
    "across the floor beside it in silence and the distraction half of the " +
    "hearing sense does not exist",
);

check(
  "and it does so before it gives up on finding a keeper",
  (() => {
    const sources = sweeper.indexOf("ILLKNoiseSource");
    const keeper = sweeper.indexOf("FindNearestKeeper(DistanceStuds)");
    return sources > 0 && keeper > 0 && sources < keeper;
  })(),
  "the world-noise scan sits after the keeper early-returns, so a machine in " +
    "a building with nobody in it - or with a downed keeper - hears nothing " +
    "at all. Row 47's machine is indifferent to whether anyone is there",
);

check(
  "the running best is reset each frame",
  /HeardSalienceStuds = 0\.f/.test(sweeper),
  "a salience high-water mark that survives the tick ratchets: the loudest " +
    "sound ever heard would silence every quieter one for the rest of the run",
);

// ---- and whether anything in a level uses any of it ----------------------
//
// The likeliest failure of this whole slice, and the one nothing else would
// report. Placing actors is the map writer's lane, so the runtime can be
// finished and correct while no player can ever find a pushable.

check(
  "the map gate reports when a level has no pushable",
  /No pushable fixture in/.test(gamemode),
  "nothing warns about a map with no pushable in it, so a chapter where the " +
    "verb is undiscoverable boots clean and looks finished",
);

{
  const content = join(root, "Content");
  const maps = existsSync(content)
    ? readdirSync(content).filter((f) => f.endsWith(".umap"))
    : [];
  // Not a failure: no map has one yet, and saying so plainly is the point.
  console.log(
    `  note ${maps.length} map(s) in Content/. Placement is the map writer's ` +
      `lane; the gate above is what reports an empty one at boot.`,
  );
}

// ---- the economy, read from the header rather than restated -------------

{
  const noise = Number((feel.match(/PushNoiseStuds\s*=\s*([0-9.]+)f/) || [])[1]);
  const speed = Number((feel.match(/PushSpeedStuds\s*=\s*([0-9.]+)f/) || [])[1]);
  const lantern = JSON.parse(read(join(root, "data", "lantern.json")));
  const gait = (name) => {
    for (const v of Object.values(lantern)) {
      if (v && typeof v === "object" && typeof v[name] === "number") return v[name];
    }
    return undefined;
  };
  const sprint = gait("sprintSpeed");
  const crouch = gait("crouchSpeed");

  check(
    `pushing is louder than a sprint (${noise} > ${sprint} studs)`,
    Number.isFinite(noise) && Number.isFinite(sprint) && noise > sprint,
    "if a fixture is quieter than running, pushing one is the stealthy " +
      "option and the verb has no cost at all",
  );
  check(
    `and slower than a crouch (${speed} < ${crouch} studs/s)`,
    Number.isFinite(speed) && Number.isFinite(crouch) && speed < crouch,
    "a fixture that moves faster than the keeper's slowest gait is not a " +
      "commitment, and the time you can be found in is the whole price",
  );
}

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("A push verb can be unreachable in play while every C++ test");
  console.log("about its arithmetic passes. That is what these checks are for.\n");
  process.exit(1);
}
console.log("");
