// A light switch with nothing on the circuit.
//
// The power state is the first thing in this chapter that changes the
// BUILDING rather than the player or an object, and it has a failure mode
// none of the others do: the runtime, the subsystem, the breaker and the
// tags can all be correct while the map contains no light tagged `LLK_Mains`
// - in which case throwing the breaker is a prompt, a loud noise, and no
// visible change whatsoever. The player would conclude the puzzle was broken.
//
// There is no C++ test here because there is nothing pure to test: the whole
// behaviour is "find tagged actors in a world and change them", which the
// boot gate proves end to end by printing what it found. These are the
// structural claims that boot cannot make for itself.
//
//   node tests/power.test.mjs

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

const SUB = read(join(root, "Source", "LLK", "Private", "World", "LLKPowerSubsystem.cpp"));
const SUBH = read(join(root, "Source", "LLK", "Public", "World", "LLKPowerSubsystem.h"));
const BREAK = read(join(root, "Source", "LLK", "Private", "World", "LLKBreakerPanel.cpp"));
const BREAKH = read(join(root, "Source", "LLK", "Public", "World", "LLKBreakerPanel.h"));
const TOOL = read(join(root, "Tools", "light_boswells_power_states.py"));
const PUSHFEEL = read(join(root, "Source", "LLK", "Public", "World", "LLKPushFeel.h"));

console.log("\nThe mains: the two states, the switch, and what it costs\n");

// ---- the two states have to exist, and agree on the tag -----------------

const runtimeTag = (SUB.match(/MainsTag\(TEXT\("([^"]+)"\)\)/) || [])[1];
const toolTag = (TOOL.match(/MAINS_TAG\s*=\s*"([^"]+)"/) || [])[1];

check(
  `the runtime and the map tool agree on the mains tag (${runtimeTag})`,
  runtimeTag !== undefined && runtimeTag === toolTag,
  `the subsystem looks for "${runtimeTag}" and the tool writes "${toolTag}". ` +
    "They are two halves of one contract written in two languages, and when " +
    "they disagree the breaker works perfectly and lights nothing",
);

const runtimeEmergency = (SUB.match(/EmergencyTag\(TEXT\("([^"]+)"\)\)/) || [])[1];
const toolEmergency = (TOOL.match(/EMERGENCY_TAG\s*=\s*"([^"]+)"/) || [])[1];
check(
  `and on the emergency tag (${runtimeEmergency})`,
  runtimeEmergency !== undefined && runtimeEmergency === toolEmergency,
  "the emergency layer is only used for reporting, but a tag mismatch there " +
    "makes the boot line claim the building has no emergency lighting",
);

// ---- the default is darkness --------------------------------------------

check(
  "the chapter opens on emergency power, asserted rather than assumed",
  /bPowerOn = false;\s*\n\s*ApplyToLights\(\);/.test(SUB),
  "OnWorldBeginPlay has to force the state and apply it. A map saved with " +
    "the mains visible would otherwise start lit, and Chris's whole note was " +
    "that emergency-only is the vibe - a chapter that opens with the lights " +
    "on has spent its best moment before the player did anything",
);

check(
  "and the map authors the mains switched off",
  /MAINS_COLOUR, False\)/.test(TOOL) && /EMERGENCY_COLOUR, True\)/.test(TOOL),
  "the tool must place the emergency layer visible and the mains hidden, so " +
    "the level looks in the editor exactly like it behaves in game",
);

// ---- the thing that makes it a mechanic rather than a setting -----------

check(
  "a breaker exists and uses the interactable seam",
  /ILLKInteractable/.test(BREAKH) && /ALLKBreakerPanel::BeginInteract/.test(BREAK),
  "without an in-world switch the power state is a console command, which " +
    "is a developer tool and not a puzzle",
);

check(
  "and it actually reaches the subsystem",
  /GetSubsystem<ULLKPowerSubsystem>\(\)/.test(BREAK) && /SetPowerOn\(true\)/.test(BREAK),
  "the breaker prompts, plays its part and changes no light - the exact " +
    "shape of this project's recurring defect, and the player concludes the " +
    "puzzle is broken",
);

check(
  "and it is one-way",
  /if \(bThrown/.test(BREAK) && !/SetPowerOn\(false\)/.test(BREAK),
  "a breaker that can be switched back off lets a player undo the price and " +
    "keep the light. There is no stealth value in darkness you can restore " +
    "at will",
);

// ---- the cost ------------------------------------------------------------

check(
  "throwing it is a noise source",
  /ILLKNoiseSource/.test(BREAKH) && /ALLKBreakerPanel::GetCurrentNoise/.test(BREAK),
  "restoring power would be a free upgrade. Every other verb in this " +
    "chapter costs noise and this one changes the whole building",
);

check(
  "and the noise outlasts a single frame",
  /NoiseRemaining\s*=\s*NoiseSeconds/.test(BREAK),
  "a sound that only exists on the tick it was made works by luck - a " +
    "listener sampling on any other frame hears nothing",
);

{
  const powerNoise = Number(
    (SUBH.match(/PowerOnNoiseStuds\s*=\s*([0-9.]+)f/) || [])[1]);
  const pushNoise = Number(
    (PUSHFEEL.match(/PushNoiseStuds\s*=\s*([0-9.]+)f/) || [])[1]);

  check(
    `and it is the loudest thing in the game (${powerNoise} > push ${pushNoise} > sprint 12.5)`,
    Number.isFinite(powerNoise) && Number.isFinite(pushNoise)
      && powerNoise > pushNoise && pushNoise > 12.5,
    "a whole floor's lighting striking at once is the building waking up, " +
      "not a local event. If it is quieter than shoving one fixture, the " +
      "acoustic vocabulary no longer describes what the player just did",
  );

  // Louder than the Sweeper can see in the light, so there is nowhere on the
  // sales floor to throw it from unheard.
  check(
    `and carries past lit sight (${powerNoise} > 38 studs)`,
    Number.isFinite(powerNoise) && powerNoise > 38,
    "if the power-on noise is shorter than a Sweeper's lit sight range, " +
      "there are corners of the floor where restoring power is free",
  );
}

// ---- the developer tool must not be mistaken for the mechanic -----------

check(
  "the console command makes no noise, and says so",
  /LLK\.Power/.test(SUB) && /makes no noise/.test(SUB),
  "LLK.Power is for looking at the two states. If it emitted the cost too, " +
    "it would be a second implementation of the mechanic that can drift from " +
    "the breaker; if it is silent and undocumented, somebody tests with it " +
    "and concludes there is no cost",
);

check(
  "and a map with no mains lighting is reported, not silently ignored",
  /NO mains lighting/.test(SUB),
  "a level where nothing is tagged boots clean, the breaker works, and not " +
    "one light changes. Nothing else in the build would say so",
);

console.log(`\n${checks - failures}/${checks} passed`);
if (failures > 0) {
  console.log(`${failures} FAILED\n`);
  console.log("The power can be switchable, costly and correct while no light");
  console.log("in any level is on the circuit. That is what these are for.\n");
  process.exit(1);
}
console.log("");
